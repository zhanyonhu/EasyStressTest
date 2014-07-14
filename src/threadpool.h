#ifndef _THREADPOLL_H_
#define _THREADPOLL_H_

#include "commondef.h"

class CThreadPool
{
public:
	CThreadPool(int threadnum=1);
	~CThreadPool();

public:
	int AddThread();
	int DelThread();
	int ReleaseThreads();

	int AddTask();

protected:
	int ReleaseTasks();

	
};

#endif	/*_THREADPOLL_H_*/