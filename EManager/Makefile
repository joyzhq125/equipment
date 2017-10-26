CC = gcc
CXX = g++
#CFLAG = -Wall -O2 
#CXXFLAGS = -Wall -O2
CFLAG = -g -Wall 
CXXFLAGS = -g -Wall 
TARGET = EManager
LIB = -lpthread -lmysqlclient
LINK = -L/usr/lib64/mysql
LINKFLAG = 
AR = ar
RM = rm -f

#VPATH = .:../../impl/core/cert:../../impl/core/ue
VPATH = .
#INC = -I"../../impl/src/wep/inc" -I"../../impl/inc/sec" 

OBJS = cfg.o chrt.o commrec.o comms.o thread.o  \
	db.o emanager.o inet.o ecdr.o exception.o pipe.o\
	logger.o misc.o msghdl.o msgqueue.o reqdata.o \
	rspdata.o signal.o time.o timer.o utils.o misc.o\
	

All : $(TARGET)

$(TARGET):$(OBJS)
	$(CXX) -o $@ $^ $(LIB) $(LINK)
#	$(AR) -r $@ $^

%.o: %.c
	@echo ---------------------------
	@echo TARGET $(@F)
	$(CC) $(INC) $(CFLAG) -c -o $@ $^
	@echo ---------------------------
	
%.o: %.cpp
	@echo ---------------------------
	@echo TARGET $(@F)
	$(CXX) $(INC) $(CXXFLAGS) -c -o $@ $^
	@echo ---------------------------

clean:
	$(RM) *.o $(TARGET)
    	

