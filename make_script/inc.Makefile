


ifeq '$(DBGOPT)' ''
#DBGOPT = -O2
#STRIP_FLAG=1
DBGOPT = -g
endif

ifeq '$(EXENAME)' ''
$(error Error:invalid EXENAME)
endif


ifeq '$(MAKEEXE)' ''
ifeq '$(LIBNAME)' ''
$(error Error:invalid LIBNAME)
endif
endif


UNAME =$(shell uname)
ifeq '$(DESTDIR)' ''
DESTDIR = ../../libso/$(UNAME)
endif

ifeq '$(UNAME)' 'Linux'
DLOPT += -ldl
LDFLAGS  +=  -rdynamic
endif


ifeq '$(CC)' '' 
CC= gcc
endif

ifeq '$(CXX)' ''
CXX = $(CC)++
endif





$(shell mkdir -p $(DESTDIR))





INC_PATH    += -I../../include/ -I/usr/include   -I/usr/include/malloc  -I/usr/local/include -I/usr/local/include/stlport
LIB_PATH    += -L./ -L/lib  -L../../libso/$(UNAME) -L/usr/local/lib

LIB_SO  +=  -lm -lpthread $(DLOPT)
LIB_A   +=

ifeq '$(UNAME)' 'Darwin'
CCFLAGS += -D_OSX_ -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk
CCFLAGS += -mmacosx-version-min=10.7  -ferror-limit=1  
ifeq '$(MAKEEXE)' ''
LDFLAGS  +=  -dynamiclib
endif
else

ifneq (,$(findstring MINGW32, $(UNAME)))
CCFLAGS += -D_MINGW32_
else

ifneq (,$(findstring CYGWIN, $(UNAME)))
CCFLAGS += -D_CYGWIN_
endif

endif

endif


ifeq '$(INSTALL_NAME)' '' 
INSTALL_NAME = ${EXENAME}
endif


SHARED=-shared
ifeq '$(MAKEEXE)' '1'
SHARED=
else
SHARED=-shared 

ifeq '$(UNAME)' 'Darwin'
LDFLAGS_DARWIN  = -install_name ${INSTALL_NAME}
endif

endif


ifeq '$(UNAME)' 'Linux'
CCFLAGS  +=  -D_ALSA_ -D_V4L2_ -D_PC_  -D_GCC_BACKTRACE_ 
endif


LDFLAGS  +=     -rdynamic  $(SHARED)  $(DBGOPT) -D_$(UNAME)_
CCFLAGS    +=  -fstack-protector    $(DBGOPT) -D_$(UNAME)_

ifeq '$(NOPIC)' '' 
LDFLAGS  +=  -fPIC
CCFLAGS    += -fPIC 
endif

RM      = rm -f

NAME    = ${DESTDIR}/${EXENAME}
ARNAME    = ${DESTDIR}/${LIBNAME}
AR    =ar
ARFLAGS=-rv
STRIP=strip

OBJ_C=${SRC:.c=.o}
OBJ_S=${SRC_S:.s=.o}
OBJM=${SRC_M:.m=.o}
OBJMM=${SRC_MM:.mm=.o}
OBJCPP=${SRC_CPP:.cpp=.o}
OBJCC=${SRC_CC:.cc=.o}
OBJASM=${SRC_ASM:.asm=.o}

OBJ+=$(OBJCC)
OBJ+=$(OBJCPP)
OBJ+=$(OBJMM)
OBJ+=$(OBJM)
OBJ+=$(OBJ_C)
OBJ+=$(OBJ_S)
OBJ+=$(OBJASM)

SRCALL =  $(SRC_CPP) $(SRC_CC) $(SRC_MM) $(SRC_M) $(SRC) $(SRC_S)

all: ${NAME}

${NAME}:${ARNAME}
	${CC} ${LDFLAGS}   -o  ${NAME}  ${OBJ} ${LDFLAGS_DARWIN} ${INC_PATH} ${LIB_PATH}  ${LIB_A} ${LIB_SO}
ifeq '$(STRIP_FLAG)' '1'
ifeq '$(MAKESTATIC)' ''
	${STRIP} ${NAME}
endif
endif
ifeq '$(MAKESTATIC)' '1'
	${RM} ${NAME}
endif

${ARNAME}:${OBJ}
	${RM} ${ARNAME}
	${AR} ${ARFLAGS}  ${ARNAME}  ${OBJ}
%.o: %.c
	$(CC) ${CCFLAGS} $(INC_PATH) -c $< -o $@
%.o: %.m
	$(CC) -x objective-c ${CCFLAGS} $(INC_PATH) -c $< -o $@
%.o: %.mm
	$(CXX) -x objective-c++ ${CCFLAGS} $(INC_PATH) -c $< -o $@
%.o: %.cpp
	$(CXX) ${CCFLAGS} $(INC_PATH) -c $< -o $@
%.o: %.cc
	$(CXX) ${CCFLAGS} $(INC_PATH) -c $< -o $@
%.o: %.s
	$(CC) ${CCFLAGS} $(INC_PATH) -c $< -o $@

 

.PHONY: clean re
clean:
	find . -name "*.o"  | xargs ${RM} -f
	find . -name "*.obj"  | xargs ${RM} -f
	${RM} ${NAME}
	${RM} $(ARNAME)
re: clean all


