#ifndef __TASK__H
#define __TASK__H

#include <boost/thread.hpp>                                        
#include <boost/bind.hpp>                                          
#include <boost/shared_ptr.hpp>
#include <boost/implicit_cast.hpp>
#include <base/utility/common.h>

using boost::thread;
using boost::thread_group;

typedef boost::function<void()> ThreadFunc;

class Task 
{
	public:
		// task runs as soon as it is constructed.
		Task(const ThreadFunc &func, const string &name = string()) 
		{
			_thread = new thread(func);
			_task_name = name;
			BLOG_INFO("new task created:  " << name);
		}

		virtual ~Task()
		{
			if(_thread->joinable())
				_thread->join(); // make sure task is finished
			delete _thread;
			BLOG_INFO("task destructed: " << _task_name);
		} 

		// whether stop function should be added, use interrupt to stop thread
		void interrupt()
		{
			_thread->interrupt();
		}

		void join()
		{
			_thread->join();
		}

		void detach()
		{
			_thread->detach();
		}

		static long id() // reinterpret_cast is unsafe
		{
			long id;
			stringstream sstrm;
			sstrm << boost::this_thread::get_id();
			sstrm >> id;
			return id;

			// how to use cast function here??
			// return boost::implicit_cast<long>(boost::this_thread::get_id());
		}

		static void sleep(int nsec)
		{
			boost::this_thread::sleep(boost::posix_time::seconds(nsec));
		}

		const string& taskName()
		{
			return _task_name;
		}

	private:
		//  boost::shared_ptr<boost::thread> _thread;
		boost::thread* _thread;
		std::string _task_name;
};

class TaskGroup
{
	public:
		TaskGroup()
		{}

		~TaskGroup()
		{
			for(std::list<Task*>::iterator it = _tasklist.begin(), end = _tasklist.end();
					it != end;
					it++)
			{
				delete *it;
			}
		}
		void add_task(Task* task)
		{
			_tasklist.push_back(task);
		}

		void remove_task(Task* task)
		{
			std::list<Task*>::iterator const it = std::find(_tasklist.begin(), _tasklist.end(), task);
			if(it != _tasklist.end())
			{
				_tasklist.erase(it);
			}
		}

		void join_all()
		{
			for(std::list<Task*>::iterator it = _tasklist.begin(), end = _tasklist.end();
					it != end;
					++it)
			{
				(*it)->join();
			}
		}

		void interrupt_all()
		{
			for(std::list<Task*>::iterator it = _tasklist.begin(), end = _tasklist.end();
					it != end;
					++it)
			{
				(*it)->interrupt();
			}
		}

		int size() const
		{
			return _tasklist.size();
		}

	private:
		std::list<Task*> _tasklist;
};
#endif

