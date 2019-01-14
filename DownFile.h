#ifndef __DownFile_h__
#define __DownFile_h__

#include <afxinet.h>

// #define DOWNLOAD_END USER+1
#define WM_DOWN_END (WM_USER + 105)

class CDownInfo
{	
public:
	CString			strUrl;		// 文件网络地址
	CString			strFile;	// 本地存储全路径名
	CString			strStatus;	// 下载状态

	CWnd			*pWndStauts;// 状态显示窗口
	CProgressCtrl	*pCtrlProg;	// 进度条

	HWND			hWnd;		// 接收消息窗口

	UINT			nID;		// 任务 ID
	UINT			nRet;		// 返回值, 0 正常
	DWORD			dwLength;	// 文件字节总数
	DWORD			dwRead;		// 已下载字节数
	double			dfUsedSec;	// 已用秒数
	BOOL			bBusy;		// 下载中...
	BOOL			bNoStatus;	// 下载线程不处理状态
	BOOL			bCancel;	// 终止下载命令

	enum { ERR_OK = 0, ERR_URL=1, ERR_SAVE=2, ERR_LENGTH=3,
		ERR_ABORT=4, ERR_READ=5, ERR_EXCEPT=6, ERR_SESSION=7
	};
	
public:
	CDownInfo()  { Reset(); }
	~CDownInfo() { Reset(); }

	void SetError(UINT nErr);
	void SetFile(UINT id, const CString &url, const CString &file=_T(""));
 	void SetProgMode(CProgressCtrl *prog = NULL, CWnd *wnd = NULL, BOOL bNo = FALSE);

	CString GetLastError() const { return !bBusy ? ERROR_INFO[nRet] : _T(""); }
	CString GetLastError(int i) const { ASSERT(i>=0 && i<(int)ERR_SESSION);  return ERROR_INFO[i]; }

	static UINT  GetFileFromHttp(LPVOID lpParam);

private:
	static const CString ERROR_INFO[];
	void Reset(BOOL bFull = TRUE);
};

// DWORD GetHttpFileLength(const CHttpFile *pFile);
// UINT  UpdateProgress(LPVOID lpParam);

#endif // __DownFile_h__
