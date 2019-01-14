#ifndef __DownFile_h__
#define __DownFile_h__

#include <afxinet.h>

// #define DOWNLOAD_END USER+1
#define WM_DOWN_END (WM_USER + 105)

class CDownInfo
{	
public:
	CString			strUrl;		// �ļ������ַ
	CString			strFile;	// ���ش洢ȫ·����
	CString			strStatus;	// ����״̬

	CWnd			*pWndStauts;// ״̬��ʾ����
	CProgressCtrl	*pCtrlProg;	// ������

	HWND			hWnd;		// ������Ϣ����

	UINT			nID;		// ���� ID
	UINT			nRet;		// ����ֵ, 0 ����
	DWORD			dwLength;	// �ļ��ֽ�����
	DWORD			dwRead;		// �������ֽ���
	double			dfUsedSec;	// ��������
	BOOL			bBusy;		// ������...
	BOOL			bNoStatus;	// �����̲߳�����״̬
	BOOL			bCancel;	// ��ֹ��������

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
