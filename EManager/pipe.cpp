#include "em.h"
#include "pipe.h"




/*============================
 pipe 操作
==============================*/
void PIPE::pipe_init (void)
{
	memset((void*)&m_stPipe,0,sizeof(St_Pipe));
	m_stPipe.validDataLen = 0;
	m_stPipe.validDataPosIdx = 0; 

	m_stPipe.pDataBufBase = m_stPipe.uInData; //data base address
	m_stPipe.databufsize = PIPE_BUF_LEN;  //sizeof data buffer size
}

int PIPE::pipe_write(string jqm,void *pvBuf, int lBufSize)
{
	Debug(1,"PIPE::pipe_write posidx is %d,validlen is %d write len is %d",m_stPipe.validDataPosIdx,m_stPipe.validDataLen,lBufSize);
	int        lDataLen = 0;	
	lDataLen = m_stPipe.databufsize - m_stPipe.validDataLen ;
	if (lDataLen >=  lBufSize)     
	{
		lDataLen = lBufSize;
	    	WriteData((unsigned char*)pvBuf,
	               lDataLen,
	               (m_stPipe.validDataPosIdx + m_stPipe.validDataLen) % m_stPipe.databufsize  ,
	               m_stPipe.pDataBufBase,
	               m_stPipe.databufsize);
	    	m_stPipe.validDataLen += lDataLen;

	}
	else
	{
		Info("pipe_write : 0");
	}
	return lDataLen;
	
}

int PIPE::pipe_read(string jqm,void *pvBuf, int lBufSize)
{
	 Debug(1,"PIPE::pipe_read posidx is %d,validlen is %d read len is %d,jqm is %s",m_stPipe.validDataPosIdx,m_stPipe.validDataLen,lBufSize,jqm.empty()?"":jqm.c_str());
	 int  lDataLen = 0;
	 char* pvTemp = (char*)pvBuf; 
	 while(m_stPipe.validDataLen >= lBufSize)
 	 { 
	 	char readChar;
		lDataLen = 1;				
		if( lDataLen != 0 )
		{
			ReadData((unsigned char*)&readChar,
			lDataLen,
			&(m_stPipe.validDataPosIdx) ,
			m_stPipe.pDataBufBase,
			m_stPipe.databufsize );
			m_stPipe.validDataLen  -= lDataLen;
		}
		
		if((readChar == '$' || readChar == '@'))
 	 	{
 	 	 	if(m_stPipe.validDataLen >= lBufSize - 1)
	 	 	{
		 	 	lDataLen = lBufSize -1;	
				*pvTemp++ = readChar;
				if( lDataLen != 0 )
				{
					ReadData((unsigned char*)pvTemp,
							lDataLen,
							&(m_stPipe.validDataPosIdx) ,
							m_stPipe.pDataBufBase,
							m_stPipe.databufsize );
					m_stPipe.validDataLen  -= lDataLen;
					lDataLen++;			
				}
				else
				{
					lDataLen = 0;
				}
 	 	 	}
			else //不够长回退
			{
				Info("PIPE::pipe_read go back -1,jqm is %s",jqm.empty()?"":jqm.c_str());
				if(m_stPipe.validDataPosIdx == 0)
				{
					m_stPipe.validDataPosIdx = m_stPipe.databufsize -1;
				}
				else
				{
					m_stPipe.validDataPosIdx --;
				}
				m_stPipe.validDataLen += 1;
				lDataLen = 0;
			}
			break;
		}
		else
		{
			Warning("PIPE::pipe_read discard char %c,jqm is %s",readChar,jqm.empty()?"":jqm.c_str());
			lDataLen = 0;
		}
		
 	 }
	return lDataLen;
	 
}

void PIPE::pipe_close(void)
{
}

void PIPE::ReadData(    unsigned char      *pDstDataBuf, 
                int            iReadDataLen,
                int           *iSrcDataOffset,
                unsigned char       *pSrcDataBase,
                int           iSrcDataBufSize)
{
        unsigned char *pDstIdx,*pSrcIdx;
        int iLen;

        pDstIdx = pDstDataBuf;
        pSrcIdx = (unsigned char*)(pSrcDataBase + *iSrcDataOffset);

        iLen = iSrcDataBufSize - *iSrcDataOffset;
        if( iLen  < iReadDataLen)
        {
            memcpy(pDstIdx,pSrcIdx,(unsigned int)iLen);
			
            pDstIdx += iLen;
            pSrcIdx = pSrcDataBase;		
            memcpy(pDstIdx,pSrcIdx,(unsigned int)(iReadDataLen - iLen));
            *iSrcDataOffset = iReadDataLen - iLen;
        }
        else
        {
            memcpy(pDstIdx,pSrcIdx,(unsigned int)iReadDataLen);

            if( iLen == iReadDataLen )
            {
                *iSrcDataOffset = 0;
            }
            else
            {
                *iSrcDataOffset += iReadDataLen;
            }
        }

        return;
        
}

void PIPE::WriteData(unsigned char         *pSrcDataBuf, 
                int             iWritenDataLen,
                int             iDstDataOffset,
                unsigned char            *pDstDataBase,
                int             iDstDataBufSize )
{
        unsigned char *pDstIdx,*pSrcIdx;
        int iLen;

         pSrcIdx = pSrcDataBuf;
         pDstIdx = (unsigned char*)(pDstDataBase + iDstDataOffset);

        iLen = iDstDataBufSize - iDstDataOffset;
        if( iLen  < iWritenDataLen)
        {
            memcpy(pDstIdx,pSrcIdx,iLen);			
            pSrcIdx += iLen;
            pDstIdx = pDstDataBase;
            memcpy(pDstIdx,pSrcIdx,(iWritenDataLen - iLen));
        }
        else
        {
            memcpy(pDstIdx,pSrcIdx,iWritenDataLen);
        }

        return;            
}



bool PIPE::check_read_data(string jqm,unsigned char* pdata)
{
	Debug(1,"PIPE::check_read_data");
	bool bres = false;
	unsigned char value;
	unsigned char nread = 0;
	unsigned char readbuff[EM_CMD_LEN] = {0};
	unsigned char readtemp[EM_CMD_LEN + 1] = {0};
	memset(readbuff,0,EM_CMD_LEN);
	nread = pipe_read(jqm,readbuff,EM_CMD_LEN);	
	if (nread == EM_CMD_LEN)
	{		
		//检查起始位
		memcpy(readtemp,readbuff,EM_CMD_LEN);
		Info("pipe::check_read_data readata is %s,jqm is %s",readtemp,jqm.empty()?"":jqm.c_str());
		if((readbuff[0] == '$') || (readbuff[0] == '@'))
		{
			memcpy(pdata,readbuff,EM_CMD_LEN);
			bres = true;
		}
		else
		{
			//dicard data
			pipe_init();
			Warning("pipe::check_read_data check header error and discard data,jqm is %s",jqm.empty()?"":jqm.c_str());
		}
		//检查校验位
		//重发机制
	}
	else
	{
		Warning("pipe::check_read_data read length is %d,jqm is %s",nread,jqm.empty()?"":jqm.c_str());
	}
	
	return bres;
}

bool PIPE::check_valid_data()
{
	if (m_stPipe.validDataLen > 0 )
	{
		return true;
	}
	else
	{
		return false;
	}
}