#ifndef _WND_H_
#define _WND_H_

// #ifdef _DEBUG
// 	#define HERE() AfxMessageBox(_T("I am here ！"));
// #else
// 	#define HERE() ;
// #endif // _DEBUG

////////////////////////////////////////////////////////////////////////////////

class WND
{
public:
	// 控制键是否按下 : CTRL / SHIFT / ALT
	static inline BOOL IsCtrlDown()  { return GetKeyState(VK_CONTROL) < 0; }
	static inline BOOL IsShiftDown() { return GetKeyState(VK_SHIFT) < 0; }
	static inline BOOL IsAltDown()   { return GetKeyState(VK_MENU) < 0; }
	// 字符键是否按键下(A-Z、a-z、0-9)
	static BOOL IsCharDown(int ch);
	
	// 文件或目录是否存在
	static inline BOOL IsExistFile(const CString &strFile) { return GetFileAttributes(strFile) != 0xFFFFFFFF; }
	// 目录 : 当前目录/临时目录、当前exe绝对目录、当前exe文件名
	static CString GetCurOrTempDir(BOOL bAddSep = FALSE, BOOL bIsTemp = FALSE);
	static CString GetAbsPath(BOOL bAddSep=FALSE);
	static CString GetExeName();
	// 搜索一级目录
	static int FileSearch(CString strRoot, CStringArray &arrFileName,
		BOOL bSearchDir=TRUE, const CString &strExt=_T(""), BOOL bSub=FALSE);
	
	// 根据类名和窗口标题查找窗口，并将窗口提到前台
	static VOID ActiveWindow(LPCTSTR lpszClassName, LPCTSTR lpszWindowName=NULL); 
	// 简化 ShellExecute
	static HINSTANCE WinExec(LPCSTR szExe, LPCSTR szPara=NULL, LPCTSTR szDir=NULL, BOOL bShow=TRUE)
	{
		return ShellExecute(NULL, NULL, szExe, szPara, szDir, bShow ? SW_SHOW : SW_HIDE);
	}
	
	// 查找 exe 是否正在运行，找到返回 PID，否则返回 -1
	static DWORD FindExePID(LPCTSTR pctsExeName);
	static BOOL  IsRunning(LPCTSTR pctsExeName) { return -1 != FindExePID(pctsExeName); }

	//	运行 szCmd 并等待进程结束，判断返回值是否为 dwExitCode
	static DWORD WaitProcess(LPCTSTR szCmd, BOOL bIsShow=TRUE, DWORD dwTimeOut=INFINITE);
    // 命令行前 iCount 项
    static int GetCommandLine(CStringArray &arrCmd, int iCount = -1);

	// 格式化系统错误信息
	static DWORD GetLastErrorInfo() {
		DWORD dwError = ::GetLastError();
		TCHAR szError[MAX_PATH];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, 0, 0, szError, sizeof(szError), NULL);
		AfxMessageBox(szError);
		return dwError;
	}
	// 控件
public:
	static BOOL FileOperation(LPCTSTR szSrc, LPCTSTR szDest, UINT nFunc, FILEOP_FLAGS flag = 0);

	static BOOL MoveFileEx(LPCTSTR szSrc,   LPCTSTR szDest) { return FileOperation(szSrc, szDest, FO_MOVE); }
	static BOOL CopyFileEx(LPCTSTR szSrc,   LPCTSTR szDest) { return FileOperation(szSrc, szDest, FO_COPY); }
	static BOOL RenameFileEx(LPCTSTR szSrc, LPCTSTR szDest) { return FileOperation(szSrc, szDest, FO_RENAME); }
	static BOOL DeleteFileEx(LPCTSTR szSrc)                 { return FileOperation(szSrc, NULL, FO_DELETE); }
	// 递归删除目录(不支持回收站)
	static BOOL DeleteDirectory(LPCTSTR pcsDirName);

	// 选择目录
	static CString SelectDirectory(HWND hWnd=NULL);

	// 选择文件
	static CString SelectFile(BOOL bIsOpen,
		LPCTSTR szPath=_T(""),
		LPCTSTR szExt=_T("txt"),
		LPCTSTR szFilter=_T("文本文件(*.txt)|*.txt|所有文件(*.*)|*.*||") );

	// 选择多个文件
	static CString SelectMutiFile( CStringArray &arrFiles, LPCTSTR szPath = _T(""),
		LPCTSTR szFilter=_T("文本文件(*.txt)\0*.txt") );
	
	// 仅当作域名空间用，禁止实例化
private:
	WND() {}
	~WND() {}
};

////////////////////////////////////////////////////////////////////////////////
#endif // _WND_H_