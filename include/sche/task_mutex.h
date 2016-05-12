#ifndef __ZKTASKMUTEX__H
#define __ZKTASKMUTEX__H


#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <zookeeper/zookeeper.h>
#include <zookeeper/zoo_lock.h>

#include "base/zk_lib/zk_client.h"
#include "base/zk_lib/zk_lock.h"
#include "base/utility/common.h"



namespace bizbase {
	namespace zktaskmutex {


		/*
		 *lock node prevent from multi-updating
		 *this lock use zkr_lock_[lock,unlock]
		 */
		class TaskMutex
		{
			public:
				typedef boost::shared_ptr<TaskMutex> ptr;
				/*
				 *
				 */
				TaskMutex(zkr_lock_mutex_t* mutex);

				/*
				 *
				 */
				~TaskMutex();

				/*
				 *
				 */
				TaskMutex::ptr operator()(zkr_lock_mutex_t* mutex);

			public:
				/*
				 *
				 */
				static int Initialize(zkr_lock_mutex_t* mutex, zhandle_t* zh, 
						const std::string& path, struct ACL_vector& acl_vec);

				/*
				 *
				 */
				int Lock();
				int UnLock();

			private:
				zkr_lock_mutex_t* mutex_;
		};


		typedef boost::shared_ptr<TaskMutex> ptr;
	}//zktasksche
}//bizbase

#endif
