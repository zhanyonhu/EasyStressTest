#ifndef _TASK_H_
#define _TASK_H_

#include "commondef.h"
#include <boost/pool/object_pool.hpp>

#include <set>
#include <map>

struct tcp_task_config
{
};

struct tcp_task 
{
	uv_tcp_t conn;
	uv_connect_t connect_req;
	uv_work_t work_req;
	struct sockaddr_in addr;
	struct tcp_task_config config;
	char read_buf[1024];
};

struct default_task_node
{
	struct tcp_task tcp;
	char reversed[512];
};

int tcp_task_post(struct tcp_task * ptask);

class CTasks
{
public:
	CTasks();
	~CTasks();

public:
	void Clear();
	struct default_task_node * Add(struct default_task_node & task);
	size_t Count(){ return task_list.size(); };

protected:
	std::map<unsigned long long int, struct default_task_node> task_list;
	unsigned long long int cur_id;
};


#endif	/*_TASK_H_*/