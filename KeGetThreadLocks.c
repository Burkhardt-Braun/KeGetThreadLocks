#include "KeGetThreadLocks.h"

#include <ntddk.h>
#include <wdm.h>
#include <aux_klib.h>
#include <crtdbg.h>
#include <stddef.h>

typedef unsigned char BYTE;
#define MAX_EVENTS (10)//delete me
#define NELEM(x) ( sizeof(x)/sizeof(*x))
/*
typedef struct _KWAIT_STATUS_REGISTER {
	 BYTE Flags;            // UChar
	 BYTE State : 2;            // Pos 0, 2 Bits
	 BYTE Affinity : 1;         // Pos 2, 1 Bit
	 BYTE Priority : 1;         // Pos 3, 1 Bit
	 BYTE Apc : 1;              // Pos 4, 1 Bit
	 BYTE UserApc : 1;          // Pos 5, 1 Bit
	 BYTE Alert : 1;            // Pos 6, 1 Bit
	 BYTE Unused : 1;           // Pos 7, 1 Bit
} KWAIT_STATUS_REGISTER, * PKWAIT_STATUS_REGISTER;
*/
typedef struct {
	char FillByte0[0x48];
	BYTE/*KWAIT_STATUS_REGISTER*/ WaitRegister;
	BYTE Running;
	char FillByte1[0x90 - 0x48 - 8/*sizeof(KWAIT_STATUS_REGISTER)*/];
	INT64 WaitStatus; //Int8B signed 64bit ...This WaitStatus describes the reason why the thread was awaken from its wait state. 
	PKWAIT_BLOCK pWaitBlockList;
	char FillByte2[0x70 - 8];
	KWAIT_BLOCK WaitBlock[4];
}KThreadPriv_type;



extern KEVENT event;
/*****************************************************************************************************/
void Stop(void)
{
#pragma message("delete me")
	__nop();
}
/*****************************************************************************************************/
/*
* 
* returns STATUS_SUCCESS if OK
* returns STATUS_INVALID_PARAMETER is any parameter is wrong
* returns -1 if Thread is running instead of waiting
* 
*/
//NTSTATUS KeGetThreadLocks(PKTHREAD pKThread, PULONGLONG pSynchObjects, PLONG pNumObjects)
NTSTATUS KeGetThreadLocks(PKTHREAD pKThread, 
	                      PKWAIT_BLOCK pKWaitBlockBuffer, 
	                      PLONG pNumObjects)
{
	int nLoopCount = 0;
	PUCHAR pHeadPtr = NULL;
	PKWAIT_BLOCK pWaitBlockList = NULL;
	NTSTATUS ntStatus = STATUS_SUCCESS;
	//KWAIT_STATUS_REGISTER WaitRegister;
	KThreadPriv_type volatile * pKThread_priv = (KThreadPriv_type*)pKThread;
	int i;


	_STATIC_ASSERT(sizeof(ULONGLONG) == 8);

	_STATIC_ASSERT(offsetof(KThreadPriv_type, WaitStatus) == 0x90);
	_STATIC_ASSERT(offsetof(KThreadPriv_type, pWaitBlockList) == 0x98);
	_STATIC_ASSERT(offsetof(KThreadPriv_type, WaitBlock) == 0x108);

	

	DbgPrintEx(0, 0, "%s\n", __FUNCTION__);

	if (NULL == pKThread ||
		NULL == pKWaitBlockBuffer ||
		NULL == pNumObjects ||
		0 >= *pNumObjects)
	{
		ntStatus = STATUS_INVALID_PARAMETER;
		return ntStatus;
	}
	if (pKThread_priv->Running != 0)
	{
		ntStatus = -1;
		return ntStatus;
	}

	DbgPrintEx(0, 0, "pKThread_priv->Running %02x\n", pKThread_priv->Running);
	__nop();
	pWaitBlockList = ((KThreadPriv_type*)pKThread)->pWaitBlockList;


	if (pWaitBlockList)
	{
		pHeadPtr = (PUCHAR)pWaitBlockList;
		do {
			pKWaitBlockBuffer[nLoopCount] = *pWaitBlockList;
			nLoopCount += 1;
			pWaitBlockList = pWaitBlockList->NextWaitBlock;
		} while (pWaitBlockList &&
			pWaitBlockList != (PKWAIT_BLOCK)pHeadPtr &&
			pKThread_priv->Running == 0 &&
			nLoopCount < *pNumObjects
			);

		ASSERT(nLoopCount < MAX_EVENTS + 1);

	}

	*pNumObjects = nLoopCount;
	if (pKThread_priv->Running != 0 )
	{
		ntStatus = -1;
	}
		

	return ntStatus;
}


