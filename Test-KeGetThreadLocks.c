#include "KeGetThreadLocks.h"
#include <ntddk.h>
#include <wdm.h>
#include <aux_klib.h>
#include <crtdbg.h>
#include <stddef.h>
#pragma warning(disable : 4995)
#pragma warning(disable : 4996)
//ExQueueWorkItem': was declared deprecated
//ExInitializeWorkItem': name was marked as #pragma deprecated

#define MAX_EVENTS (10)
#define NELEM(x) ( sizeof(x)/sizeof(*x))
#define TIMEOUT_1SEC (-10000000LL)//(-100000000LL) == 10sec
#define TIMEOUT_2SEC ( 2LL*TIMEOUT_1SEC)

                           
#define USE_MULTIPLEOBJECTS // if this is not defined, KeWaitForSingleObject is used
#define TAG '_BB_'

static void Unload(IN PDRIVER_OBJECT DriverObject);
static VOID BlockingRoutine(PVOID pContext);
static VOID NTAPI TimerDPC
(
	IN  PKDPC   Dpc,
	IN  PVOID   DeferredContext,
	IN  PVOID   SystemArgument1,
	IN  PVOID   SystemArgument2
);
static VOID MyWorkItemCheckingRoutine(PVOID pContext);

static KEVENT event;//simple check if KeWaitForSingleObject is working
static HANDLE hThread;
static KTIMER              m_TimerEvent = { 0 };           // timer
static KDPC                m_Dpc;                  // DPC for timer

static PWORK_QUEUE_ITEM workItemChecking = (PWORK_QUEUE_ITEM)NULL;
static KEVENT EventArray[MAX_EVENTS+1];
static PKTHREAD pKThread;
static char isUnloading = 0;//atomic

static PKWAIT_BLOCK pKWaitBlock;
static PVOID pEventBlocking[MAX_EVENTS + 1];
static PKWAIT_BLOCK pWaitBlock = NULL;
static INT CheckNOTOK = 0, CheckOK = 0;
/*****************************************************************************************************/
static VOID MyWorkItemCheckingRoutine(PVOID pContext)
{
	static LARGE_INTEGER   bMillisecIn100ns;
	int i;
	static KWAIT_BLOCK pWaitBlocks[MAX_EVENTS * 2];
	static LONG uNumEvents = NELEM(pWaitBlocks);
	NTSTATUS ntStatus = (NTSTATUS)~0;

	


	DbgPrintEx( 0,0, "%s\n", __FUNCTION__ );
	if (isUnloading) 
	{
		return;
	}
	
	uNumEvents = NELEM(pWaitBlocks);
	for (i = 0; i < NELEM(pWaitBlocks); ++i)
	{
		RtlFillMemory(&pWaitBlocks[i], sizeof(pWaitBlocks[i]), 0);
	}
	ntStatus = KeGetThreadLocks(pKThread, &pWaitBlocks[0], &uNumEvents);
	DbgPrintEx( 0,0, "ntStatus %u uNumEvents %u\n", ntStatus, uNumEvents);
	if (0 == ntStatus)
	{
		for (i = 0; i < (int)uNumEvents; ++i)
		{
			DbgPrintEx( 0,0, "[%02d] %p\n", i, pWaitBlocks[i].Object);
		}
	}

		

		
#ifdef USE_MULTIPLEOBJECTS	
		ASSERT(i == MAX_EVENTS);
		KeSetEvent(&EventArray[0], 0, FALSE);
#else
		ASSERT(i == 1);
		KeSetEvent(&event, 0, FALSE);
#endif
		if (0 == ntStatus)
		{

#ifdef USE_MULTIPLEOBJECTS	
			for (i = 0; i < (int)uNumEvents; ++i)
			{
				if (pEventBlocking[i] != (PVOID)pWaitBlocks[i].Object)
#else
		    	if ((PVOID) & event != (PVOID)pWaitBlocks[i].Object)
#endif
			    {
				   CheckNOTOK += 1;
			    }
			    else
			    {
				   CheckOK += 1;
			     }
		    }

     		DbgPrintEx( 0,0, "Check OK %d Check NOT OK %d\n",CheckOK,CheckNOTOK);

		}

}
/*****************************************************************************************************/
static VOID BlockingRoutine(PVOID pContext)
{
	NTSTATUS ret;
	int i;
	
	
	for (; 0 == isUnloading; )
	{
		DbgPrintEx( 0,0, "%s\n", __FUNCTION__);
#ifdef USE_MULTIPLEOBJECTS	
		pWaitBlock = ExAllocatePoolWithTag(NonPagedPool, sizeof(*pWaitBlock) * (MAX_EVENTS + 1), TAG);
		memset(pWaitBlock, 0x0, sizeof(*pWaitBlock) * (MAX_EVENTS + 1));


		for (i = 0; i < MAX_EVENTS; ++i)
		{
			KeInitializeEvent(&EventArray[i], NotificationEvent, FALSE);
			pEventBlocking[i] = &EventArray[i];
		}

#endif
		pKThread = KeGetCurrentThread();
		DbgPrintEx( 0,0, "KeGetCurrentThread %p\n", pKThread);

#ifdef USE_MULTIPLEOBJECTS	
		DbgPrintEx( 0,0, "calling KeWaitForMultipleObject\n");
#else
		DbgPrintEx( 0,0, "calling KeWaitForSingleObject\n");
#endif


		KeResetEvent(&event);

		DbgPrintEx( 0,0, "Searching for %p %p\n", event, &event);
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
		ret = KeWaitForSingleObject(
			&event,
			Executive, // Waiting for reason of a driver
			KernelMode, // Waiting in kernel mode
			FALSE, // alert
			0);

#endif
		
		
#ifdef USE_MULTIPLEOBJECTS	
		DbgPrintEx( 0,0, "KeWaitForMultipleObjects %d\n", ret);
#else
		DbgPrintEx( 0,0, "KeWaitForSingleObject %d\n", ret);
#endif
	}

    
	PsTerminateSystemThread(0);
	

}
/*****************************************************************************************************/
static VOID NTAPI TimerDPC
(
	IN  PKDPC   Dpc,
	IN  PVOID   DeferredContext,
	IN  PVOID   SystemArgument1,
	IN  PVOID   SystemArgument2
)
{	BOOLEAN fRet;
    LARGE_INTEGER   aMillisecIn100ns;

	UNREFERENCED_PARAMETER(Dpc);
	UNREFERENCED_PARAMETER(DeferredContext);
	UNREFERENCED_PARAMETER(SystemArgument1);
	UNREFERENCED_PARAMETER(SystemArgument2);


	DbgPrintEx( 0,0, "%s\n", __FUNCTION__ );

	if (!isUnloading)
	{
		if (workItemChecking)
		{
			ExQueueWorkItem(workItemChecking, DelayedWorkQueue);
		}

		aMillisecIn100ns.QuadPart = TIMEOUT_1SEC;
		fRet = KeSetTimer(&m_TimerEvent, aMillisecIn100ns, &m_Dpc);
		DbgPrintEx( 0,0,"KeSetTimer %d\n", fRet);
	}
}
/*****************************************************************************************************/
static void Unload(IN PDRIVER_OBJECT DriverObject)
{
	BOOLEAN fRet;
	LARGE_INTEGER delay;
	delay.QuadPart = TIMEOUT_2SEC;


	isUnloading = 1;
	
#ifdef USE_MULTIPLEOBJECTS	
	KeSetEvent(&EventArray[0], 0, FALSE);
#else
	KeSetEvent(&event, 0, FALSE);
#endif

    fRet = KeCancelTimer(&m_TimerEvent);
	DbgPrintEx( 0,0, "KeCancelTimer() %d\n", fRet);


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
	DbgPrintEx( 0,0, "driver unload\r\n");
	
}
/*****************************************************************************************************/
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,
	IN PUNICODE_STRING RegistryPath)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	BOOLEAN fRet;
	LARGE_INTEGER   aMillisecIn100ns;

	DbgPrintEx( 0,0, "%s %s %s\n", __FUNCTION__, __TIME__, __DATE__ );


	DriverObject->DriverUnload = Unload;


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
	DbgPrintEx( 0,0, "KeSetTimer %s fRet %d\n", __FUNCTION__, fRet);



	workItemChecking = (PWORK_QUEUE_ITEM)ExAllocatePoolWithTag(NonPagedPool, sizeof(WORK_QUEUE_ITEM),TAG);
	ExInitializeWorkItem(workItemChecking, MyWorkItemCheckingRoutine, NULL);
	

	memset(&EventArray, 0, sizeof(EventArray));
	
	ntStatus = PsCreateSystemThread(
		&hThread,               
		THREAD_ALL_ACCESS,      
		NULL,                   
		NULL,                   
		NULL,                   
		BlockingRoutine,       
		NULL                    
	);
	
	DbgPrintEx( 0,0, "%s PsCreateSystemThread %d\n",__FUNCTION__, ntStatus);

	return ntStatus;


}

