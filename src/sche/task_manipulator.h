#ifndef __ZKTASKMAN__H
#define __ZKTASKMAN__H


#include <iostream>
#include <vector>
#include <map>

#include "base/zk_lib/zk_client.h"
#include "base/zk_lib/zk_acl.h"
#include "base/zk_lib/zk_lock.h"
#include "base/utility/common.h"

#include <zookeeper/zoo_lock.h>

using namespace bizbase::zkclient;
using namespace bizbase::zklock;

namespace bizbase {
	namespace zktasksche {


		/*
		 *taskmanipulator to operate task in zk
		 *it will create or update nodes and do some verify works
		 */
		class TaskManipulator
		{
			public:

				TaskManipulator();
				TaskManipulator(std::map<std::string, std::string> conf);
				virtual ~TaskManipulator();

			public:

				/*
				 *create tasks environment
				 */
				bool CreateTaskNodes(std::string task_name, std::vector<std::string> pre_nodes);

			public:

				/*
				 *log when nodes changed or do something what you wanna
				 */
				void ZkCallBack(Event ev);

				/*
				 *check task whether validity or not, if and only if checkvalidity true, task continue to run
				 *here task seperated by task_node_type, and each type task has different decision algorithm
				 */
				bool CheckValidity(const std::string& task_name);
				bool CheckPreTypeValidity(const std::string& task_name);
				bool CheckNextTypeValidity(const std::string& task_name);
				bool CheckAllTypeValidity(const std::string& task_name);

				/*
				 *
				 */
				void GetTaskPath(std::string& ret, 
						const std::string& task_name, 
						const std::string& base_node_name="", 
						const std::string& tag="");
				void GetTaskPathValue(std::string& ret, 
						const std::string& path);


				/*
				 * get task_node_type. reference from below enum task_node_type
				 */
				int GetTaskType(const std::string& task_name);

				/*
				 *get number of count(task->next-tasks)
				 */
				int GetNextTaskCount(const std::string& task_name);

				/*
				 *increase or  decrease task_name/validity
				 */
				int DecreaseTaskValidity(const std::string& task_name);
				int IncreaseTaskValidity(const std::string& task_name);

			private:
				std::map<std::string, std::string> conf_;
				boost::shared_ptr<ZkClient> zkclient_ptr_;

				/*
				 *taska->taskc->taskm
				 *TASK_ONLY_EXIST_PREV : taskm
				 *TASK_ONLY_EXIT_NEXT : taska
				 *TASK_EXIST_ALL : taskc
				 */
				enum task_node_type {
					TASK_ONLY_EXIST_PREV = 0,
					TASK_ONLY_EXIT_NEXT = 1,
					TASK_ONLY_SELF= 2,
					TASK_EXIST_ALL = 3,
					TASK_DEFAULT = 99,
				};

				/*
				 *task->validity lock
				 *prevent more subtask simultaneous update validity
				 */
				std::map<std::string, /*task_name*/
					zkr_lock_mutex_t*> task_mutexes_;

				/*
				 *task ha lock
				 *impl master-slave
				 */
				std::map<std::string,/*task_name*/
					boost::shared_ptr<ZkLock> > task_ha_locks_;
				/*
				 *default = ZOO_OPEN_ACL_UNSAFE
				 *if conf_ set acl, use digest acl with user:pass
				 */
				struct ACL_vector acl_vec;
		};


	}//zktasksche
}//bizbase


#endif
