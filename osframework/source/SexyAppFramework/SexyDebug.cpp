#include "SexyDebug.h"

#ifdef WIN32
#include <process.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>
#include <cstdio>
#include <cstdlib>

#if defined(__linux__) && defined(__ELF__) && !defined(__UCLIBC__)
#define HAVE_BACKTRACE
#include <execinfo.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>
#endif
#define HAVE_SETRLIMIT
#endif

using namespace Sexy;

#ifndef WIN32
struct SigHandled {
     int              signum;
     struct sigaction old_action;
};

static int sigs_to_handle[] = {
	/*SIGALRM,*/
	SIGHUP,
	SIGTERM,
	/*SIGUSR1, */
	/*SIGUSR2,*/
	/*SIGVTALRM,*/
	/*SIGSTKFLT,*/
	SIGABRT,
	SIGFPE,
	SIGILL,
	SIGQUIT,
	SIGSEGV,
	SIGTRAP,
	/*SIGSYS, */
	/*SIGEMT, */
	SIGBUS,
	SIGXCPU,
	SIGXFSZ
};

#define NUM_SIGS_TO_HANDLE ((int)(sizeof(sigs_to_handle) / sizeof(sigs_to_handle[0])))

static SigHandled sigs_handled[NUM_SIGS_TO_HANDLE];

static void RemoveHandlers()
{
     int i;

     for (i = 0; i < NUM_SIGS_TO_HANDLE; i++)
     {
	     if (sigs_handled[i].signum != -1)
	     {
		     int signum = sigs_handled[i].signum;

		     sigaction(signum, &sigs_handled[i].old_action, NULL);
		     sigs_handled[i].signum = -1;
	     }
     }
}

static bool
SigInfoShowSegv(const siginfo_t *info)
{
     switch (info->si_code)
     {
#ifdef SEGV_MAPERR
          case SEGV_MAPERR:
               printf(" (at %p, invalid address)\n", info->si_addr );
               return true;
#endif
#ifdef SEGV_ACCERR
          case SEGV_ACCERR:
               printf(" (at %p, invalid permissions)\n", info->si_addr );
               return true;
#endif
     }
     return false;
}

static bool
SigInfoShowBus(const siginfo_t *info)
{
     switch (info->si_code)
     {
#ifdef BUG_ADRALN
          case BUS_ADRALN:
               printf(" (at %p, invalid address alignment)\n", info->si_addr );
               return true;
#endif
#ifdef BUS_ADRERR
          case BUS_ADRERR:
               printf(" (at %p, non-existent physical address)\n", info->si_addr );
               return true;
#endif
#ifdef BUS_OBJERR
          case BUS_OBJERR:
               printf(" (at %p, object specific hardware error)\n", info->si_addr );
               return true;
#endif
     }

     return false;
}

static bool
SigInfoShowIll(const siginfo_t *info)
{
	switch (info->si_code)
	{
#ifdef ILL_ILLOPC
	case ILL_ILLOPC:
		printf(" (at %p, illegal opcode)\n", info->si_addr);
		return true;
#endif
#ifdef ILL_ILLOPN
	case ILL_ILLOPN:
		printf(" (at %p, illegal operand)\n", info->si_addr);
		return true;
#endif
#ifdef ILL_ILLADR
	case ILL_ILLADR:
		printf(" (at %p, illegal addressing mode)\n", info->si_addr);
		return true;
#endif
#ifdef ILL_ILLTRP
	case ILL_ILLTRP:
		printf(" (at %p, illegal trap)\n", info->si_addr);
		return true;
#endif
#ifdef ILL_PRVOPC
	case ILL_PRVOPC:
		printf(" (at %p, privileged opcode)\n", info->si_addr);
		return true;
#endif
#ifdef ILL_PRVREG
	case ILL_PRVREG:
		printf(" (at %p, privileged register)\n", info->si_addr);
		return true;
#endif
#ifdef ILL_COPROC
	case ILL_COPROC:
		printf(" (at %p, coprocessor error)\n", info->si_addr);
		return true;
#endif
#ifdef ILL_BADSTK
	case ILL_BADSTK:
		printf(" (at %p, internal stack error)\n", info->si_addr);
		return true;
#endif
	}

	return false;
}

static bool
SigInfoShowFPE(const siginfo_t *info)
{
	switch (info->si_code)
	{
#ifdef FPE_INTDIV
	case FPE_INTDIV:
		printf(" (at %p, integer divide by zero)\n", info->si_addr);
		return true;
#endif
#ifdef FPE_FLTDIV
	case FPE_FLTDIV:
		printf(" (at %p, floating point divide by zero)\n", info->si_addr);
		return true;
#endif
	}

	printf(" (at %p) \n", info->si_addr);

	return true;
}

static bool
SigInfoShowAny(const siginfo_t *info)
{
	switch (info->si_code)
	{
#ifdef SI_USER
	case SI_USER:
		printf(" (sent by pid %d, uid %d) \n", info->si_pid, info->si_uid);
		return true;
#endif
#ifdef SI_KERNEL
	case SI_KERNEL:
		printf(" (sent by the kernel) \n");
		return true;
#endif
	}
	return false;
}

static void SignalHandler(int signum, siginfo_t * siginfo, void * data)
{
	fflush(stdout);
	fflush(stderr);

	RemoveHandlers();

	printf("Critical error: %d --> Caught signal %d", getpid(), signum);

	if (siginfo && siginfo > (siginfo_t*)0x100) {
		bool shown = false;

		if (siginfo->si_code > 0 && siginfo->si_code < 0x80)
		{
			switch (signum)
			{
			case SIGSEGV:
				shown = SigInfoShowSegv(siginfo);
				break;
			case SIGBUS:
				shown = SigInfoShowBus(siginfo);
				break;

			case SIGILL:
				shown = SigInfoShowIll(siginfo);
				break;

			case SIGFPE:
				shown = SigInfoShowFPE(siginfo);
				break;

			default:
				printf(" \n");
				shown = true;
				break;
			}
		}
		else
			shown = SigInfoShowAny(siginfo);

		if (!shown)
			printf(" (unknown origin)\n");
	}
	else
	{
		printf(", no siginfo available\n");
	}

	DebugPrintBackTrace();

	fflush(stdout);
	fflush(stderr);

	raise(signum);
	abort();
	exit(-signum);
}

static bool InstallSignalHandler(int signum, struct sigaction *old_action)
{
	struct sigaction action;

	action.sa_sigaction = SignalHandler;
	action.sa_flags     = SA_SIGINFO;

	if (signum != SIGSEGV)
		action.sa_flags |= SA_NODEFER;

	sigemptyset(&action.sa_mask);
	return sigaction(signum, &action, old_action) == 0;
}

static void InstallSignalHanlders()
{
	if (getenv("GAME_DONOT_HANDLE_SIGS"))
	    return;

	for (int i = 0; i < NUM_SIGS_TO_HANDLE; i++)
	{
		sigs_handled[i].signum = -1;
		if (InstallSignalHandler(sigs_to_handle[i],
					 &sigs_handled[i].old_action))
			sigs_handled[i].signum = sigs_to_handle[i];
	}
}

static void
EnableCoreDump(void)
{
#if defined (HAVE_SETRLIMIT)
	struct rlimit rlim;

	if (getrlimit (RLIMIT_CORE, &rlim))
		return;

	rlim.rlim_cur = rlim.rlim_max;
	setrlimit (RLIMIT_CORE, &rlim);
#endif
}

static void PosixPrintBackTrace()
{
#ifdef HAVE_BACKTRACE
	void *array[64];
	const char *mod;
	int size, i;
	Dl_info info;

	printf("\nBacktrace:\n");
	size = backtrace(array, 64);
	for (i = 0; i < size; i++)
	{
		dladdr(array[i], &info);
		mod = (info.dli_fname && *info.dli_fname) ? info.dli_fname : "(vdso)";
		if (info.dli_saddr)
			printf("%d: %s (%s+0x%lx) [%p]\n", i, mod,
			       info.dli_sname, (char*)array[i] - (char*)info.dli_saddr, array[i]);
		else
			printf("%d: %s (%p+0x%lx) [%p]\n", i, mod,
			       info.dli_fbase, (char*)array[i] - (char*)info.dli_fbase, array[i]);
	}
	fflush(stdout);
	fflush(stderr);
#endif
}
#endif

void Sexy::DebugInit(int options)
{
#ifdef WIN32
#else
	if (options & FAULT_HANDLER)
		InstallSignalHanlders();
	if (options & CORE_DUMP)
		EnableCoreDump();
#endif
}

void Sexy::DebugPrintBackTrace()
{
#ifdef WIN32
#else
	PosixPrintBackTrace();
#endif
}




#ifdef SEXY_DEBUG_TEST
/**
 * compile with:
 * gcc -DSEXY_DEBUG_TEST -o SexyDebug SexyDebug.cpp -lstdc++ -ldl -O0 -export-dynamic
 */
void crasha()
{
	int * ptr = (int *)0xbeafdead;
	*ptr = 0;
}

void crash()
{
	crasha();
}

int main (int argc, char ** argv)
{
	Sexy::DebugInit(Sexy::FAULT_HANDLER);

	crash();

	return 0;
}
#endif
