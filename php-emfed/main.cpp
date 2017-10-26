#include <phpcpp.h>
#include <iostream>
#include <string>

#include "logger.h"
#include "cfg.h"

#include "comms.h"
#include "ecdr.h"
#include "emfed.h"


#define EM_REQ_LEN  17

typedef enum 
{
	E_SUCCESS = 0x01,
	E_TIMEOUT,
	E_FAIL,
	E_OTHER
}E_RET;

typedef enum
{
	CMD_NONE = 0x00,
	CMD_ORG,
	CMD_DIGIT,
	CMD_STR,
	CMD_NORMAL		
}E_CMD_TYPE;

void UnsolMsg(Php::Parameters &params)
{
	string func = params[0];
	logInit("PHP_EMFED");
	LoadConfig();
	//EMFEnd::emfed_Init(func);
}

//devid typid cmd data(digit or string)
//return success/timeout/fail
Php::Value SendCmdAndReply(Php::Parameters &params)
{
/*
	logInit("PHP_EMFED");
	LoadConfig();
	Info("SendCmdAndReply");
	char revbuffer[EM_REQ_LEN];
	int devid = params[0];
	int type = params[1];
	string cmd;
	if(CMD_ORG == type)
	{
		cmd = params[2];
		
	}
	else if(CMD_DIGIT == type)
	{
		float digit = params[3];
		cmd = params[2];
		//cmd += itoa(params[3]);
		
	}	
	else if(CMD_STR == type)
	{
		//string str = params[3];
		cmd = params[2];
	}		
	else
	{
		return E_OTHER;
	}
	if(EM_CMD_LEN != cmd.length())
	{
		Error("SendCmdAndReply:cmd length is %d not %d",cmd.length(),EM_REQ_LEN);
		return E_OTHER;
	}
	Info("SendCmdAndReply:cmd is %s",cmd.c_str());

	
	UdpInetClient udpClnt;
	SockAddrInet sockAddr;
	Select udpSelect(30);
	int ilen = 0;
	int waitvalue =0;
	int iRecv  = 0;
	sockAddr.resolve("localhost",emConfig.UDP_SRV_PORT,"udp");

	//send cmd
	if((ilen = udpClnt.sendto(cmd.c_str(),EM_REQ_LEN)) < EM_REQ_LEN)
	{
		Error("SendCmdAndReply: send length is %d not %d",ilen,EM_REQ_LEN);
		return E_OTHER;
	}

	//get reply
	if(!udpSelect.addReader(&udpClnt ))
	{
		return E_OTHER;
	}
	waitvalue = udpSelect.wait();
	if (0 == waitvalue)
	{
		return E_TIMEOUT;
	}
	else if (0 > waitvalue)
	{
		return E_OTHER;
	}
	iRecv = udpClnt->recvfrom(revbuffer,sizeof(revbuffer));
*/
	return E_SUCCESS;

	
}

Php::Value SendCmd(Php::Parameters &params)
{
	logInit("PHP-EMFED");
	LoadConfig();
	Info("SendCmd");
	int devid = params[0];
	int type = params[1];
	string cmd  = params[2];

	if(CMD_ORG == type)
	{
		//cmd = params[2];		
	}
	else if(CMD_DIGIT == type)
	{
		//float digit = params[3];
		//cmd = params[2];
		//cmd += itoa(params[3]);	
	}	
	else if(CMD_STR == type)
	{
		//string str = params[3];
		//cmd = params[2];
	}		
	else
	{
		logTerm();
		return E_OTHER;
	}
	Info("SendCmd:devid %d,type %d,cmd is %s",devid,type,cmd.c_str());
	St_ecdrEncoder * pstEncoder = NULL;
	pstEncoder = ecdr_CreateEncoder(EM_REQ_LEN);
	ecdr_EncodeUint32(pstEncoder,(unsigned int* )&devid);
	ecdr_EncodeUint8(pstEncoder, (unsigned char*)&type);
	ecdr_EncodeOctets(pstEncoder,(char*)cmd.c_str(),cmd.length());
	if(EM_REQ_LEN  != pstEncoder->iPosition)
	{
		Error("SendCmd encode fail %d",pstEncoder->iPosition);
		ecdr_DestroyEncoder( pstEncoder );
		logTerm();
		return E_OTHER;
	}		
	//Info("SendCmd:endcode is %s",pstEncoder->pucData);
	UdpInetClient udpClnt;
	SockAddrInet sockAddr;
	int ilen = 0;
	if(!udpClnt.connect(emConfig.EM_HOST.c_str(),emConfig.UDP_SRV_PORT))
	{
		Error("connect fail");
		udpClnt.close();
		ecdr_DestroyEncoder( pstEncoder );
		logTerm();	
		return E_FAIL;
	}

	//send cmd
	if((ilen = udpClnt.send(pstEncoder->pucData,EM_REQ_LEN)) < EM_REQ_LEN)
	{
		Error("SendCmd: send length is %d not %d",ilen,EM_REQ_LEN);
		udpClnt.close();
		ecdr_DestroyEncoder( pstEncoder );
		logTerm();	
		return E_FAIL;
	}
	udpClnt.close();
	ecdr_DestroyEncoder( pstEncoder );
	logTerm();
	return E_SUCCESS;
}

void test(Php::Parameters &params)
{
	//Php::out << params[0];
	//std::string func = params[0];
	//Php::call(func.c_str(),11,22);
	    // second parameter is a callback function
	    Php::Value callback = params[0];
	    
	    // call the callback function
	    callback();
}

/**
 *  Native function that is callable from PHP
 *
 *  This function gets two parameters: an associative array and a callback.
 *  It does not do anything meaningful, it is just a demonstration function.
 *
 *  @param  params      The parameters passed to the function
 */
/*
void example_function(Php::Parameters &params)
{
    // first parameter is an array
    Php::Value array = params[0];
    
    // call the PHP array_keys() function to get the parameter keys
    std::vector<std::string> keys = Php::array_keys(array);
    
    // loop through the keys
    for (auto &key : keys) 
    {
        // output key
        Php::out << "key: " << key << std::endl;
    }
    
    // call a function from user space
    Php::Value data = Php::call("some_function", "some_parameter");
    
    // create an object (this will also call __construct())
    Php::Object time("DateTime", "now");
    
    // call a method on the datetime object
    Php::out << time.call("format", "Y-m-d H:i:s") << std::endl;
    
    // second parameter is a callback function
    Php::Value callback = params[1];
    
    // call the callback function
    callback("some","parameter");
    
    // in PHP it is possible to create an array with two parameters, the first
    // parameter being an object, and the second parameter should be the name
    // of the method, we can do that in PHP-CPP too
    Php::Array time_format({time, "format"});
    
    // call the method that is stored in the array
    Php::out << time_format("Y-m-d H:i:s") << std::endl;
}
*/
/**
 *  tell the compiler that the get_module is a pure C function
 */
extern "C" {
    
    /**
     *  Function that is called by PHP right after the PHP process
     *  has started, and that returns an address of an internal PHP
     *  strucure with all the details and features of your extension
     *
     *  @return void*   a pointer to an address that is understood by PHP
     */
    PHPCPP_EXPORT void *get_module() 
    {
        // static(!) Php::Extension object that should stay in memory
        // for the entire duration of the process (that's why it's static)
        static Php::Extension extension("php-emfed", "0.1");
        
        // @todo    add your own functions, classes, namespaces to the extension
       // add the example function so that it can be called from PHP scripts
        //extension.add("example_function", example_function);
        extension.add("test", test);      
	 extension.add("UnsolMsg",UnsolMsg);
	 extension.add("SendCmdAndReply",SendCmdAndReply);
	 extension.add("SendCmd",SendCmd);
        // return the extension
        return extension;
    }
}

