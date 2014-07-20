#ifndef _HTTP_TASK_H_
#define _HTTP_TASK_H_

#include "commondef.h"
#include "task.h"

int http_on_init(struct tcp_task * ptask);
void http_on_connected_failed(struct tcp_task * ptask, uint32_t error);
void http_on_connected_successful(struct tcp_task * ptask);
void http_on_recv(struct tcp_task * ptask, const uv_buf_t* buf, ssize_t);
void http_on_send_ok(struct tcp_task * ptask);
void http_on_send_error(struct tcp_task * ptask, uint32_t error);
void http_on_close(struct tcp_task * ptask, uint32_t error);

#endif	/*_HTTP_TASK_H_*/