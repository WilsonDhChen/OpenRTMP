
UNAME=$(shell uname)


EXENAME=libmediasrv.so
LIBNAME=libmediasrv.a
LIB_A= ../../libso/$(UNAME)/libpolarssl.a
LIB_SO += -lrde  -ladapi -lpthread  -lmalloc2 -lcpputil

STLPORT_SUPPORT=




ifeq '$(STLPORT_SUPPORT)' '1'

	CCFLAGS += -D_STLP_DONT_USE_AUTO_LINK -D_STLP_NO_IOSTREAMS  
	CCFLAGS += -D_STLPORT_SUPPORT

	ifeq '$(UNAME)' 'FreeBSD'
	INC_PATH +=-I/usr/ports/devel/stlport/work/STLport-5.1.5/stlport
	else
	INC_PATH +=-I./stlport -I../../include/stlport
	LIB_SO +=
	endif

endif




LIB_SO += -lstdc++




LIB_SO +=  -lpthread  

INC_PATH += -I../../libpolarssl/src/include  -I../../libpolarssl/src/include/polarssl -I../../libjsonc/include
INC_PATH += -I../../libmp4v2/src/include
INC_PATH += -I../../libspeex/include
INC_PATH += -I./



CCFLAGS += -D_STLP_DONT_USE_AUTO_LINK -D_STLP_NO_IOSTREAMS  
CCFLAGS += -D_RTMP_SSL
CCFLAGS += -D_RTMP_SUPPORT_
CCFLAGS += -D_MAX_CONNECTIONS=50000










CCFLAGS += -D_SELECT_IO




SRC_CPP += $(shell ls -1 *.cpp) 



include	../../make_script/inc.Makefile
