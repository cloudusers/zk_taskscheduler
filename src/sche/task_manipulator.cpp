/*
 * Copyright (c) 2016
 * Copyright Owner: clouduser
 * Author: clouduser@163.com
 */



#include <iostream>
#include <sstream>

#include "sche/task_mutex.h"
#include "sche/mutex_guard.h"
#include "sche/task_manipulator.h"


using namespace bizbase::zkacl;
using namespace bizbase::zkclient;
using namespace bizbase::zktaskmutex;

namespace bizbase {
	namespace zktasksche {


/*
 *
 */
TaskManipulator::TaskManipulator()
{/*{{{*/
}/*}}}*/

/*
 *
 */
TaskManipulator::TaskManipulator(std::map<std::string, std::string> conf)
:conf_(conf)
{/*{{{*/
	std::string zk_hostport = conf["zk_hostport"];
	int zk_timeout = atoi(conf["zk_timeout"].c_str());
	std::string log_info = conf["log_info"];
	std::string log_error= conf["log_error"];

	zkclient_ptr_ = boost::shared_ptr<ZkClient>(new ZkClient(
				zk_hostport,
				zk_timeout,
				log_info,
				log_error));

	zkclient_ptr_->SetEventHandler(boost::bind(&TaskManipulator::ZkCallBack, this, _1));

	//add auth digest
	acl_vec = ZOO_OPEN_ACL_UNSAFE;
	if(std::string("true") == conf_["use_acl"]) {
		std::string auth_user = conf_["auth_user"];
		std::string auth_pass = conf_["auth_pass"];
		bool ret = InitAcl(auth_user, auth_pass, acl_vec);
		if(ret) {
			ret = AddAuth(zkclient_ptr_->_zk, auth_user, auth_pass);
		}

		if(! ret) {
			BLOG_ERROR_LEVEL(conf_["log_error"], "auth digest fail");
			exit(-1);
		}
		BLOG_INFO_LEVEL(conf_["log_info"], "add auth success");
	}
}/*}}}*/

/*
 *
 */
TaskManipulator::~TaskManipulator()
{/*{{{*/
	FOREACH(iter, task_mutexes_) {
		zkr_lock_destroy(iter->second);
		delete iter->second;
	}
}/*}}}*/

/*
 *
 */
bool TaskManipulator::CreateTaskNodes(std::string task_name, std::vector<std::string> pre_nodes)
{/*{{{*/
	//init base nodes
	std::string root_node = conf_["root_node"];
	std::vector<std::string> base_nodes;
	base_nodes.push_back("pre-tasks");
	base_nodes.push_back("next-tasks");
	base_nodes.push_back("lock");
	base_nodes.push_back("validity");

	//create task_name base_nodes
	FOREACH(iter, base_nodes) {
		std::string base_path;
		GetTaskPath(base_path, task_name, *iter);
		zkclient_ptr_->CreateRecursion(base_path, "0", 0, NULL, 0, acl_vec);
	}

	//create task_name->pre_tasks
	FOREACH(iter, pre_nodes) {
		std::string pre_path;
		std::string pre_context;
		GetTaskPath(pre_path, task_name, "pre-tasks", *iter);
		GetTaskPath(pre_context, *iter);
		zkclient_ptr_->CreateRecursion(pre_path, pre_context, 0, NULL, 0, acl_vec);
	}

	//create task_name->pre_tasks->next-tasks
	FOREACH(pre_iter, pre_nodes) {
		if((*pre_iter).empty()) {
			continue;
		}
		//base_nodes
		FOREACH(base_iter, base_nodes) {
			std::string base_path;
			GetTaskPath(base_path, *pre_iter, *base_iter);
			zkclient_ptr_->CreateRecursion(base_path, "0", 0, NULL, 0, acl_vec);
		}
		//task_name->pre_tasks->next-tasks
		std::string pre_next_path;
		std::string pre_next_context;
		GetTaskPath(pre_next_path, *pre_iter, "next-tasks", task_name);
		GetTaskPath(pre_next_context, *pre_iter);
		zkclient_ptr_->CreateRecursion(pre_next_path, pre_next_context, 0, NULL, 0, acl_vec);
	}

	/*
	 *create task_name lock
	 */
	std::string lock_path;
	GetTaskPath(lock_path, task_name, "validity");
	if(task_mutexes_.find(task_name) == task_mutexes_.end()) {
		zkr_lock_mutex_t *mutex = new zkr_lock_mutex_t();
		TaskMutex::Initialize(mutex, zkclient_ptr_->_zk, lock_path, acl_vec);
		task_mutexes_.insert(std::pair<std::string, zkr_lock_mutex_t*>(task_name, mutex));
	}

	//master-slave mutex-lock
	GetTaskPath(lock_path, task_name, "lock");
	if(task_ha_locks_.find(task_name) == task_ha_locks_.end()) {
		boost::shared_ptr<ZkLock>zklock_ptr(new ZkLock(
					zkclient_ptr_.get(),
					lock_path,
					"lock_data",
					"content"));
		task_ha_locks_.insert(
				std::pair<std::string, boost::shared_ptr<ZkLock> >(task_name, zklock_ptr));
	}

	return true;
}/*}}}*/

/*
 *log when nodes changed
 */
void TaskManipulator::ZkCallBack(Event ev)
{/*{{{*/
	BLOG_INFO_LEVEL(conf_["log_info"], "" << ev);
}/*}}}*/

/*
 *taska->taskc->taskm
 *if(taskc) exist pre-tasks and next-tasks
 *if(taska) only next-tasks
 *if(taskm) only pre-tasks
 */
int TaskManipulator::GetTaskType(const std::string& task_name)
{/*{{{*/
	std::string pre_path;
	std::string next_path;
	std::vector<std::string> pre_children;
	std::vector<std::string> next_children;

	GetTaskPath(pre_path, task_name, "pre-tasks");
	int rc_pre = zkclient_ptr_->GetChildren(pre_path, pre_children, 0); 
	GetTaskPath(next_path, task_name, "next-tasks");
	int rc_next = zkclient_ptr_->GetChildren(next_path, next_children, 0); 

	if(ZOK != rc_next || ZOK != rc_pre) {
		return TASK_DEFAULT;
	}

	if(! pre_children.empty() && ! next_children.empty()) {
		return TASK_EXIST_ALL;
	}

	if(! pre_children.empty()) {
		return TASK_ONLY_EXIST_PREV;
	} else if(! next_children.empty()) {
		return TASK_ONLY_EXIT_NEXT;
	} else {
		return TASK_ONLY_SELF;
	}
}/*}}}*/

/*
 *taska->taskc->taskm
 *here we process taskm
 */
bool TaskManipulator::CheckPreTypeValidity(const std::string& task_name)
{/*{{{*/
	int rc = 0;
	std::string pre_path;
	std::string context;
	std::vector<std::string> pre_children;
	GetTaskPath(pre_path, task_name, "pre-tasks");
	rc = zkclient_ptr_->GetChildren(pre_path, pre_children, 0); 
	if(rc != ZOK) {
		return false;
	}

	//check pre-tasks->validity
	FOREACH(iter, pre_children) {
		pre_path="";
		{//lock
			GetTaskPath(pre_path, *iter, "validity");

			TaskMutex::ptr tm = TaskMutex::ptr(new TaskMutex(task_mutexes_[*iter]));
			MutexGuard<TaskMutex::ptr> guard(tm);
			GetTaskPathValue(context, pre_path);
			//(1)self validity
			if(context==std::string("0")) {
				return false;
			}
		}

		//(2)avoid task_name repeatly exec and other siblings no chance to exec
		pre_path="";
		GetTaskPath(pre_path, *iter, "next-tasks", task_name);
		GetTaskPathValue(context, pre_path);
		if(context==std::string("1")) {
			return false;
		}
	}

	return true;
}/*}}}*/

/*
 *taska->taskc->taskm
 *here we process taska
 */
bool TaskManipulator::CheckNextTypeValidity(const std::string& task_name)
{/*{{{*/
	int rc = 0;
	std::string context;
	std::string task_path;
	GetTaskPath(task_path, task_name, "validity");

	{//guard
		TaskMutex::ptr tm = TaskMutex::ptr(new TaskMutex(task_mutexes_[task_name]));
		MutexGuard<TaskMutex::ptr> guard(tm);
		GetTaskPathValue(context, task_path);
	}

	//(1)self 
	if(context != std::string("0")) {
		return false;
	} else {
		if(std::string("false") == conf_["use_check_chain"]) {
			return true;
		}
	}

	//(2)next-task
	std::string next_path;
	std::vector<std::string> next_children;
	GetTaskPath(next_path, task_name, "next-tasks");
	rc = zkclient_ptr_->GetChildren(next_path, next_children, 0); 

	if(ZOK != rc) {
		return false;
	}

	FOREACH(next_iter, next_children) {
		context="";
		next_path="";
		GetTaskPath(next_path, *next_iter, "validity");

		TaskMutex::ptr tm = TaskMutex::ptr(new TaskMutex(task_mutexes_[*next_iter]));
		MutexGuard<TaskMutex::ptr> guard(tm);
		GetTaskPathValue(context, next_path);

		if(std::string("0") != context) {
			return false;
		}
	}

	return true;
}/*}}}*/

/*
 *decrease pre_task->validity
 */
int TaskManipulator::DecreaseTaskValidity(const std::string& task_name)
{/*{{{*/
	int rc = 0;
	std::string context;
	std::string pre_path;
	std::vector<std::string> pre_children;
	GetTaskPath(pre_path, task_name, "pre-tasks");
	rc = zkclient_ptr_->GetChildren(pre_path, pre_children, 0); 
	if(rc != ZOK) {
		return rc;
	}

	FOREACH(iter, pre_children) {
		pre_path="";
		GetTaskPath(pre_path, *iter, "validity");
		//TaskMutex::ptr tm = TaskMutex::ptr(new TaskMutex(task_mutexes_[*iter]));
		//MutexGuard<TaskMutex::ptr> guard(tm);
		GetTaskPathValue(context, pre_path);

		if(context==std::string("0")) {
			continue;
		}
		int update_val = atoi(context.c_str());
		--update_val;

		char ibuf[32] = {'\0'};
		sprintf(ibuf, "%d", update_val>0?update_val : 0);
		context = ibuf;
		rc = zkclient_ptr_->Update(pre_path, context, 1); 
		if(ZOK != rc) {
			return rc;
		}
	}

	return 0;
}/*}}}*/

/*
 *
 */
int TaskManipulator::GetNextTaskCount(const std::string& task_name)
{/*{{{*/
	int rc = 0;
	std::string next_path;
	std::vector<std::string> next_children;
	GetTaskPath(next_path, task_name, "next-tasks");
	rc = zkclient_ptr_->GetChildren(next_path, next_children, 0); 

	if(ZOK != rc) {
		return rc;
	}

	int size = next_children.size();
	FOREACH(iter, next_children) {
		if((*iter).find("x-") != std::string::npos) {
			size--;
		}
	}

	return size >0 ? size: 0;
}/*}}}*/

/*
 *increase val = count(next-tasks)
 *clear task_name->next_task
 */
int TaskManipulator::IncreaseTaskValidity(const std::string& task_name)
{/*{{{*/
	int rc = 0;
	std::string context;
	std::string task_path;
	GetTaskPath(task_path, task_name, "validity");

	{//lock
		TaskMutex::ptr tm = TaskMutex::ptr(new TaskMutex(task_mutexes_[task_name]));
		MutexGuard<TaskMutex::ptr> guard(tm);
		GetTaskPathValue(context, task_path);
	}

	int type = GetTaskType(task_name);
	switch(type) {
		case TASK_ONLY_EXIST_PREV:
			rc = 0;
			break;
		case TASK_ONLY_EXIT_NEXT:
			rc = GetNextTaskCount(task_name);
			break;
		case TASK_ONLY_SELF:
			rc = 0;
			break;
		case TASK_EXIST_ALL:
			rc = GetNextTaskCount(task_name);
			break;
		default:
			break;
	};

	if(rc < 0) {
		return rc;
	}

	//(1)update validity
	char ibuf[32] = {'\0'};
	sprintf(ibuf, "%d", rc);
	context = ibuf;
	rc = zkclient_ptr_->Update(task_path, context, 1); 
	//getchar();
	if(ZOK != rc) {
		return rc;
	}

	//(2)update next-tasks/task_name
	if(TASK_ONLY_EXIT_NEXT == type || TASK_EXIST_ALL == type) {
		std::string next_path;
		std::vector<std::string> next_children;
		GetTaskPath(next_path, task_name, "next-tasks");
		rc = zkclient_ptr_->GetChildren(next_path, next_children, 0); 

		if(ZOK != rc) {
			return rc;
		}

		context = "0";
		FOREACH(iter, next_children) {
			std::string sub_task;
			GetTaskPath(next_path, task_name, "next-tasks", *iter);
			rc = zkclient_ptr_->Update(next_path, context, 1); 
			if(ZOK != rc) {
				continue;
			}
		}
	}


	return 0;
}/*}}}*/

/*
 *
 */
bool TaskManipulator::CheckAllTypeValidity(const std::string& task_name)
{/*{{{*/
	return CheckPreTypeValidity(task_name) && CheckNextTypeValidity(task_name);
}/*}}}*/

/*
 *
 */
bool TaskManipulator::CheckValidity(const std::string& task_name)
{/*{{{*/
	bool ret = false;
	int ret_lock = (task_ha_locks_[task_name])->TryLock();
	if(-1 == ret_lock) {
		BLOG_INFO_LEVEL(conf_["log_info"], "this is a slave task");
		return false;
	}

	int type = GetTaskType(task_name);
	switch(type) {
		case TASK_ONLY_EXIST_PREV:
			ret = CheckPreTypeValidity(task_name);
			break;
		case TASK_ONLY_EXIT_NEXT:
			ret = CheckNextTypeValidity(task_name);
			break;
		case TASK_ONLY_SELF:
			ret = true;
			break;
		case TASK_EXIST_ALL:
			ret = CheckAllTypeValidity(task_name);
			break;
		default:
			break;
	};

	return ret;
}/*}}}*/

/*
 *
 */
void TaskManipulator::GetTaskPath(std::string& ret, const std::string& task_name, 
		const std::string& base_node_name, const std::string& tag)
{/*{{{*/
	ret = conf_["root_node"] + std::string("/") + task_name;
	if(! base_node_name.empty()) {
		ret += std::string("/") + base_node_name;
	}

	if(! tag.empty()) {
		ret += std::string("/") + tag;
	}

	return;
}/*}}}*/

/*
 *
 */
void TaskManipulator::GetTaskPathValue(std::string& ret, const std::string& path)
{/*{{{*/
	zkclient_ptr_->GetNode(path, ret, 0, NULL);

	return;
}/*}}}*/


}//zktasksche
}//bizbase
