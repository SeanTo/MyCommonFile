#ifndef _WND_H_
#define _WND_H_

// #ifdef _DEBUG
// 	#define HERE() AfxMessageBox(_T("I am here ��"));
// #else
// 	#define HERE() ;
// #endif // _DEBUG

////////////////////////////////////////////////////////////////////////////////

class WND
{
public:
	// ���Ƽ��Ƿ��� : CTRL / SHIFT / ALT
	static inline BOOL IsCtrlDown()  { return GetKeyState(VK_CONTROL) < 0; }
	static inline BOOL IsShiftDown() { return GetKeyState(VK_SHIFT) < 0; }
	static inline BOOL IsAltDown()   { return GetKeyState(VK_MENU) < 0; }
	// �ַ����Ƿ񰴼���(A-Z��a-z��0-9)
	static BOOL IsCharDown(int ch);
	
	// �ļ���Ŀ¼�Ƿ����
	static inline BOOL IsExistFile(const CString &strFile) { return GetFileAttributes(strFile) != 0xFFFFFFFF; }
	// Ŀ¼ : ��ǰĿ¼/��ʱĿ¼����ǰexe����Ŀ¼����ǰexe�ļ���
	static CString GetCurOrTempDir(BOOL bAddSep = FALSE, BOOL bIsTemp = FALSE);
	static CString GetAbsPath(BOOL bAddSep=FALSE);
	static CString GetExeName();
	// ����һ��Ŀ¼
	static int FileSearch(CString strRoot, CStringArray &arrFileName,
		BOOL bSearchDir=TRUE, const CString &strExt=_T(""), BOOL bSub=FALSE);
	
	// ���������ʹ��ڱ�����Ҵ��ڣ����������ᵽǰ̨
	static VOID ActiveWindow(LPCTSTR lpszClassName, LPCTSTR lpszWindowName=NULL); 
	// �� ShellExecute
	static HINSTANCE WinExec(LPCSTR szExe, LPCSTR szPara=NULL, LPCTSTR szDir=NULL, BOOL bShow=TRUE)
	{
		return ShellExecute(NULL, NULL, szExe, szPara, szDir, bShow ? SW_SHOW : SW_HIDE);
	}
	
	// ���� exe �Ƿ��������У��ҵ����� PID�����򷵻� -1
	static DWORD FindExePID(LPCTSTR pctsExeName);
	static BOOL  IsRunning(LPCTSTR pctsExeName) { return -1 != FindExePID(pctsExeName); }

	//	���� szCmd ���ȴ����̽������жϷ���ֵ�Ƿ�Ϊ dwExitCode
	static DWORD WaitProcess(LPCTSTR szCmd, BOOL bIsShow=TRUE, DWORD dwTimeOut=INFINITE);
    // ������ǰ iCount ��
    static int GetCommandLine(CStringArray &arrCmd, int iCount = -1);

	// ��ʽ��ϵͳ������Ϣ
	static DWORD GetLastErrorInfo() {
		DWORD dwError = ::GetLastError();
		TCHAR szError[MAX_PATH];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, 0, 0, szError, sizeof(szError), NULL);
		AfxMessageBox(szError);
		return dwError;
	}
	// �ؼ�
public:
	static BOOL FileOperation(LPCTSTR szSrc, LPCTSTR szDest, UINT nFunc, FILEOP_FLAGS flag = 0);

	static BOOL MoveFileEx(LPCTSTR szSrc,   LPCTSTR szDest) { return FileOperation(szSrc, szDest, FO_MOVE); }
	static BOOL CopyFileEx(LPCTSTR szSrc,   LPCTSTR szDest) { return FileOperation(szSrc, szDest, FO_COPY); }
	static BOOL RenameFileEx(LPCTSTR szSrc, LPCTSTR szDest) { return FileOperation(szSrc, szDest, FO_RENAME); }
	static BOOL DeleteFileEx(LPCTSTR szSrc)                 { return FileOperation(szSrc, NULL, FO_DELETE); }
	// �ݹ�ɾ��Ŀ¼(��֧�ֻ���վ)
	static BOOL DeleteDirectory(LPCTSTR pcsDirName);

	// ѡ��Ŀ¼
	static CString SelectDirectory(HWND hWnd=NULL);

	// ѡ���ļ�
	static CString SelectFile(BOOL bIsOpen,
		LPCTSTR szPath=_T(""),
		LPCTSTR szExt=_T("txt"),
		LPCTSTR szFilter=_T("�ı��ļ�(*.txt)|*.txt|�����ļ�(*.*)|*.*||") );

	// ѡ�����ļ�
	static CString SelectMutiFile( CStringArray &arrFiles, LPCTSTR szPath = _T(""),
		LPCTSTR szFilter=_T("�ı��ļ�(*.txt)\0*.txt") );
	
	// �����������ռ��ã���ֹʵ����
private:
	WND() {}
	~WND() {}
};

////////////////////////////////////////////////////////////////////////////////
#endif // _WND_H_