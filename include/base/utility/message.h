#ifndef __BASE_MESSAGE_H_
#define __BASE_MESSAGE_H_


#include <string>
#include <sstream>

namespace Biz {
	class message
	{
		public:
			message() : _message("") { }

			message(const std::string msg) : _message(msg) { }
			message(const char* msg) : _message(msg) { }

			std::string str() const
			{
				return _message;
			}

			operator std::string()
			{
				return _message;
			}

			template<typename T>
				message& operator<<(const T msg)
				{
					std::ostringstream oss;
					oss << msg;
					_message += oss.str();

					return *this;
				}

		private:
			std::string _message;
	};

}
#endif
