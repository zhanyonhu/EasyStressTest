#ifndef _TASK_H_
#define _TASK_H_

#include "commondef.h"
#include <boost/pool/object_pool.hpp>

#include <set>
#include <map>
#include "allocator.h"

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
	boost::object_pool<std::_Tree_node<std::pair<unsigned long long int, struct default_task_node>, struct default_task_node>> task_pool;
	std::stl_allocator<std::pair<unsigned long long int, struct default_task_node>, std::_Tree_node<std::pair<unsigned long long int, struct default_task_node>, struct default_task_node>> task_allocator;
	std::map<unsigned long long int, struct default_task_node, std::less<unsigned long long int>, std::stl_allocator<std::pair<unsigned long long int, struct default_task_node>, std::_Tree_node<std::pair<unsigned long long int, struct default_task_node>, struct default_task_node>>> task_list;
	unsigned long long int cur_id;
};


#endif	/*_TASK_H_*/