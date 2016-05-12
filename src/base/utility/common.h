#ifndef __BASE_COMMON_H_
#define __BASE_COMMON_H_

#define MAX_PRE_TASKS_NUM 20
#ifndef FOREACH
#define FOREACH(iter, container) \
	for (__typeof((container).begin()) iter = (container).begin(); iter != (container).end(); ++iter)
#endif

#include <assert.h>
#include <base/utility/logger.h>
#include <base/utility/exception.h>
#include <base/utility/message.h>

#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp> 
#include <boost/shared_ptr.hpp>

using std::string;
using std::stringstream;
using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::map;
using boost::posix_time::ptime;
using boost::shared_ptr;

#endif

