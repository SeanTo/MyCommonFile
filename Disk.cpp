#include "..\StdAfx.h"
#include "wnd.h"
#include "emfc.h"
#include "Disk.h"

/*
 *  ���豸
 *	C �� : "\\\\.\\C:"
 *	����0: "\\\\?\\PhysicalDrive0"
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
 *	�򿪴����ļ�
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
 *	�رվ��
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
 *  ��ֹ USB �ƶ�Ӳ�̱�ʶ��Ϊ�̶�Ӳ��
 *  ���� OpenDevice �������ٿ��ô˺����ж�
 */
BOOL DISK::IsUsb(HANDLE &hDevice)
{
	STORAGE_DEVICE_DESCRIPTOR sdd;
	sdd.Size = sizeof(STORAGE_DEVICE_DESCRIPTOR);
	
	return GetDiskProperty(hDevice, &sdd) && BusTypeUsb == sdd.BusType;
}

/*
 *	��ȡ������Ϣ������� pDevDesc ��
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
 *	��ȡӲ����������λ M(���������֣��Ǿ�ȷֵ)
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
 *	��������Ϣ������һ������ aDPT
 *  �� windows ������Ϊ��������ӦԪ�ص�ֵ-1 Ϊ grub ������
 */
BOOL DISK::GetDirveLayout(HANDLE &hDevice, CUIntArray &aDPT, LPCTSTR szFileName)
{
	aDPT.RemoveAll();
	aDPT.Add(0);

	// ���� 32 ��������Ϣ��Ԫ
	DWORD dwSize = sizeof(DRIVE_LAYOUT_INFORMATION) - 1
		+ 32 * sizeof(PARTITION_INFORMATION);

	PDRIVE_LAYOUT_INFORMATION pdli = (PDRIVE_LAYOUT_INFORMATION)new BYTE[dwSize];
	if( NULL == pdli )
		return FALSE;
	ZeroMemory(pdli, dwSize);
	
	// ��ȡ��Ϣ
	DWORD dwBytesReturned = 0;
	BOOL bRet = DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_LAYOUT,
		NULL, 0, pdli, dwSize, &dwBytesReturned, (LPOVERLAPPED)NULL);

	if( bRet )
	{
		// ������ȡ������
		PPARTITION_INFORMATION p = (PPARTITION_INFORMATION)pdli->PartitionEntry;
		int iExtPos = 0;

		// ǰ4����������
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

		// ��չ����
		for(int i = 4; i < pdli->PartitionCount; ++i)
		{
			if(p[i].PartitionNumber >= aDPT[0])
				aDPT.Add(p[i].PartitionNumber-aDPT[0]+4);
		}

		// ��һ����Ԫ���ص�����
		// ��λ����չ����λ��(1-N)
		// ʮλ������������
		// ��λ���߼���������
		aDPT[0] = (aDPT.GetSize() - 1 - aDPT[0]) * 10 + aDPT[0];
		aDPT[0] = aDPT[0] * 10 + iExtPos;

		if( NULL != szFileName )
		{
			// ������Ϣ������������Ϣ������ļ�
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
 *	��ȡ������Ϣ : ���̺š������š���������
 *  sDrv �Ǵ�ð�ŵ��̷����� C:
 */
BOOL DISK::GetDriveNumber(HANDLE &hDevice, PSTORAGE_DEVICE_NUMBER pDevNum)
{
	ZeroMemory(pDevNum, sizeof(PSTORAGE_DEVICE_NUMBER));

	DWORD dwBytes = 0; 
	return DeviceIoControl(hDevice, IOCTL_STORAGE_GET_DEVICE_NUMBER,
		NULL, 0, pDevNum, sizeof(STORAGE_DEVICE_NUMBER), &dwBytes, (LPOVERLAPPED)NULL);
}

/*
 *	��ȡ���������Ϣ(Windows�µı��)
 *  sDrv �Ǵ�ð�ŵ��̷����� C:
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
 *	��ȡ������������λM(���������֣��Ǿ�ȷֵ)
 *  sDrv �Ǵ�ð�ŵ��̷�
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
 *	��ȡ�ļ�ϵͳ����
 *  sDrv �Ǵ�ð�ŵ��̷�
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
 *	���� hDevice �����������ļ� sFileName
 *
 *  hDevice		�Ѵ򿪵��豸���
 *  sFileName	�����ļ���
 *	dwSectors	������������
 *  dwStartSector	��ʼ������(��0��)
 *	dwBytePerSector	ÿ�����ֽ���
 *	dwOffsetSectors	�ļ��ڵ�ƫ��������(��0��)
 */
BOOL DISK::ReadBootSector(HANDLE &hDevice, const CString &sFileName,
						  DWORD dwSectors, DWORD dwStartSector,
						  DWORD dwBytePerSector, DWORD dwOffsetSectors)
{
	// ֻ���� 1-126 ������
	dwSectors = dwSectors > 126 ? 126 : dwSectors;
	
	BOOL bRet = FALSE;
	MBR bs[126];

	// �����̣���ʼ����=dwStartSector��������=dwSectors
	if( ReadSectors(hDevice, &bs, dwSectors, dwStartSector, dwBytePerSector) )
	{
		// ���ļ�
		HANDLE hFile;

		const DWORD dwCreationDisposition = 
			WND::IsExistFile(sFileName) ? OPEN_EXISTING : CREATE_ALWAYS;

		if( OpenFile(hFile, sFileName, dwCreationDisposition) )
		{
			// д�ļ���ƫ������=dwOffsetSectors
			if( WriteSectors(hFile, &bs, dwSectors, dwOffsetSectors, dwBytePerSector) )
					bRet = TRUE;

			DISK::CloseHandle(hFile);
		}
	}

	// ɾ�� 0 �ֽ��ļ�
	if( !bRet && WND::IsExistFile(sFileName) )
		DeleteFile(sFileName);

	return bRet;
}

/*
 *	���ļ� sFileName �ָ� hDevice ��������
 *
 *  hDevice		�Ѵ򿪵��豸���
 *  sFileName	�����ļ���
 *	dwSectors	������������
 *  dwStartSector	��ʼ������(��0��)
 *	dwBytePerSector	ÿ�����ֽ���
 *	dwOffsetSectors	�ļ��ڵ�ƫ��������(��0��)
 */
BOOL DISK::WriteBootSector(HANDLE &hDevice, const CString &sFileName,
						   DWORD dwSectors, DWORD dwStartSector,
						   DWORD dwBytePerSector, DWORD dwOffsetSectors)
{
	// ֻ���� 1-126 ������
	if(dwSectors < 1 || dwSectors > 126)
		return FALSE;
	
	BOOL bRet = FALSE;
	HANDLE hFile;

	// ���ļ�
	if( OpenFile(hFile, sFileName, OPEN_EXISTING, GENERIC_READ) )
	{
		MBR bs[126];
		
		// ���ļ���ƫ������=dwOffsetSectors
		if( ReadSectors(hFile, &bs, dwSectors, dwOffsetSectors, dwBytePerSector) )
		{
			// д���̣���ʼ����=dwStartSector��������=dwSectors
			if( WriteSectors(hDevice, &bs, dwSectors, dwStartSector, dwBytePerSector) )
				bRet = TRUE;
		}
		DISK::CloseHandle(hFile);
	}

	return bRet;
}

/*
 *	������������
 *
 *  hDevice			�Ѵ򿪵��豸���ļ����
 *  pBuf			���ݻ�����
 *  dwSectors		������������
 *  dwStartSector	��ʼ������
 *  dwBytePerSecotr	ÿ�����ֽ���
 */
BOOL DISK::ReadSectors(HANDLE &hDevice, LPVOID pBuf, DWORD dwSectors,
				 DWORD dwStartSector, DWORD dwBytePerSector)
{
	//SetFilePointer(hDevice, dwBytePerSector * dwStartSector, NULL, FILE_BEGIN);
	if( ! SeekLargeFile(hDevice, (__int64)dwBytePerSector * dwStartSector, FILE_BEGIN) )
		return FALSE;

	// �ϸ��ж��ֽ������ļ����Ȳ���ʱʧ��
	DWORD dwBytes = 0;
	return ::ReadFile(hDevice, pBuf, dwBytePerSector * dwSectors, &dwBytes, (LPOVERLAPPED)NULL)
		&& dwBytePerSector * dwSectors == dwBytes;
}

/*
 *	������д����
 *
 *  hDevice			�Ѵ򿪵��豸���ļ����
 *  pBuf			���ݻ�����
 *  dwSectors		������������
 *  dwStartSector	��ʼ������
 *  dwBytePerSecotr	ÿ�����ֽ���
 */
BOOL DISK::WriteSectors(HANDLE &hDevice, LPVOID pBuf, DWORD dwSectors,
				  DWORD dwStartSector, DWORD dwBytePerSector)
{
	//SetFilePointer(hDevice, dwBytePerSector * dwStartSector, NULL, FILE_BEGIN);
	if( ! SeekLargeFile(hDevice, (__int64)dwBytePerSector * dwStartSector, FILE_BEGIN) )
		return FALSE;

	// �ϸ��ж��ֽ�����д���ֽڲ���ʱʧ��
	DWORD dwBytes = 0;
	return ::WriteFile(hDevice, pBuf, dwBytePerSector * dwSectors, &dwBytes, (LPOVERLAPPED)NULL)
		&& dwBytePerSector * dwSectors == dwBytes;
}

// �ƶ��ļ�ָ��
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
 *	��ӡһ���������ݵ��ַ���
 *
 *	pBuf			����512�ֽڵĻ�����
 *	dwOffsetSector	��ַƫ������������Ϊ��λ
 *	sEnter			���з�
 *	bTailEnter		β������ӻ��з�
 */
CString DISK::PrintSector(const BYTE *pBuf, DWORD dwOffsetSector,
						  const CString sEnter, BOOL bTailEnter)
{
	CString ss, str = _T("");
	if(NULL == pBuf)
		return str;

	// 32 �У�ÿ�� 16 ���ֽ�
	const int lines = 32;
	for(int i = 0; i < lines; ++i)
	{
		// 16 ����
		str = str 
			+ EMFC::Int2Str((dwOffsetSector<<9)+(i<<4), _T("%04X: "))
			+ EMFC::PrintArray(pBuf+(i<<4), 8,  0, _T("%02X ")) + _T("- ")
		    + EMFC::PrintArray(pBuf+(i<<4), 16, 8, _T("%02X "))	+ _T("| ")
			;

		// �ַ�
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

