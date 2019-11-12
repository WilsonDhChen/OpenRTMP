
UNAME=$(shell uname)
CC=$(CXX)
ifeq '$(DBG)' '1'
DBGOPT = -g
EXENAME=libcpputild.so
LIBNAME=libcpputild.a
#LIB_A=../../libso/$(UNAME)/libadapid.a ../../libso/$(UNAME)/librded.a
LIB_SO += -lrded  -ladapid   -lmalloc2d
else
EXENAME=libcpputil.so
LIBNAME=libcpputil.a
#LIB_A=../../libso/$(UNAME)/libadapi.a ../../libso/$(UNAME)/librde.a
LIB_SO += -lrde  -ladapi   -lmalloc2
endif

CCFLAGS += -D_STLP_DONT_USE_AUTO_LINK -D_STLP_NO_IOSTREAMS -D_STL_SUPPORT 
LIB_SO +=  -lpthread





ifeq '$(UNAME)' 'FreeBSD'
INC_PATH=-I/usr/ports/devel/stlport/work/STLport-5.1.5/stlport
LIB_A +=
else

ifeq '$(UNAME)' 'Darwin'
INC_PATH = -I../../include/stlport/
else
INC_PATH = -I../../include/stlport/
LIB_SO +=   -lrt
endif

endif




SRC_CPP += $(shell ls -1 *.cpp) 
include	../../make_script/inc.Makefile
