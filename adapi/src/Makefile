
UNAME=$(shell uname)


ifeq '$(DBG)' '1'
DBGOPT = -g
EXENAME=libadapid.so
LIBNAME=libadapid.a
LIB_SO=
else
EXENAME=libadapi.so
LIBNAME=libadapi.a
LIB_SO=
endif

ifeq '$(UNAME)' 'Darwin'
SRC_M += $(shell ls -1 *.m) 
LIB_SO +=  -framework Foundation 
endif


ifeq '$(GCC_ATOMIC)' '1'
CCFLAGS += -D_GCC_ATOMIC_
endif


SRC += $(shell ls -1 *.c) 

include	../../make_script/inc.Makefile
