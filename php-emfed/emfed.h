#ifndef EMFED_H
#define EMFED_H
#include <string>
#include "thread.h"
using namespace std;


class EMFEnd: public Thread
{
public:
    EMFEnd(string& value):mStop(false),m_sfunc(value) 
   {
/*   
    	Php::Value function(m_func);
		
   	m_func = function.clone();

	if (!m_func.isCallable()) 
	{
		Error("m_func not a function");
	}
	else
	{
		Info("m_func is a function");
		m_func(11,12);
	}
*/
   }
    virtual ~EMFEnd() {}
	
    void stop()
    {
        mStop = true;
    }
    bool stopped() const
    {
        return( mStop );
    }
    int run();
    static bool  emfed_Inited ;
    static void emfed_Init(string func);
    string m_sfunc;
    Php::Value m_func;// = params[1]
private:
    bool mStop;
};

//type 1
//id 4
#define EM_UNSOL_LEN  5 

#endif //EMFED_H