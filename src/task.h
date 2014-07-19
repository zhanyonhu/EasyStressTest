#ifndef _TASK_H_
#define _TASK_H_

#include "commondef.h"

typedef int(*PCallbackInit)(struct tcp_task * ptask);
typedef void(*PCallbackErr)(struct tcp_task * ptask, uint32_t error);
typedef void(*PCallback)(struct tcp_task * ptask);
typedef void(*PCallbackData)(struct tcp_task * ptask, const uv_buf_t* buf, ssize_t);

struct tcp_task_callback
{
	PCallbackInit on_init;
	PCallbackErr on_connected_failed;
	PCallback on_connected_successful;
	PCallbackData on_recv;
	PCallbackData on_send;
	PCallbackErr on_close;
};

struct tcp_task 
{
	uv_tcp_t conn;
	uv_connect_t connect_req;
	uv_work_t work_req;
	uint64_t id;
	struct sockaddr_in addr;
	char read_buf[1024];
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
	typedef STL::fixed_hash_map<uint64_t, struct tcp_task, MAX_TASK_COUNT> TASK_LIST;
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
	struct tcp_task * Add(struct tcp_task & task);
	size_t Count(){ return task_list.size(); };

protected:
	TASK_LIST task_list;
	TO_BE_DELETED_TASK_LIST to_delete_task_list;
	uint64_t cur_id;
	uv_mutex_t to_delete_task_list_mutex;
};


#endif	/*_TASK_H_*/