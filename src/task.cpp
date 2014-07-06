#include "task.h"
#include "StressTest.h"

static void connect_cb(uv_connect_t* req, int status)
{
	ASSERT(req != NULL);
	if (status != 0)
	{
		struct tcp_task * ptask = (struct tcp_task *)req->data;

// 		switch (status)
// 		{
// 		case UV_ETIMEDOUT:
// 			LOGF("err>>connect failed! time out, status=%d, addr=%s:%d\n", status,
// 				inet_ntoa(ptask->addr.sin_addr), ntohs(ptask->addr.sin_port));
// 			break;
// 		case UV_ECANCELED:
// 			LOGF("err>>connect failed! canceled, status=%d, addr=%s:%d\n", status,
// 				inet_ntoa(ptask->addr.sin_addr), ntohs(ptask->addr.sin_port));
// 			break;
// 		default:
// 			LOGF("err>>connect failed! status=%d, addr=%s:%d\n", status,
// 				inet_ntoa(ptask->addr.sin_addr), ntohs(ptask->addr.sin_port));
//			break;
// 		}
		uv_close((uv_handle_t*)&ptask->conn, NULL);
 		main_info.AddTask_ToBeDeleted((struct tcp_task *)req->data);
	}
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
	ASSERT(r == 0);

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
	ptask->timer.data = ptask;
	r = uv_queue_work(uv_default_loop(), &ptask->work_req, work_cb, after_work_cb);
	ASSERT(r == 0);
	return 0;
}