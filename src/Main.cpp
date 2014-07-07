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

#define TIMEOUT_FOR_RELEASE							20			/*seconds*/

#define QUEUE_PREV(q)       (*(QUEUE **) &((*(q))[1]))
#define QUEUE_EMPTY(q)                                                        \
	((const QUEUE *) (q) == (const QUEUE *) QUEUE_NEXT(q))

#ifdef WIN32
HANDLE g_Heap = GetProcessHeap();
#ifdef DEBUG
LONGLONG g_MallocCount = 0;
LONGLONG g_FreeCount = 0;
#endif // DEBUG
#endif /*WIN32*/


struct _main_config main_config={
	-1,
	50000,
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

static void timer_release_cb(uv_timer_t *handle)
{
	if (main_info.to_delete_task_list.size()>0)
	{
		uv_mutex_lock(&main_info.to_delete_task_list_mutex);
		map<struct tcp_task *, time_t>::iterator piter;
		int count = main_info.to_delete_task_list.size()/2;
		int i = 0;
		time_t t = time(NULL);
		for (piter = main_info.to_delete_task_list.begin(); piter != main_info.to_delete_task_list.end() && i<count; i++)
		{
			if ((uv_is_closing((uv_handle_t*)&piter->first->conn))
				&& piter->first->conn.reqs_pending == 0
				&& piter->first->conn.activecnt == 0
				//&& t - piter->second>TIMEOUT_FOR_RELEASE
				)
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
}

static void timer_cb(uv_timer_t *handle)
{
	if (main_info.task_list.size() < main_config.task_min_running)
	{
		printf("timer>>taskcount=%d, will add=%d\n", main_info.task_list.size(), main_config.task_add_once);

		int r = 0;
		//add tasks
		for (unsigned int i = 0; i < main_config.task_add_once; i++)
		{
			struct default_task_node * ptask = (struct default_task_node *)malloc(sizeof(struct default_task_node));
			ASSERT(ptask != NULL);
			memset(ptask, 0, sizeof(struct default_task_node));
			r = uv_ip4_addr("1.1.1.1", 80, &ptask->tcp.addr);
			ASSERT(r == 0);
			r = tcp_task_post(&ptask->tcp);
			ASSERT(r == 0);
			main_info.task_list.insert(&ptask->tcp);
		}
	}
}

void signal_unexpected_cb(uv_signal_t* handle, int signum)
{ 
	if (SIGINT == signum || 
		SIGBREAK == signum)
	{
		uv_stop(main_info.loop);

		return;
	}
}

void init(int argc, char** argv)
{
	main_info.loop = uv_default_loop();

	int r = 0;
	r = uv_signal_init(main_info.loop, &main_info.signal_int);
	ASSERT(r == 0);
	r = uv_signal_start(&main_info.signal_int, signal_unexpected_cb, SIGINT);			//Ctrl+C
	ASSERT(r == 0);
	r = uv_signal_init(main_info.loop, &main_info.signal_break);
	ASSERT(r == 0);
	r = uv_signal_start(&main_info.signal_break, signal_unexpected_cb, SIGBREAK);		//Ctrl+Break
	ASSERT(r == 0);

	argv = uv_setup_args(argc, argv);

	r = uv_timer_init(main_info.loop, &main_info.timer);
	ASSERT(r == 0);
	r = uv_timer_start(&main_info.timer, timer_cb, 0, 100);
	ASSERT(r == 0);

	r = uv_timer_init(main_info.loop, &main_info.timer_release);
	ASSERT(r == 0);
	r = uv_timer_start(&main_info.timer_release, timer_release_cb, 1000, 1000);
	ASSERT(r == 0);
}

void uninit()
{
	int r = 0;

	close_loop(main_info.loop);
	r = uv_loop_close(main_info.loop);
	ASSERT(r == 0);
	uv_loop_delete(main_info.loop);

	r = uv_signal_stop(&main_info.signal_int);
	ASSERT(r == 0);
	r = uv_signal_stop(&main_info.signal_break);
	ASSERT(r == 0);

	set<struct tcp_task *>::iterator piter;
	for (piter = main_info.task_list.begin(); piter != main_info.task_list.end(); piter++)
	{
		free(*piter);
	}

	uv_mutex_lock(&main_info.to_delete_task_list_mutex);
	main_info.to_delete_task_list.clear();
	main_info.task_list.clear();
	uv_mutex_unlock(&main_info.to_delete_task_list_mutex);
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
	init(argc, argv);

	int r = 0;

	r = uv_run(main_info.loop, UV_RUN_DEFAULT);

	printf("\n\nFinished!");
	printf("\nPress any key to continue ......");
	getchar();

	uninit();
	platform_exit();

	return 0;
}

#ifdef WIN32

/* Do platform-specific initialization. */
void platform_init(int argc, char **argv) 
{
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

	g_Heap = HeapCreate(HEAP_GENERATE_EXCEPTIONS | HEAP_ZERO_MEMORY, 0x100000, 0x40000000);
	ASSERT(g_Heap != NULL);
}

void platform_exit()
{
	HeapDestroy(g_Heap);
	g_Heap = GetProcessHeap();

#ifdef DEBUG
	printf("alloc=%lld, free=%lld\n", g_MallocCount, g_FreeCount);
#endif // DEBUG
}
#else /*WIN32*/

/* Do platform-specific initialization. */
void platform_init(int argc, char **argv) 
{
	/* Disable stdio output buffering. */
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	signal(SIGPIPE, SIG_IGN);
}

void platform_exit()
{
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
