
#if  defined(_WIN32_WCE)


//#pragma pack(1)
#include <windows.h>
#pragma comment (lib,"coredll.lib")
#pragma comment (lib,"commctrl.lib")
//#pragma comment (lib,"Mmtimer.lib")

#elif defined(WIN32)

#pragma comment (lib,"advapi32.lib")

#define  _WIN32_WINNT 0x0500
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utime.h>
#include <fcntl.h>
#include <tchar.h>
#include <process.h>
#include <Sddl.h>
#include <aclapi.h>
#else



#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <time.h>
#include <sys/syscall.h>

#if  !defined(__DARWIN_UNIX03)  && !defined(_STREAMBOX_) && !defined(_FreeBSD_) && !defined(_android_) && !defined(ANDROID)
#include <sys/io.h>

#endif

#if  !defined(__DARWIN_UNIX03) && !defined(_FreeBSD_)
#include <sys/vfs.h>
#endif

#include <utime.h>
#include <net/if.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <unistd.h>
#include <dirent.h>
#include <pthread.h>

#if defined(_Darwin_) || defined(__DARWIN_UNIX03)
#include <mach/mach_time.h>
#define  _USE_PTHREAD_MUTEX_RECURSIVE
#include <libkern/OSAtomic.h> 
#else
#define  _USE_PTHREAD_MUTEX_RECURSIVE_NP
#endif



#endif


#include <stdio.h>
#ifndef _NO_STDARG_
#include <stdarg.h>
#include <ctype.h>
#endif



#include <stdlib.h>
#include <string.h>

#if !defined(_FreeBSD_) && !defined(__DARWIN_UNIX03)
#include <malloc.h>
#endif

#include <math.h>
#include <time.h>
#include <errno.h>

#include "adtypes.h"
#include "adapi.h"
#include "memdbg.h"

#ifndef EOF
#define EOF  (-1)
#endif

#if !defined(__cplusplus)
#if  defined(_MSC_VER)
#define inline __forceinline
#endif
#endif

#if !defined(PATH_MAX)
#define PATH_MAX  300
#endif

#define LEN_ERROR 400
extern char szADPI_ERROR[];


void * MYAPI CreateMemoryPool_OSX();
void MYAPI ReleaseMemoryPool_OSX(void *p);













