#ifndef __MUTEXGUARD__H
#define __MUTEXGUARD__H

#include <boost/noncopyable.hpp>

template<class T>
class MutexGuard : boost::noncopyable {
	public:
		MutexGuard(T value)
			: mutex_(value)  {
				mutex_->Lock();
			}

		~MutexGuard() {
			if(mutex_) {
				mutex_->UnLock();
			}
		}

		/*
		   void operator()(T value) const {
		   mutex_ = value;
		   mutex_->Lock();
		   }
		   */
		operator bool() const {
			return (mutex_ != NULL);
		}

	private:
		T mutex_;
};

#endif
