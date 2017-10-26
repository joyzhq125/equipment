

#ifndef THREAD_H
#define THREAD_H
#include "config.h"
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#ifdef HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif // HAVE_SYS_SYSCALL_H
#include "exception.h"
#include "utils.h"


class ThreadException : public Exception
{
public:
    ThreadException( const std::string &message ) : Exception( stringtf( "(%d) "+message, (long int)syscall(SYS_gettid) ) )
    {
    }
};

class Mutex
{
friend class Condition;

private:
    pthread_mutex_t mMutex;

public:
    Mutex();
    ~Mutex();

private:
    pthread_mutex_t *getMutex()
    {
        return( &mMutex );
    }

public:
    void lock();
    void lock( int secs );
    void lock( double secs );
    void unlock();
    bool locked();
};

class ScopedMutex
{
private:
    Mutex &mMutex;

public:
    ScopedMutex( Mutex &mutex ) : mMutex( mutex )
    {
        mMutex.lock();
    }
    ~ScopedMutex()
    {
        mMutex.unlock();
    }

private:
    ScopedMutex( const ScopedMutex & );
};

class Condition
{
private:
    Mutex &mMutex;
    pthread_cond_t mCondition;

public:
    Condition( Mutex &mutex );
    ~Condition();

    void wait();
    bool wait( int secs );
    bool wait( double secs );
    void signal();
    void broadcast();
};

class Semaphore : public Condition
{
private:
    Mutex mMutex;

public:
    Semaphore() : Condition( mMutex )
    {
    }

    void wait()
    {
        mMutex.lock();
        Condition::wait();
        mMutex.unlock();
    }
    bool wait( int secs )
    {
        mMutex.lock();
        bool result = Condition::wait( secs );
        mMutex.unlock();
        return( result );
    }
    bool wait( double secs )
    {
        mMutex.lock();
        bool result = Condition::wait( secs );
        mMutex.unlock();
        return( result );
    }
    void signal()
    {
        mMutex.lock();
        Condition::signal();
        mMutex.unlock();
    }
    void broadcast()
    {
        mMutex.lock();
        Condition::broadcast();
        mMutex.unlock();
    }
};

template <class T> class ThreadData
{
private:
    T mValue;
    mutable bool mChanged;
    mutable Mutex mMutex;
    mutable Condition mCondition;

public:
    __attribute__((used)) ThreadData() : mCondition( mMutex )
    {
    }
    __attribute__((used)) ThreadData( T value ) : mValue( value ), mCondition( mMutex )
    {
    }
    //~ThreadData() {}

    __attribute__((used)) operator T() const
    {
        return( getValue() );
    }
    __attribute__((used)) const T operator=( const T value )
    {
        return( setValue( value ) );
    }

    __attribute__((used)) const T getValueImmediate() const
    {
        return( mValue );
    }
    __attribute__((used)) T setValueImmediate( const T value )
    {
        return( mValue = value );
    }
    __attribute__((used)) const T getValue() const;
    __attribute__((used)) T setValue( const T value );
    __attribute__((used)) const T getUpdatedValue() const;
    __attribute__((used)) const T getUpdatedValue( double secs ) const;
    __attribute__((used)) const T getUpdatedValue( int secs ) const;
    __attribute__((used)) void updateValueSignal( const T value );
    __attribute__((used)) void updateValueBroadcast( const T value );
};

class Thread
{
public:
    typedef void *(*ThreadFunc)( void * );

protected:
    pthread_t mThread;

    Mutex mThreadMutex;
    Condition mThreadCondition;
    pid_t mPid;
    bool  mStarted;
    bool  mRunning;

protected:
    Thread();
    virtual ~Thread();

    pid_t id() const
    {
        return( (pid_t)syscall(SYS_gettid) );
    }
    void exit( int status = 0 )
    {
        //INFO( "Exiting" );
        pthread_exit( (void *)&status );
    }
    static void *mThreadFunc( void *arg );

public:
    virtual int run() = 0;

    void start();
    void join();
    void kill( int signal );
    bool isThread()
    {
        return( mPid > -1 && pthread_equal( pthread_self(), mThread ) );
    }
    bool isStarted() const { return( mStarted ); }
    bool isRunning() const { return( mRunning ); }
};

#endif // THREAD_H