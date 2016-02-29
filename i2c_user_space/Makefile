#*******************************************************************************
#  File Name  : Makefile  
#  Author     : weiming   
#  Date       : 2012/06/14  
#  cmd        : make  
#*******************************************************************************  
  
SER_NAME = i2c
#M_DATE = `date '+%y%m%d'`
M_DATE = test


CROSS_COMPILE = arm-none-linux-gnueabi-
CXX = $(CROSS_COMPILE)gcc
#AR  = ar cr
COMPILE_FLAGS = -Wall

ifdef CROSS_COMPILE
THIRDLIBS = arm-lib
else
THIRDLIBS = x86-lib
endif
  
INCLUDE_PATH = 

LIB_PATH = 

LIBS = 

SRC = $(wildcard *.c)

OBJS = $(SRC:.c=.o)

all: $(OBJS) $(SER_NAME)

.PHONY:all clean

$(SER_NAME):$(OBJS)
	$(CXX) -o $(SER_NAME)_$(M_DATE) $(OBJS) $(INCLUDE_PATH) $(LIB_PATH) $(LIBS)
#	cp -rPf $(SER_NAME)_$(M_DATE) /home/qinbh/Work/imx/nfsroot

%.o : %.c
	$(CXX) -c $(COMPILE_FLAGS) $(INCLUDE_PATH) $< -o $@

clean:
	-rm *.o $(SER_NAME)_$(M_DATE)
