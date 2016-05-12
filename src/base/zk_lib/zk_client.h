#ifndef __ZKCLIENT__H
#define __ZKCLIENT__H

#include <zookeeper/zookeeper.h>

#include <iostream>
#include <unistd.h>
#include <set>
#include <vector>

#include <boost/function.hpp>

#include "base/utility/common.h"
#include "base/utility/lock.h"
#include "base/task/task.h"
#include "base/task/block_queue.h"
#include "base/utility/logger.h"

using std::ostream;
using boost::function;
using Biz::Mutex;

namespace bizbase {
	namespace zkclient{


		/*
		 *  This struct is to describe the event caught by zookeeper client 
		 */
		class EventType
		{
			public:
				EventType(): _type(-1) {}
				EventType(int var): _type(var) {}
				void operator= (int etype) {
					_type = etype;
				}
				friend ostream& operator<< (ostream& os, EventType etype) {
					if (etype._type == ZOO_CREATED_EVENT)
						os << "ZOO_CREATED_EVENT ";
					else if( etype._type == ZOO_DELETED_EVENT)
						os << "ZOO_DELETED_EVENT " ;
					else if( etype._type == ZOO_CHANGED_EVENT)
						os << "ZOO_CHANGED_EVENT ";
					else if( etype._type == ZOO_CHILD_EVENT)
						os << "ZOO_CHILD_EVENT ";
					else if( etype._type == ZOO_SESSION_EVENT)
						os << "ZOO_SESSION_EVENT ";
					else if( etype._type == ZOO_NOTWATCHING_EVENT)
						os << "ZOO_NOTWATCHING_EVENT ";
					else
						os << "unknown event type: " << etype._type << " ";

					return os;
				}

				int _type;
		};

		struct Event 
		{
			public:
				string _path;
				EventType _type;
				int _state;

				friend ostream& operator << (ostream& os, const Event& ev) {
					os << "path:" << ev._path << "type:" << ev._type << "state:" << ev._state; 
					return os;
				}
		};

		typedef boost::function<void (Event ev)> EventHandler;

		class ZkClient
		{
			public:
				ZkClient(string hostPorts, int recvTimeout, 
						std::string LOG_INFO="info",
						std::string LOG_ERROR="error");


				~ZkClient();

				/*
				 *  register a node on zookeeper, by default is ZOO_EPHEMERAL type 
				 */
				int Create(const string& path,
						const string& content, int nodeType = ZOO_EPHEMERAL, 
						char* path_buffer = NULL, int path_buffer_len = 0,
						const struct ACL_vector& acl = ZOO_OPEN_ACL_UNSAFE);

				/*
				 * create node recursively
				 */
				int CreateRecursion(const string& path,
						const string& content, int nodeType = 0,/*persistent*/
						char* path_buffer = NULL, int path_buffer_len = 0,
						const struct ACL_vector& acl = ZOO_OPEN_ACL_UNSAFE,/*world:anyone*/
						int watch=1/*watch zknode*/);

				/*
				 *  update the node with raw content
				 *  if this node does not exist, create it,
				 *  by default a ephemeral node is created
				 */
				int Update( const string& path,
						const string& content, int nodeType = ZOO_EPHEMERAL, 
						int version = -1);

				/*
				 *  delete zk node
				 */
				int Del( const string& path, int version = -1);	

				/*
				 * return 0 if exits
				 */
				int Exists(const string& path, int watch=1);


				/* 
				 * if watch is not zero, when this node changed, zk will notify zkclient, 
				 * and call back function will be called.
				 */
				int GetNode(const string& path, string & context, 
						int watch, struct Stat *stat);

				/*
				   if watch is not zero, when this node changed, zk will notify zkclient, 
				   and call back function will be called.
				   use getChildren, if node want to be notified when it has child node created or removed
				   */
				int GetChildren(const string& path, vector<string>& children, int watch);

				// get and handle event thread, event handler func is defined outside.
				void TaskFunc();

				// this function should be called outside.
				void SetEventHandler(EventHandler evh);

				void PutEvent(Event ev);

				bool IsConnected() {  return _connected;}

				void SetConnected( bool var) { _connected = var;}

			private:
			public:
				zhandle_t* _zk;     //real zk client struct
				string _hostPorts;   //zookeeper host and port, can be connected with comma
				int _recvTimeout;	 //zookeeper recv timeout, ms
				clientid_t _clientId; //the unique client ID of the zk client    

				Biz::Mutex _mutex;   // a mutex to protect _connected, this value may be changed in global call back 
				bool _connected;

				BlockQueue<Event> _evq;
				Task *_task; // task to handle event in eventq
				bool _run;  // use to close task;
				EventHandler _eventHandler;

				static const int MAX_GET_BUF_LEN = 40960;
				static const int MAXRETRY = 3;
		};

	}//namespace zkclient
}//namespace bizbase


#endif
