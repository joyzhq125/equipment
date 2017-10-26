#ifndef MISC_H
#define MISC_H
#include <mysql/mysql.h>
#include "timer.h"
class Misc:public Timer
{
public:
	Misc(int timeout, bool repeat=false):Timer(timeout,repeat){dbconn();}
	virtual ~Misc() {}
	void expire();	
	void HbChk();
	bool dbconn();
protected:
	MYSQL m_dbconn;

};

class CheckMethod:public Misc
{
public:
	CheckMethod(int timeout, bool repeat=false):Misc(timeout,repeat) {}
	virtual ~CheckMethod() {}
	void expire();
protected:
	 int getZjChkMethod();
	 int getSbChkMethod();
};
#endif// MISC_H
