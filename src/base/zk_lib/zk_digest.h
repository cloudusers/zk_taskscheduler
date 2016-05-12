#ifndef __ZK_DIGEST__H
#define __ZK_DIGEST__H


#include <string.h>
#include <iostream> 
#include <sstream>
#include <openssl/sha.h>

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>

namespace bizbase {
	namespace zkdigest {


		/*
		 *sha1
		 */
		bool Sha1(const char* src, unsigned char* md);

		/*
		 *bae64encode
		 */
		bool Base64Encode(const std::string& input, std::string& output);

		/*
		 *base64 decode
		 */
		bool Base64Decode(const std::string& input, std::string& output);

		/*
		 *base64 decode
		 */
		bool Digest(const std::string& input, std::string& output);

	}//namespace zkdigest
}//namespace bizbase


#endif
