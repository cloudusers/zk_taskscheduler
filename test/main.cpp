#include <stdio.h>
#include <iostream>
#include <string.h>

#include "sche/zk_tasksche.h"
using namespace bizbase::zktasksche;


void aTaskCallBack(void* data)
{
	char task_name[32] = {'\0'};
	strcpy(task_name, (char*)data);

	std::cout << "################## EXEC a task_name:" 
<<  task_name << std::endl;

	return;
}

void bTaskCallBack(void* data)
{
	char task_name[32] = {'\0'};
	strcpy(task_name, (char*)data);

	std::cout << "################## EXEC b task_name:" 
<<  task_name << std::endl;

	return;
}

void TaskCallBack(void* data)
{
	char task_name[32] = {'\0'};
	strcpy(task_name, (char*)data);

	std::cout << "################## EXEC yourself task_name:" 
<<  task_name << std::endl;

	return;
}


int main() 
{
	Biz::Logger().config("test.prop");

	std::map<std::string, std::string>conf;
	conf["zk_hostport"] = "127.0.0.1:2181";
	conf["zk_timeout"] = "3000";
	conf["root_node"] = "/task-sche-root";
	conf["log_info"] = "info";
	conf["log_error"] = "error";
	conf["use_check_chain"] = "true";
	conf["use_crontab"] = "false";
	conf["use_acl"] = "false";
	conf["auth_user"] = "world";
	conf["auth_pass"] = "anyone";

	std::string no;
	std::string a = "a";
	std::string b = "b";
	std::string c = "c";
	std::string m = "m";
	std::string n = "n";

	std::vector<std::string>cron_vec;
	cron_vec.push_back("* * * * * cmd1");

	TaskSche task(conf);
	task.AddTask(
			a,//task_name
			aTaskCallBack,
			no,//pre->task_name*
			&cron_vec, //crontab
			0, //max_ts(s)
			1, //priority
			1);//reserve

	task.AddTask(b,  bTaskCallBack, no, &cron_vec, 0, 2, 2);
	task.AddTask(c,  TaskCallBack, a, &cron_vec, 0, 0, 31);
	task.AddTask(c,  TaskCallBack, b, &cron_vec, 0, 0, 32);
	task.AddTask(m,  TaskCallBack, c, &cron_vec, 0, 0, 4);
	task.AddTask(n,  TaskCallBack, c, &cron_vec, 0, 0, 5);

	//task.RunOnce();
	task.RunLoop();

	std::cout << task << std::endl;

	//getchar();

	return 0;
}
