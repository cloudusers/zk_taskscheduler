/**
 * Copyright (c) 2016
 * Copyright Owner: clouduser
 * Author: clouduser@163.com
 */



#include "zk_client.h"
#include "base/utility/exception.h"

#include <iostream>

std::string LOG_INFO ="info";
std::string LOG_ERROR ="error";

namespace bizbase {
namespace zkclient {


/*
 *
 */
static void GlobalCallBack(zhandle_t *zk,
		int type, int state, 
		const char * path, void *v)
{/*{{{*/
	BLOG_INFO_LEVEL(LOG_INFO, "GlobalCallBack, stat:" << state << ", type:" << type << ", path:" << path);

	ZkClient *zkc = static_cast<ZkClient*> (v);
	if(state == ZOO_CONNECTED_STATE) {
		zkc->SetConnected(true);
	} else {
		BLOG_ERROR_LEVEL(LOG_ERROR, "Connection Lost, stat:" << state << ", type:" << type);
		zkc->SetConnected(false);
		zkc->_run = false;
	}

	Event evt;
	evt._path = path;
	evt._type = type;
	evt._state = state;
	zkc->PutEvent(evt);
}/*}}}*/

/*
 *
 */
ZkClient::ZkClient(string hostPorts, int recvTimeout,
		std::string log_info,
		std::string log_error)
{/*{{{*/
	LOG_INFO = log_info;
	LOG_ERROR = log_error;

	_hostPorts = hostPorts;
	_recvTimeout = recvTimeout;
	//   _zk = zookeeper_init(_hostPorts.c_str(), GlobalCallBack, _recvTimeout, 
	//       &_clientId, this, 0);
	_zk = zookeeper_init(_hostPorts.c_str(), GlobalCallBack, _recvTimeout, 
			0, this, 0);

	if(!_zk) {
		BLOG_ERROR_LEVEL(LOG_ERROR, "zookeeper_init failed");
		THROWEXP(BizException, "zookeeper_init failed");
	}

	// wait for connected.
	int i = 0;
	while(zoo_state(_zk) != ZOO_CONNECTED_STATE) {
		int zk_stat = zoo_state(_zk);	
		sleep(1);
		if(i++ > MAXRETRY) {
			BLOG_ERROR_LEVEL(LOG_ERROR, "zookeeper ZOO_CONNECTED_STATE failed, zk_stat:" << zk_stat);
			THROWEXP(BizException, "zookeeper ZOO_CONNECTED_STATE failed");
		}
	}

	_run = true;
	_task = new Task(boost::bind(&ZkClient::TaskFunc, this), "zkclient-taskthread");
}/*}}}*/

/*
 *
 */
ZkClient::~ZkClient()
{/*{{{*/
	_run = false;
	_task->join();
	zookeeper_close(_zk);
}/*}}}*/

/*
 *
 */
void ZkClient::TaskFunc()
{/*{{{*/
	while(_run) {
		Event ev;
		try{
			bool ret = _evq.poll(10, ev);
			if(ret) {
				if(_eventHandler) {
					_eventHandler(ev);
				}
			}
		} catch (...) {
			BLOG_ERROR_LEVEL(LOG_ERROR, "zookeeper evq.poll failed");
		}
	}
}/*}}}*/

/*
 *
 */
void ZkClient::SetEventHandler(EventHandler evhandler)
{/*{{{*/
	_eventHandler = evhandler;
}/*}}}*/

/*
 *
 */
int ZkClient::GetNode(const string& path, string & context, 
		int watch, struct Stat *stat) 
{/*{{{*/
	char buf[MAX_GET_BUF_LEN];
	int buflen = MAX_GET_BUF_LEN;

	int ret = ZCONNECTIONLOSS;
	int count = 0;
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 500000;

	while(ZCONNECTIONLOSS == ret && count < MAXRETRY) {
		ret = zoo_get(const_cast<zhandle_t *>(_zk),
				path.c_str(), watch, &buf[0], &buflen, stat);
		if(ZCONNECTIONLOSS == ret) {
			nanosleep(&ts, 0);
			count++;
		}
	}

	if(ZOK == ret) {
		context.assign(const_cast<const char*>(&buf[0]), buflen);
		return ret;
	} else if(ZNONODE == ret) {
		BLOG_ERROR_LEVEL(LOG_ERROR, "getNode failed: " << path << " node not exist");
	} else if(ZBADARGUMENTS == ret) {
		BLOG_ERROR_LEVEL(LOG_ERROR,"getNode failed: " << path <<" bad arguments");
	} else if(ZINVALIDSTATE == ret) {
		BLOG_ERROR_LEVEL(LOG_ERROR,"getNode failed: " << path <<" invalid zk connection state");
	} else {
		BLOG_ERROR_LEVEL(LOG_ERROR,"getNode failed: " << path <<" return value: " << ret );
	}

	return ret;
}/*}}}*/

/*
 * get children info
 */
int ZkClient::GetChildren( const string& path, vector<string>& children, int watch) 
{/*{{{*/
	struct String_vector childPath;

	int ret = ZCONNECTIONLOSS;
	int count = 0;
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 500000;

	while(ZCONNECTIONLOSS == ret && count < 3) {
		ret = zoo_get_children(const_cast<zhandle_t*>(_zk),
				path.c_str(), watch, &childPath);
		if(ret == ZCONNECTIONLOSS) {
			nanosleep(&ts, 0);
			count++;
		}
	}

	if(ZOK == ret) {
		for(int i = 0 ; i < childPath.count ; ++i) {
			children.push_back(string(childPath.data[i]));
		}
		deallocate_String_vector(&childPath);
	} else if(ZNONODE == ret) {
		BLOG_ERROR_LEVEL(LOG_ERROR,"getChildren failed: " << path << " node not exist");
	} else if(ZBADARGUMENTS == ret) {
		BLOG_ERROR_LEVEL(LOG_ERROR,"getChildren failed: " << path <<" bad arguments");
	} else if(ZINVALIDSTATE == ret) {
		BLOG_ERROR_LEVEL(LOG_ERROR,"getChildren failed: " << path <<" invalid zk connection state");
	} else {
		BLOG_ERROR_LEVEL(LOG_ERROR,"getChildren failed: " << path <<" return value: " << ret );
	}

	return ret;
}/*}}}*/

/*
 *
 */
int ZkClient::CreateRecursion(const string& path,
		const string & content,    int nodeType, 
		char* path_buffer,  int path_buffer_len,
		const struct ACL_vector& acl, int watch)
{/*{{{*/
	int ret = Exists(path, watch);
	if(ret != ZOK) {
		//find all '/' position
		std::vector<int>pos_vec;
		for(size_t i=0; i<path.size(); ++i) {
			if(path[i] == '/') {
				pos_vec.push_back(i);
			}
		}
		pos_vec.push_back(path.size());

		//create all sub path
		for(size_t i=1; i<pos_vec.size(); ++i) {
			std::string sub_path = path.substr(0, pos_vec[i]);
			Create(sub_path, content, 
					nodeType, path_buffer, 
					path_buffer_len, acl);
			Exists(sub_path, watch);//add watch to each sub_path if watch != 0
		}
	}

	ret = Exists(path, watch);
	if(ret != ZOK) {
		BLOG_ERROR_LEVEL(LOG_ERROR,"create node: " << path << " failed: " << ret );
	} else {
		BLOG_INFO_LEVEL(LOG_INFO,"create node: " << path << " succeed: " << ret );
	}

	return ret;
}/*}}}*/

/*
 *
 */
int ZkClient::Create(const string& path,
		const string & content, int nodeType, 
		char* path_buffer, int path_buffer_len,
		const struct ACL_vector& acl)
{/*{{{*/
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 500000;
	int ret = ZCONNECTIONLOSS;
	int retry = 0;

	while(ret != ZOK && retry < MAXRETRY) {
		retry++;

		ret = zoo_create(const_cast<zhandle_t *>(_zk), path.c_str(), 
				content.c_str(), content.size(),
				//&ZOO_OPEN_ACL_UNSAFE, nodeType, 
				&acl, nodeType, 
				path_buffer, path_buffer_len);

		nanosleep(&ts, 0);
	}

	if(ret != ZOK) {
		BLOG_ERROR_LEVEL(LOG_ERROR,"create node: " << path << " failed: " << ret );
	} else {
		BLOG_INFO_LEVEL(LOG_INFO,"create node: " << path << " succeed: " << ret );
	}

	return ret;
}/*}}}*/

/*
 *
 */
int ZkClient::Update( const string& path,
		const string& content, int nodeType, int version)
{/*{{{*/

	int count = 0;
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 500000;
	int ret = ZCONNECTIONLOSS;
	while((ZOK != ret ) && (count < MAXRETRY)) {
		count++;
		// retry the operation
		if(ZNONODE != ret) {
			ret = zoo_set(const_cast<zhandle_t *>(_zk),
					path.c_str(), content.c_str(), content.size(), version);
		} else if(ZNONODE == ret) {
			ret = zoo_create(const_cast<zhandle_t *>(_zk),
					path.c_str(), content.c_str(), content.size(),
					&ZOO_OPEN_ACL_UNSAFE, nodeType, NULL, 0);
		} 
		nanosleep(&ts, 0);
	}

	if(ret != ZOK) {
		BLOG_ERROR_LEVEL(LOG_ERROR,"update node: " << path << " failed: " << ret );
	} else {
		BLOG_INFO_LEVEL(LOG_INFO,"update node: " << path << " succeed: " << ret );
	}

	return ret;
}/*}}}*/

/*
 *
 */
int ZkClient::Del(const string& path, int version) 
{/*{{{*/
	int count = 0;
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 500000;
	int ret = ZCONNECTIONLOSS;
	while(ZOK != ret && count < 3) {
		count++;
		// retry the operation
		if(ZOK != ret) {
			ret = zoo_delete(const_cast<zhandle_t *>(_zk),
					path.c_str(), version);
		}

		nanosleep(&ts, 0);
	}
	if(ret == ZOK ) {
		BLOG_INFO_LEVEL(LOG_INFO,"delete node: " << path << " succeed: " << ret );
	} else if(ret == ZNONODE) {
		BLOG_ERROR_LEVEL(LOG_ERROR,"delete node: " << path << " failed, node not exists" );
	} else {
		BLOG_ERROR_LEVEL(LOG_ERROR,"delete node: " << path << " failed: " << ret );
	}

	return ret;
}/*}}}*/

/*
 *
 */
int ZkClient::Exists(const string& path, int watch)
{/*{{{*/
	int ret = zoo_exists(_zk, path.c_str(), watch, NULL);
	return ret;
}/*}}}*/

/*
 *
 */
void ZkClient::PutEvent(Event ev)
{/*{{{*/
	_evq.put(ev);
}/*}}}*/

}//namespace zkclient
}//namespace bizbase
