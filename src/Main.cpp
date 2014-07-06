/* Copyright zhanyonhu@163.com, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "StressTest.h"
#include "default_task.h"
#include <time.h>

struct _main_config main_config={
	-1,
	10000,
	100
};

struct _main_info main_info;

void _main_info::AddTask_ToBeDeleted(struct tcp_task * ptask)
{
	uv_mutex_lock(&to_delete_task_list_mutex);
	time_t t = time(NULL);
	to_delete_task_list.insert(make_pair(ptask, t));
	uv_mutex_unlock(&to_delete_task_list_mutex);
}

static void timer_cb(uv_timer_t *handle)
{
	if (main_info.to_delete_task_list.size()>0)
	{
		uv_mutex_lock(&main_info.to_delete_task_list_mutex);
		map<struct tcp_task *, time_t>::iterator piter;
		int count = main_info.to_delete_task_list.size()/2;
		int i = 0;
		for (piter = main_info.to_delete_task_list.begin(); piter != main_info.to_delete_task_list.end() && i<count; i++)
		{
#define UV__HANDLE_CLOSING					0x01
			if (piter->first->conn.flags & UV__HANDLE_CLOSING &&
				piter->first->conn.reqs_pending == 0 &&
				time(NULL)-piter->second>3000)
			{
				free(piter->first);
				main_info.task_list.erase(piter->first);
				piter = main_info.to_delete_task_list.erase(piter);
			}
			else
			{
				piter++;
				break;
			}
		}
		uv_mutex_unlock(&main_info.to_delete_task_list_mutex);
	}

	if (main_info.task_list.size()<main_config.task_min_running)
	{
		printf("timer>>taskcount=%d, will add=%d\n", main_info.task_list.size(), main_config.task_add_once);

		int r = 0;
		//add tasks
		for (unsigned int i = 0; i < main_config.task_add_once; i++)
		{
			struct default_task_node * ptask = (struct default_task_node *)malloc(sizeof(struct default_task_node));
			ASSERT(ptask != NULL);
			memset(ptask, 0, sizeof(struct default_task_node));
			r = uv_ip4_addr("123.125.114.144", 80, &ptask->tcp.addr);
			ASSERT(r == 0);
			r = tcp_task_post(&ptask->tcp);
			ASSERT(r == 0);
			main_info.task_list.insert(&ptask->tcp);
		}
	}
}

int main(int argc, char **argv)
{
	const char * pargv = NULL;
	for (int arg_i = 0; arg_i < argc; arg_i++)
	{
		if ((pargv = strstr(argv[arg_i], "-taskcount=")) != NULL)
		{
			main_config.task_count = atol(pargv + strlen("-taskcount="));
		}
		else if ((pargv = strstr(argv[arg_i], "-c=")) != NULL)
		{
			main_config.task_count = atol(pargv + strlen("-c="));
		}
		else if ((pargv = strstr(argv[arg_i], "-task_min_running=")) != NULL)
		{
			main_config.task_min_running = atol(pargv + strlen("-task_min_running="));
		}
		else if ((pargv = strstr(argv[arg_i], "-mc=")) != NULL)
		{
			main_config.task_min_running = atol(pargv + strlen("-mc="));
		}
		else if ((pargv = strstr(argv[arg_i], "-task_add_once=")) != NULL)
		{
			main_config.task_add_once = atol(pargv + strlen("-task_add_once="));
		}
		else if ((pargv = strstr(argv[arg_i], "-ac=")) != NULL)
		{
			main_config.task_add_once = atol(pargv + strlen("-ac="));
		}


		else if (stricmp(argv[arg_i], "-help") == 0 ||
			stricmp(argv[arg_i], "/help") == 0 ||
			stricmp(argv[arg_i], "help") == 0)
		{
			show_help();
			return 1;
		}
	}

	if (main_config.task_add_once<=0)
	{
		main_config.task_add_once = 1;
	}

	if (main_config.task_min_running <= 0)
	{
		main_config.task_add_once = 1;
	}

	printf("stress test tool is running!\n");
	printf("taskcount=%d\n", main_config.task_count);
	printf("task_min_running=%d\n", main_config.task_min_running);
	printf("task_add_once=%d\n", main_config.task_add_once);

	platform_init(argc, argv);

	argv = uv_setup_args(argc, argv);

	int r = 0;
	uv_timer_t timer;
	r = uv_timer_init(uv_default_loop(), &timer);
	ASSERT(r == 0);
	r = uv_timer_start(&timer, timer_cb, 0, 10);
	ASSERT(r == 0);

	r = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
	MAKE_VALGRIND_HAPPY();

	printf("\n\nFinished!");
	printf("\nPress any key to continue ......");
	getchar();

	return 0;
}

#ifdef WIN32

/* Do platform-specific initialization. */
void platform_init(int argc, char **argv) {
	/* Disable the "application crashed" popup. */
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX |
		SEM_NOOPENFILEERRORBOX);
#if !defined(__MINGW32__) && defined(_DEBUG)
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_DEBUG);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
#endif

	_setmode(0, _O_BINARY);
	_setmode(1, _O_BINARY);
	_setmode(2, _O_BINARY);

	/* Disable stdio output buffering. */
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
}

#else /*WIN32*/

/* Do platform-specific initialization. */
void platform_init(int argc, char **argv) {
	/* Disable stdio output buffering. */
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	signal(SIGPIPE, SIG_IGN);
}

#endif /*WIN32*/

void show_help()
{
	printf("=====================================================\n");
	printf("Example: -taskcount=10000 \n\n");
	printf("-taskcount=***\t\t-c: the task execution times\n");
	printf("-task_min_running=***\t\t-mc: minimize running at the same time\n");
	printf("-task_add_once=***\t\t-ac: When the running instance count less than a certain value <task_min_running> , automatically increase the number of running instance\n");
	printf("\n\nPress any key to continue ......");
	getchar();
}
