#ifndef __ZKTASKSCHE__H
#define __ZKTASKSCHE__H


#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <zookeeper/zookeeper.h>
#include <boost/unordered_map.hpp>

#include "base/zk_lib/zk_client.h"
#include "base/zk_lib/zk_lock.h"
#include "base/utility/common.h"

#include "base/utility/crontab.h"
#include "sche/task_manipulator.h"

using namespace bizbase::zkclient;
using namespace bizbase::zkcrontab;

//declear
class Ccrontab;


namespace bizbase {
	namespace zktasksche {

		/*
		 *task callback func
		 */
		typedef void(*TaskCallbackFunc)(void*);

		/*
		 *this is a task scheduler  that support below asurence.
		 *first. a task start to run if and ONLY IF its pre-tasks(depended task) done
		 *second. higher priority task executed befor lower priority task
		 *third. if time_wait not zero, the task be scheduled in time_wait
		 *some basic required such:
		 *the tasks should be DAG(a->b->c).  
		 *here the performance not important compare with other system, so we not save pointer of meta in data structure. 
		 *Its JUST a scheduler 
		 */
		class TaskSche :public TaskManipulator{
			public:
				typedef struct meta_data {
					//ctor
					meta_data()
					{
						cron = boost::shared_ptr<Crontab>(new Crontab());
					}
					//ctor
					meta_data(std::vector<std::string>vec,
							TaskCallbackFunc callback, 
							int pri=0, int ts=0, int f=0)
						: cb(callback),priority(pri), time_wait(ts),
						flag(f), pre_task_names(vec)
					{
						cron = boost::shared_ptr<Crontab>(new Crontab());
					}
					//ctor
					meta_data(std::string tname, 
							TaskCallbackFunc callback,
							int pri=0, int ts=0, int f=0)
						: cb(callback), priority(pri), time_wait(ts), flag(f)
					{
						pre_task_names = std::vector<std::string>();
						pre_task_names.reserve(10);
						pre_task_names.push_back(tname);
						cron = boost::shared_ptr<Crontab>(new Crontab());
					}

					int priority;
					int time_wait;
					int flag;
					boost::shared_ptr<Crontab> cron;
					std::vector<std::string>pre_task_names;
					TaskCallbackFunc cb;
				}meta_t;

				/*
				 *output for debuging
				 */
				friend ostream& operator<<(ostream& os, const TaskSche& tasksche)
				{
					FOREACH(iter, tasksche.task_infos_) {
						os << "task_name:" << iter->first << "\n";
						boost::shared_ptr<meta_t> meta = iter->second;
						os << "  priority:" << meta->priority << "\n";
						os << "  time_wait:" << meta->time_wait << "\n";
						os << "  flag:" << meta->flag << "\n";
						os << "  pre_tasks:";
						size_t size = meta->pre_task_names.size();
						for(size_t i=0; i<size; i++) {
							os << meta->pre_task_names[i] << "|";
						}
						os << "\n";
					}

					return os;
				}

			public:
				TaskSche();
				TaskSche(std::map<std::string, std::string> conf);
				virtual ~TaskSche();

			public:
				/*
				 *@param task_name: 
				 *@param pre_task_name: dependency task
				 *@param priority: min value is max priority
				 *@param max_ts: max exec time of task,if 0 is infinity
				 *@param flag: 
				 */
				int AddTask(std::string& task_name,
						TaskCallbackFunc cb,
						std::string& pre_task_name,
						std::vector<std::string>*cron_vec=NULL,
						int max_ts=0,
						int priority=0,
						int flag=0);

				int AddTask(std::string& task_name,
						TaskCallbackFunc cb,
						std::vector<std::string>& pre_task_names,
						std::vector<std::string>*cron_vec=NULL,
						int max_ts=0,
						int priority=0,
						int flag=0);

				/*
				 *inotify next-tasks
				 */
				int BroadcastTasks(const std::string& task_name);

				/*
				 *delete task_name
				 */
				int DelTask(const std::string& task_name);

				/*
				 *
				 */
				bool IsReady(const std::string& task_name);

				/*
				 *init environment such create local nodes and dependent nodes.
				 *this will be call by Run(const int run_opt)
				 */
				bool Initialize();

				/*
				 *@param run_opt reference as enum run_option
				 */
				void RunOnce();
				void RunLoop();
				void Run(const int run_opt);

				/*
				 *what you wanna functions should be put
				 */
				void Execute(const std::string& task_name);

			private:
				std::map<std::string, std::string> conf_;

				boost::unordered_map<std::string, boost::shared_ptr<meta_t> > task_infos_;

				enum run_option {
					RUN_ONCE = 0,
					RUN_LOOP = 1,
				};
		};


	}//zktasksche
}//bizbase

#endif
