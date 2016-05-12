#ifndef _BASE_LOGGER_H_
#define _BASE_LOGGER_H_

#include <string>
#include <sstream>
#include <boost/noncopyable.hpp>
#include <log4cplus/logger.h>
#include <log4cplus/configurator.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/ndc.h> 
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/loggingmacros.h>

using std::string;
using std::stringstream;
using log4cplus::PropertyConfigurator;

#define ERROR "error"
#define INFO  "info"
#define DEBUG "debug"

namespace Biz {
	class Logger : public boost::noncopyable 
	{
		public:
			Logger() {}

			void config(const string& config_file_name)
			{        
				PropertyConfigurator::doConfigure(config_file_name);
			}
			void trace(const string& logger_name, const string& msg)
			{
				log4cplus::Logger log = log4cplus::Logger::getInstance(logger_name);
				LOG4CPLUS_TRACE(log, msg);
			}
			void debug(const string& logger_name, const string& msg) const
			{
				log4cplus::Logger log = log4cplus::Logger::getInstance(logger_name);
				LOG4CPLUS_DEBUG(log, msg);           
			}
			void info(const string& logger_name, const string& msg) const
			{
				log4cplus::Logger log = log4cplus::Logger::getInstance(logger_name);
				LOG4CPLUS_INFO(log, msg);
			}
			void warn(const string& logger_name, const string& msg) const
			{
				log4cplus::Logger log = log4cplus::Logger::getInstance(logger_name);
				LOG4CPLUS_WARN(log, msg);
			}
			void error(const string& logger_name, const string& msg) const
			{

				log4cplus::Logger log = log4cplus::Logger::getInstance(logger_name);
				LOG4CPLUS_ERROR(log, msg);
			}
			void fatal(const string& logger_name, const string& msg) const
			{
				log4cplus::Logger log = log4cplus::Logger::getInstance(logger_name);
				LOG4CPLUS_FATAL(log, msg);
			}
		private:
			Logger& operator=(const Logger&);
	};
}

#define BLOG_ERROR(msg) \
	do { stringstream sstream;    \
		sstream << "["<<__FILE__ <<"]["<<__func__ <<"][" << __LINE__<<"]" << msg; \
		Biz::Logger().error(ERROR, sstream.str()); \
	} while(0)

#define BLOG_WARN(msg) \
	do { stringstream sstream;  \
		sstream << "["<<__FILE__ <<"]["<<__func__ <<"][" << __LINE__<<"]" << msg; \
		Biz::Logger().warn(INFO, sstream.str()); \
	} while(0)


#define BLOG_INFO(msg) \
	do { stringstream sstream;  \
		sstream << "["<<__FILE__ <<"]["<<__func__ <<"][" << __LINE__<<"]" << msg; \
		Biz::Logger().info(INFO, sstream.str()); \
	} while(0)

#define BLOG_DEBUG(msg) \
	do { stringstream sstream;     \
		sstream << "["<<__FILE__ <<"]["<<__func__ <<"][" << __LINE__<<"]" << msg; \
		Biz::Logger().debug(INFO, sstream.str()); \
	} while(0)

#define BLOG_TRACE(msg) \
	do { stringstream sstream;     \
		sstream << "["<<__FILE__ <<"]["<<__func__ <<"][" << __LINE__<<"]" << msg; \
		Biz::Logger().trace(INFO, sstream.str()); \
	} while(0)

#define BLOG_INFO_LEVEL(LoggerName, msg) \
	do { stringstream sstream;  \
		sstream << "["<<__FILE__ <<"]["<<__func__ <<"][" << __LINE__<<"]" << msg; \
		Biz::Logger().info(LoggerName, sstream.str()); \
	} while(0)

#define BLOG_ERROR_LEVEL(LEVEL, msg) \
	do { stringstream sstream;  \
		sstream << "["<<__FILE__ <<"]["<<__func__ <<"][" << __LINE__<<"]" << msg; \
		Biz::Logger().error(LEVEL, sstream.str()); \
	} while(0)

#define BLOG_DEBUG_LEVEL(LEVEL, msg) \
	do { stringstream sstream;  \
		sstream << "["<<__FILE__ <<"]["<<__func__ <<"][" << __LINE__<<"]" << msg; \
		Biz::Logger().debug(LEVEL, sstream.str()); \
	} while(0)

#define BLOG_FATAL_LEVEL(LEVEL, msg) \
	do { stringstream sstream;  \
		sstream << "["<<__FILE__ <<"]["<<__func__ <<"][" << __LINE__<<"]" << msg; \
		Biz::Logger().fatal(LEVEL, sstream.str()); \
	} while(0)



#endif
