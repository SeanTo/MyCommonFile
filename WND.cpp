#include "stdafx.h"
#include "wnd.h"

////////////////////////////////////////////////////////////////////////////////

// for CreateToolhelp32Snapshot
#include "tlhelp32.h"

////////////////////////////////////////////////////////////////////////////////

/*
 *	�ַ����Ƿ񰴼���(A-Z��a-z��0-9)
 */
BOOL WND::IsCharDown(int ch)
{
	if( _istlower(ch) )
		ch = toupper(ch);
	
	if( ch >= _T('A') && ch <= _T('Z') || ch >= _T('0') && ch <= _T('9') )
		return -1 == GetKeyState(ch);
	
	return FALSE;
}

/*
 *	ȡ�õ�ǰ����ʱĿ¼
 *  bAddSet : �����Ƿ����·���ָ��� \
 *  bIsTemp : Ϊ TRUE ʱ��ȡϵͳ��ʱĿ¼
 */
CString WND::GetCurOrTempDir(BOOL bAddSep/* = FALSE*/, BOOL bIsTemp/* = FALSE*/)
{
    TCHAR tszBuf[MAX_PATH];
	if( bIsTemp )
		::GetTempPath(MAX_PATH, tszBuf);
	else
		::GetCurrentDirectory(MAX_PATH, tszBuf);

	CString str(tszBuf);
	str.TrimRight(_T("\\"));

	if(bAddSep)
		str += _T("\\");

	return str;
}

/*
 *	�񵽵�ǰ�������·��
 *  bAddSet : �����Ƿ����·���ָ��� \
 */
CString WND::GetAbsPath(BOOL bAddSep/* == FALSE*/)
{
//  TCHAR tszBuf[MAX_PATH];
// 	::GetModuleFileName(NULL, tszBuf, MAX_PATH);
// 	CString str(tszBuf);
// 	str = str.Left(str.ReverseFind(_T('\\')));
    CString str = __targv[0];
    str = str.Left(str.ReverseFind(_T('\\')));
	if(bAddSep)
	 	str += _T("\\");

	return str;
}

/*
 *	��ǰexe�����ļ���
 */
CString WND::GetExeName()
{
//  TCHAR tszBuf[MAX_PATH];
// 	GetModuleFileName(NULL, tszBuf, MAX_PATH);
	
    CString str(__targv[0]);
	str = str.Mid(str.ReverseFind(_T('\\')) + 1);
	return str;
}

/*
 *	����һ��Ŀ¼
 */
int WND::FileSearch(CString strRoot, CStringArray &arrFileName,
					BOOL bSearchDir /*= TRUE*/, const CString &strExt/* = _T("")*/,
					BOOL bSub/* = FALSE*/)
{
	strRoot.TrimRight(_T("\\"));
	if( bSearchDir)
		strRoot += _T("\\*.*");
	else
		strRoot += _T("\\*.") + strExt;
	
	CString strFilePath, str;
	CFileFind ff;
	BOOL res = ff.FindFile(strRoot);
	
    while (res)
	{
		res = ff.FindNextFile();
		if( ff.IsDots() )
			continue;
		
		strFilePath = ff.GetFilePath();
		if( bSearchDir && ff.IsDirectory())
		{
			str = strFilePath.Mid( strFilePath.ReverseFind( _T('\\') )+1 );
			str.MakeUpper();
			if( str.GetLength() == 4 && str.Left(1) == _T("G")
				&& isdigit(str.GetAt(1)) && isdigit(str.GetAt(2)) && isdigit(str.GetAt(3)) )
			{
				arrFileName.Add( strFilePath );
			}
			// 	FileSearch( strFilePath, arrFileName, bSearchSub, strFilter );	// �ݹ�
		}
		else if ( !bSearchDir && !ff.IsDirectory() )
		{
			str = strFilePath.Right(strExt.GetLength() + 1);
			if( strExt.IsEmpty() || 0 == str.CompareNoCase(_T(".") + strExt))
			{
				arrFileName.Add( strFilePath );
			}
		}
    }
	
	return arrFileName.GetSize();
}

/*
 *	���������ʹ��ڱ�����Ҵ��ڣ����������ᵽǰ̨
 */
VOID WND::ActiveWindow(LPCTSTR lpszClassName, LPCTSTR lpszWindowName /* = NULL*/)
{
	// �����ڵ���ǰ̨
	CWnd *pWnd = CWnd::FindWindow(lpszClassName, lpszWindowName);
	
	if(pWnd != NULL)
	{
		pWnd->SetForegroundWindow();
		pWnd->ShowWindow(SW_SHOW);
		pWnd->ShowWindow(SW_RESTORE);
		
//		CWnd* pWndPopup = pWnd->GetLastActivePopup();
//		if(pWnd != pWndPopup)
//			pWndPopup->SetForegroundWindow();
	}
}

/*
*  ��Ҫͷ�ļ� tlhelp32.h
 *	���� pctsExeName ָ���Ŀ�ִ�г����Ƿ�����
 *  ��������ʱ�������н��̵� PID�����򷵻� -1
 */
DWORD WND::FindExePID(LPCTSTR pctsExeName)
{
	HANDLE handle=::CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0);
	
	PROCESSENTRY32 Info;
	Info.dwSize = sizeof(PROCESSENTRY32);
	
	if(::Process32First(handle, &Info))
	{
		CString ss = _T("");
		do{
			ss = Info.szExeFile;
			if( 0 == ss.CompareNoCase(pctsExeName) )
			{
				::CloseHandle(handle);
				return Info.th32ProcessID;
			}
		}
		while(::Process32Next(handle, &Info));
		
		::CloseHandle(handle);
	}
	
	return -1;
}

/*
 *	���� szCmd ���ȴ����̽������жϷ���ֵ�Ƿ�Ϊ dwExitCode
 */
DWORD WND::WaitProcess(LPCTSTR szCmd,
					 BOOL	bShow /*= TRUE*/,
					 DWORD	dwTimeOut/*=INFINITE*/)
{
	TCHAR szBuf[MAX_PATH];
	lstrcpy(szBuf, szCmd);
	
	PROCESS_INFORMATION	pi;
	STARTUPINFO			si;
	ZeroMemory(&si, sizeof(STARTUPINFO));

/*
	si.cb			= sizeof(STARTUPINFO);
	si.lpReserved	= NULL;
	si.lpDesktop	= NULL;
	si.lpTitle		= NULL;
	si.dwX		= 0;
	si.dwY		= 0;
	si.dwXSize	= 0;
	si.dwYSize	= 0;
	si.dwXCountChars	= 0; 
	si.dwYCountChars	= 0;
	si.dwFillAttribute	= 0;
	si.dwFlags			= 0;
	si.wShowWindow		= 0;
	si.lpReserved2		= NULL;
	si.cbReserved2		= 0;
	si.hStdInput	= NULL;
	si.hStdOutput	= NULL;
	si.hStdError	= NULL;
*/

	GetStartupInfo(&si);
	si.cb			= sizeof(STARTUPINFO);
	si.wShowWindow	= bShow ? SW_SHOW : SW_HIDE;
	
	DWORD exitCode = -1;
	if( CreateProcess(NULL, szBuf,
		NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi) )
	{
// 		if( INFINITE != dwTimeOut )
// 			WaitForInputIdle(pi.hProcess, dwTimeOut);
// 		else
			WaitForSingleObject(pi.hProcess, dwTimeOut);
		GetExitCodeProcess(pi.hProcess, &exitCode);
	}
	
	return exitCode;
}

/*
 *	�ݹ�ɾ��Ŀ¼ pcsDirName(��֧�ֻ���վ)
 */
/*
BOOL WND::DeleteDirectory(LPCTSTR pcsDirName)
{
	CFileFind tmpFFind;

	CString sFileFind = CString(pcsDirName) + _T("\\*.*");
	BOOL IsFinded = (BOOL)tmpFFind.FindFile(sFileFind);

	while(IsFinded)
	{
		IsFinded = tmpFFind.FindNextFile();

		if( !tmpFFind.IsDots() )
		{
			CString sFileName = tmpFFind.GetFileName().GetBuffer(MAX_PATH);
			tmpFFind.GetFileName().ReleaseBuffer();

			sFileName = CString(pcsDirName) + _T("\\") + sFileName;

			if( tmpFFind.IsDirectory() )
			{
				DeleteDirectory(sFileName);
			}
			else
			{
				DeleteFile(sFileName);
			}
		}
	}
	tmpFFind.Close();

	if( !RemoveDirectory(pcsDirName) )
	{
		return FALSE;
	}

	return TRUE;
}
*/

/*
 * �ļ���Ŀ¼����
 * ����OP:
 *		�ƶ�=FO_MOVE, ����=FO_COPY, ɾ��=FO_DELETE, ����=FO_RENAME
 * �ɹ����� TRUE��ʧ�ܷ��� FALSE
 */
BOOL WND::FileOperation(LPCTSTR szSrc, LPCTSTR szDest,
						UINT nFunc, FILEOP_FLAGS flag/* = 0*/)
{
	ASSERT(FO_MOVE==nFunc || FO_COPY==nFunc || FO_DELETE==nFunc || FO_RENAME==nFunc);
	ASSERT( lstrlen(szSrc) < MAX_PATH && lstrlen(szDest) < MAX_PATH );

	TCHAR bufSrc[MAX_PATH+1];
// 	ZeroMemory(bufSrc,  MAX_PATH+1);
	lstrcpy(bufSrc, szSrc);
	bufSrc[lstrlen(bufSrc)+1] = '\0';

	TCHAR bufDest[MAX_PATH+1];
// 	ZeroMemory(bufDest, MAX_PATH+1);
	lstrcpy(bufDest, szDest);
	bufDest[lstrlen(bufDest)+1] = '\0';

	SHFILEOPSTRUCT	fo;
 	ZeroMemory( &fo, sizeof(fo) );

	fo.hwnd = NULL;
	fo.wFunc = nFunc;
	fo.pFrom = bufSrc;
	fo.pTo = bufDest;

	if(0 == flag)
	{
		fo.fFlags = 0
			| FOF_ALLOWUNDO				// Preserve Undo information, if possible.
// 			| FOF_CONFIRMMOUSE			// Not currently implemented.  
// 			| FOF_FILESONLY				// operation files only if a wildcard file name (*.*) is specified.  
// 			| FOF_MULTIDESTFILES		// The pTo member specifies multiple destination files (one for each source file) rather than one directory where all source files are to be deposited.  
			| FOF_NOCONFIRMATION		// Respond with Yes to All for any dialog box that is displayed.  
			| FOF_NOCONFIRMMKDIR		// Does not confirm the creation of a new directory if the operation requires one to be created.  
// 			| FOF_NOCOPYSECURITYATTRIBS	// Version 4.71. Microsoft? Windows NT? only. The security attributes of the file will not be copied.  
// 			| FOF_NOERRORUI				// No user interface will be displayed if an error occurs.  
// 			| FOF_RENAMEONCOLLISION		// Give the file being operated on a new name in a move, copy, or rename operation if a file with the target name already exists.  
			| FOF_SILENT				// Does not display a progress dialog box.  
// 			| FOF_SIMPLEPROGRESS		// Displays a progress dialog box but does not show the file names.  
// 			| FOF_WANTMAPPINGHANDLE		// If FOF_RENAMEONCOLLISION is specified, the hNameMappings member will be filled in if any files were renamed. 
			;
	}
	else
		fo.fFlags = flag;

	fo.fAnyOperationsAborted = TRUE;
	fo.hNameMappings = NULL;
	fo.lpszProgressTitle = NULL;
	
	return 0 == SHFileOperation(&fo);
}

/*
 *  Ŀ¼�Ի���
 *	ʾ�� SelectDirectory( GetSafeHwnd() );
 */
CString WND::SelectDirectory(HWND hWnd/* = NULL*/)
{
	CString str = _T("");
	TCHAR		buf[MAX_PATH + 1];
	TCHAR		path[MAX_PATH + 1];
	lstrcpy(path, _T(""));
	
	BROWSEINFO	m_pbi;
	::ZeroMemory( &m_pbi, sizeof(BROWSEINFO) );
	m_pbi.hwndOwner			= hWnd;
	m_pbi.pidlRoot			= NULL;
	m_pbi.pszDisplayName	= buf;
	m_pbi.lpszTitle			= _T("ѡ����Ŀ¼");
	m_pbi.ulFlags			= BIF_RETURNONLYFSDIRS;
    
	ITEMIDLIST *idl =  SHBrowseForFolder(&m_pbi);
	if( idl )
	{
		SHGetPathFromIDList(idl, buf);
		str = CString(buf);
	}
	
	return str;
}

/*
 *  �ļ��Ի���ѡ�񵥸��ļ�
 *  ʾ��
 *	WND::SelectFile(FALSE, _T("txt"), _T("�ı��ļ�(*.txt)|*.txt|�����ļ�(*.*)|*.*||"), strFileName );
 */
CString WND::SelectFile(BOOL bIsOpen, LPCTSTR szPath, LPCTSTR szExt, LPCTSTR szFilter)
{
	CString str = szPath;
	TCHAR tszFile[MAX_PATH] = { _T('\0') };
	
	if( ! str.IsEmpty() )
		lstrcpy(tszFile, str);
	
	CFileDialog dlg(bIsOpen, szExt, szPath,NULL, szFilter, NULL);
	
	if( IDOK == dlg.DoModal() )
		str = dlg.GetPathName();
	else
		str.Empty();
	
	return str;
}

/*
 *  �ļ��Ի���ѡ�����ļ����ļ��б���arrFiles������
 *	ʾ��
 *  WND::SelectMutiFile(arrFiles, _T("C:\\"), _T("�ı��ļ�(*.txt)\0*.txt"));
 */
CString WND::SelectMutiFile(CStringArray &arrFiles, LPCTSTR szPath/* = _T*/, LPCTSTR szFilter/* =_T*/)
{
	const DWORD MAXFILE = 8192;
	TCHAR		buf[MAXFILE];
	TCHAR tszPath[MAX_PATH];

	lstrcpy( tszPath, szPath );
	
	CFileDialog dlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT, NULL, NULL);
	dlg.m_ofn.nMaxFile = MAXFILE; 
	dlg.m_ofn.lpstrInitialDir = tszPath;
	dlg.m_ofn.lpstrFile = buf; 
	dlg.m_ofn.lpstrFile[0] = NULL;
	dlg.m_ofn.lpstrFilter = szFilter;
	
	CString str(_T(""));
	if(IDOK == dlg.DoModal()) 
	{ 
		POSITION pos = dlg.GetStartPosition(); 
		while (pos != NULL) 
		{
			str = dlg.GetNextPathName(pos);
			arrFiles.Add( str ); 
		} 
		
		// ��ס����ѡ��·��
		str = str.Mid(0, str.ReverseFind(_T('\\'))+1);
	}

	return str;
}

/*
 *	ȡ��������
 */
int WND::GetCommandLine(CStringArray &arrCmd, int iCount/* = -1*/)
{
    int loop = (iCount < 1 || iCount > __argc) ? __argc : iCount;

    for(int i = 0; i < loop; ++i)
        arrCmd.Add( __targv[i] );

    return __argc;
}

////////////////////////////////////////////////////////////////////////////////
