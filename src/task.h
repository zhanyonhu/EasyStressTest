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
	uint64_t id;
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
	struct DELETE_NODE
	{
		struct tcp_task * ptask;
		time_t time_deleted;
	};

public:
	typedef STL::fixed_hash_map<uint64_t, struct default_task_node, MAX_TASK_COUNT> TASK_LIST;
	typedef TASK_LIST::iterator TASK_ITER;
	typedef STL::pair<TASK_ITER, bool> TASK_PAIR;

	typedef STL::fixed_list<DELETE_NODE, MAX_TASK_COUNT> TO_BE_DELETED_TASK_LIST;
	typedef TO_BE_DELETED_TASK_LIST::iterator TO_BE_DELETED_TASK_ITER;

public:
	CTasks();
	~CTasks();

public:
	void AddTask_ToBeDeleted(struct tcp_task * ptask);
	void DeleteTask_ToBeDeleted();

	void Clear();
	struct default_task_node * Add(struct default_task_node & task);
	size_t Count(){ return task_list.size(); };

protected:
	TASK_LIST task_list;
	TO_BE_DELETED_TASK_LIST to_delete_task_list;
	uint64_t cur_id;
	uv_mutex_t to_delete_task_list_mutex;
};


#endif	/*_TASK_H_*/