#include "SexyThread.h"
#include <stdlib.h>
#include <string.h>

using namespace Sexy;

Thread::Thread(void)
{
	mValid = false;
	memset (&mThread, 0, sizeof (mThread));
}

Thread::Thread(const Thread & other)
{
	mValid = other.mValid;
	mThread = other.mThread;
}

bool Thread::operator == (const Thread & other)
{
	if (!mValid || !other.mValid)
		return false;

#ifdef WIN32
	return mThread == other.mThread;
#else
	return pthread_equal (mThread, other.mThread);
#endif
}

bool Thread::operator != (const Thread & other)
{
	if (!mValid || !other.mValid)
		return false;

#ifdef WIN32
	return mThread != other.mThread;
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
	thread.mThread = GetCurrentThread ();
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
