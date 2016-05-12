/*
 * Copyright (c) 2016
 * Copyright Owner: clouduser
 * Author: clouduser@163.com
 */


#include "crontab.h"

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>


namespace bizbase {
	namespace zkcrontab {


/*
 *
 */
Crontab::Crontab()
{/*{{{*/
	entry = boost::shared_ptr<cront_t>(new cront_t());
	head = entry;
}/*}}}*/

/*
 *
 */
Crontab::~Crontab()
{/*{{{*/
}/*}}}*/


/*
 *
 */
void Crontab::AddCrontab(const std::string& line)
{/*{{{*/
	std::vector<std::string>res_vec;
	boost::split(res_vec , line, boost::is_any_of(SEPARATOR));

	strcpy(entry->mn, const_cast<char*>(res_vec[0].c_str()));
	strcpy(entry->hr, const_cast<char*>(res_vec[1].c_str()));
	strcpy(entry->day, const_cast<char*>(res_vec[2].c_str()));
	strcpy(entry->mon, const_cast<char*>(res_vec[3].c_str()));
	strcpy(entry->wkd, const_cast<char*>(res_vec[4].c_str()));
	strcpy(entry->cmd, const_cast<char*>(res_vec[5].c_str()));
/*
	entry->mn     = const_cast<char*>(res_vec[0].c_str()); 
	entry->hr     = const_cast<char*>(res_vec[1].c_str()); 
	entry->day    = const_cast<char*>(res_vec[2].c_str()); 
	entry->mon    = const_cast<char*>(res_vec[3].c_str()); 
	entry->wkd    = const_cast<char*>(res_vec[4].c_str()); 
	entry->cmd    = const_cast<char*>(res_vec[5].c_str()); 
*/

	entry->next = boost::shared_ptr<cront_t>(new cront_t());
	entry->next->next = boost::shared_ptr<cront_t>();
	entry = entry->next;

	//std::cout << *this << std::endl;

	return;
}/*}}}*/


/*
 *check crontab line 
 */
bool Crontab::IsMatch(char* left, register int right)
{/*{{{*/
	register int n=0;
	register char c='\0';

	if(!strcmp(left, "*")) {
		return true;
	}

	n = 0;
	while((c = *left++) && (c >= '0') && (c <= '9')) {
		n = (n * 10) + c - '0';
	}

	switch (c) {
		case '\0':
			return right == n;
			/*NOTREACHED*/
			break;
		case ',':
			if(right == n) {
				return true;
			}

			do{
				n = 0;
				while((c = *left++) && (c >= '0') && (c <= '9')) {
					n = (n * 10) + c - '0';
				}

				if(right == n) {
					return true;
				}
			}while(c == ',');

			return false;
			/*NOTREACHED*/
			break;
		case '-':
			if(right < n) {
				return false;
			}
			n = 0;
			while((c = *left++) && (c >= '0') && (c <= '9')) {
				n = (n * 10) + c - '0';
			}

			return right <= n;
			/*NOTREACHED*/
			break;
		default:
			break;
	}

	return false;
}/*}}}*/

/*
 *
 */
bool Crontab::IsReady()
{/*{{{*/
	boost::shared_ptr<cront_t>this_entry = head;

	register struct tm *tm;
	time_t cur_time;
	time(&cur_time);
	tm = localtime(&cur_time);//ignore localtime_r

	while(this_entry->next) {
		//std::cout << *this << std::endl;
		if(IsMatch(this_entry->mn, tm->tm_min) &&
				IsMatch(this_entry->hr, tm->tm_hour) &&
				IsMatch(this_entry->day, tm->tm_mday) &&
				IsMatch(this_entry->mon, tm->tm_mon + 1) &&
				IsMatch(this_entry->wkd, tm->tm_wday)) {

			   printf("crontab_match: %02d/%02d-%02d:%02d  cmd:%s\n",
			   tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min,
			   this_entry->cmd);

			   return true;
		}

		this_entry = this_entry->next;
	}

	return false;
}/*}}}*/


	}//zkcrontab
}//bizbase
