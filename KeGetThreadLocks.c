#include "KeGetThreadLocks.h"

#include <ntddk.h>
#include <wdm.h>
#include <aux_klib.h>
#include <crtdbg.h>
#include <stddef.h>


#ifndef NELEM
   #define NELEM(x) ( sizeof(x)/sizeof(*x))
#endif

typedef struct {
	UCHAR filler0[0x48];
	UCHAR WaitRegister;
	UCHAR Running;
	UCHAR filler1[0x90 - 0x48 - 8/*sizeof(KWAIT_STATUS_REGISTER)*/];
	INT64 WaitStatus; //Int8B signed 64b
	PKWAIT_BLOCK pWaitBlockList;
	UCHAR filler2[0x70 - 8];
	KWAIT_BLOCK WaitBlock[4];
	/*uncomplete!*/
}KThreadPriv_type;




/*****************************************************************************************************/
/*
extern NTSTATUS KeGetThreadLocks(
		PKTHREAD     IN     pKThread,//thread that is researched
		PKWAIT_BLOCK OUT    pKWaitBlockBuffer,//copy(!) of the wait objects
		PLONG        IN OUT pNumObjects // number of elements in and out
	);
*
* The maximum number of elements that pKWaitBlockBuffer can store is provided
* in *pNumObjects. The elements delivered is stored there.
* pKWaitBlockBuffer is memoryx provided by the caler of the function.
* 
* returns STATUS_SUCCESS if OK
* returns STATUS_INVALID_PARAMETER is any parameter is wrong
* returns -1 if Thread is running instead of waiting
* 
*/
NTSTATUS KeGetThreadLocks(PKTHREAD     IN     pKThread, 
	                      PKWAIT_BLOCK OUT    pKWaitBlockBuffer, 
	                      PLONG        IN OUT pNumObjects)
{
	INT nLoopCount = 0;
	PUCHAR pHeadPtr = NULL;
	PKWAIT_BLOCK pWaitBlockList = NULL;
	NTSTATUS ntStatus = STATUS_SUCCESS;
	KThreadPriv_type* pKThread_priv = (KThreadPriv_type*)pKThread;
	


	_STATIC_ASSERT(sizeof(ULONGLONG) == 8);

	_STATIC_ASSERT(offsetof(KThreadPriv_type, WaitStatus) == 0x90);
	_STATIC_ASSERT(offsetof(KThreadPriv_type, pWaitBlockList) == 0x98);
	_STATIC_ASSERT(offsetof(KThreadPriv_type, WaitBlock) == 0x108);

	if (NULL == pKThread ||
		NULL == pKWaitBlockBuffer ||
		NULL == pNumObjects ||
		0 >= *pNumObjects)
	{
		ntStatus = STATUS_INVALID_PARAMETER;
		return ntStatus;
	}

	//thread running?
	if (pKThread_priv->Running )
	{
		ntStatus = -1;
		return ntStatus;
	}



	if (NULL != (pWaitBlockList = ((KThreadPriv_type*)pKThread)->pWaitBlockList ) ) 
	{
		pHeadPtr = (PUCHAR)pWaitBlockList;
		do {
			pKWaitBlockBuffer[nLoopCount] = *pWaitBlockList;//copy!
			nLoopCount += 1;
			pWaitBlockList = pWaitBlockList->NextWaitBlock;
		} while (pWaitBlockList &&
				 pWaitBlockList != (PKWAIT_BLOCK)pHeadPtr &&
				 pKThread_priv->Running == 0 &&
				 nLoopCount < *pNumObjects
				);
	}

	//thread running?
	if (pKThread_priv->Running )
	{
		ntStatus = -1;
	}
		
	*pNumObjects = nLoopCount;
	return ntStatus;
}


