/**
 * Copyright (c) 2016
 * Copyright Owner: clouduser
 * Author: clouduser@163.com
 */




#include <iostream>
#include <stdio.h>
#include <vector>
#include <algorithm>

#include "zk_lock.h"
#include "base/utility/common.h"

extern std::string LOG_INFO;
extern std::string LOG_ERROR;

namespace bizbase {
namespace zklock {


/*
 *  first create the lock path node if it not exists,
 *  such as /zzlock/lockpath, this node is a permanent node,
 *  then create the child sequence node, such as /zzlock/lockpath/lockname-1001
 */
ZkLock::ZkLock(ZkClient* zk, std::string path, std::string lockName, std::string identity)
{/*{{{*/
	_zkclient = zk;
	_path = path;
	_name = lockName;
	_isLocked = false;

	int ret = _zkclient->Exists(path);

	//if _path not exist, we should create it first
	if(ret != ZOK) {
		ret = _zkclient->CreateRecursion(_path, "content");
		if(ret != ZOK) {
			BLOG_ERROR_LEVEL(LOG_ERROR, "create lock node failed");
			THROWEXP(BizException, "create lock node failed");
		}
	}

	//lock part
	char childPath[512];
	sprintf(childPath, "%s/%s-", _path.c_str(), _name.c_str());
	char pathBuffer[512];
	ret = _zkclient->Create(childPath, identity, ZOO_SEQUENCE|ZOO_EPHEMERAL,
			pathBuffer, 512);

	// childPath is a full path
	if(ret == ZOK) {
		char* nameStr = rindex(pathBuffer, '/');
		if(nameStr != NULL) {
			nameStr += 1;
			_nodeName = nameStr;
			return;
		}
	}

	THROWEXP(BizException, "zklock construct failed");
}/*}}}*/

/*
 *
 */
ZkLock::~ZkLock()
{/*{{{*/
}/*}}}*/

/*
 * try lock and sleep until succeed.
 */
int ZkLock::Lock()
{/*{{{*/
	BLOG_INFO_LEVEL(LOG_INFO, "geting lock.........");
	int ret = -1;
	while(0!= (ret=TryLock())) {
		if(ZINVALIDSTATE == ret) {
			/*
			 *connection lost and zkhandle invalid
			 */
			return -1;
		}
		::sleep(1);
	}

	_isLocked = true;
	BLOG_INFO_LEVEL(LOG_INFO, "get lock succeed");
	return 0;
}/*}}}*/

/* 
 * get all children, and check whether there is a node less than mine. if not, lock succeed.
 */
int ZkLock::TryLock()
{/*{{{*/
	int i = 0;
	std::vector<std::string> children;
	int ret = _zkclient->GetChildren(_path, children, 1); 

	if(ret != ZOK) {
		BLOG_ERROR_LEVEL(LOG_ERROR, "getchildren failed" << _path);
		return ret;
	} else {
		std::sort(children.begin(), children.end());
		if(children[0] != _nodeName) {
			std::string owner;
			std::string ownerPath = _path + "/" + children[0];
			int getOwner = _zkclient->GetNode(ownerPath, owner, 0, NULL);
			BLOG_INFO_LEVEL(LOG_INFO, "try lock failed, this lock is owned by: " << owner);
			return -1;
		}
	}

	BLOG_INFO_LEVEL(LOG_INFO, "try lock succeed"); 
	return 0;
}/*}}}*/

/*
 *
 */
int ZkLock::Unlock()
{/*{{{*/  
	_isLocked = false;
	int ret = _zkclient->Del(_path + "/" + _nodeName);
	if(ret == ZOK) {
		BLOG_INFO_LEVEL(LOG_INFO, "unlock succeed;");
	} else {
		BLOG_ERROR_LEVEL(LOG_ERROR, "unlock failed;");
	}

	return ret;
}/*}}}*/

/*
 *
 */
bool ZkLock::IsLocked()
{/*{{{*/
	if(_isLocked && _zkclient->IsConnected()) {
		return true;
	} else {
		return false;
	}
}/*}}]*/

}//namespace zklock
}//namespace bizbase
