#if !defined(__DISK_H__)
#define __DISK_H__

//#include <dbt.h>		// For DeviceChange
#include <winioctl.h>	// For DeviceIOCtl

#define DISABLE_USE_CHS

//////////////////////////////////////////////////////////////////////////
namespace DISK
{
	// IOCTL ������
#define IOCTL_STORAGE_QUERY_PROPERTY   CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)

	// �洢�豸����������
	typedef enum _STORAGE_BUS_TYPE
	{
		BusTypeUnknown = 0x00,
		BusTypeScsi,
		BusTypeAtapi,
		BusTypeAta,
		BusType1394,
		BusTypeSsa,
		BusTypeFibre,
		BusTypeUsb,
		BusTypeRAID,
		BusTypeMaxReserved = 0x7F
	} STORAGE_BUS_TYPE, *PSTORAGE_BUS_TYPE;
	
	// ��ѯ�洢�豸���Ե�����
	typedef enum _STORAGE_QUERY_TYPE
	{
		PropertyStandardQuery = 0,  // ��ȡ����
		PropertyExistsQuery,        // �����Ƿ�֧��
		PropertyMaskQuery,          // ��ȡָ��������
		PropertyQueryMaxDefined		// ��֤����
	} STORAGE_QUERY_TYPE, *PSTORAGE_QUERY_TYPE;
	
	// ��ѯ�洢�豸��������������
	typedef enum _STORAGE_PROPERTY_ID
	{
		StorageDeviceProperty = 0,	// ��ѯ�豸����
		StorageAdapterProperty      // ��ѯ����������
	} STORAGE_PROPERTY_ID, *PSTORAGE_PROPERTY_ID;
	
	// ��ѯ������������ݽṹ
	typedef struct _STORAGE_PROPERTY_QUERY
	{
		STORAGE_PROPERTY_ID PropertyId;     // �豸/������
		STORAGE_QUERY_TYPE QueryType;       // ��ѯ���� 
		UCHAR AdditionalParameters[1];      // ���������(��̬���䣬������1���ֽ�)
	} STORAGE_PROPERTY_QUERY, *PSTORAGE_PROPERTY_QUERY;
	
	// ��ѯ������������ݽṹ
	typedef struct _STORAGE_DEVICE_DESCRIPTOR
	{
		ULONG Version;						// �汾
		ULONG Size;							// �ṹ��С
		UCHAR DeviceType;					// �豸����
		UCHAR DeviceTypeModifier;			// SCSI-2������豸����
		BOOLEAN RemovableMedia;				// �Ƿ���ƶ�
		BOOLEAN CommandQueueing;			// �Ƿ�֧���������
		ULONG VendorIdOffset;				// �����趨ֵ��ƫ��
		ULONG ProductIdOffset;				// ��ƷID��ƫ��
		ULONG ProductRevisionOffset;		// ��Ʒ�汾��ƫ��
		ULONG SerialNumberOffset;			// ���кŵ�ƫ��
		STORAGE_BUS_TYPE BusType;			// ��������
		ULONG RawPropertiesLength;			// ������������ݳ���
		UCHAR RawDeviceProperties[512];		// �������������
	} STORAGE_DEVICE_DESCRIPTOR, *PSTORAGE_DEVICE_DESCRIPTOR;

	typedef struct _WORD_CHS {
		WORD	c;	// Cylinder
		WORD	h;	// Head
		WORD	s;	// Sector
	} WORD_CHS, *PWORD_CHS;
	
	// ��ֹ���������������Ż�����֤ MBR �ṹ���С�̶�
#pragma pack(push)
#pragma pack(1)

	typedef struct _BYTE_CHS {
		BYTE	byHead;
		BYTE	byLowSC;
		BYTE	byHighSC;
	} BYTE_CHS, *PBYTE_CHS;
	
	// ��������(16�ֽ�)
	typedef struct _DPT_ITEM {
		BYTE byState;		// ����״̬, 0 = δ����, 0x80 = ����
		
		BYTE_CHS bchsStartCHS;
// 		BYTE byStartHead;	// ������ʼ��ͷ��
// 		WORD wStartSC;		// ������ʼ�����������, ���ֽڵĵ�6λΪ������,
							// ��2λΪ����ŵ� 8��9 λ, ���ֽ�Ϊ����ŵĵ� 8 λ
		
		BYTE byType;		// �����ļ�ϵͳ: 0x00 δ�á�0x05/0x0F ��չ����
		BYTE_CHS bchsEndCHS;
// 		BYTE byEndHead;		// ����������ͷ��
// 		WORD wEndSC;		// �������������������, ����ͬǰ
		
		DWORD dwRelative;	// ������Ѱַ��ʽ�µķ������������ַ
		DWORD dwSectors;	// ������С (��������)
	} DPT_ITEM, *PDPT_ITEM;
	
	// ����������(512�ֽ�)
	typedef struct _MBR_512 {
		BYTE	 mbr[446];	// ��������
		DPT_ITEM dpt[4];	// ������
		WORD     id;		// ����������־
	} MBR, *PMBR;
	
	/*
	// ������������(512�ֽ�)
	typedef struct _DBR_512 {
		BYTE	 Jmp[3];	// ��תָ��
		TCHAR	 Oem[8];	// ����ϵͳ���̱�ʶ���汾��
		BYTE	 BPB[19];	// BPB
		BYTE	 Dbr[480];	// ��������
		WORD     id;		// ����������־
	} DBR, *PDBR;
	*/
	
	// �ָ����������������Ż�
#pragma pack(pop)
	
	
	// �߼������ռ���Ϣ
	typedef struct _FREE_SPACE {
		DWORD	dwSectorsPerCluster;		// ÿ��������
		DWORD	dwBytesPerSector;			// ÿ�����ֽ���
		DWORD	dwNumberOfFreeClusters;		// ʣ�����
		DWORD	dwTotalNumberOfClusters;	// �ܹ�����
	} FREE_SPACE, *PFREE_SPACE;
	
	//////////////////////////////////////////////////////////////////////////
	
	// ���豸(��CreateFile������)
	BOOL OpenDevice(HANDLE &hDevice, LPCTSTR szPathName,
		DWORD dwDesiredAccess=GENERIC_READ|GENERIC_WRITE,
		DWORD dwShareMode=FILE_SHARE_READ|FILE_SHARE_WRITE,
		DWORD dwCreationDisposition=OPEN_EXISTING,
		DWORD dwFlagsAndAttributes=FILE_ATTRIBUTE_NORMAL,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes=(LPSECURITY_ATTRIBUTES)NULL,
		HANDLE hTemplateFile=NULL);
	
	// �򿪴����ļ�(��CreateFile������)
	// ע���� OpenDevice ����˳��ͬ
	BOOL OpenFile(HANDLE &hFile, LPCTSTR szPathName,
		DWORD dwCreationDisposition=OPEN_EXISTING,
		DWORD dwDesiredAccess=GENERIC_READ|GENERIC_WRITE,
		DWORD dwShareMode=0,
		DWORD dwFlagsAndAttributes=FILE_ATTRIBUTE_NORMAL,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes=(LPSECURITY_ATTRIBUTES)NULL,
		HANDLE hTemplateFile=NULL);

	BOOL CloseHandle(HANDLE &hDevOrFileHandle);
	
	inline BOOL IsValidHandle(HANDLE &hDevice) { return NULL != hDevice && INVALID_HANDLE_VALUE != hDevice; }
	
	// ʶ�� USB �ƶ�Ӳ��
	BOOL IsUsb(HANDLE &hDevice);
	
	// ��������
	BOOL GetDiskProperty(HANDLE &hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc);
	// ���̴�С����λM
	DWORD GetDiskSize(HANDLE &hDevice, PDISK_GEOMETRY pGeometry=NULL);
	// ���̷�����
	BOOL GetDirveLayout(HANDLE &hDevice, CUIntArray &aDPT, LPCTSTR szFileName=NULL);
	
	// �������
	BOOL GetDriveNumber(HANDLE &hDevice, PSTORAGE_DEVICE_NUMBER pDevNum);
	BOOL GetDriveNumber(const CString &sDrv, PSTORAGE_DEVICE_NUMBER pDevNum);
	// ������С����λM
	DWORD GetDriveSize(const CString &sDrv, PFREE_SPACE pFs=NULL);
	// �����ļ�ϵͳ
	CString GetFileSystem(const CString &sDrv);
	
	// �豸��
	CString GetDiskName(int i);
	CString GetDriveName(const TCHAR &ch);
	
	//////////////////////////////////////////////////////////////////////////

	// ��д��������
	BOOL ReadBootSector(HANDLE &hDevice, const CString &sFileName, DWORD dwSectors=63,
		DWORD dwStartSector=0, DWORD dwBytePerSector=512, DWORD dwOffsetSectors=0);
	BOOL WriteBootSector(HANDLE &hDevice, const CString &sFileName, DWORD dwSectors=63,
		DWORD dwStartSector=0, DWORD dwBytePerSector=512, DWORD dwOffsetSectors=0);
	
	// ��������д���̻��ļ�
	BOOL ReadSectors(HANDLE &hDevice, LPVOID pBuf, DWORD dwSectors=1,
		DWORD dwStartSector=0, DWORD dwBytePerSector=512);
	BOOL WriteSectors(HANDLE &hDevice, LPVOID pBuf, DWORD dwSectors=1,
		DWORD dwStartSector=0, DWORD dwBytePerSector=512);
	
	// �ƶ��ļ�ָ��
	BOOL SeekLargeFile(HANDLE &hFile, __int64 liOffset, DWORD dwMoveMethod);
	
	// 16 �鿴һ����������(512�ֽ�)
	CString PrintSector(const BYTE *pBuf, DWORD dwOffset=0,
		const CString sEnter=_T("\n"), BOOL bTailEnter=TRUE);

#if !defined(DISABLE_USE_CHS)

	// �� CHS ������д����
	BOOL AbsRead(HANDLE &hDevice, LPVOID pBuf, WORD c, WORD h, WORD s, DWORD dwSectors, DWORD dwBytePerSector=512);
	BOOL ABSWrite(HANDLE &hDevice, LPVOID pBuf, WORD c, WORD h, WORD s, DWORD dwSectors, DWORD dwBytePerSector=512);

	// CHS �� LBA ת��
	DWORD Chs2Lba(WORD c, WORD h, WORD s,	// ���桢�ŵ�������
		DWORD PS=63, DWORD PH=255,			// ÿ�ŵ���������ÿ����ŵ���
		WORD sc=0, WORD sh=0, WORD ss=0);			// ��ʼ���桢�ŵ�������
	
	DWORD Chs2Lba(const WORD_CHS &wchs, DWORD PS=63, DWORD PH=255, const PWORD_CHS pschs=NULL);
	DWORD Chs2Lba(const BYTE_CHS &bchs, DWORD PS=63, DWORD PH=255, const PBYTE_CHS pschs=NULL);

	VOID  Lba2Chs(DWORD lba, WORD_CHS &wchs, DWORD PS=63, DWORD PH=255, WORD sc=0, WORD sh=0, WORD ss=0);
	VOID  Lba2Chs(DWORD lba, BYTE_CHS &bchs, DWORD PS=63, DWORD PH=255, WORD sc=0, WORD sh=0, WORD ss=0);

	VOID ChsB2W(const BYTE_CHS &bchs, WORD_CHS &wchs);
	VOID ChsW2B(const WORD_CHS &wchs, BYTE_CHS &bchs);

#endif // DISABLE_USE_CHS
};

#endif // __DISK_H__
//////////////////////////////////////////////////////////////////////////


/*
# �ļ�ϵͳ��־
0 Empty
1 FAT12
2 XENIX root
3 XENIX usr
4 FAT16 <32M
5 Extended
6 FAT16
7 HPFS/NTFS
8 AIX
9 AIX bootable
a OS/2 Boot Manag
b Win95 FAT32
c Win95 FAT32 (LB e Win95 FAT16 (LB f Win95 Ext'd (LB
10 OPUS
11 Hidden FAT12
12 Compaq diagnost
16 Hidden FAT16
14 Hidden FAT16 <3
17 Hidden HPFS/NTF
18 AST Windows swa
24 NEC DOS 3c PartitionMagic
40 Venix 80286
41 PPC PReP Boot
42 SFS
4d QNX4.x
4e QNX4.x 2nd part
4f QNX4.x 3rd part
50 OnTrack DM
51 OnTrack DM6 Aux
52 CP/M
53 OnTrack DM6 Aux
54 OnTrackDM6
55 EZ-Drive
56 Golden Bow
5c Priam Edisk
61 SpeedStor
63 GNU HURD or Sys
64 Novell Netware
65 Novell Netware
70 DiskSecure Mult
75 PC/IX
80 Old Minix
81 Minix / old Lin
82 Linux swap
83 Linux
84 OS/2 hidden C:
85 Linux extended
86 NTFS volume set
87 NTFS volume set
93 Amoeba
94 Amoeba BBT
a0 IBM Thinkpad hi
a5 BSD/386
a6 OpenBSD
a7 NeXTSTEP
b7 BSDI fs
b8 BSDI swap
c1 DRDOS/sec (FAT-
c4 DRDOS/sec (FAT-
c6 DRDOS/sec (FAT-
c7 Syrinx
db CP/M / CTOS / .
e1 DOS access
e3 DOS R/O
e4 SpeedStor
eb BeOS fs
f1 SpeedStor
f4 SpeedStor
f2 DOS secondary
fe LANstep ff BBT
*/
