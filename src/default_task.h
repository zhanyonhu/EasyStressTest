#ifndef _DEFAULT_TASK_H_
#define _DEFAULT_TASK_H_

#include "task.h"

struct default_task_node 
{
	struct tcp_task tcp;
	char reversed[512];
};


#endif	/*_DEFAULT_TASK_H_*/