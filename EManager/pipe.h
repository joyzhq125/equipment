#ifndef PIPE_H
#define PIPE_H

#define PIPE_BUF_LEN 2048

typedef struct tagSt_Pipe
{
	unsigned char  uInData[PIPE_BUF_LEN];
	unsigned char* pDataBufBase; //data base address
	int  databufsize;  //sizeof data buffer size
	int  validDataPosIdx; //valid data position offset from data buffer base
	int  validDataLen; //valid data length
}St_Pipe;

class PIPE
{
public:
	PIPE() {pipe_init();}
	virtual ~PIPE(){}

	void pipe_init (void);
	int pipe_write(string jqm,void *pvBuf, int lBufSize);
	int pipe_read(string jqm,void *pvBuf, int lBufSize);
	void pipe_close(void);
	void ReadData(    unsigned char      *pDstDataBuf, 
                int            iReadDataLen,
                int           *iSrcDataOffset,
                unsigned char       *pSrcDataBase,
                int           iSrcDataBufSize);
	
	void WriteData(unsigned char         *pSrcDataBuf, 
                int             iWritenDataLen,
                int             iDstDataOffset,
                unsigned char            *pDstDataBase,
                int             iDstDataBufSize );

	bool check_read_data(string jqm,unsigned char* pdata);
	bool check_valid_data();

protected:
	St_Pipe m_stPipe;
};

#endif //PIPE_H