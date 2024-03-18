#ifndef _KEGETTHREADLOCKS_H
#define _KEGETTHREADLOCKS_H
#include <ntddk.h>
#ifndef WINVER
#error "Something is wrong in the build settings"
#endif
#if (WINVER != _WIN32_WINNT_WIN7)
#error "Software not tested for other OS versions than Windows 7"
#endif 

extern NTSTATUS KeGetThreadLocks(PKTHREAD pKThread, //thread that is researched
	                             PKWAIT_BLOCK pKWaitBlockBuffer, //copy(!) of the wait objects
	                             PLONG pNumBuffer);// number of elements
//extern NTSTATUS KeGetThreadLocks(PKTHREAD volatile pKThread, PULONGLONG pSynchObjects, PLONG pNumObjects);
#endif /*#ifndef _KEGETTHREADLOCKS_H*/