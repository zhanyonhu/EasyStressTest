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
#include "http_task.h"
#include <time.h>

struct _main_config main_config={
	false,
	-1,
	MAX_TASK_COUNT,
	DEAULT_TASK_ADD,
	DEAULT_THREAD_NUM,
};

struct _main_info main_info;

void _main_info::AddTask_ToBeDeleted(struct tcp_task * ptask)
{
	tasks.AddTask_ToBeDeleted(ptask);
}

static void timer_release_cb(uv_timer_t *handle)
{
	main_info.tasks.DeleteTask_ToBeDeleted();
}

static void timer_cb(uv_timer_t *handle)
{
	if (main_info.tasks.Count() < main_config.task_min_running)
	{
		printf("timer>>taskcount=%d, will add=%d\n", main_info.tasks.Count(), main_config.task_add_once);

		int r = 0;
		//add tasks
		for (unsigned int i = 0; i < main_config.task_add_once; i++)
		{
			struct tcp_task task = {0};
			r = tcp_task_post(&task);
			ASSERT(r >= 0);
		}
	}
}

void signal_unexpected_cb(uv_signal_t* handle, int signum)
{ 
	if (SIGINT == signum ||
		SIGBREAK == signum ||
		SIGHUP == signum)
	{
		uv_stop(main_info.loop);

		return;
	}
}

int init(int argc, char** argv)
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
	r = uv_signal_init(main_info.loop, &main_info.signal_close);
	ASSERT(r == 0);
	r = uv_signal_start(&main_info.signal_close, signal_unexpected_cb, SIGHUP);			//Close
	ASSERT(r == 0);

	argv = uv_setup_args(argc, argv);

	r = uv_timer_init(main_info.loop, &main_info.timer);
	ASSERT(r == 0);
	r = uv_timer_start(&main_info.timer, timer_cb, 0, TIMER_TIME_ADD);
	ASSERT(r == 0);

	r = uv_timer_init(main_info.loop, &main_info.timer_release);
	ASSERT(r == 0);
	r = uv_timer_start(&main_info.timer_release, timer_release_cb, TIMER_TIME_RELASE, TIMER_TIME_RELASE);
	ASSERT(r == 0);

//	main_info.threads.SetThreadNumber(main_config.thread_num);

	if (main_config.script_file.empty() ||
		main_config.libpath.empty())
	{
		main_info.tcp_task_callback.on_init = http_on_init;
		main_info.tcp_task_callback.on_connected_failed = http_on_connected_failed;
		main_info.tcp_task_callback.on_connected_successful = http_on_connected_successful;
		main_info.tcp_task_callback.on_recv = http_on_recv;
		main_info.tcp_task_callback.on_send_ok = http_on_send_ok;
		main_info.tcp_task_callback.on_send_error = http_on_send_error;
		main_info.tcp_task_callback.on_close = http_on_close;
	}
	else
	{
		TCCState * tcc = tcc_new();
		main_info.tcc.tcc = tcc;
		ASSERT(tcc != NULL);
		tcc_set_output_type(tcc, TCC_OUTPUT_MEMORY);
		if (tcc_add_file(tcc, main_config.script_file.c_str()) == -1)
		{
			printf("\nCannot add script file = %s", main_config.script_file.c_str());
			return -1;
		}

		tcc_add_include_path(tcc, "E:\\ProjectTemp\\EasyStressTest\\third\\tcc");
		tcc_set_lib_path(tcc, "E:\\ProjectTemp\\EasyStressTest\\third\\tcc");

		if (tcc_relocate(tcc, TCC_RELOCATE_AUTO) < 0)
		{
			printf("\nCannot relocate script file = %s", main_config.script_file.c_str());
			return -1;
		}
	}

	return 0;
}

void uninit()
{
	int r = 0;

//	main_info.threads.WaitAll();

	close_loop(main_info.loop);
	r = uv_loop_close(main_info.loop);
	ASSERT(r == 0);
	uv_loop_delete(main_info.loop);

	r = uv_signal_stop(&main_info.signal_int);
	ASSERT(r == 0);
	r = uv_signal_stop(&main_info.signal_break);
	ASSERT(r == 0);

	main_info.tasks.Clear();
	tcc_delete(main_info.tcc.tcc);
	main_info.tcc.tcc = NULL;
}

int main(int argc, char **argv)
{
	const char * pargv = NULL;
	int r = 0;

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
		else if ((pargv = strstr(argv[arg_i], "-thread=")) != NULL)
		{
			main_config.thread_num = atol(pargv + strlen("-thread="));
		}
		else if ((pargv = strstr(argv[arg_i], "-tn=")) != NULL)
		{
			main_config.thread_num = atol(pargv + strlen("-tn="));
		}
		else if (0 == stricmp(argv[arg_i], "-flood"))
		{
			main_config.flood_begin = true;
		}
		else if (0 == stricmp(argv[arg_i], "-f"))
		{
			main_config.flood_begin = true;
		}
		else if ((pargv = strstr(argv[arg_i], "-tcc=")) != NULL)
		{
			main_config.script_file = pargv + strlen("-tcc=");
		}
		else if ((pargv = strstr(argv[arg_i], "-libpath=")) != NULL)
		{
			main_config.libpath = pargv + strlen("-libpath=");
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
		main_config.task_min_running = 1;
	}

	if (main_config.thread_num <= 0)
	{
		main_config.thread_num = 3;
	}

	printf("stress test tool is running!\n");
	printf("taskcount=%d\n", main_config.task_count);
	printf("task_min_running=%d\n", main_config.task_min_running);
	printf("task_add_once=%d\n", main_config.task_add_once);

	platform_init(argc, argv);
	if (init(argc, argv) < 0)
	{
		goto end;
	}

	if (main_config.flood_begin)
	{
		if (main_info.tasks.Count() < main_config.task_min_running)
		{
			timer_cb(NULL);
		}
	}

	r = uv_run(main_info.loop, UV_RUN_DEFAULT);

	printf("\n\nFinished!");
	printf("\nPress any key to continue ......");

end:
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
}

void platform_exit()
{
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
	printf("-thread=***\t\t-tn: task thread number\n");
	printf("-tcc==***\t\t: tcc script file path\n");
	printf("-libpath==***\t\t: tcc's libtcc1.a file path\n");
	printf("\n\nPress any key to continue ......");
	getchar();
}
