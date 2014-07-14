#ifndef _THREADPOLL_H_
#define _THREADPOLL_H_

#include "commondef.h"

#include <vector>
#include <list>
#include <thread>

#include "allocator.h"

typedef void(*PWorker)(void* req);

class CThreadPool
{
public:
	CThreadPool(int threadnum=1);
	~CThreadPool();

protected:
	struct THREAD_NODE
	{
		std::thread thread;
		bool is_exit;
	};

	struct WORKER_NODE
	{
		PWorker worker;
		void * context;
	};

public:
	int AddThread(int threadnum = 1);
	int SetThreadNumber(int threadnum = 1);
	int DelThread();
	int DelThread(int threadnum);
	int ReleaseThreads();

	int AddTask(PWorker worker, void * context);

	int WaitAll();

protected:
	int ReleaseTasks();

protected:
	void ThreadProc(THREAD_NODE * pnode);

protected:

	std::vector<THREAD_NODE *> m_threads;
	uv_cond_t m_cond;
	uv_mutex_t m_mutex;
	boost::object_pool<WORKER_NODE> m_task_pool;
	std::stl_allocator<std::_List_node<WORKER_NODE, void *>, WORKER_NODE> m_task_allocator;
	std::list<WORKER_NODE, std::stl_allocator<std::_List_node<WORKER_NODE, void *>, WORKER_NODE>> m_tasks;
};

#endif	/*_THREADPOLL_H_*/