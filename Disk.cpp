#include "..\StdAfx.h"
#include "wnd.h"
#include "emfc.h"
#include "Disk.h"

/*
 *  打开设备
 *	C 盘 : "\\\\.\\C:"
 *	磁盘0: "\\\\?\\PhysicalDrive0"
 */
BOOL DISK::OpenDevice(HANDLE &hDevice, LPCTSTR szPathName,
				DWORD dwDesiredAccess,
				DWORD dwShareMode,
				DWORD dwCreationDisposition,
				DWORD dwFlagsAndAttributes,
				LPSECURITY_ATTRIBUTES lpSecurityAttributes,
				HANDLE hTemplateFile)
{
	hDevice = NULL;
	hDevice = CreateFile(szPathName, dwDesiredAccess,
		dwShareMode, lpSecurityAttributes, dwCreationDisposition,
		dwFlagsAndAttributes, hTemplateFile);

	return IsValidHandle(hDevice);
}

/*
 *	打开磁盘文件
 */
BOOL DISK::OpenFile(HANDLE &hFile, LPCTSTR szPathName,
				DWORD dwCreationDisposition,
				DWORD dwDesiredAccess,
				DWORD dwShareMode,
				DWORD dwFlagsAndAttributes,
				LPSECURITY_ATTRIBUTES lpSecurityAttributes,
				HANDLE hTemplateFile)
{
	hFile = NULL;
	hFile = CreateFile(szPathName, dwDesiredAccess,
		dwShareMode, lpSecurityAttributes, dwCreationDisposition,
		dwFlagsAndAttributes, hTemplateFile);

	return IsValidHandle(hFile);
}

/*
 *	关闭句柄
 */
BOOL DISK::CloseHandle(HANDLE &hDevOrFileHandle)
{
	if( IsValidHandle(hDevOrFileHandle) && ::CloseHandle(hDevOrFileHandle) )
	{
		hDevOrFileHandle = NULL;
		return TRUE;
	}
	return !IsValidHandle(hDevOrFileHandle);
}

/*
 *  防止 USB 移动硬盘被识别为固定硬盘
 *  先用 OpenDevice 打开它，再可用此函数判断
 */
BOOL DISK::IsUsb(HANDLE &hDevice)
{
	STORAGE_DEVICE_DESCRIPTOR sdd;
	sdd.Size = sizeof(STORAGE_DEVICE_DESCRIPTOR);
	
	return GetDiskProperty(hDevice, &sdd) && BusTypeUsb == sdd.BusType;
}

/*
 *	获取磁盘信息，结果在 pDevDesc 中
 */
BOOL DISK::GetDiskProperty(HANDLE &hDevice, PSTORAGE_DEVICE_DESCRIPTOR pDevDesc)
{
	STORAGE_PROPERTY_QUERY	Query;
	Query.PropertyId = StorageDeviceProperty;
	Query.QueryType  = PropertyStandardQuery;
	DWORD dwOutBytes;

	return DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&Query, sizeof(STORAGE_PROPERTY_QUERY),
		pDevDesc, pDevDesc->Size,
		&dwOutBytes, (LPOVERLAPPED)NULL);
}

/*
 *	获取硬盘容量，单位 M(仅整数部分，非精确值)
 */
DWORD DISK::GetDiskSize(HANDLE &hDevice, PDISK_GEOMETRY pGeometry)
{
	DISK_GEOMETRY dg;
	DWORD dwSize = 0;

	if( DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY,
		NULL, 0, &dg, sizeof(dg), &dwSize, (LPOVERLAPPED)NULL) )
	{
		__int64 nCylinder = *(__int64*)&dg.Cylinders;
		
		dwSize = (DWORD)(nCylinder
			* dg.TracksPerCylinder
			* dg.SectorsPerTrack
			* dg.BytesPerSector
			/ 1024 / 1024 );

		if( NULL != pGeometry )
			CopyMemory(pGeometry, &dg, sizeof(dg));
	}

	return dwSize;
}

/*
 *	分区表信息，生成一个数组 aDPT
 *  以 windows 分区号为索引，对应元素的值-1 为 grub 分区号
 */
BOOL DISK::GetDirveLayout(HANDLE &hDevice, CUIntArray &aDPT, LPCTSTR szFileName)
{
	aDPT.RemoveAll();
	aDPT.Add(0);

	// 分配 32 个分区信息单元
	DWORD dwSize = sizeof(DRIVE_LAYOUT_INFORMATION) - 1
		+ 32 * sizeof(PARTITION_INFORMATION);

	PDRIVE_LAYOUT_INFORMATION pdli = (PDRIVE_LAYOUT_INFORMATION)new BYTE[dwSize];
	if( NULL == pdli )
		return FALSE;
	ZeroMemory(pdli, dwSize);
	
	// 获取信息
	DWORD dwBytesReturned = 0;
	BOOL bRet = DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_LAYOUT,
		NULL, 0, pdli, dwSize, &dwBytesReturned, (LPOVERLAPPED)NULL);

	if( bRet )
	{
		// 分析读取的数据
		PPARTITION_INFORMATION p = (PPARTITION_INFORMATION)pdli->PartitionEntry;
		int iExtPos = 0;

		// 前4个主分区项
		for(int i = 0; i < (pdli->PartitionCount<4 ? pdli->PartitionCount : 4); ++i)
		{
			if(p[i].PartitionNumber > 0)
			{
				aDPT.Add(i+1);//p[i].PartitionNumber);
				++aDPT[0];
			}
			else if( iExtPos <= 0 )
				iExtPos = i+1;
		}

		// 扩展分区
		for(int i = 4; i < pdli->PartitionCount; ++i)
		{
			if(p[i].PartitionNumber >= aDPT[0])
				aDPT.Add(p[i].PartitionNumber-aDPT[0]+4);
		}

		// 第一个单元返回的数据
		// 个位：扩展分区位置(1-N)
		// 十位：主分区个数
		// 百位：逻辑分区个数
		aDPT[0] = (aDPT.GetSize() - 1 - aDPT[0]) * 10 + aDPT[0];
		aDPT[0] = aDPT[0] * 10 + iExtPos;

		if( NULL != szFileName )
		{
			// 调试信息：分区而已信息输出到文件
			CFile fo;
			if( fo.Open(szFileName, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary) )
			{
				fo.Write(p, dwBytesReturned-8);
				fo.Close();
			}
		}
	}

	delete [] pdli;
	return bRet;
}

/*
 *	获取分区信息 : 磁盘号、分区号、分区类型
 *  sDrv 是带冒号的盘符，如 C:
 */
BOOL DISK::GetDriveNumber(HANDLE &hDevice, PSTORAGE_DEVICE_NUMBER pDevNum)
{
	ZeroMemory(pDevNum, sizeof(PSTORAGE_DEVICE_NUMBER));

	DWORD dwBytes = 0; 
	return DeviceIoControl(hDevice, IOCTL_STORAGE_GET_DEVICE_NUMBER,
		NULL, 0, pDevNum, sizeof(STORAGE_DEVICE_NUMBER), &dwBytes, (LPOVERLAPPED)NULL);
}

/*
 *	获取分区编号信息(Windows下的编号)
 *  sDrv 是带冒号的盘符，如 C:
 */
BOOL DISK::GetDriveNumber(const CString &sDrv, PSTORAGE_DEVICE_NUMBER pDevNum)
{
	BOOL bRet = FALSE;
	HANDLE hDevice = NULL;

	if( OpenDevice(hDevice, _T("\\\\.\\")+sDrv) )
	{
		bRet = GetDriveNumber(hDevice, pDevNum);
		DISK::CloseHandle(hDevice);
	}

	return bRet;
}

/*
 *	获取分区容量，单位M(仅整数部分，非精确值)
 *  sDrv 是带冒号的盘符
 */
DWORD DISK::GetDriveSize(const CString &sDrv, PFREE_SPACE pFs)
{
	DWORD dwSize = 0;
	FREE_SPACE fs;

	if( GetDiskFreeSpace( _T("\\\\.\\")+sDrv+_T("\\"),
		&fs.dwSectorsPerCluster, &fs.dwBytesPerSector,
		&fs.dwNumberOfFreeClusters, &fs.dwTotalNumberOfClusters) )
	{
		dwSize = (DWORD)((__int64)fs.dwTotalNumberOfClusters
			* fs.dwSectorsPerCluster * fs.dwBytesPerSector / 1024 / 1024);

		if(NULL != pFs)
			CopyMemory(pFs, &fs, sizeof(fs));
	}

	return dwSize;
}

/*
 *	获取文件系统名字
 *  sDrv 是带冒号的盘符
 */
CString DISK::GetFileSystem(const CString &sDrv)
{
	TCHAR FileSystemNameBuffer[32];
	DWORD dwFileSystemNameSize=32;
	
	if( GetVolumeInformation(sDrv+_T("\\"), NULL, 0, NULL, NULL,
		NULL, FileSystemNameBuffer, dwFileSystemNameSize) )
	{
		return CString(FileSystemNameBuffer);
	}
	return _T("");
}

/*
 *	备份 hDevice 引导扇区到文件 sFileName
 *
 *  hDevice		已打开的设备句柄
 *  sFileName	磁盘文件名
 *	dwSectors	操作的扇区数
 *  dwStartSector	起始扇区号(从0起)
 *	dwBytePerSector	每扇区字节数
 *	dwOffsetSectors	文件内的偏移扇区数(从0起)
 */
BOOL DISK::ReadBootSector(HANDLE &hDevice, const CString &sFileName,
						  DWORD dwSectors, DWORD dwStartSector,
						  DWORD dwBytePerSector, DWORD dwOffsetSectors)
{
	// 只接受 1-126 个扇区
	dwSectors = dwSectors > 126 ? 126 : dwSectors;
	
	BOOL bRet = FALSE;
	MBR bs[126];

	// 读磁盘，起始扇区=dwStartSector，扇区数=dwSectors
	if( ReadSectors(hDevice, &bs, dwSectors, dwStartSector, dwBytePerSector) )
	{
		// 打开文件
		HANDLE hFile;

		const DWORD dwCreationDisposition = 
			WND::IsExistFile(sFileName) ? OPEN_EXISTING : CREATE_ALWAYS;

		if( OpenFile(hFile, sFileName, dwCreationDisposition) )
		{
			// 写文件，偏移扇区=dwOffsetSectors
			if( WriteSectors(hFile, &bs, dwSectors, dwOffsetSectors, dwBytePerSector) )
					bRet = TRUE;

			DISK::CloseHandle(hFile);
		}
	}

	// 删除 0 字节文件
	if( !bRet && WND::IsExistFile(sFileName) )
		DeleteFile(sFileName);

	return bRet;
}

/*
 *	从文件 sFileName 恢复 hDevice 引导扇区
 *
 *  hDevice		已打开的设备句柄
 *  sFileName	磁盘文件名
 *	dwSectors	操作的扇区数
 *  dwStartSector	起始扇区号(从0起)
 *	dwBytePerSector	每扇区字节数
 *	dwOffsetSectors	文件内的偏移扇区数(从0起)
 */
BOOL DISK::WriteBootSector(HANDLE &hDevice, const CString &sFileName,
						   DWORD dwSectors, DWORD dwStartSector,
						   DWORD dwBytePerSector, DWORD dwOffsetSectors)
{
	// 只接受 1-126 个扇区
	if(dwSectors < 1 || dwSectors > 126)
		return FALSE;
	
	BOOL bRet = FALSE;
	HANDLE hFile;

	// 打开文件
	if( OpenFile(hFile, sFileName, OPEN_EXISTING, GENERIC_READ) )
	{
		MBR bs[126];
		
		// 读文件，偏移扇区=dwOffsetSectors
		if( ReadSectors(hFile, &bs, dwSectors, dwOffsetSectors, dwBytePerSector) )
		{
			// 写磁盘，起始扇区=dwStartSector，扇区数=dwSectors
			if( WriteSectors(hDevice, &bs, dwSectors, dwStartSector, dwBytePerSector) )
				bRet = TRUE;
		}
		DISK::CloseHandle(hFile);
	}

	return bRet;
}

/*
 *	按扇区读磁盘
 *
 *  hDevice			已打开的设备或文件句柄
 *  pBuf			数据缓冲区
 *  dwSectors		操作的扇区数
 *  dwStartSector	起始扇区号
 *  dwBytePerSecotr	每扇区字节数
 */
BOOL DISK::ReadSectors(HANDLE &hDevice, LPVOID pBuf, DWORD dwSectors,
				 DWORD dwStartSector, DWORD dwBytePerSector)
{
	//SetFilePointer(hDevice, dwBytePerSector * dwStartSector, NULL, FILE_BEGIN);
	if( ! SeekLargeFile(hDevice, (__int64)dwBytePerSector * dwStartSector, FILE_BEGIN) )
		return FALSE;

	// 严格判定字节数，文件长度不够时失败
	DWORD dwBytes = 0;
	return ::ReadFile(hDevice, pBuf, dwBytePerSector * dwSectors, &dwBytes, (LPOVERLAPPED)NULL)
		&& dwBytePerSector * dwSectors == dwBytes;
}

/*
 *	按扇区写磁盘
 *
 *  hDevice			已打开的设备或文件句柄
 *  pBuf			数据缓冲区
 *  dwSectors		操作的扇区数
 *  dwStartSector	起始扇区号
 *  dwBytePerSecotr	每扇区字节数
 */
BOOL DISK::WriteSectors(HANDLE &hDevice, LPVOID pBuf, DWORD dwSectors,
				  DWORD dwStartSector, DWORD dwBytePerSector)
{
	//SetFilePointer(hDevice, dwBytePerSector * dwStartSector, NULL, FILE_BEGIN);
	if( ! SeekLargeFile(hDevice, (__int64)dwBytePerSector * dwStartSector, FILE_BEGIN) )
		return FALSE;

	// 严格判定字节数，写入字节不够时失败
	DWORD dwBytes = 0;
	return ::WriteFile(hDevice, pBuf, dwBytePerSector * dwSectors, &dwBytes, (LPOVERLAPPED)NULL)
		&& dwBytePerSector * dwSectors == dwBytes;
}

// 移动文件指针
BOOL DISK::SeekLargeFile(HANDLE &hFile, __int64 liOffset, DWORD dwMoveMethod)
{
	LARGE_INTEGER li;
	li.QuadPart = liOffset;
	li.LowPart = SetFilePointer (hFile, li.LowPart, &li.HighPart, dwMoveMethod);

// 	if( 0xFFFFFFFF == li.LowPart && NO_ERROR != GetLastError() )
// 		li.LowPart = -1;

	return NO_ERROR == GetLastError();
}

/*
 *	打印一个扇区内容到字符串
 *
 *	pBuf			至少512字节的缓冲区
 *	dwOffsetSector	地址偏移量，以扇区为单位
 *	sEnter			换行符
 *	bTailEnter		尾部是添加换行符
 */
CString DISK::PrintSector(const BYTE *pBuf, DWORD dwOffsetSector,
						  const CString sEnter, BOOL bTailEnter)
{
	CString ss, str = _T("");
	if(NULL == pBuf)
		return str;

	// 32 行，每行 16 个字节
	const int lines = 32;
	for(int i = 0; i < lines; ++i)
	{
		// 16 进制
		str = str 
			+ EMFC::Int2Str((dwOffsetSector<<9)+(i<<4), _T("%04X: "))
			+ EMFC::PrintArray(pBuf+(i<<4), 8,  0, _T("%02X ")) + _T("- ")
		    + EMFC::PrintArray(pBuf+(i<<4), 16, 8, _T("%02X "))	+ _T("| ")
			;

		// 字符
		for(int j = 0; j < 16; ++j)
		{
			ss.Format(_T("%C"), isprint(pBuf[(i<<4)+j]) ? pBuf[(i<<4)+j] : _T('.'));
			str += ss;
		}

		if(i < lines-1 || bTailEnter)
			str += sEnter;
	}

	return str;
}

CString DISK::GetDiskName(int i)
{
	CString str;
	str.Format( _T("\\\\?\\PHYSICALDRIVE%d"), i);
	return str;
}

CString DISK::GetDriveName(const TCHAR &ch)
{
	CString str;
	str.Format( _T("\\\\.\\%C:"), ch);
	return str;
}

#if !defined(DISABLE_USE_CHS)

BOOL DISK::AbsRead(HANDLE &hDevice, LPVOID pBuf,
				   WORD c, WORD h, WORD s,
				   DWORD dwSectors, DWORD dwBytePerSector/ * =512 * /)
{
	DWORD dwStartSector = Chs2Lba(c, h, s);
	return ReadSectors(hDevice, pBuf, dwSectors, dwStartSector, dwBytePerSector);
}

BOOL DISK::ABSWrite(HANDLE &hDevice, LPVOID pBuf,
					WORD c, WORD h, WORD s,
					DWORD dwSectors, DWORD dwBytePerSector/ * =512 * /)
{
	DWORD dwStartSector = Chs2Lba(c, h, s);
	return WriteSectors(hDevice, pBuf, dwSectors, dwStartSector, dwBytePerSector);
}

DWORD DISK::Chs2Lba(WORD c, WORD h, WORD s,
					DWORD PS/ * =63 * /, DWORD PH/ * =255 * /,
					WORD sc/ *=0* /, WORD sh/ *=0* /, WORD ss/ *=0* /)
{
	return (c-sc) * PS * PH + (h-sh) * PS + (s-ss) - 1;
}

DWORD DISK::Chs2Lba(const WORD_CHS &wchs,
					DWORD PS/ * =63 * /, DWORD PH/ * =255 * /,
					const PWORD_CHS pschs/ *=NULL* /)
{
	if(NULL == pschs)
		return Chs2Lba(wchs.c, wchs.h, wchs.s, PS, PH);

	return Chs2Lba(wchs.c, wchs.h, wchs.s, PS, PH, pschs->c, pschs->h, pschs->s);
}

DWORD DISK::Chs2Lba(const BYTE_CHS &bchs,
					DWORD PS/ * =63 * /, DWORD PH/ * =255 * /,
					const PBYTE_CHS pschs/ *=NULL* /)
{
	WORD_CHS wchs, swchs={0,0,0};
	ChsB2W(bchs, wchs);

	return Chs2Lba(wchs, PS, PH, NULL==pschs? NULL : &swchs);
}

VOID DISK::Lba2Chs(DWORD lba, WORD_CHS &wchs,
				   DWORD PS/ * =63 * /, DWORD PH/ * =255 * /,
				   WORD sc/ *=0* /, WORD sh/ *=0* /, WORD ss/ *=0* /)
{
	wchs.c = lba / PH / PS + sc;
	wchs.h = ((lba / PS) % PH) + sh;
	wchs.s = (lba % PS) + ss;
}

VOID DISK::Lba2Chs(DWORD lba, BYTE_CHS &bchs,
				   DWORD PS/ * =63 * /, DWORD PH/ * =255 * /,
				   WORD sc/ * =0 * /, WORD sh/ * =0 * /, WORD ss/ * =0 * /)
{
	WORD_CHS wchs;
	Lba2Chs(lba, wchs, PS, PH, sc, sh, ss);
	ChsW2B(wchs, bchs);
}

VOID DISK::ChsB2W(const BYTE_CHS &bchs, WORD_CHS &wchs)
{
	wchs.h = bchs.byHead;
	wchs.s = bchs.byLowSC & 0x3F;

	wchs.c  = bchs.byHighSC;
	wchs.c |= (((WORD)bchs.byLowSC)<<2) & 0x0300;
}

VOID DISK::ChsW2B(const WORD_CHS &wchs, BYTE_CHS &bchs)
{
	bchs.byHead = wchs.h;
	bchs.byLowSC = (wchs.s & 0x3F) | ((wchs.c >> 2) & 0xC0);
	bchs.byHighSC = wchs.c & 0xFF;
}

#endif // DISABLE_USE_CHS

