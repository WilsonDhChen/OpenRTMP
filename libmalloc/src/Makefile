
UNAME=$(shell uname)
CC=$(CXX)
ifeq '$(DBG)' '1'
DBGOPT = -g
EXENAME=libmalloc2d.so
LIBNAME=libmalloc2d.a
LIB_A=
else
EXENAME=libmalloc2.so
LIBNAME=libmalloc2.a
LIB_A=
endif


CCFLAGS = -D_STLP_DONT_USE_AUTO_LINK -D_STLP_NO_IOSTREAMS 
CCFLAGS += -D_NO_STLPORT_
ifeq '$(UNAME)' 'FreeBSD'
#LIB_SO=-lstlport_gcc
INC_PATH=-I/usr/ports/devel/stlport/work/STLport-5.1.5/stlport
LIB_A += ../../libso/$(UNAME)/libstlport.a
else
INC_PATH=-I../../include/stlport 
LIB_SO=
endif

ifeq '$(GCC_ATOMIC)' '1'
CCFLAGS += -D_GCC_ATOMIC_
endif

SRC_CPP += $(shell ls -1 *.cpp) 
include	../../make_script/inc.Makefile