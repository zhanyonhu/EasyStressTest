#include "http_task.h"
#include "StressTest.h"

#define TEST_HTTP_HOST			"www.1631111111111111111111111111.com"

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

	ret = do_read(ptask);
	ASSERT(ret == 0);

	struct http_data * phttp_data = (struct http_data *)ptask->reversed;
	phttp_data->http_data::http_data();
}

void http_on_recv(struct tcp_task * ptask, const uv_buf_t* buf, ssize_t n)
{
	struct http_data * phttp_data = (struct http_data *)ptask->reversed;
	phttp_data->strHtml.append(buf->base, (int)n);
	phttp_data->total_len += n;

	//printf("0x%p>>recv %d, total=%d\n", ptask, n, phttp_data->total_len);

	//This is an error, but we just do it, because it is only a test example.
	stl_size_t offset = (stl_size_t)(phttp_data->total_len - 40);
	if ((int)offset<0)
	{
		offset = 0;
	}

	stl_size_t pos = phttp_data->strHtml.find("</html>", offset);
	if (pos == STL::string::npos)
	{
		pos = phttp_data->strHtml.find("</HTML>", offset);
	}

	if (pos != STL::string::npos)
	{
		pos = phttp_data->strHtml.find("\r\n\r\n", pos);
		if (pos != STL::string::npos)
		{
			do_close(ptask, true);
			return;
		}
	}
}

void http_on_send_ok(struct tcp_task * ptask)
{
}

void http_on_send_error(struct tcp_task * ptask, uint32_t error)
{

}

void http_on_close(struct tcp_task * ptask)
{
	struct http_data * phttp_data = (struct http_data *)ptask->reversed;
	printf("0x%p>>task closed %I64d, total=%d\n", ptask, ptask->id, phttp_data->total_len);
}

