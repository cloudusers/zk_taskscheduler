#ifndef __BLOCK_QUEUE__H
#define __BLOCK_QUEUE__H

#include <queue>
#include <boost/thread.hpp>

using boost::mutex;
using boost::condition_variable;
using boost::unique_lock;
using boost::shared_lock;

template <class T>
class BlockQueue : boost::noncopyable
{
	public:
		BlockQueue(size_t size = 100000) : _maxSize(size) {}
		~BlockQueue() {}
		void put(T& t)
		{
			boost::unique_lock<boost::mutex> lock(_mutex);  
			while(_queue.size() >= _maxSize)
			{
				_fullCond.wait(lock);
			}
			_queue.push(t);
			_emptyCond.notify_one();
		}

		T get()
		{
			boost::unique_lock<boost::mutex> lock(_mutex);
			while(_queue.empty())
			{
				_emptyCond.wait(lock);
			}

			T front = _queue.front();
			_queue.pop();
			_fullCond.notify_one();
			return front; 
		}

		/*return true if queue not empty in waitSeconds with front member stored in ret,
		  false if timeout */
		bool poll(int waitSeconds, T& ret)
		{
			boost::unique_lock<boost::mutex> lock(_mutex);
			if(_queue.empty())
			{
				boost::system_time const timeout = boost::get_system_time() + 
					boost::posix_time::seconds(waitSeconds);
				_emptyCond.timed_wait(lock, timeout);
			}
			if(!_queue.empty())
			{
				T front = _queue.front();
				_queue.pop();
				_fullCond.notify_one();
				ret = front;
				return true;
			}
			else
				return false;

		}

		size_t size()
		{
			boost::lock_guard<boost::mutex> lock(_mutex);
			return _queue.size();
		}

	private:	
		size_t _maxSize;
		mutex _mutex;
		condition_variable _fullCond;
		condition_variable _emptyCond;
		std::queue<T> _queue;
};

#endif

