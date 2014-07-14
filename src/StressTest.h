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

#ifndef _STRESS_TEST_H_
#define _STRESS_TEST_H_

extern "C"
{
#include "commondef.h"
};

#include "task.h"
#include "threadpool.h"

struct _main_config
{
	bool flood_begin;				//flood begin (The initial rapid filling)
	unsigned int task_count;				//execute times
	unsigned int task_min_running;			//minimize running at the same time
	unsigned int task_add_once;				//When the running instance count less than a certain value <task_min_running> , 
											//automatically increase the number of running instance
	unsigned int thread_num;				//task thread number
};
extern struct _main_config main_config;

struct _main_info
{
	CTasks tasks;
	uv_signal_t signal_int;
	uv_signal_t signal_break;
	uv_signal_t signal_close;
	uv_timer_t timer;
	uv_timer_t timer_release;

	uv_loop_t * loop;

	CThreadPool threads;

public:
	void AddTask_ToBeDeleted(struct tcp_task * ptask);
	_main_info()
	{
	}
	~_main_info()
	{
	}
};
extern struct _main_info main_info;

//show the help menu
void show_help();

/* Do platform-specific initialization. */
void platform_init(int argc, char** argv);
void platform_exit();
void init(int argc, char** argv);
void uninit();




#endif


