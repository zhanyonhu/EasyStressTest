#include "http_task.h"
#include "StressTest.h"



int http_on_init(struct tcp_task * ptask)
{
	int ret = 0;
	struct hostent FAR * phost = gethostbyname("www.baidu.com");
	if (phost==NULL)
	{
		return -1;
	}

	ptask->addr.sin_port = htons(80);
	ptask->addr.sin_family = AF_INET;
	ptask->addr.sin_addr = *(struct in_addr *)phost->h_addr_list[0];
	return 0;
}

void http_on_connected_failed(struct tcp_task * ptask, uint32_t error)
{

}

void http_on_connected_successful(struct tcp_task * ptask)
{

}

void http_on_recv(struct tcp_task * ptask, const uv_buf_t* buf, ssize_t)
{

}

void http_on_send(struct tcp_task * ptask, const uv_buf_t* buf, ssize_t)
{

}

void http_on_close(struct tcp_task * ptask, uint32_t error)
{

}

