
#ifndef EXCEPTION_H
#define EXCEPTION_H

//#include "em.h"

#include <string>

class Exception
{
protected:
    typedef enum { INFO, WARNING, ERROR, FATAL } Severity;

protected:
    std::string mMessage;
    Severity mSeverity;

public:
    Exception( const std::string &message, Severity severity=ERROR ) : mMessage( message ), mSeverity( severity )
    {
    }

public:
    const std::string &getMessage() const
    {
        return( mMessage );
    }
    Severity getSeverity() const
    {
        return( mSeverity );
    }
    bool isInfo() const
    {
        return( mSeverity == INFO );
    }
    bool isWarning() const
    {
        return( mSeverity == WARNING );
    }
    bool isError() const
    {
        return( mSeverity == ERROR );
    }
    bool isFatal() const
    {
        return( mSeverity == FATAL );
    }
};

#endif // EXCEPTION_H
