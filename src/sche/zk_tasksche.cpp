/*
 * Copyright (c) 2016
 * Copyright Owner: clouduser
 * Author: clouduser@163.com
 */



#include <iostream>
#include <algorithm>

#include "base/task/task.h"
#include "sche/zk_tasksche.h"


namespace bizbase {
	namespace zktasksche {


/*
 *
 */
TaskSche::TaskSche()
{/*{{{*/
}/*}}}*/

/*
 *
 */
TaskSche::TaskSche(std::map<std::string,std::string> conf)
:TaskManipulator(conf), conf_(conf)
{/*{{{*/
}/*}}}*/

/*
 *
 */
TaskSche::~TaskSche()
{/*{{{*/
}/*}}}*/

/*
 *
 */
int TaskSche::AddTask(std::string& task_name, 
		TaskCallbackFunc callback,
		std::string& pre_task_name,
		std::vector<std::string>* cron_vec,
		int max_ts,
		int priority,
		int flag)
{/*{{{*/
	if(task_infos_.find(task_name) == task_infos_.end()) {
		boost::shared_ptr<meta_t> meta( new meta_t(pre_task_name, callback,
					priority, max_ts, flag));
		if(cron_vec) {
			FOREACH(iter, *cron_vec) {
				(meta->cron)->AddCrontab(*iter);
			}
		}

		task_infos_.insert(std::make_pair<std::string, boost::shared_ptr<meta_t> >(task_name, meta));
	} else {
		(task_infos_[task_name]->pre_task_names).push_back(pre_task_name);
	}

	return 0;
}/*}}}*/

/*
 *
 */
int TaskSche::AddTask(std::string& task_name, 
		TaskCallbackFunc callback,
		std::vector<std::string>& pre_task_names,
		std::vector<std::string>* cron_vec,
		int max_ts,
		int priority,
		int flag)
{/*{{{*/
	if(task_infos_.find(task_name) == task_infos_.end()) {
		boost::shared_ptr<meta_t> meta( new meta_t(pre_task_names, callback,
					priority, max_ts, flag));
		if(cron_vec) {
			FOREACH(iter, *cron_vec) {
				(meta->cron)->AddCrontab(*iter);
			}
		}

		task_infos_.insert(std::make_pair<std::string, boost::shared_ptr<meta_t> >(task_name, meta));
	} else {
		size_t size = pre_task_names.size();
		for(size_t i=0; i<size; i++) {
			(task_infos_[task_name]->pre_task_names).push_back(pre_task_names[i]);
		}
	}

	return 0;
}/*}}}*/

/*
 *increase(task_name/validity)
 */
int TaskSche::BroadcastTasks(const std::string& task_name)
{/*{{{*/
	int ret = IncreaseTaskValidity(task_name);

	return ret;
}/*}}}*/

/*
 *decrease(pre-task/validity)
 */
int TaskSche::DelTask(const std::string& task_name)
{/*{{{*/
	int ret = DecreaseTaskValidity(task_name);

	return ret;
}/*}}}*/

/*
 *
 */
bool TaskSche::IsReady(const std::string& task_name)
{
	return CheckValidity(task_name);
}

/*
 *create local sequence nodes and dependent tasks nodes
 */
bool TaskSche::Initialize()
{/*{{{*/
	FOREACH(iter, task_infos_) {
		CreateTaskNodes(iter->first,//task_name
				(iter->second)->pre_task_names);
	}
	return true;
}/*}}}*/

/*
 *exec and increase(task_name/validity)
 */
void TaskSche::Execute(const std::string& task_name)
{/*{{{*/
	//int max_ts = task_infos_[task_name]->time_wait;
	task_infos_[task_name]->cb((void*)(const_cast<char*>(task_name.c_str())));

	BroadcastTasks(task_name);
	DelTask(task_name);

	return;
}/*}}}*/

/*
 *
 */
void TaskSche::Run(const int run_opt)
{/*{{{*/
	//init environment nodes
	bool ret_init = Initialize();
	if(!ret_init) {
		return;
	}

	//iterate tasks
	std::vector<std::string>finish_tasks;
	finish_tasks.reserve(10);
	do {
		finish_tasks.clear();

		do {
			FOREACH(iter, task_infos_) {
				std::cout << "###Run task_name:" << iter->first << std::endl;

				if(std::string("true") == conf_["use_crontab"]) {
					time_t clk;
					time(&clk);
					sleep((unsigned) (60 - clk % 60));
/*
					//debug begin
					register struct tm *tm;
					time_t cur_time;
					time(&cur_time);
					tm = localtime(&cur_time);//ignore localtime_r
					printf("%02d/%02d-%02d:%02d:%02d\n",
					   tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
					//sleep(1);
					//debug end
*/
					bool ret = ((iter->second)->cron)->IsReady();
					if(! ret) {
						continue;
					}
				}

				bool ready = IsReady(iter->first);
				if(ready) {
					Execute(iter->first);
					finish_tasks.push_back(iter->first);
				}
			}

			sleep(1);
		}while(finish_tasks.size() != task_infos_.size());

		sleep(1);
	}while(run_opt);
}/*}}}*/

/*
 *
 */
void TaskSche::RunOnce()
{/*{{{*/
	Run(RUN_ONCE);
}/*}}}*/

/*
 *
 */
void TaskSche::RunLoop()
{/*{{{*/
	Run(RUN_LOOP);
}/*}}}*/


}//zktasksche
}//bizbase
