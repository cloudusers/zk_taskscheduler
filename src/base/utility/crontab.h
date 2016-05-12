#ifndef __ZKCRONTAB__H
#define __ZKCRONTAB__H


#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <boost/shared_ptr.hpp>

#define SEPARATOR " "


using boost::shared_ptr;
using namespace std;

namespace bizbase {
	namespace zkcrontab {


		/*
		 *this is a crontab that like linux/crontab
		 *each crontab entry has six fields:
		 *minute hour day-of-month month day-of-week command
		 */
		class Crontab {

			public:
				typedef struct cron_entry {
					//ctor
					cron_entry()
					{}
					//dtor
					~cron_entry()
					{}

					//members
					boost::shared_ptr<cron_entry>next;
					char mn[32];
					char hr[32];
					char day[32];
					char mon[32];
					char wkd[32];
					char cmd[512];
				}cront_t;

				/*
				 *output for debuging
				 */
				friend ostream& operator<<(ostream& os, const Crontab& cron)
				{
					boost::shared_ptr<cront_t> entry = cron.head;
					int i = 0;
					while(entry) {
						os << "#" << ++i;
						os << "entry->mn:" <<  entry->mn  ; 
						os << ",entry->hr:" << entry->hr  ; 
						os << ",entry->day:" << entry->day; 
						os << ",entry->mon:" << entry->mon; 
						os << ",entry->wkd:" << entry->wkd; 
						os << ",entry->cmd:" << entry->cmd; 
						os << "\n";

						entry = entry->next;
					}

					return os;
				}

			public:

				Crontab();
				virtual ~Crontab();

			public:

				/*
				 *
				 */
				void AddCrontab(const std::string& line);

				/*
				 *
				 */
				bool IsReady();
				bool IsMatch(char* left, int right);

			private:

				boost::shared_ptr<cront_t> entry;
				boost::shared_ptr<cront_t> head;
		};


	}//crontab
}//bizbase

#endif
