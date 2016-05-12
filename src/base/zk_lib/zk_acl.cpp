/**
 * Copyright (c) 2016
 * Copyright Owner: clouduser
 * Author: clouduser@163.com
 */




#include "zk_digest.h"

#include <zookeeper/zookeeper.h>

namespace bizbase {
namespace zkacl {

/*
 *here we set ONLY two digest, first is set by user with user:pass, the other set default with read:only
 *note: user:pass will own read|write|create|delete|admin  all permision
 *but read:only ONLY own read permision
 */
bool InitAcl(
		const std::string& user,/*admin*/
		const std::string& pass,/*pass*/
		struct ACL_vector& acl_vec,
		const std::string scheme/*default = digest*/
		)
{/*{{{*/
	std::string digest_admin, digest_read;
	std::string admin_up = user + std::string(":") + pass;
	std::string read_up = "read:only";
	if(! bizbase::zkdigest::Digest(admin_up, digest_admin) ||
			! bizbase::zkdigest::Digest(read_up, digest_read) ) {
		return false;
	}

	std::string acl_admin = user + std::string(":") + digest_admin; 
	std::string acl_read= std::string("read:") + digest_read; 

	/*
	   struct Id id_admin, id_readonly;
	   id_admin.scheme = const_cast<char*>(scheme.c_str());
	   id_admin.id = const_cast<char*>(acl_admin.c_str());
	   id_readonly.scheme = const_cast<char*>(scheme.c_str());
	   id_readonly.id = const_cast<char*>(acl_read.c_str());

	   struct ACL acls[] = {
	   {ZOO_PERM_ALL, id_admin},
	   {ZOO_PERM_READ, id_readonly}
	   };
	   acl_vec = {
	   2,
	   acls
	   };
	//struct ACL_vector acl_vec = { 
	//	2, 
	//	acls
	//};
	*/
	/*
	 *here acl[2] leak memory, you MUST explicite delete if exit in your application
	 */
	struct ACL *acls = new struct ACL[2];
	acls[0].perms = ZOO_PERM_ALL;
	acls[0].id.scheme  = const_cast<char*>(scheme.c_str()); 
	acls[0].id.id = const_cast<char*>(acl_admin.c_str());
	acls[1].perms = ZOO_PERM_READ;
	acls[1].id.scheme = const_cast<char*>(scheme.c_str());
	acls[1].id.id = const_cast<char*>(acl_read.c_str());

	acl_vec.count = 2;
	acl_vec.data = acls;

	return true;
}/*}}}*/


/*
 *add digest auth
 *there are two accounts which admin and readonly
 */
bool AddAuth( zhandle_t* zk,
		const std::string& user,
		const std::string& pass,
		const std::string scheme = "digest"
		)
{/*{{{*/
	std::string user_pass= user + std::string(":") + pass;
	std::string read_only = std::string("read:only");
	int ret = zoo_add_auth(zk, 
			scheme.c_str(), 
			user_pass.c_str(),
			user_pass.size(),
			NULL,
			NULL);

	if(ZOK == ret) {
		zoo_add_auth(zk,/*skip handel checker*/
				scheme.c_str(),/*default=digest*/ 
				read_only.c_str(),
				read_only.size(),
				NULL,
				NULL);

		return true;
	}

	return false;
}/*}}}*/


}//namespace zkacl
}//namespace bizbase
