#include "http_task.h"
#include "StressTest.h"

#define TEST_HTTP_HOST			"www.163.com"

int http_on_init(struct tcp_task * ptask)
{
	int ret = 0;
	struct hostent FAR * phost = gethostbyname(TEST_HTTP_HOST);
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
	int ret = 0;
	uv_buf_t buf;
	buf.base = "GET / HTTP/1.1\r\nHost: " TEST_HTTP_HOST "\r\n\r\n";
	buf.len = (unsigned long)strlen(buf.base);
	ret = do_write(ptask, &buf, 1);
	ASSERT(ret == 0);
}

void http_on_recv(struct tcp_task * ptask, const uv_buf_t* buf, ssize_t n)
{
	ASSERT(n >=0);
	if (n<=0)
	{
	}
}

void http_on_send_ok(struct tcp_task * ptask)
{
	int ret = 0;
	ret = do_read(ptask);
	ASSERT(ret == 0);
}

void http_on_send_error(struct tcp_task * ptask, uint32_t error)
{

}

void http_on_close(struct tcp_task * ptask, uint32_t error)
{

}

