

#include"adapi_def.h"


#ifndef _ADAPI_NO_THREAD_FUNC_




INT_THREADID MYAPI Thread2GetID()
{
#ifdef WIN32
	return (INT_THREADID)GetCurrentThreadId();
#else
	return (INT_THREADID)pthread_self();
#endif
}
unsigned int MYAPI Thread2GetIdNumber()
{
#ifdef WIN32
	return (unsigned int)GetCurrentThreadId();
#elif defined(_Darwin_) || defined(__DARWIN_UNIX03)
    
    return (unsigned int)syscall(SYS_thread_selfid);
#elif defined(_android_) 
    
    return (unsigned int)syscall(__NR_gettid);
#else
	return (unsigned int)syscall(SYS_gettid);
#endif
}

void * MYAPI Thread2Create(void *RoutineProc, void *vParameter,   INT_THREADID *vThreadId,unsigned int nStackSize ,BOOL bJoin)
{

#ifdef _WIN32_WCE
	HANDLE hThread;
	DWORD tid = 0 ;
	hThread=CreateThread(0,nStackSize,(LPTHREAD_START_ROUTINE)RoutineProc,vParameter,
		0,&tid);
	if( vThreadId != NULL )
	{
		*vThreadId = tid ;
	}
    return (void *)hThread;
#elif  defined(WIN32)
    HANDLE hThread;
    unsigned int tid = 0 ;
    hThread = (HANDLE)_beginthreadex(NULL,nStackSize,(_thread_proc)RoutineProc,vParameter,0,&tid);

    if( vThreadId )
    {
        *vThreadId = tid ;
    }
    return (void *)hThread;
#else
     pthread_t threadId=0;
     void * hThread=(void * )-1;
     pthread_attr_t  attr;
     int inher;

     /*
    if(nStackSize)
     {
     */
        if(pthread_attr_init(&attr))
        {
          return (void *)-1;
        }
		#ifdef PTHREAD_INHERIT_SCHED
        if( pthread_attr_getinheritsched(&attr, &inher) == 0 )
        {
            if (inher == PTHREAD_INHERIT_SCHED)
            {
                inher = PTHREAD_EXPLICIT_SCHED;
                pthread_attr_setinheritsched(&attr, inher);
            }

        }
		#endif

		if( bJoin )
		{
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
		}
		else
		{
			pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
		}

        

        if(nStackSize)
        {
          pthread_attr_setstacksize(&attr,nStackSize);
        }


        hThread = (void *)pthread_create((pthread_t*)&threadId,&attr,(void*(*)(void*))RoutineProc,(void *)vParameter);
        



        pthread_attr_destroy(&attr);
     /*
     }
     else
     {
         hThread = (void *)pthread_create((pthread_t*)&threadId,0,(void *)RoutineProc,vParameter);
         if( !hThread ) pthread_detach(threadId);
     }
     */



     if(vThreadId)
     {
        *(vThreadId) = threadId;
     }
     if(hThread)
     {
        return 0;
     }


     return (void *)threadId;
#endif

}
void * MYAPI Thread2CreateEx(void *RoutineProc, void *vParameter,   void *vThreadId,unsigned int nStackSize,
    int nSchedType,int nPriority)
{

#ifdef _WIN32_WCE
	HANDLE hThread;
	hThread=CreateThread(0,nStackSize,(LPTHREAD_START_ROUTINE)RoutineProc,vParameter,
		0,(DWORD *)vThreadId);
	if(hThread)
	{
		CloseHandle(hThread);
	}
    return (void *)hThread;
#elif  defined(WIN32)
    HANDLE hThread;
    unsigned int tid = 0 ;
    hThread = (HANDLE)_beginthreadex(NULL,nStackSize,(_thread_proc)RoutineProc,vParameter,0,&tid);
    if(hThread)
    {
       // CloseHandle(hThread);
    }
    if( vThreadId )
    {
        *(unsigned int*)vThreadId = tid ;
    }
    return (void *)hThread;
#else
     pthread_t threadId=0;
     void * hThread=(void * )-1;
     pthread_attr_t  attr;
     int inher;
     int policy ;
     struct sched_param param;

     /*
    if(nStackSize)
     {
     */
        if(pthread_attr_init(&attr))
        {
          return (void *)-1;
        }
		#ifdef PTHREAD_INHERIT_SCHED
        if( pthread_attr_getinheritsched(&attr, &inher) == 0 )
        {
            if (inher == PTHREAD_INHERIT_SCHED)
            {
                inher = PTHREAD_EXPLICIT_SCHED;
                pthread_attr_setinheritsched(&attr, inher);
            }
            policy = nSchedType;
            if( pthread_attr_setschedpolicy(&attr, policy) == 0 )
            {
                param.sched_priority = nPriority;
                pthread_attr_setschedparam(&attr, &param);
            }
        }
		#endif

        pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

        if(nStackSize)
        {
          pthread_attr_setstacksize(&attr,nStackSize);
        }


        hThread = (void *)pthread_create((pthread_t*)&threadId,&attr,(void*(*)(void*))RoutineProc,(void *)vParameter);
        pthread_attr_destroy(&attr);

     /*
     }
     else
     {
         hThread = (void *)pthread_create((pthread_t*)&threadId,0,(void *)RoutineProc,vParameter);
         if( !hThread ) pthread_detach(threadId);
     }
     */



     if(vThreadId)
     {
        *((pthread_t*)vThreadId) = threadId;
     }
     if(hThread)
     {
        return 0;
     }


     return (void *)threadId;
#endif

}

void MYAPI Thread2WaitForExit( void *handle )
{
#ifdef WIN32
	if( handle == NULL )
	{
		return ;
	}
	WaitForSingleObject(handle,INFINITE);
#else
	void *value = NULL ;
	if( handle == (void *)-1 || handle == (void *) 0 )
	{
		return ;
	}
	pthread_join((pthread_t)handle,&value);
#endif

}
void MYAPI Thread2Close(void * handle)
{
#ifdef WIN32
	CloseHandle(handle);
#endif
}
BOOL MYAPI Thread2IsValid(void * handle)
{
    if( handle == NULL )
	{
		return FALSE ;
	}
	else
	{
		return TRUE ;
	}
}
void MYAPI Thread2Exit(void * nExitCode)
{
#ifdef _WIN32_WCE
    ExitThread((unsigned int)nExitCode);
    return ;
#elif defined(WIN32)
    _endthreadex((unsigned int)nExitCode);
    return ;
#else
    pthread_exit(nExitCode);
    return ;
#endif
}


#endif
































































