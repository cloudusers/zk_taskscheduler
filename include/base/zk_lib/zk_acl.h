#ifndef __ZK_ACL__H
#define __ZK_ACL__H

#include "zk_digest.h"

#include <zookeeper/zookeeper.h>

namespace bizbase {
	namespace zkacl {


		/*
		 *init acl_vec
		 */
		bool InitAcl(
				const std::string& user,
				const std::string& pass,
				struct ACL_vector& acl_vec,
				const std::string scheme = "digest"
				);

		/*
		 *add auth to zk with zhandle_t* zk
		 */
		bool AddAuth(
				zhandle_t* zk,
				const std::string& user,
				const std::string& pass,
				const std::string scheme = "digest"
				);
	}//namespace zkacl
}//namespace bizbase

#endif
