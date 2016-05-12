#ifndef __ZKLOCK__H
#define __ZKLOCK__H


#include <iostream>

#include "zk_client.h"
#include <base/utility/logger.h>

using namespace bizbase::zkclient;

namespace bizbase {
	namespace zklock {


		// this lock can only be occupied by one thread.
		class ZkLock
		{
			public:
				/*  lockName is prefix of the sequece node;
				 *  identity is the name of this local process, set value of lock to be identity,
				 *  so one can know who obtain this lock from zk server.
				 */
				ZkLock(ZkClient* zk, std::string path, std::string lockName, std::string identity);

				~ZkLock();

				int Lock();
				int Unlock();
				int TryLock();
				bool IsLocked();

			private:
				bool _isLocked;
				std::string _path; // lock path
				std::string _name; // lock name and lock prefix
				std::string _nodeName; // lock name with sequence number, in this format:  lockname-100
				ZkClient* _zkclient;  // this zk should be connected to the zookeeper server	
		};

	}//namespace zklock
}//namespace bizbase


#endif
