#if !defined(__DISK_H__)
#define __DISK_H__

//#include <dbt.h>		// For DeviceChange
#include <winioctl.h>	// For DeviceIOCtl

#define DISABLE_USE_CHS

//////////////////////////////////////////////////////////////////////////
namespace DISK
{
	// IOCTL 控制码
#define IOCTL_STORAGE_QUERY_PROPERTY   CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)

	// 存储设备的总线类型
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
	
	// 查询存储设备属性的类型
	typedef enum _STORAGE_QUERY_TYPE
	{
		PropertyStandardQuery = 0,  // 读取描述
		PropertyExistsQuery,        // 测试是否支持
		PropertyMaskQuery,          // 读取指定的描述
		PropertyQueryMaxDefined		// 验证数据
	} STORAGE_QUERY_TYPE, *PSTORAGE_QUERY_TYPE;
	
	// 查询存储设备还是适配器属性
	typedef enum _STORAGE_PROPERTY_ID
	{
		StorageDeviceProperty = 0,	// 查询设备属性
		StorageAdapterProperty      // 查询适配器属性
	} STORAGE_PROPERTY_ID, *PSTORAGE_PROPERTY_ID;
	
	// 查询属性输入的数据结构
	typedef struct _STORAGE_PROPERTY_QUERY
	{
		STORAGE_PROPERTY_ID PropertyId;     // 设备/适配器
		STORAGE_QUERY_TYPE QueryType;       // 查询类型 
		UCHAR AdditionalParameters[1];      // 额外的数据(动态分配，仅定义1个字节)
	} STORAGE_PROPERTY_QUERY, *PSTORAGE_PROPERTY_QUERY;
	
	// 查询属性输出的数据结构
	typedef struct _STORAGE_DEVICE_DESCRIPTOR
	{
		ULONG Version;						// 版本
		ULONG Size;							// 结构大小
		UCHAR DeviceType;					// 设备类型
		UCHAR DeviceTypeModifier;			// SCSI-2额外的设备类型
		BOOLEAN RemovableMedia;				// 是否可移动
		BOOLEAN CommandQueueing;			// 是否支持命令队列
		ULONG VendorIdOffset;				// 厂家设定值的偏移
		ULONG ProductIdOffset;				// 产品ID的偏移
		ULONG ProductRevisionOffset;		// 产品版本的偏移
		ULONG SerialNumberOffset;			// 序列号的偏移
		STORAGE_BUS_TYPE BusType;			// 总线类型
		ULONG RawPropertiesLength;			// 额外的属性数据长度
		UCHAR RawDeviceProperties[512];		// 额外的属性数据
	} STORAGE_DEVICE_DESCRIPTOR, *PSTORAGE_DEVICE_DESCRIPTOR;

	typedef struct _WORD_CHS {
		WORD	c;	// Cylinder
		WORD	h;	// Head
		WORD	s;	// Sector
	} WORD_CHS, *PWORD_CHS;
	
	// 禁止编译器变量对齐优化，保证 MBR 结构体大小固定
#pragma pack(push)
#pragma pack(1)

	typedef struct _BYTE_CHS {
		BYTE	byHead;
		BYTE	byLowSC;
		BYTE	byHighSC;
	} BYTE_CHS, *PBYTE_CHS;
	
	// 分区表项(16字节)
	typedef struct _DPT_ITEM {
		BYTE byState;		// 分区状态, 0 = 未激活, 0x80 = 激活
		
		BYTE_CHS bchsStartCHS;
// 		BYTE byStartHead;	// 分区起始磁头号
// 		WORD wStartSC;		// 分区起始扇区和柱面号, 底字节的低6位为扇区号,
							// 高2位为柱面号的 8、9 位, 高字节为柱面号的低 8 位
		
		BYTE byType;		// 分区文件系统: 0x00 未用、0x05/0x0F 扩展分区
		BYTE_CHS bchsEndCHS;
// 		BYTE byEndHead;		// 分区结束磁头号
// 		WORD wEndSC;		// 分区结束扇区和柱面号, 定义同前
		
		DWORD dwRelative;	// 在线性寻址方式下的分区相对扇区地址
		DWORD dwSectors;	// 分区大小 (总扇区数)
	} DPT_ITEM, *PDPT_ITEM;
	
	// 主引导扇区(512字节)
	typedef struct _MBR_512 {
		BYTE	 mbr[446];	// 引导代码
		DPT_ITEM dpt[4];	// 分区表
		WORD     id;		// 引导扇区标志
	} MBR, *PMBR;
	
	/*
	// 分区引导扇区(512字节)
	typedef struct _DBR_512 {
		BYTE	 Jmp[3];	// 跳转指令
		TCHAR	 Oem[8];	// 操作系统厂商标识及版本号
		BYTE	 BPB[19];	// BPB
		BYTE	 Dbr[480];	// 引导代码
		WORD     id;		// 引导扇区标志
	} DBR, *PDBR;
	*/
	
	// 恢复编译器变量对齐优化
#pragma pack(pop)
	
	
	// 逻辑分区空间信息
	typedef struct _FREE_SPACE {
		DWORD	dwSectorsPerCluster;		// 每簇扇区数
		DWORD	dwBytesPerSector;			// 每扇区字节数
		DWORD	dwNumberOfFreeClusters;		// 剩余簇数
		DWORD	dwTotalNumberOfClusters;	// 总共簇数
	} FREE_SPACE, *PFREE_SPACE;
	
	//////////////////////////////////////////////////////////////////////////
	
	// 打开设备(简化CreateFile参数行)
	BOOL OpenDevice(HANDLE &hDevice, LPCTSTR szPathName,
		DWORD dwDesiredAccess=GENERIC_READ|GENERIC_WRITE,
		DWORD dwShareMode=FILE_SHARE_READ|FILE_SHARE_WRITE,
		DWORD dwCreationDisposition=OPEN_EXISTING,
		DWORD dwFlagsAndAttributes=FILE_ATTRIBUTE_NORMAL,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes=(LPSECURITY_ATTRIBUTES)NULL,
		HANDLE hTemplateFile=NULL);
	
	// 打开磁盘文件(简化CreateFile参数行)
	// 注意与 OpenDevice 参数顺不同
	BOOL OpenFile(HANDLE &hFile, LPCTSTR szPathName,
		DWORD dwCreationDisposition=OPEN_EXISTING,
		DWORD dwDesiredAccess=GENERIC_READ|GENERIC_WRITE,
		DWORD dwShareMode=0,
		DWORD dwFlagsAndAttributes=FILE_ATTRIBUTE_NORMAL,
		LPSECURITY_ATTRIBUTES lpSecurityAttributes=(LPSECURITY_ATTRIBUTES)NULL,
		HANDLE hTemplateFile=NULL);

	BOOL CloseHandle(HANDLE &hDevOrFileHandle);
	
	inline BOOL IsValidHandle(HANDLE &hDevice) { return NULL != hDevice && INVALID_HANDLE_VALUE != hDevice; }
	
	// 识别 USB 移动硬盘
	BOOL IsUsb(HANDLE &hDevice);
	
	// 磁盘属性
	BOOL GetDiskProperty(HANDLE &hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc);
	// 磁盘大小，单位M
	DWORD GetDiskSize(HANDLE &hDevice, PDISK_GEOMETRY pGeometry=NULL);
	// 磁盘分区表
	BOOL GetDirveLayout(HANDLE &hDevice, CUIntArray &aDPT, LPCTSTR szFileName=NULL);
	
	// 分区编号
	BOOL GetDriveNumber(HANDLE &hDevice, PSTORAGE_DEVICE_NUMBER pDevNum);
	BOOL GetDriveNumber(const CString &sDrv, PSTORAGE_DEVICE_NUMBER pDevNum);
	// 分区大小，单位M
	DWORD GetDriveSize(const CString &sDrv, PFREE_SPACE pFs=NULL);
	// 分区文件系统
	CString GetFileSystem(const CString &sDrv);
	
	// 设备名
	CString GetDiskName(int i);
	CString GetDriveName(const TCHAR &ch);
	
	//////////////////////////////////////////////////////////////////////////

	// 读写引导扇区
	BOOL ReadBootSector(HANDLE &hDevice, const CString &sFileName, DWORD dwSectors=63,
		DWORD dwStartSector=0, DWORD dwBytePerSector=512, DWORD dwOffsetSectors=0);
	BOOL WriteBootSector(HANDLE &hDevice, const CString &sFileName, DWORD dwSectors=63,
		DWORD dwStartSector=0, DWORD dwBytePerSector=512, DWORD dwOffsetSectors=0);
	
	// 按扇区读写磁盘或文件
	BOOL ReadSectors(HANDLE &hDevice, LPVOID pBuf, DWORD dwSectors=1,
		DWORD dwStartSector=0, DWORD dwBytePerSector=512);
	BOOL WriteSectors(HANDLE &hDevice, LPVOID pBuf, DWORD dwSectors=1,
		DWORD dwStartSector=0, DWORD dwBytePerSector=512);
	
	// 移动文件指针
	BOOL SeekLargeFile(HANDLE &hFile, __int64 liOffset, DWORD dwMoveMethod);
	
	// 16 查看一个扇区数据(512字节)
	CString PrintSector(const BYTE *pBuf, DWORD dwOffset=0,
		const CString sEnter=_T("\n"), BOOL bTailEnter=TRUE);

#if !defined(DISABLE_USE_CHS)

	// 按 CHS 参数读写磁盘
	BOOL AbsRead(HANDLE &hDevice, LPVOID pBuf, WORD c, WORD h, WORD s, DWORD dwSectors, DWORD dwBytePerSector=512);
	BOOL ABSWrite(HANDLE &hDevice, LPVOID pBuf, WORD c, WORD h, WORD s, DWORD dwSectors, DWORD dwBytePerSector=512);

	// CHS 与 LBA 转换
	DWORD Chs2Lba(WORD c, WORD h, WORD s,	// 柱面、磁道、扇区
		DWORD PS=63, DWORD PH=255,			// 每磁道扇区数、每柱面磁道数
		WORD sc=0, WORD sh=0, WORD ss=0);			// 起始柱面、磁道、扇区
	
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
# 文件系统标志
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
