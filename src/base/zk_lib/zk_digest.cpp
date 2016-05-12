/**
 * Copyright (c) 2016
 * Copyright Owner: clouduser
 * Author: clouduser@163.com
 */



#include "zk_digest.h"

using namespace std;

namespace bizbase {
namespace zkdigest {


/*
   void printsh1(unsigned char *md, int len)  
   {  
   for(int i = 0; i < len; i++)  {   
   printf("%02x", md[i]);  
   }

   printf("\n");
   }
   */

/*
 *sha1
 */
bool Sha1(const char* src, unsigned char* md)
{/*{{{*/
	unsigned char* ret = SHA1((unsigned char*)src, strlen(src), md);
	if(0) {
		return false;
	}

	return true;
}/*}}}*/

/*
 *bae64encode
 */
bool Base64Encode(const std::string& input, std::string& output)
{/*{{{*/
	typedef boost::archive::iterators::base64_from_binary<boost::archive::iterators::transform_width<std::string::const_iterator, 6, 8> > Base64EncodeIterator;
	std::stringstream result;
	copy(Base64EncodeIterator(input.begin()) , Base64EncodeIterator(input.end()), ostream_iterator<char>(result));
	size_t equal_count = (3 - input.length() % 3) % 3;
	for(size_t i = 0; i < equal_count; i++) {
		result.put('=');
	}
	output = result.str();
	return output.empty() == false;
}/*}}}*/

/*
 *base64 decode
 */
bool Base64Decode(const std::string& input, std::string& output)
{/*{{{*/
	typedef boost::archive::iterators::transform_width<boost::archive::iterators::binary_from_base64<std::string::const_iterator>, 8, 6> Base64DecodeIterator;  
	std::stringstream result;  
	try {  
		copy(Base64DecodeIterator(input.begin()) , Base64DecodeIterator(input.end()), ostream_iterator<char>(result));  
	} catch(...) {  
		return false;  
	}  
	output = result.str();
	return output.empty() == false;  
}/*}}}*/ 

/*
 *digest = base64encode(sha1(user:pass))
 *@param input: usr:pass
 *@param output: digest
 *return bool
 */
bool Digest(const std::string& input, std::string& output)
{/*{{{*/
	unsigned char md[SHA_DIGEST_LENGTH];  
	if(Sha1(input.c_str(), md)) {
		std::string sha1_str(SHA_DIGEST_LENGTH, '\0');
		strcpy(const_cast<char*>(sha1_str.c_str()), (char*)md);
		if(Base64Encode(sha1_str, output)) {
			return true;
		}
	}
	return false;
}/*}}}*/

}//namespace zkacl
}//namespace bizbase
