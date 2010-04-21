#include "SexyThread.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using namespace Sexy;

Thread::Thread(void)
{
	mValid = false;
	memset (&mThread, 0, sizeof (mThread));
#ifdef WIN32
	mThreadId = 0;
#endif
}

Thread::Thread(const Thread & other)
{
	if (this == &other)
		return;

#ifdef WIN32
	if (other.mValid)
	{
		HANDLE handle = other.mThread;
		HANDLE process = GetCurrentProcess ();

		DuplicateHandle (process, handle, process,
				 &mThread, 0, FALSE,
				 DUPLICATE_SAME_ACCESS);
		mThreadId = other.mThreadId;
		mValid = true;
	}
	else
	{
		mValid = false;
		memset (&mThread, 0, sizeof (mThread));
		mThreadId = 0;
	}
#else
	mValid = other.mValid;
	mThread = other.mThread;
#endif
}

Thread::~Thread()
{
#ifdef WIN32
	if (mValid)
		CloseHandle(mThread);
#endif
}

Thread& Thread::operator = (const Thread &other)
{
	if (this == &other)
		return *this;

#ifdef WIN32
	if (other.mValid)
	{
		HANDLE handle = other.mThread;
		HANDLE process = GetCurrentProcess ();

		DuplicateHandle (process, handle, process,
				 &mThread, 0, FALSE,
				 DUPLICATE_SAME_ACCESS);
		mThreadId = other.mThreadId;
		mValid = true;
	}
	else
	{
		mValid = false;
		memset (&mThread, 0, sizeof (mThread));
		mThreadId = 0;
	}
#else
	mValid = other.mValid;
	mThread = other.mThread;
#endif

	return *this;
}

bool Thread::operator == (const Thread & other)
{
	if (!mValid || !other.mValid)
		return false;

#ifdef WIN32
	return mThreadId == other.mThreadId;
#else
	return pthread_equal (mThread, other.mThread);
#endif
}

bool Thread::operator != (const Thread & other)
{
	if (!mValid || !other.mValid)
		return false;

#ifdef WIN32
	return mThreadId != other.mThreadId;
#else
	return !pthread_equal (mThread, other.mThread);
#endif
}

struct ThreadArg {
	void (*start_routine) (void *);
	void * arg;
};

#ifdef WIN32
static unsigned __stdcall
#else
static void *
#endif
thread_func (void * arg)
{
	ThreadArg * theThreadArg = (ThreadArg *)arg;

	theThreadArg->start_routine (theThreadArg->arg);
	delete theThreadArg;

	return 0;
}


Thread Thread::Create (void (*start_routine) (void *),
		       void * arg)
{
	ThreadArg * theThreadArg = new ThreadArg;
	Thread thread;

	theThreadArg->start_routine = start_routine;
	theThreadArg->arg = arg;

#ifdef WIN32
	unsigned thrdaddr;

	thread.mThread = (HANDLE)
		_beginthreadex (NULL, 0, thread_func,
				theThreadArg,
				0, &thrdaddr);
	if (!thread.mThread)
	{
		delete theThreadArg;
		return thread;
	}
	thread.mThreadId = thrdaddr;
#else
	pthread_attr_t attr;
	int ret;

	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE);
	ret = pthread_create (&thread.mThread, &attr, thread_func, theThreadArg);
	pthread_attr_destroy (&attr);

	if (ret)
	{
		delete theThreadArg;
		return thread;
	}
#endif

	thread.mValid = true;
	return thread;
}

Thread Thread::Self (void)
{
	Thread thread;

#ifdef WIN32
	HANDLE handle = GetCurrentThread ();
	HANDLE process = GetCurrentProcess ();
	DuplicateHandle (process, handle, process,
			 &thread.mThread, 0, FALSE,
			 DUPLICATE_SAME_ACCESS);
	thread.mThreadId = GetCurrentThreadId ();
#else
	thread.mThread = pthread_self ();
#endif

	thread.mValid = true;
	return thread;
}

void Thread::Join (void)
{
	if (!mValid)
		return;

#ifdef WIN32
	WaitForSingleObject (mThread, INFINITE);
	CloseHandle (mThread);
#else
	pthread_join (mThread, NULL);
#endif

	mValid = false;
}

bool Thread::IsValid (void)
{
	return mValid;
}
