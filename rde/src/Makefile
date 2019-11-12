

ifeq '$(DYNPKG)' '1'
CCFLAGS += -D_DYN_PKG_
endif

ifeq '$(DBG)' '1'
DBGOPT = -g
EXENAME=librded.so
LIBNAME=librded.a
LIB_SO=-ladapid
else
EXENAME=librde.so
LIBNAME=librde.a
LIB_SO=-ladapi
endif

CCFLAGS += -D_RDE_NTP

SRC += $(shell ls -1 *.c) 
include	../../make_script/inc.Makefile