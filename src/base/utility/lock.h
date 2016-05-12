#ifndef __BASE_LOCK_H_
#define __BASE_LOCK_H_

#include <boost/thread.hpp>

namespace Biz 
{
   typedef boost::mutex  Mutex;
   typedef boost::condition_variable_any  ConditionVariable;
   typedef boost::unique_lock<Mutex> UniqueLock;
   typedef boost::shared_lock<Mutex> SharedLock;

   typedef boost::thread  Thread;
}

#endif

