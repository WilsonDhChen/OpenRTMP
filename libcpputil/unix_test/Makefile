
UNAME=$(shell uname)
EXENAME =libcpputiltest
DESTDIR =/usr/local/bin/
SRC     = unix_service.cpp

ifeq '$(UNAME)' 'Linux'
DLOPT += -ldl
endif

INC_PATH    = -I/usr/include   -I../../include
LIB_PATH    = -L/lib -L/usr/lib/mysql/ -L../../libso/$(UNAME)

LIB_SO  = $(DLOPT) -lm  -lpthread
LIB_A   =  ../../libso/$(UNAME)/libcpputild.a  \
           ../../libso/$(UNAME)/librded.a \
           ../../libso/$(UNAME)/libadapid.a   \
           ../../libso/$(UNAME)/libmalloc2d.a 

LDFLAGS  =  -fPIC -rdynamic  -g -D_DEBUG_
CC      = g++
RM      = rm -f
CCFLAGS    =  -fPIC -rdynamic  -g -D_DEBUG_ -D_$(UNAME)_
NAME    = ${DESTDIR}/${EXENAME}

OBJ=${SRC:.cpp=.o}

all: ${NAME}

${NAME}:${OBJ}              
	${CC} ${LDFLAGS}   -o  ${NAME}  ${OBJ}  ${INC_PATH} ${LIB_PATH}  ${LIB_A} ${LIB_SO} 
	${RM} ${OBJ}
        
${OBJ}:${SRC}
	${CC}  ${CCFLAGS}   ${INC_PATH}  -c $?   

.PHONY: clean re
clean:
	${RM} ${OBJ} ${NAME}
re: clean all


