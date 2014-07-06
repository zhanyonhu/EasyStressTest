#ifndef _TASK_H_
#define _TASK_H_

#include "commondef.h"

struct tcp_task_config
{
};

struct tcp_task 
{
	uv_tcp_t conn;
	uv_connect_t connect_req;
	uv_work_t work_req;
	uv_timer_t timer;
	struct sockaddr_in addr;
	struct tcp_task_config config;
};

int tcp_task_post(struct tcp_task * ptask);

#endif	/*_DEFAULT_TASK_H_*/