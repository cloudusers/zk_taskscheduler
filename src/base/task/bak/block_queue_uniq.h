#ifndef _BLOCK_QUEUE_UNIQ_H_
#define _BLOCK_QUEUE_UNIQ_H_

#include <deque>
#include <boost/thread.hpp>

using namespace std;
using boost::mutex;
using boost::condition_variable;
using boost::unique_lock;
using boost::shared_lock;

template<class T>
class BlockQueueUniq : boost::noncopyable
{
 public:
  BlockQueueUniq(size_t size = 100000) : _maxSize(size) {}
  ~BlockQueueUniq() {}

  void PushFront(T& t)
  {
    boost::unique_lock<boost::mutex> lock(_mutex);  
    while(_queue.size() >= _maxSize)
    {
      _fullCond.wait(lock);
    }
    _queue.push_front(t);
    _emptyCond.notify_one();
  }

  bool TryPushFront(T& t)
  {
    boost::unique_lock<boost::mutex> lock(_mutex);  
    if(_queue.size() >= _maxSize)
    {
      return false;
    }
    _queue.push_front(t);
    _emptyCond.notify_one();
	return true;
  }

  void PushBack(T& t)
  {
    boost::unique_lock<boost::mutex> lock(_mutex);  
    while(_queue.size() >= _maxSize)
    {
      _fullCond.wait(lock);
    }
    _queue.push_back(t);
    _emptyCond.notify_one();
  }

  bool TryPushBack(T& t)
  {
    boost::unique_lock<boost::mutex> lock(_mutex);  
    if(_queue.size() >= _maxSize)
    {
      return false;
    }
    _queue.push_back(t);
    _emptyCond.notify_one();
	return true;
  }


  void PushBackUniq(T& t)
  {
    boost::unique_lock<boost::mutex> lock(_mutex);  
    while(_queue.size() >= _maxSize)
    {
      _fullCond.wait(lock);
    }
	if(_queue.size() > 0)
	{
		typename std::deque<T>::iterator it = _queue.begin();
		for(; it != _queue.end(); it++){
			if( *it == t)
			{
				return;
			}
		}
	}
    _queue.push_back(t);
    _emptyCond.notify_one();
  }

  bool TryPushBackUniq(T& t)
  {
    boost::unique_lock<boost::mutex> lock(_mutex);  
    if(_queue.size() >= _maxSize)
    {
      return false;
    }
	if(_queue.size() > 0)
	{
		typename std::deque<T>::iterator it = _queue.begin();
		for(; it != _queue.end(); it++){
			if( *it == t)
			{
				return true;
			}
		}
	}
    _queue.push_back(t);
    _emptyCond.notify_one();
	return true;
  }


  void PopFront(T &value)
  {
    boost::unique_lock<boost::mutex> lock(_mutex);
    while(_queue.empty())
    {
      _emptyCond.wait(lock);
    }

    value = _queue.front();
    _queue.pop_front();
    _fullCond.notify_one();
  }

  bool TryPopFront(T &value)
  {
    boost::unique_lock<boost::mutex> lock(_mutex);
    if(_queue.empty())
    {
	  return false;
    }

    value = _queue.front();
    _queue.pop_front();
    _fullCond.notify_one();
	return true;
  }
  
  void PopBack(T &value)
  {
    boost::unique_lock<boost::mutex> lock(_mutex);
    while(_queue.empty())
    {
      _emptyCond.wait(lock);
    }

    value = _queue.back();
    _queue.pop_back();
    _fullCond.notify_one();
  }

  bool TryPopBack(T &value)
  {
    boost::unique_lock<boost::mutex> lock(_mutex);
    if(_queue.empty())
    {
	  return false;
    }

    value = _queue.back();
    _queue.pop_back();
    _fullCond.notify_one();
	return true;
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
      _queue.pop_front();
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
  std::deque<T> _queue;
};

#endif

