KeGetThreadLocks

Requirements:
Platform: Windows 7 
It is only developed and tested under this OS.
It is only tested as an x64 build.

Purpose:
The purpose of the functionality is to figure out the locking objects bound to a windows kernel thread
which is deadlocking.

KeGetThreadLocks implements a function that is returning the wait blocks of a windows kernel thread.
To identify the waiting objects you can use the object member of a block like this: pWaitBlocks->Object


Remarks:
The function KeGetThreadLocks() is NOT delivering a pointer to wait block array in the KThread 
structure, but a copy(!) of the objects.

Assume that a waiting object is at address 0xfffff88003de85b0.
Using the kernel debugger you can retrieve more information with the dt command like this
dt 0xfffff88003de85b0
EventArray
[11]    +0x000 Header           : _DISPATCHER_HEADER
0: kd> dx -id 0,0,fffffa800c7725f0 -r1 (*((Hellodriver!_DISPATCHER_HEADER *)0xfffff88003de85b0))
(*((Hellodriver!_DISPATCHER_HEADER *)0xfffff88003de85b0))                 [Type: _DISPATCHER_HEADER]
    [+0x000] Type             : 0x0 [Type: unsigned char]
    [+0x001] TimerControlFlags : 0x0 [Type: unsigned char]
    [+0x001 ( 0: 0)] Absolute         : 0x0 [Type: unsigned char]
    [+0x001 ( 1: 1)] Coalescable      : 0x0 [Type: unsigned char]
    [+0x001 ( 2: 2)] KeepShifting     : 0x0 [Type: unsigned char]
    [+0x001 ( 7: 3)] EncodedTolerableDelay : 0x0 [Type: unsigned char]
    [+0x001] Abandoned        : 0x0 [Type: unsigned char]
    [+0x001] Signalling       : 0x0 [Type: unsigned char]
    [+0x002] ThreadControlFlags : 0x6 [Type: unsigned char]
    [+0x002 ( 0: 0)] CpuThrottled     : 0x0 [Type: unsigned char]
    [+0x002 ( 1: 1)] CycleProfiling   : 0x1 [Type: unsigned char]
    [+0x002 ( 2: 2)] CounterProfiling : 0x1 [Type: unsigned char]
    [+0x002 ( 7: 3)] Reserved         : 0x0 [Type: unsigned char]
    [+0x002] Hand             : 0x6 [Type: unsigned char]
    [+0x002] Size             : 0x6 [Type: unsigned char]
    [+0x003] TimerMiscFlags   : 0x0 [Type: unsigned char]
    [+0x003 ( 5: 0)] Index            : 0x0 [Type: unsigned char]
    [+0x003 ( 6: 6)] Inserted         : 0x0 [Type: unsigned char]
    [+0x003 ( 7: 7)] Expired          : 0x0 [Type: unsigned char]
    [+0x003] DebugActive      : 0x0 [Type: unsigned char]
    [+0x003 ( 0: 0)] ActiveDR7        : 0x0 [Type: unsigned char]
    [+0x003 ( 1: 1)] Instrumented     : 0x0 [Type: unsigned char]
    [+0x003 ( 5: 2)] Reserved2        : 0x0 [Type: unsigned char]
    [+0x003 ( 6: 6)] UmsScheduled     : 0x0 [Type: unsigned char]
    [+0x003 ( 7: 7)] UmsPrimary       : 0x0 [Type: unsigned char]
    [+0x003] DpcActive        : 0x0 [Type: unsigned char]
    [+0x000] Lock             : 393216 [Type: long]
    [+0x004] SignalState      : 0 [Type: long]
    [+0x008] WaitListHead     [Type: _LIST_ENTRY]
