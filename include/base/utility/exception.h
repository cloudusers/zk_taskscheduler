#ifndef __BASE_TOOL_EXCEPTION_H_
#define __BASE_TOOL_EXCEPTION_H_

#include <string>
#include <exception>
#include "base/utility/message.h"

using std::string;

class BizException
{
	public:
		BizException(const Biz::message &msg)
		{
			_message = msg.str();
		}
		const char*  what() const throw()
		{
			return _message.c_str();
		}
		~BizException() throw() {}

	protected:
		string  _message;
};

class BadParameterException : public BizException
{
	public:
		BadParameterException(const Biz::message &msg) : BizException("BadParameterException-" + msg.str())
	{}
};

class JobException : public BizException
{
	public:
		JobException(const Biz::message& msg) 
			: BizException("[EXCEPTION]JobException" + msg.str())
		{}
};

#define THROWEXP(exceptName,msg) \
	do { stringstream sstream;  \
		sstream << "["<<__FILE__ <<"]["<<__func__ <<"][" << __LINE__<<"]" << msg; \
		throw exceptName(sstream.str()); \
	} while(0)

#endif
