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

static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
	ASSERT(stream != NULL);
	struct tcp_task * ptask = (struct tcp_task *)stream->data;
	if (nread < 0) 
	{
		LOGF_TASK_ERR("read_cb error: %s\n", uv_err_name(nread));
		ASSERT(nread == UV_ECONNRESET || nread == UV_EOF);

		uv_close((uv_handle_t*)&ptask->conn, NULL);
		main_info.AddTask_ToBeDeleted(ptask);

		return;
	}
}

static void connect_cb(uv_connect_t* req, int status)
{
	ASSERT(req != NULL);

	struct tcp_task * ptask = (struct tcp_task *)req->data;
	if (status != 0)
	{
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

	int r = 0;
	r = uv_read_start((uv_stream_t*)&ptask->conn, alloc_cb, read_cb);
	ASSERT(r == 0);
}

static void work_cb(uv_work_t* req) 
{
	int r = 0;
	struct tcp_task * ptask = (struct tcp_task *)req->data;

	r = uv_tcp_init(uv_default_loop(), &ptask->conn);
	ASSERT(r == 0);

	r = uv_tcp_connect(&ptask->connect_req,
		&ptask->conn,
		(const struct sockaddr*) &ptask->addr,
		connect_cb);

}

static void after_work_cb(uv_work_t* req, int status) 
{
}

int tcp_task_post(struct tcp_task * ptask)
{
	int r=0;
	ptask->work_req.data = ptask;
	ptask->connect_req.data = ptask;
	ptask->conn.data = ptask;
	//r = uv_queue_work(uv_default_loop(), &ptask->work_req, work_cb, after_work_cb);
	//ASSERT(r == 0);
	work_cb(&ptask->work_req);
	return 0;
}