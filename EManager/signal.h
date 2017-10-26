

#ifndef SIGNAL_H
#define SIGNAL_H

#include <signal.h>
#include "config.h"
#if HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#if HAVE_UCONTEXT_H
#include <ucontext.h>
#endif
#include "config.h"

typedef RETSIGTYPE (SigHandler)( int );

extern bool g_reload;
extern bool g_terminate;

RETSIGTYPE hup_handler( int signal );
RETSIGTYPE term_handler( int signal );
#if ( HAVE_SIGINFO_T && HAVE_UCONTEXT_T )
RETSIGTYPE die_handler( int signal, siginfo_t *info, void *context );
#else
RETSIGTYPE die_handler( int signal );
#endif

void SetHupHandler( SigHandler *handler );
void SetTermHandler( SigHandler *handler );
void SetDieHandler( SigHandler *handler );

void SetDefaultHupHandler();
void SetDefaultTermHandler();
void SetDefaultDieHandler();

#endif // ZM_SIGNAL_H
