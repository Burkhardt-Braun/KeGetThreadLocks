#include "KeGetThreadLocks.h"
#include <ntddk.h>
#include <wdm.h>
#include <aux_klib.h>
#include <crtdbg.h>
#include <stddef.h>
/*
* 0: kd> dt _KTHREAD
ntdll!_KTHREAD
   +0x000 Header           : _DISPATCHER_HEADER
   +0x018 CycleTime        : Uint8B
   +0x020 QuantumTarget    : Uint8B
   +0x028 InitialStack     : Ptr64 Void
   +0x030 StackLimit       : Ptr64 Void
   +0x038 KernelStack      : Ptr64 Void
   +0x040 ThreadLock       : Uint8B
   +0x048 WaitRegister     : _KWAIT_STATUS_REGISTER
   +0x049 Running          : UChar
   +0x04a Alerted          : [2] UChar
   +0x04c KernelStackResident : Pos 0, 1 Bit
   +0x04c ReadyTransition  : Pos 1, 1 Bit
   +0x04c ProcessReadyQueue : Pos 2, 1 Bit
   +0x04c WaitNext         : Pos 3, 1 Bit
   +0x04c SystemAffinityActive : Pos 4, 1 Bit
   +0x04c Alertable        : Pos 5, 1 Bit
   +0x04c GdiFlushActive   : Pos 6, 1 Bit
   +0x04c UserStackWalkActive : Pos 7, 1 Bit
   +0x04c ApcInterruptRequest : Pos 8, 1 Bit
   +0x04c ForceDeferSchedule : Pos 9, 1 Bit
   +0x04c QuantumEndMigrate : Pos 10, 1 Bit
   +0x04c UmsDirectedSwitchEnable : Pos 11, 1 Bit
   +0x04c TimerActive      : Pos 12, 1 Bit
   +0x04c SystemThread     : Pos 13, 1 Bit
   +0x04c Reserved         : Pos 14, 18 Bits
   +0x04c MiscFlags        : Int4B
   +0x050 ApcState         : _KAPC_STATE
   +0x050 ApcStateFill     : [43] UChar
   +0x07b Priority         : Char
   +0x07c NextProcessor    : Uint4B
   +0x080 DeferredProcessor : Uint4B
   +0x088 ApcQueueLock     : Uint8B
   +0x090 WaitStatus       : Int8B
   +0x098 WaitBlockList    : Ptr64 _KWAIT_BLOCK
   +0x0a0 WaitListEntry    : _LIST_ENTRY
   +0x0a0 SwapListEntry    : _SINGLE_LIST_ENTRY
   +0x0b0 Queue            : Ptr64 _KQUEUE
   +0x0b8 Teb              : Ptr64 Void
   +0x0c0 Timer            : _KTIMER
   +0x100 AutoAlignment    : Pos 0, 1 Bit
   +0x100 DisableBoost     : Pos 1, 1 Bit
   +0x100 EtwStackTraceApc1Inserted : Pos 2, 1 Bit
   +0x100 EtwStackTraceApc2Inserted : Pos 3, 1 Bit
   +0x100 CalloutActive    : Pos 4, 1 Bit
   +0x100 ApcQueueable     : Pos 5, 1 Bit
   +0x100 EnableStackSwap  : Pos 6, 1 Bit
   +0x100 GuiThread        : Pos 7, 1 Bit
   +0x100 UmsPerformingSyscall : Pos 8, 1 Bit
   +0x100 VdmSafe          : Pos 9, 1 Bit
   +0x100 UmsDispatched    : Pos 10, 1 Bit
   +0x100 ReservedFlags    : Pos 11, 21 Bits
   +0x100 ThreadFlags      : Int4B
   +0x104 Spare0           : Uint4B
   +0x108 WaitBlock        : [4] _KWAIT_BLOCK
   +0x108 WaitBlockFill4   : [44] UChar
   +0x134 ContextSwitches  : Uint4B
   +0x108 WaitBlockFill5   : [92] UChar
   +0x164 State            : UChar
   +0x165 NpxState         : Char
   +0x166 WaitIrql         : UChar
   +0x167 WaitMode         : Char
   +0x108 WaitBlockFill6   : [140] UChar
   +0x194 WaitTime         : Uint4B
   +0x108 WaitBlockFill7   : [168] UChar
   +0x1b0 TebMappedLowVa   : Ptr64 Void
   +0x1b8 Ucb              : Ptr64 _UMS_CONTROL_BLOCK
   +0x108 WaitBlockFill8   : [188] UChar
   +0x1c4 KernelApcDisable : Int2B
   +0x1c6 SpecialApcDisable : Int2B
   +0x1c4 CombinedApcDisable : Uint4B
   +0x1c8 QueueListEntry   : _LIST_ENTRY
   +0x1d8 TrapFrame        : Ptr64 _KTRAP_FRAME
   +0x1e0 FirstArgument    : Ptr64 Void
   +0x1e8 CallbackStack    : Ptr64 Void
   +0x1e8 CallbackDepth    : Uint8B
   +0x1f0 ApcStateIndex    : UChar
   +0x1f1 BasePriority     : Char
   +0x1f2 PriorityDecrement : Char
   +0x1f2 ForegroundBoost  : Pos 0, 4 Bits
   +0x1f2 UnusualBoost     : Pos 4, 4 Bits
   +0x1f3 Preempted        : UChar
   +0x1f4 AdjustReason     : UChar
   +0x1f5 AdjustIncrement  : Char
   +0x1f6 PreviousMode     : Char
   +0x1f7 Saturation       : Char
   +0x1f8 SystemCallNumber : Uint4B
   +0x1fc FreezeCount      : Uint4B
   +0x200 UserAffinity     : _GROUP_AFFINITY
   +0x210 Process          : Ptr64 _KPROCESS
   +0x218 Affinity         : _GROUP_AFFINITY
   +0x228 IdealProcessor   : Uint4B
   +0x22c UserIdealProcessor : Uint4B
   +0x230 ApcStatePointer  : [2] Ptr64 _KAPC_STATE
   +0x240 SavedApcState    : _KAPC_STATE
   +0x240 SavedApcStateFill : [43] UChar
   +0x26b WaitReason       : UChar
   +0x26c SuspendCount     : Char
   +0x26d Spare1           : Char
   +0x26e CodePatchInProgress : UChar
   +0x270 Win32Thread      : Ptr64 Void
   +0x278 StackBase        : Ptr64 Void
   +0x280 SuspendApc       : _KAPC
   +0x280 SuspendApcFill0  : [1] UChar
   +0x281 ResourceIndex    : UChar
   +0x280 SuspendApcFill1  : [3] UChar
   +0x283 QuantumReset     : UChar
   +0x280 SuspendApcFill2  : [4] UChar
   +0x284 KernelTime       : Uint4B
   +0x280 SuspendApcFill3  : [64] UChar
   +0x2c0 WaitPrcb         : Ptr64 _KPRCB
   +0x280 SuspendApcFill4  : [72] UChar
   +0x2c8 LegoData         : Ptr64 Void
   +0x280 SuspendApcFill5  : [83] UChar
   +0x2d3 LargeStack       : UChar
   +0x2d4 UserTime         : Uint4B
   +0x2d8 SuspendSemaphore : _KSEMAPHORE
   +0x2d8 SuspendSemaphorefill : [28] UChar
   +0x2f4 SListFaultCount  : Uint4B
   +0x2f8 ThreadListEntry  : _LIST_ENTRY
   +0x308 MutantListHead   : _LIST_ENTRY
   +0x318 SListFaultAddress : Ptr64 Void
   +0x320 ReadOperationCount : Int8B
   +0x328 WriteOperationCount : Int8B
   +0x330 OtherOperationCount : Int8B
   +0x338 ReadTransferCount : Int8B
   +0x340 WriteTransferCount : Int8B
   +0x348 OtherTransferCount : Int8B
   +0x350 ThreadCounters   : Ptr64 _KTHREAD_COUNTERS
   +0x358 XStateSave       : Ptr64 _XSTATE_SAVE

*/
#pragma warning(disable : 4995)
#pragma warning(disable : 4996)
//ExQueueWorkItem': was declared deprecated
//ExInitializeWorkItem': name was marked as #pragma deprecated

#define MAX_EVENTS (10)
#define NELEM(x) ( sizeof(x)/sizeof(*x))
#define TIMEOUT_1SEC (-10000000LL)//(-100000000LL) == 10sec
#define TIMEOUT_2SEC ( 2LL*TIMEOUT_1SEC)

                           
//#define USE_MULTIPLEOBJECTS
#define TAG '_BB_'

typedef unsigned char BYTE;
#if 0
typedef struct _KWAIT_STATUS_REGISTER {
	/*0x000*/ BYTE Flags;            // UChar
	/*0x000*/ BYTE State : 2;            // Pos 0, 2 Bits
	/*0x000*/ BYTE Affinity : 1;         // Pos 2, 1 Bit
	/*0x000*/ BYTE Priority : 1;         // Pos 3, 1 Bit
	/*0x000*/ BYTE Apc : 1;              // Pos 4, 1 Bit
	/*0x000*/ BYTE UserApc : 1;          // Pos 5, 1 Bit
	/*0x000*/ BYTE Alert : 1;            // Pos 6, 1 Bit
	/*0x000*/ BYTE Unused : 1;           // Pos 7, 1 Bit
} KWAIT_STATUS_REGISTER, * PKWAIT_STATUS_REGISTER;
#endif
typedef struct {
	char FillByte0[0x48];
	BYTE/*KWAIT_STATUS_REGISTER*/ WaitRegister;
	BYTE Running;
	char FillByte1[0x90 - 0x48 - 8/*sizeof(KWAIT_STATUS_REGISTER)*/];
	INT64 WaitStatus; //Int8B signed 64bit ...This WaitStatus describes the reason why the thread was awaken from its wait state. 
	PKWAIT_BLOCK WaitBlockList;
	char FillByte2[0x70 - 8];
	KWAIT_BLOCK WaitBlock[4];
	/*uncomplete*/
}KThreadPriv_type;


KEVENT event;
HANDLE hThread;




//extern void* PoQueryWatchdogTime;
KTIMER              m_TimerEvent = { 0 };           // timer
KDPC                m_Dpc;                  // DPC for timer
LARGE_INTEGER   aMillisecIn100ns;

LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
LARGE_INTEGER Frequency;
BOOLEAN	fRet;




PWORK_QUEUE_ITEM workItemChecking = (PWORK_QUEUE_ITEM)NULL;
KEVENT EventArray[MAX_EVENTS+1];
PKTHREAD pKThread;
char isUnloading = 0;//atomic




PKWAIT_BLOCK pKWaitBlock;
static PVOID pEventBlocking[MAX_EVENTS + 1];
PKWAIT_BLOCK pWaitBlock = NULL;

INT CheckNOTOK = 0, CheckOK = 0;

/*****************************************************************************************************/
PVOID EventFound(PVOID pObject,PUINT32 pIndex)
{	int i;

	for (i = 0; i < MAX_EVENTS; ++i)
	{
		if (pObject && pObject == &EventArray[i])
		{
			*pIndex = (UINT32)i;
			return(pObject);
		}
	}
	return NULL;
}
/*****************************************************************************************************/
VOID MyWorkItemCheckingRoutine(PVOID pContext)
{
	static LARGE_INTEGER   bMillisecIn100ns;
	int i;
	static KWAIT_BLOCK pEvents[MAX_EVENTS * 2];
	static LONG uNumEvents = NELEM(pEvents);
	NTSTATUS ntStatus = (NTSTATUS)~0;
	LONG lRet;
	static UINT32 fooCallCount = 0;


	DbgPrintEx(0, 0, "Hellodriver: %s [%u]\n", __FUNCTION__, fooCallCount++);
	if (isUnloading) {

		return;
	}
	
	uNumEvents = NELEM(pEvents);
	for (i = 0; i < NELEM(pEvents); ++i)
	{
		RtlFillMemory(&pEvents[i], sizeof(pEvents[i]), 0);
	}
	ntStatus = KeGetThreadLocks(pKThread, &pEvents[0], &uNumEvents);
		DbgPrintEx(0, 0, "ntStatus %u uNumEvents %u\n", ntStatus, uNumEvents);
		if (0 == ntStatus)
		{
			for (i = 0; i < (int)uNumEvents; ++i)
			{
				DbgPrintEx(0, 0, "[%02d] %p\n", i, pEvents[i]);
			}
		}
#ifdef USE_MULTIPLEOBJECTS	
		KeSetEvent(&EventArray[0], 0, FALSE);
#else
		KeSetEvent(&event, 0, FALSE);
#endif

#ifdef USE_MULTIPLEOBJECTS	
#else

		if (0 == ntStatus)
		{
			ASSERT(i == 1);
//			ASSERT((PVOID)&event == (PVOID)pEvents[0]);
			if ((PVOID) & event != (PVOID)pEvents[0].Object)
			{
				CheckNOTOK += 1;
			}
			else
			{
				CheckOK += 1;
			}
				
			DbgPrintEx(0, 0, "Check OK %d Check NOT OK %d\n",CheckOK,CheckNOTOK);

		}
#endif

		
		
}
/*****************************************************************************************************/
VOID BlockingRoutine(PVOID pContext)
{
	NTSTATUS ret = 0xbb;
	static LARGE_INTEGER   bMillisecIn100ns;
	int i;
	static UINT32 fooCallCount = 0;
	
	for (; 0 == isUnloading; )
	{


		DbgPrintEx(0, 0, "Hellodriver: %s [%u]\n", __FUNCTION__, fooCallCount++);
#ifdef USE_MULTIPLEOBJECTS	
		pWaitBlock = ExAllocatePoolWithTag(NonPagedPool, sizeof(*pWaitBlock) * (MAX_EVENTS + 1), TAG);
		memset(pWaitBlock, 0x0, sizeof(*pWaitBlock) * (MAX_EVENTS + 1));
		for (i = 0; i < MAX_EVENTS; ++i)
		{
			//pEvent[i] = ExAllocatePoolWithTag(NonPagedPool, sizeof(pEvent[0]) , TAG);
			KeInitializeEvent(&EventArray[i], NotificationEvent, FALSE);
			//KeInitializeEvent(&pEvent[i], NotificationEvent, FALSE);
			pEventBlocking[i] = &EventArray[i];
			//		DbgPrintEx(0, 0, "pEvent[%d] %p\n", i, pEvent[i]);
		}

#endif
		pKThread = KeGetCurrentThread();
		DbgPrintEx(0, 0, "KeGetCurrentThread %p\n", pKThread);

#ifdef USE_MULTIPLEOBJECTS	
		DbgPrintEx(0, 0, "calling KeWaitForMultipleObject\n");
#else
		DbgPrintEx(0, 0, "calling KeWaitForSingleObject\n");
#endif


		KeResetEvent(&event);


#ifdef USE_MULTIPLEOBJECTS	
		ret = KeWaitForMultipleObjects(
			MAX_EVENTS,
			(PVOID)&pEventBlocking,
			WaitAny, // Waiting for reason of a driver
			Executive, // Waiting in kernel mode
			KernelMode,
			FALSE,
			NULL,
			pWaitBlock
		);
#else
		//


		DbgPrintEx(0, 0, "Searching for %p %p\n", event, &event);
		
		ret = KeWaitForSingleObject(
			&event,
			Executive, // Waiting for reason of a driver
			KernelMode, // Waiting in kernel mode
			FALSE, // alert
			0);

#endif
		
		
#ifdef USE_MULTIPLEOBJECTS	
		DbgPrintEx(0, 0, "KeWaitForMultipleObjects %d\n", ret);
#else
		DbgPrintEx(0, 0, "KeWaitForSingleObject %d\n", ret);
#endif
	}

    
	PsTerminateSystemThread(0);
	

}
/*****************************************************************************************************/
VOID NTAPI TimerDPC
(
	IN  PKDPC   Dpc,
	IN  PVOID   DeferredContext,
	IN  PVOID   SystemArgument1,
	IN  PVOID   SystemArgument2
)
{	BOOLEAN fRet;


	DbgPrintEx(0, 0, "Hellodriver: %s\n", __FUNCTION__ );

	if (!isUnloading)
	{
		if (workItemChecking)
		{
			ExQueueWorkItem(workItemChecking, DelayedWorkQueue);
		}

		aMillisecIn100ns.QuadPart = TIMEOUT_1SEC;
		fRet = KeSetTimer(&m_TimerEvent, aMillisecIn100ns, &m_Dpc);
		DbgPrintEx(0, 0, "KeSetTimer %d\n", fRet );
	}
}
/*****************************************************************************************************/

void Unload(IN PDRIVER_OBJECT DriverObject)
{
	LARGE_INTEGER delay;
	delay.QuadPart = TIMEOUT_2SEC;


	isUnloading = 1;
	
#ifdef USE_MULTIPLEOBJECTS	
	KeSetEvent(&EventArray[0], 0, FALSE);
#else
	KeSetEvent(&event, 0, FALSE);
#endif

	__nop();

    fRet = KeCancelTimer(&m_TimerEvent);
	DbgPrintEx(0, 0, "KeCancelTimer() %d\n", fRet);


	KeDelayExecutionThread(KernelMode, FALSE, &delay);

	if (workItemChecking)
	{
		ExFreePoolWithTag(workItemChecking, TAG);
		workItemChecking = (PWORK_QUEUE_ITEM)NULL;
	}
		

	if (pWaitBlock)
	{
		ExFreePoolWithTag(pWaitBlock, TAG);
        pWaitBlock = NULL;
	}
	DbgPrintEx(0, 0, "driver unload\r\n");
	
}
/*****************************************************************************************************/



NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	/*
	PKTHREAD pThread;
	ULONG threadState;
	pThread = KeGetCurrentThread();
	threadState = (ULONG)pThread->ThreadState;
	KeQueryRuntimeThread();
	*/

	DbgPrintEx(0, 0, "%s %s %s\n", __FUNCTION__, __TIME__, __DATE__ );
	//__asm nop; __asm nop; __asm nop; __asm nop; 

	

	__nop();
	__nop();
	DriverObject->DriverUnload = Unload;
	//__asm nop; __asm nop; __asm nop; __asm nop; __asm nop;
	__nop();

	




	KeInitializeEvent(&event, NotificationEvent, FALSE);
	KeResetEvent(&event);

	KeInitializeDpc
	(
		&m_Dpc,
		&TimerDPC,
		NULL
	);

	KeInitializeTimer(&m_TimerEvent);
	aMillisecIn100ns.QuadPart = TIMEOUT_1SEC; 
	fRet = KeSetTimer(&m_TimerEvent, aMillisecIn100ns, &m_Dpc);
	DbgPrintEx(0, 0, "Hellodriver: %s fRet %d\n", __FUNCTION__, fRet);



	workItemChecking = (PWORK_QUEUE_ITEM)ExAllocatePoolWithTag(NonPagedPool, sizeof(WORK_QUEUE_ITEM),TAG);
	ExInitializeWorkItem(workItemChecking, MyWorkItemCheckingRoutine, NULL);
	

	memset(&EventArray, 0xbb, sizeof(EventArray));
	DbgPrintEx(0, 0, "second output %s %s %x\r\n", __TIME__, __DATE__, 187);
	__nop();
	
	ntStatus = PsCreateSystemThread(
		&hThread,               
		THREAD_ALL_ACCESS,      
		NULL,                   
		NULL,                   
		NULL,                   
		BlockingRoutine,       
		NULL                    
	);
	
	DbgPrintEx(0, 0, "%s PsCreateSystemThread %d\n",__FUNCTION__, ntStatus);

	return ntStatus;


}

