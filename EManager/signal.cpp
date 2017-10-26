

#include "signal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "logger.h"

#define TRACE_SIZE 16

bool g_reload = false;
bool g_terminate = false;

RETSIGTYPE hup_handler(int signal)
{
	Info("Got signal %d (%s), reloading", signal, strsignal(signal));
	g_reload = true;
}

RETSIGTYPE term_handler(int signal)
{
	Info("Got signal %d (%s), exiting", signal, strsignal(signal));
	g_terminate = true;
}

#if ( HAVE_SIGINFO_T && HAVE_UCONTEXT_T )
RETSIGTYPE die_handler(int signal, siginfo_t * info, void *context)
#else
RETSIGTYPE die_handler(int signal)
#endif
{
#if (defined(__i386__) || defined(__x86_64__))
	void *cr2 = 0;
	void *ip = 0;
#endif
	Error("Got signal %d (%s), crashing", signal, strsignal(signal));

#if (defined(__i386__) || defined(__x86_64__))
	// Get more information if available
#if ( HAVE_SIGINFO_T && HAVE_UCONTEXT_T )
	if (info && context) {

		Debug(1,
		      "Signal information: number %d code %d errno %d pid %d uid %d status %d",
		      signal, info->si_code, info->si_errno, info->si_pid,
		      info->si_uid, info->si_status);

		ucontext_t *uc = (ucontext_t *) context;
#if defined(__x86_64__)
		cr2 = info->si_addr;
		ip = (void *)(uc->uc_mcontext.gregs[REG_RIP]);
#else
		cr2 = info->si_addr;
		ip = (void *)(uc->uc_mcontext.gregs[REG_EIP]);
#endif				// defined(__x86_64__)

		// Print the signal address and instruction pointer if available
		if (ip) {
			Error("Signal address is %p, from %p", cr2, ip);
		} else {
			Error("Signal address is %p, no instruction pointer", cr2);
		}
	}
#endif				// ( HAVE_SIGINFO_T && HAVE_UCONTEXT_T )


	// Print backtrace if enabled and available
#if ( !defined(NO_CRASHTRACE) && HAVE_DECL_BACKTRACE && HAVE_DECL_BACKTRACE_SYMBOLS )
	void *trace[TRACE_SIZE];
	int trace_size = 0;
	trace_size = backtrace(trace, TRACE_SIZE);

	char cmd[1024] = "addr2line -e ";
	char *cmd_ptr = cmd + strlen(cmd);
	//cmd_ptr += snprintf(cmd_ptr, sizeof(cmd) - (cmd_ptr - cmd), "%s", self);

	char **messages = backtrace_symbols(trace, trace_size);
	// Print the full backtrace
	for (int i = 0; i < trace_size; i++) {
		Error("Backtrace %u: %s", i, messages[i]);
		cmd_ptr +=
		    snprintf(cmd_ptr, sizeof(cmd) - (cmd_ptr - cmd), " %p",
			     trace[i]);
	}
	free(messages);

	Info("Backtrace complete, please execute the following command for more information");
	Info(cmd);
#endif				// ( !defined(NO_CRASHTRACE) && HAVE_DECL_BACKTRACE && HAVE_DECL_BACKTRACE_SYMBOLS )
#endif                          // (defined(__i386__) || defined(__x86_64__)
	exit(signal);
}

void SetHupHandler(SigHandler * handler)
{
	sigset_t block_set;
	sigemptyset(&block_set);
	struct sigaction action, old_action;

	action.sa_handler = (SigHandler *) handler;
	action.sa_mask = block_set;
	action.sa_flags = SA_RESTART;
	sigaction(SIGHUP, &action, &old_action);
	signal(SIGPIPE, SIG_IGN);
}

void SetTermHandler(SigHandler * handler)
{
	sigset_t block_set;
	sigemptyset(&block_set);
	struct sigaction action, old_action;

	action.sa_handler = (SigHandler *) handler;
	action.sa_mask = block_set;
	action.sa_flags = SA_RESTART;
	sigaction(SIGTERM, &action, &old_action);
	sigaction(SIGINT, &action, &old_action);
	sigaction(SIGQUIT, &action, &old_action);
}

void SetDieHandler(SigHandler * handler)
{
	sigset_t block_set;
	sigemptyset(&block_set);
	struct sigaction action, old_action;

	action.sa_mask = block_set;
#if ( HAVE_SIGINFO_T && HAVE_UCONTEXT_T )
	action.sa_sigaction = (void (*)(int, siginfo_t *, void *))handler;
	action.sa_flags = SA_SIGINFO;
#else
	action.sa_handler = (SigHandler *) handler;
	action.sa_flags = 0;
#endif

	sigaction(SIGBUS, &action, &old_action);
	sigaction(SIGSEGV, &action, &old_action);
	sigaction(SIGABRT, &action, &old_action);
	sigaction(SIGILL, &action, &old_action);
	sigaction(SIGFPE, &action, &old_action);
}

void SetDefaultHupHandler()
{
	SetHupHandler((SigHandler *) hup_handler);
}

void SetDefaultTermHandler()
{
	//SetTermHandler((SigHandler *) term_handler);
}

void SetDefaultDieHandler()
{

	SetDieHandler((SigHandler *) die_handler);
	
}
