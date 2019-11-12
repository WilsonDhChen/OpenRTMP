
MAKESTATIC=1
UNAME=$(shell uname)


ifeq '$(DBG)' '1'
DBGOPT = -g
EXENAME=libpolarssld.so
LIBNAME=libpolarssld.a
LIB_SO=
else
EXENAME=libpolarssl.so
LIBNAME=libpolarssl.a
LIB_SO=
endif


INC_PATH = -I./include/  -I./include/polarssl
SRC += $(shell ls -1 library/*.c) 

include	../../make_script/inc.Makefile
