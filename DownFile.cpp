#include "StdAfx.h"

#include "DownFile.h"
#include "Mmsystem.h"			/* timeGetTime() */
#pragma comment( lib, "winmm" )	/* timeGetTime() */

//////////////////////////////////////////////////////////////////////////
// class CDownInfo

const CString CDownInfo::ERROR_INFO[] = {
	_T("���سɹ���"),
	_T("Դ��ַ���Ϸ���"),
	_T("�����ļ�ʧ�ܣ�"),
	_T("�޷���ȡ�ļ���С��"),
	_T("�û���ֹ���أ�"),
	_T("���س��ȴ���"),
	_T("�����쳣"),
	_T("�Ự�쳣��")
};

void CDownInfo::Reset(BOOL bFull/* = TRUE*/)
{
	strUrl.Empty();
	strFile.Empty();
	strStatus.Empty();
	
	nID		 = -1;
	nRet	 = -1;
	dwLength = 0;
	dwRead   = 0;
	dfUsedSec = 0.0;
	bBusy     = FALSE;
	bCancel   = FALSE;

	if( bFull )
	{
		hWnd = NULL;
		SetProgMode();
	}
}

void CDownInfo::SetFile(UINT id, const CString &url, const CString &file/* =_T */)
{
	Reset(FALSE);

	nID = id;
	strUrl = url;
	if( file.IsEmpty() )
		strFile = url.Mid(url.ReverseFind(_T('/')) + 1);
	else
		strFile = file;

	bBusy = TRUE;
	bNoStatus = FALSE;
}

void CDownInfo::SetProgMode(CProgressCtrl *prog /* = NULL */, CWnd *wnd /* = NULL */, BOOL bNo/* = FALSE*/)
{
	pCtrlProg = prog;
	pWndStauts= wnd;
	bNoStatus = bNo;
}

void CDownInfo::SetError(UINT nErr)
{
	if( nErr < sizeof(ERROR_INFO) / sizeof(CString)-1 )
		strStatus = ERROR_INFO[nErr];
	else if( strStatus.IsEmpty() )
		strStatus = ERROR_INFO[sizeof(ERROR_INFO) / sizeof(CString)-1];
	
	nRet = nErr;
	bBusy = FALSE;
	bCancel = FALSE;
	if( NULL != hWnd )
	{
		// �򸸴��ڷ�����Ϣ��֪ͨ�߳�ֹͣ
		::PostMessage(hWnd, WM_DOWN_END, nRet, nID);
	}
}

//////////////////////////////////////////////////////////////////////////
// static mehod

/*
 *	Get Http file
 */
UINT CDownInfo::GetFileFromHttp(LPVOID lpParam)
{
	ASSERT(lpParam);
	CDownInfo     *pInfo = (CDownInfo*)lpParam;
	CWnd	      *pWnd  = pInfo->bNoStatus ? NULL : pInfo->pWndStauts;

	pInfo->bBusy = TRUE;

	DWORD	dwServiceType;	// ��������
	CString strServerName;	// ��������
	CString strObject;		// ��ѯ������
	INTERNET_PORT nPort;	// �˿�
	
	UINT nRet = (UINT)ERR_OK;
	BOOL bUrlOK = AfxParseURL(pInfo->strUrl, dwServiceType, strServerName, strObject, nPort);

	// ֻ���� HTTP ����
	if( !bUrlOK && (dwServiceType == INTERNET_SERVICE_HTTP) )
	{
		nRet = (UINT)ERR_URL;
		pInfo->SetError(nRet);
// 		if( NULL != pWnd )
// 			pWnd->SetWindowText( pInfo->strStatus );
		return nRet;
	}
	
	CInternetSession	session;			// �Ự����
	CHttpConnection		*pServer = NULL;	// ָ���������ַ
	CHttpFile			*pHttpFile = NULL;	// HTTP�ļ�ָ��

	// �����־
	DWORD dwHttpRequestFlags = INTERNET_FLAG_NO_AUTO_REDIRECT;
	const TCHAR szHeaders[]=_T("Accept: */*\r\nUser-Agent:HttpClient\r\n");

	try
	{
		// �򿪱����ļ�
		CFile fout;
		if( !fout.Open(pInfo->strFile, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary) )
		{
			pHttpFile->Close();
			pServer ->Close();
			if (pHttpFile != NULL) delete pHttpFile;
			if (pServer != NULL) delete pServer;
			session.Close();

			nRet = (UINT)ERR_SAVE;
			pInfo->SetError(nRet);
// 			if( NULL != pWnd )
// 				pWnd->SetWindowText( pInfo->strStatus );
			return nRet;
		}
		
		// ����http���ӡ������ϵ� http �ļ�ָ��
		pServer = session.GetHttpConnection(strServerName, dwServiceType, nPort);
		pHttpFile = pServer-> OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject,
			NULL, 1, NULL, NULL, dwHttpRequestFlags);
		pHttpFile->AddRequestHeaders(szHeaders);
		pHttpFile->SendRequest();
		
		// ����
		const int BUF_LEN = 1024 * 2;
		BYTE  byBuff[BUF_LEN];

		// �ļ�����
		CString queryinfo(_T(""));
		pHttpFile->QueryInfo(HTTP_QUERY_STATUS_CODE, queryinfo);
		if(queryinfo == _T("200")
			&& pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, queryinfo) )
		{   
			pInfo->dwLength = _ttoi(queryinfo);
		}
		else
		{
			pHttpFile->Close();
			pServer ->Close();
			if (pHttpFile != NULL) delete pHttpFile;
			if (pServer != NULL) delete pServer;
			session.Close();
			
			nRet = (UINT)ERR_LENGTH;
			pInfo->SetError(nRet);
// 			if( NULL != pWnd )
// 				pWnd->SetWindowText( pInfo->strStatus );
			return nRet;
		}
		
		CString str;
		double dfTotalSecs  = 0.0;	// Ԥ��ʱ��(��)
		double dfAveSpeed   = 0.0;	// ƽ���ٶ�(KB/s)
		
		UINT nRead = 0;
		DWORD tStart = timeGetTime();
		while ( (nRead = pHttpFile->Read(byBuff, BUF_LEN)) > 0 )
		{
			if( pInfo->bCancel )
			{
				nRet = (UINT)ERR_ABORT;
				pInfo->SetError(nRet);
// 				if( NULL != pWnd )
// 					pWnd->SetWindowText( pInfo->strStatus );
				return nRet;
			}

			fout.Write(byBuff, nRead);
			pInfo->dwRead += nRead;
			pInfo->dfUsedSec = (timeGetTime() - tStart) / 1000.0;

			// ������
			if( NULL != pInfo->pCtrlProg )
				pInfo->pCtrlProg->SetPos(pInfo->dwRead * 100 / pInfo->dwLength);

			// ����״̬
			if( !pInfo->bNoStatus )
			{
				if(pInfo->dfUsedSec > 0.0)
				{
					dfAveSpeed = pInfo->dwRead / pInfo->dfUsedSec / 1024.0;
					dfTotalSecs = (double)pInfo->dwLength / 1024.0 / dfAveSpeed;
					
					pInfo->strStatus.Format(_T("\n���أ�%.1f/%.1f(KB) ʱ�䣺%.0f/%.0f(Sec) ƽ���ٶȣ�%.1f(KB/S)"),
						pInfo->dwRead / 1024.0, pInfo->dwLength / 1024.0,
						pInfo->dfUsedSec, dfTotalSecs, dfAveSpeed);
				}
				else
				{
					pInfo->strStatus.Format(_T("\n���أ�%.1f/%.1f(K)"),
						pInfo->dwRead / 1024.0, pInfo->dwLength / 1024.0);
				}
				str = pInfo->strFile;
				str = str.Mid(str.ReverseFind(_T('\\'))+1);
				pInfo->strStatus = CString(_T("�ļ���...\\")) + str + pInfo->strStatus;
				
				if( NULL != pWnd )
					pWnd->SetWindowText(pInfo->strStatus);
			}

/*
			// ��������ʱ����Ϣѭ��
			MSG  msg;   
			while(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if(WM_DOWN_CANCEL == msg.message)
				{
					fout.Close();
					pHttpFile ->Close();
					pServer ->Close();
					if (pHttpFile != NULL) delete pHttpFile;
					if (pServer != NULL) delete pServer;
					session.Close();
					
					Sleep(1000);
					nRet = 4;
					pInfo->SetError(nRet);
					if( NULL != pWnd )
						pWnd->SetWindowText( pInfo->strStatus );
 					//::PostMessage(pInfo->hWnd, WM_CLOSE);
					return nRet;
					
				}
				::TranslateMessage(&msg);   
				::DispatchMessage(&msg);
			}
*/
		}
		fout.Close();
			

		// �Ͽ�����
		pHttpFile->Close();
		pServer->Close();
		if (pHttpFile != NULL) delete pHttpFile;
		if (pServer != NULL) delete pServer;
		session.Close();

		// ������س���
		if( pInfo->dwRead < pInfo->dwLength )
			nRet = 5;
	}
    catch(CInternetException *e)
    {
        TCHAR tszErr[1024];
        if(e->GetErrorMessage(tszErr, sizeof(tszErr)))
		{
            pInfo->strStatus = tszErr;
			e->Delete();
		}
		else
			pInfo->strStatus.Empty();
		
		nRet = (UINT)ERR_EXCEPT;
    }
	catch(...)
	{
		nRet = (UINT)ERR_SESSION;
	}

	pInfo->SetError(nRet);
// 	if( NULL != pWnd )
// 		pWnd->SetWindowText( pInfo->strStatus );

	return nRet;
}

// end CDownInfo
//////////////////////////////////////////////////////////////////////////

// UINT  UpdateProgress(LPVOID lpParam)
// {
// 	ASSERT(lpParam);
// 	CDownInfo *pInfo = (CDownInfo*)lpParam;
// 
// 	while( TRUE )
// 	{
// 		::SetDlgItemText(AfxGetMainWnd()->m_hWnd,IDC_STATIC_STATUS,pInfo->strStatus);
// 		if( pInfo->dwRead >= pInfo->dwLength )
// 			break;
// 	}
// 	return 0;
// }




/*
BOOL GetSourceHtml(CString theUrl, CString Filename)
{
	CInternetSession session;
	CInternetFile* file = NULL;
	try
	{
		// �������ӵ�ָ��URL
		file = (CInternetFile*) session.OpenURL(theUrl); 
	}
	catch (CInternetException* m_pException)
	{
		// ����д���Ļ������ļ�Ϊ��
		file = NULL; 
		m_pException->Delete();
		return FALSE;
	}
	
	// ��dataStore�������ȡ����ҳ�ļ�
	CStdioFile dataStore;
	if (file)
	{
		CString  somecode;	//Ҳ�ɲ���LPTSTR���ͣ�������ɾ���ı��е�\n�س���
		BOOL bIsOk = dataStore.Open(Filename,
			CFile::modeCreate 
			| CFile::modeWrite 
			| CFile::shareDenyWrite 
			| CFile::typeText);
		
		if (!bIsOk)
			return FALSE;
		
		// ��д��ҳ�ļ���ֱ��Ϊ��
		while (file->ReadString(somecode) != NULL) //�������LPTSTR���ͣ���ȡ������nMax��0��ʹ�������ַ�ʱ����
		{
			dataStore.WriteString(somecode);
			dataStore.WriteString("\n");		   //���somecode����LPTSTR����,�ɲ��ô˾�
		}
		
		file->Close();
		delete file;
	}
	else
	{
		dataStore.WriteString(_T("��ָ�������������ӽ���ʧ��..."));	
		return FALSE;
	}
	return TRUE;
}
*/

/*
HRESULT CreateInterShortcut (LPCSTR pszURL, LPCSTR pszURLfilename,
							 LPCSTR szDescription,LPCTSTR szIconFile = NULL,int nIndex = -1)
{//ͨ��ShellLink�ӿ�������URL
	HRESULT hres;
	
	CoInitialize(NULL); 
	
	IUniformResourceLocator *pHook;
	
	hres = CoCreateInstance (CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
		IID_IUniformResourceLocator, (void **)&pHook);
	//���CLSID_InternetShortcut�ӿ�
	
	if (SUCCEEDED (hres))
	{
		IPersistFile *ppf;
		IShellLink *psl;
		
		// Query IShellLink for the IPersistFile interface for 
		hres = pHook->QueryInterface (IID_IPersistFile, (void **)&ppf);
		hres = pHook->QueryInterface (IID_IShellLink, (void **)&psl);
		
		if (SUCCEEDED (hres))
		{ 
			WORD wsz [MAX_PATH]; // buffer for Unicode string
			
			// Set the path to the shortcut target.
			pHook->SetURL(pszURL,0);
			
			hres = psl->SetIconLocation(szIconFile,nIndex);
			
			if (SUCCEEDED (hres))
			{
				// Set the description of the shortcut.
				hres = psl->SetDescription (szDescription);
				
				if (SUCCEEDED (hres))
				{
					// Ensure that the string consists of ANSI characters.
					MultiByteToWideChar (CP_ACP, 0, pszURLfilename, -1, wsz, MAX_PATH);
					
					// Save the shortcut via the IPersistFile::Save member function.
					hres = ppf->Save (wsz, TRUE);
				}
			}
			
			// Release the pointer to IPersistFile.
			ppf->Release ();
			psl->Release ();
		}
		
		// Release the pointer to IShellLink.
		pHook->Release ();
		
	}
	return hres;
}
*/
