#include "task.h"
#include "StressTest.h"

static void alloc_cb(uv_handle_t* handle,
	size_t suggested_size,
	uv_buf_t* buf) 
{
	ASSERT(handle != NULL);
	struct tcp_task * ptask = (struct tcp_task *)handle->data;
	buf->len = sizeof(ptask->read_buf);
	buf->base = ptask->read_buf;
	memset(buf->base, 0, buf->len);
	ASSERT(buf->base != NULL);
}

static void close_cb(uv_handle_t* handle)
{
	struct tcp_task * ptask = (struct tcp_task *)handle->data;
	main_info.AddTask_ToBeDeleted(ptask);

	if (main_info.tcp_task_callback.on_close != NULL)
	{
		main_info.tcp_task_callback.on_close(ptask);
	}
}

static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	if (nread == 0)
	{
		//WSAEWOULDBLOCK
		return;
	}

	ASSERT(stream != NULL);
	struct tcp_task * ptask = (struct tcp_task *)stream->data;
	if (nread < 0) 
	{
		LOGF_TASK_ERR("read_cb error: %s\n", uv_err_name((int)nread));
		ASSERT(nread == UV_ECONNRESET || nread == UV_EOF);

		uv_close((uv_handle_t*)&ptask->conn, close_cb);

		return;
	}

	if (main_info.tcp_task_callback.on_recv!=NULL)
	{
		main_info.tcp_task_callback.on_recv(ptask, buf, nread);
	}
}

static void write_cb(uv_write_t* req, int status)
{
	struct tcp_task * ptask = (struct tcp_task *)req->data;
	if (status!=0)
	{
		LOGF_TASK_ERR("write_cb error: %s\n", uv_err_name((int)status));
		ASSERT(status == UV_ECONNRESET || status == UV_EOF);

		if (main_info.tcp_task_callback.on_send_error != NULL)
		{
			main_info.tcp_task_callback.on_send_error(ptask, status);
		}

		uv_close((uv_handle_t*)&ptask->conn, close_cb);

		return;
	}

	if (main_info.tcp_task_callback.on_send_ok != NULL)
	{
		main_info.tcp_task_callback.on_send_ok(ptask);
	}
}

int do_close(struct tcp_task * ptask, bool release_it/* = false*/)
{
	uv_read_stop((uv_stream_t*)&ptask->conn);
	uv_close((uv_handle_t*)&ptask->conn, close_cb);
	ptask->delete_immediately = release_it;
	return 0;
}

int do_read(struct tcp_task * ptask)
{
	int r = 0;
	r = uv_read_start((uv_stream_t*)&ptask->conn, alloc_cb, read_cb);
	ASSERT(r == 0);
	return r;
}

int do_write(struct tcp_task * ptask, const uv_buf_t * bufs, unsigned int nbufs)
{
	int r = 0;
	r = uv_write((uv_write_t *)&ptask->connect_req, (uv_stream_t*)&ptask->conn, bufs, nbufs, write_cb);
	ASSERT(r == 0);
	return r;
}

static void on_connect(uv_connect_t* req, int status)
{
	ASSERT(req != NULL);

	struct tcp_task * ptask = (struct tcp_task *)req->data;
	if (status != 0)
	{
		if (main_info.tcp_task_callback.on_connected_failed != NULL)
		{
			main_info.tcp_task_callback.on_connected_failed(ptask, status);
		}

		switch (status)
		{
		case UV_ETIMEDOUT:
			LOGF_TASK_ERR("err>>connect failed! time out, status=%d, addr=%s:%d\n", status,
				inet_ntoa(ptask->addr.sin_addr), ntohs(ptask->addr.sin_port));
			break;
		case UV_ECANCELED:
			LOGF_TASK_ERR("err>>connect failed! canceled, status=%d, addr=%s:%d\n", status,
				inet_ntoa(ptask->addr.sin_addr), ntohs(ptask->addr.sin_port));
			break;
		default:
			LOGF_TASK_ERR("err>>connect failed! status=%d, addr=%s:%d\n", status,
				inet_ntoa(ptask->addr.sin_addr), ntohs(ptask->addr.sin_port));
			break;
		}

		if (uv_is_closing((uv_handle_t*)&ptask->conn))
		{
			return;
		}

		uv_close((uv_handle_t*)&ptask->conn, NULL);
 		main_info.AddTask_ToBeDeleted((struct tcp_task *)req->data);
		return;
	}

	if (main_info.tcp_task_callback.on_connected_successful != NULL)
	{
		main_info.tcp_task_callback.on_connected_successful(ptask);
	}
	else
	{
		do_read(ptask);
	}
}

static void work_cb(void* req)
{
	int r = 0;
	struct tcp_task * ptask = (struct tcp_task *)req;

	r = uv_tcp_init(main_info.loop, &ptask->conn);
	ASSERT(r == 0);

	r = uv_tcp_connect(&ptask->connect_req,
		&ptask->conn,
		(const struct sockaddr*) &ptask->addr,
		on_connect);

}

static void work_uv_cb(uv_work_t* req)
{
	int r = 0;
	struct tcp_task * ptask = (struct tcp_task *)req->data;

	r = uv_tcp_init(main_info.loop, &ptask->conn);
	ASSERT(r == 0);

	r = uv_tcp_connect(&ptask->connect_req,
		&ptask->conn,
		(const struct sockaddr*) &ptask->addr,
		on_connect);

}

static void after_work_cb(uv_work_t* req, int status) 
{
}

int tcp_task_post(struct tcp_task * pttask)
{
	int r = 0;
	if (main_info.tcp_task_callback.on_init != NULL)
	{
		r = main_info.tcp_task_callback.on_init(pttask);
	}

	if (r<0)
	{
		return -1;
	}

	struct tcp_task * ptask = main_info.tasks.Add(*pttask);

	ptask->work_req.data = ptask;
	ptask->connect_req.data = ptask;
	ptask->conn.data = ptask;

	r = uv_queue_work(main_info.loop, &ptask->work_req, work_uv_cb, after_work_cb);
	ASSERT(r == 0);

	//main_info.threads.AddTask(work_cb, ptask);

	return 0;
}

CTasks::CTasks()
{
	uv_mutex_init(&to_delete_task_list_mutex);
	cur_id = 0;
}

CTasks::~CTasks()
{
	Clear();
	uv_mutex_destroy(&to_delete_task_list_mutex);
}

void CTasks::Clear()
{
	task_list.clear();
}

struct tcp_task * CTasks::Add(struct tcp_task & task)
{
	TASK_PAIR pair;
	pair = task_list.insert(STL::make_pair(++cur_id, task));
	ASSERT(pair.second);
	if (!pair.second)
	{
		return NULL;
	}

	pair.first->second.id = cur_id;
	return &pair.first->second;
}

void CTasks::AddTask_ToBeDeleted(struct tcp_task * ptask)
{
	uv_mutex_lock(&to_delete_task_list_mutex);
	DELETE_NODE node;
	node.ptask = ptask;
	node.time_deleted = time(NULL);
	to_delete_task_list.push_back(node);
	uv_mutex_unlock(&to_delete_task_list_mutex);
}

void CTasks::DeleteTask_ToBeDeleted()
{
	if (to_delete_task_list.size()>0)
	{
		uv_mutex_lock(&to_delete_task_list_mutex);
		TO_BE_DELETED_TASK_ITER piter;
		int count = CHECK_DELETE_COUNT;
		int i = 0;
		time_t t = time(NULL);
		for (piter = to_delete_task_list.begin(); piter != to_delete_task_list.end() && i<count; i++)
		{
			if ((piter->ptask->delete_immediately || t - piter->time_deleted>TIMEOUT_FOR_RELEASE)
				&& (uv_is_closing((uv_handle_t*)&piter->ptask->conn))
				&& piter->ptask->conn.reqs_pending == 0
				&& piter->ptask->conn.activecnt == 0
				)
			{
				task_list.erase(piter->ptask->id);
				piter = to_delete_task_list.erase(piter);
			}
			else
			{
				piter++;
				break;
			}
		}
		uv_mutex_unlock(&to_delete_task_list_mutex);
	}
}
