#include "threadpool.h"


CThreadPool::CThreadPool(int threadnum/* = 1*/)
{
	uv_mutex_init(&m_mutex);
	uv_cond_init(&m_cond);
	AddThread(threadnum);
}

CThreadPool::~CThreadPool()
{
	ReleaseThreads();
	uv_cond_destroy(&m_cond);
	uv_mutex_destroy(&m_mutex);
}

void CThreadPool::ThreadProc(CThreadPool::THREAD_NODE * pnode)
{
	WORKER_NODE worker;
	while (true)
	{
		while (m_tasks.size() == 0 && !pnode->is_exit)
		{
			uv_cond_wait(&m_cond, &m_mutex);
		}

		if (pnode->is_exit)
		{
			uv_cond_signal(&m_cond);
			break;
		}

		uv_mutex_lock(&m_mutex);
		worker=m_tasks.front();
		m_tasks.pop_front();
		uv_mutex_unlock(&m_mutex);

		worker.worker(worker.context);
	}
}

int CThreadPool::AddThread(int threadnum)
{
	for (int i = 0; i < threadnum; i++)
	{
		THREAD_NODE * pnode = new THREAD_NODE;
		pnode->is_exit = false;
		pnode->thread = std::thread(std::bind(&CThreadPool::ThreadProc, this, pnode));
		m_threads.push_back(pnode);
	}
	return 0;
}

int CThreadPool::SetThreadNumber(int threadnum)
{
	AddThread(threadnum - (int)m_threads.size());

	if ((int)m_threads.size()>threadnum)
	{
		DelThread((int)m_threads.size()-threadnum);
	}
	return 0;
}

int CThreadPool::DelThread()
{
	if ((int)m_threads.size()>0)
	{
		THREAD_NODE * pnode = m_threads[(int)m_threads.size() - 1];
		pnode->is_exit = true;
		uv_cond_signal(&m_cond);
		if (pnode->thread.joinable())pnode->thread.join();
		delete pnode;
		m_threads.erase(m_threads.end()-1);
	}

	return 0;
}

int CThreadPool::DelThread(int threadnum)
{
	for (int i = 0; i < threadnum; i++)
	{
		DelThread();
	}
	return 0;
}

int CThreadPool::ReleaseThreads()
{
	DelThread((int)m_threads.size());
	ReleaseTasks();
	return 0;
}

int CThreadPool::WaitAll()
{
	int i = 0;
	for (int i = 0; i < (int)m_threads.size(); i++)
	{
		THREAD_NODE * pnode = m_threads[i];
		pnode->is_exit = true;
		uv_cond_signal(&m_cond);
	}

	for (i = 0; i < (int)m_threads.size(); i++)
	{
		THREAD_NODE * pnode = m_threads[i];
		if (pnode->thread.joinable())pnode->thread.join();
	}

	return 0;
}

int CThreadPool::AddTask(PWorker worker, void * context)
{
	uv_mutex_lock(&m_mutex);

	WORKER_NODE node;
	node.context = context;
	node.worker = worker;
	m_tasks.push_back(node);

	uv_cond_signal(&m_cond);
	uv_mutex_unlock(&m_mutex);

	return 0;
}

int CThreadPool::ReleaseTasks()
{
	return 0;
}
