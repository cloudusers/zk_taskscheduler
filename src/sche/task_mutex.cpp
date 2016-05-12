#include "task_mutex.h"

using namespace bizbase::zkclient;


namespace bizbase {
	namespace zktaskmutex {


/*
 *
 */
TaskMutex::TaskMutex(zkr_lock_mutex_t* mutex)
:mutex_(mutex)
{/*{{{*/
}/*}}}*/

/*
 *
 */
TaskMutex::~TaskMutex()
{/*{{{*/
}/*}}}*/

/*
 *
 */
TaskMutex::ptr TaskMutex::operator()(zkr_lock_mutex_t* mutex)
{/*{{{*/
	TaskMutex::ptr tm = TaskMutex::ptr(new TaskMutex(mutex));

	return tm;
}/*}}}*/

/*
 *
 */
int TaskMutex::Initialize(zkr_lock_mutex_t* mutex, zhandle_t* zh, 
		const std::string& path, struct ACL_vector& acl_vec)
{/*{{{*/
	int ret = zkr_lock_init(mutex, zh, const_cast<char*>(path.c_str()), &acl_vec);

	return ret;
}/*}}}*/

/*
 *
 */
int TaskMutex::Lock()
{/*{{{*/
	int ret = zkr_lock_lock(mutex_);

	return ret;
}/*}}}*/

/*
 *
 */
int TaskMutex::UnLock()
{/*{{{*/
	int ret = zkr_lock_unlock(mutex_);

	return ret;
}/*}}]*/


}//zktasksche
}//bizbase
