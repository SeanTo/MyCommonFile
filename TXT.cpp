#include "stdafx.h"
#include "TXT.h"

////////////////////////////////////////////////////////////////////////////////

/*
 *	从文件读到文件，每行间用 strCRLF 隔开
 */
CString TXT::Txt2String(const CString &strFile, const CString &strCRLF/*=_T(" ")*/)
{
	CString strLine, str = _T("");
	CStdioFile fin(strFile, CFile::modeRead | CFile::typeText );
	if( fin )
	{
		while( fin.ReadString(strLine) )
			str += strLine + strCRLF;

		fin.Close();
	}
	return str;
}

/*
 *	写数组内容文件，strFmt 控制格式，每行 iLineCnt 个元素
 */
BOOL TXT::Array2Txt(const CString strFileName, const CUIntArray &arr,
					const CString &strFmt/*=_T("%d")*/,
					const CString &strCRLF/*=_T("\n")*/,
					int iLineCnt/* =-1*/, BOOL bAdd/*=FALSE*/)
{
	int iCount = arr.GetSize();
	if( iCount < 1)
		return FALSE;

	CStdioFile fout(strFileName, CFile::modeCreate | CFile::modeWrite | (bAdd ? CFile::modeNoTruncate : 0));
	if(!fout)
	{
		AfxMessageBox( _T("打开文件失败！\n") + strFileName );
		return FALSE;
	}

	BOOL bCRLF = !strCRLF.IsEmpty();

	if(bAdd)
	{
		fout.SeekToEnd();
		if(bCRLF)
			fout.WriteString(strCRLF);
	}

	int cnt = 0;
	CString str, strLine(_T(""));
	for(int i = 0; i < iCount; ++i)
	{
		str.Format(strFmt, arr[i]);
		strLine += str;

		if(iLineCnt > 0 && (++cnt)==iLineCnt || i==iCount-1)
		{
			fout.WriteString(strLine + strCRLF);
			strLine.Empty();
			cnt = 0;
		}
	}
	fout.Close();
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
