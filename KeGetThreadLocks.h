#ifndef _KEGETTHREADLOCKS_H
#define _KEGETTHREADLOCKS_H
#include <ntddk.h>

#ifndef WINVER
   #error "Something is wrong in the build settings"
#endif
#if (WINVER != _WIN32_WINNT_WIN7)
   #error "Software not tested for other OS versions than Windows 7"
#endif 
#ifndef _AMD64_
   #error "Software not tested for other architecures than x64"
#endif


extern NTSTATUS KeGetThreadLocks(
		PKTHREAD     IN     pKThread,//thread that is researched
		PKWAIT_BLOCK OUT    pKWaitBlockBuffer,//copy(!) of the wait objects
		PLONG        IN OUT pNumObjects // number of elements in and out
	);


#endif /*#ifndef _KEGETTHREADLOCKS_H*/