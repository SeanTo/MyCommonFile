/*==================================================================
=  �ļ���:CIniFile ��ʵ���ļ�                                      
=  �޸�����:2002-12-28                                              
=  ����:��Ƥ                                                         
=   E_Mail:peijikui@sd365.com  msn:peijikui@sina.com                                                              = 
====================================================================*/
#include "stdafx.h"
#include "IniFile.h"

CIniFile::CIniFile() : bFileExsit(FALSE)
{
}

CIniFile::~CIniFile()
{
	if(bFileExsit)
	{
		if(stfFile.Open(strInIFileName, CFile::modeCreate | CFile::modeWrite) && FileContainer.GetSize() > 0)
		{
			CString strParam;
			for(int i = 0; i < FileContainer.GetSize(); i++)
			{
				strParam = FileContainer[i];

				// SECTION ���һ�У�2008��2��18�գ�By: ������
				if( i > 0 && strParam.Find(_T("[")) >= 0 )
					stfFile.WriteString(_T("\n"));
				
				stfFile.WriteString(strParam + _T("\n"));
			}
		}
		stfFile.Close();
	}
	
	if(FileContainer.GetSize() > 0)
	{
		FileContainer.RemoveAll();
	}
}

BOOL CIniFile::Create(const CString &strFileName)
{
	bFileExsit = FALSE;
	strInIFileName = strFileName;
	if(!stfFile.Open(strFileName, CFile::modeRead))
	{
		return bFileExsit;
	}
	
	CString strFileLine;
	while(bFileExsit = stfFile.ReadString(strFileLine))
	{
		if(bFileExsit == FALSE)
			return bFileExsit;

		// ����ʱ�˵����У�2008��2��18�գ�by: ������
		strFileLine.TrimLeft();
		strFileLine.TrimRight();
		if( !strFileLine.IsEmpty() )
			FileContainer.Add(strFileLine);
	}
	stfFile.Close();
    
	bFileExsit = TRUE;
	return bFileExsit;
}

BOOL CIniFile::GetVar(const CString &strSection, const CString &strVarName, CString &strReturnValue)
{
	if(bFileExsit == FALSE || FileContainer.GetSize() < 0)
		return bFileExsit;
	
	int iLine = SearchLine(strSection, strVarName);
	if(iLine > 0)
	{
		CString strParam = FileContainer[iLine - 1];
	    strReturnValue = strParam.Mid(strParam.Find(_T('=')) + 1);
		return TRUE;
	}
	return FALSE;
}

BOOL CIniFile::GetVarStr(const CString &strSection, const CString &strVarName, CString &strReturnValue)
{
	return(GetVar(strSection, strVarName, strReturnValue));
}

BOOL CIniFile::GetVarInt(const CString &strSection, const CString &strVarName, int &iValue)
{
	CString  strReturnVar;
	if(GetVar(strSection, strVarName, strReturnVar))
	{
		strReturnVar.TrimLeft();
		int iLen = strReturnVar.GetLength();
		iValue = _ttoi(strReturnVar.GetBuffer(iLen));
		strReturnVar.ReleaseBuffer();
		return TRUE;
	}
	return FALSE;
}

BOOL CIniFile::SetVar(const CString &strSection, const CString &strVarName, const CString &strVar, const int iType)
{
	if(bFileExsit == FALSE )
		return bFileExsit;

	if(FileContainer.GetSize() == 0)
	{
		FileContainer.Add(_T("[") + strSection + _T("]"));
		FileContainer.Add(strVarName + _T("=") + strVar);
		return TRUE;
	}

	int i = 0;
	int iFileLines = FileContainer.GetSize();

	while(i < iFileLines)
	{
		CString strValue = FileContainer.GetAt(i++);
		strValue.TrimLeft();
		if((strValue.Find(_T("[")) >=0) && (strValue.Find(strSection) >=0))
		{	
			while(i < iFileLines)
			{
			
				CString strSectionList = FileContainer[i++];
				strSectionList.TrimLeft();
				
				if(strSectionList.Find(_T('#')) >= 0)	// �ҵ�ע����
					continue;
				
				if(strSectionList.Find(strVarName) >= 0)	// �ҵ�
				{
					CString strParam = strVarName + "=" + strVar;
					FileContainer[i-1] = strParam;
					return TRUE;
				}

				/*
				 * ��ԭ���ļ���SECTION��, û����Ӧ�ı���, ��Ҫ���
				 * ��������������±߻��б��section
				 * ����������������, ���Ȱѵ�ǰ����ֵ��������ƶ�
				 * Ȼ���ڵ�ǰλ�ü����³��ֵ���ֵ
				 */
				if(strSectionList.Find(_T("["), 0) >= 0)
				{
					if( iType != 0 )
						return FALSE;

					CString strParam;
					FileContainer.Add(strParam);
					int iPre = FileContainer.GetSize() - 1;
					
					while(iPre >= i)
					{
						CString strBehind = FileContainer[iPre - 1];
						FileContainer[iPre] = strBehind;
						--iPre;

					}
					
					strParam = strVarName + "=" + strVar;
					FileContainer.SetAt(i - 1, strParam);
					return TRUE;
				}
				
				if(i == iFileLines && iType == 0)
				{
					FileContainer.Add(strVarName + "=" + strVar);
					return TRUE;
				}
			}
		}
	}

	if(iType == 0)
	{
		FileContainer.Add(_T("[") + strSection + _T("]"));
		FileContainer.Add(strVarName + "=" + strVar);
	}
	
	return TRUE;

}

BOOL CIniFile::SetVarStr(const CString &strSection, const CString &strVarName, const CString &strValue, const int iType)
{
	return SetVar(strSection, strVarName, strValue, iType);
}

BOOL CIniFile::SetVarInt(const CString &strSection, const CString &strVarName, const int &iValue, const int iType)
{
	CString strVar;
	strVar.Format("%d", iValue);
	return (SetVar(strSection, strVarName, strVar, iType));
}

int CIniFile::SearchLine(const CString &strSection, const CString &strVarName)
{
	if(FileContainer.GetSize() > 0)
	{
		int i = 0;
		int iFileLines = FileContainer.GetSize();

		while(i < iFileLines)
		{
			CString strValue = FileContainer[i++];
			strValue.TrimLeft();
			
			if( strValue.Find(_T("[")) >= 0 && strValue.Find(strSection, 1) >= 0 )
			{	
				while(i < iFileLines)
				{
					CString strSectionList = FileContainer[i++];
					strSectionList.TrimLeft();

					if( strSectionList.Find(_T('#')) >= 0 ) // ע����
						continue;

					if( strSectionList.Find(strVarName) >= 0 ) // �ҵ�
					{
						return i;
					}
					
					if(strSectionList.Find(_T("["), 0) >= 0) // ����һ���������,Ѱ��ʧ��
					{
						return -2;
					}
				}
			}
		}
	}

	return -1;
}