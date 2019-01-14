#include "stdafx.h"
#include "EMFC.h"

//////////////////////////////////////////////////////////////////////////

TCHAR EMFC::m_ch[] = {
	_T('0'), _T('1'), _T('2'), _T('3'), _T('4'),
	_T('5'), _T('6'), _T('7'), _T('8'), _T('9')
};

CString EMFC::STL2MFC(string ss)
{
	CString str;
	str.Format(_T("%s"), ss.c_str());
	return str;
}

string EMFC::MFC2STL(CString &str)
{
	string ss;
	ss.append(str.GetBuffer(str.GetLength()));
	str.ReleaseBuffer();
	return ss;
}

// 数字串
BOOL EMFC::IsNumber(const CString &str, const CString &ss/* = _T("")*/)
{
	if( str.IsEmpty() )
		return FALSE;
	
	CString strDigit = _T("1234567890") + ss;
	
	for(int i = 0; i < str.GetLength(); ++i)
	{
		if( !IsInArray( str.GetAt(i), strDigit) )
			return FALSE;
	}
	
	return TRUE;
}

/*
 *	浮点数判断，pNum 返回小数后的位数
 */
BOOL EMFC::IsFloat(const CString &str, int *pNum/* = NULL*/)
{
	int iDot = str.ReverseFind(_T('.'));
	if(-1 == iDot)
		return FALSE;

	int iLen = str.GetLength();
	if(NULL != pNum)
		*pNum = iLen - iDot - 1;

	if( !IsNumber(str.Left(iDot)) || !IsNumber(str.Right(iLen-iDot-1)) )
		return FALSE;
	
	return TRUE;
}

// 浮点数
// BOOL EMFC::IsFloat(const CString &str, int l/*=11*/, int r/*=2*/)
// {
// 	int iLen = str.GetLength();
// 	if(iLen > l + 1 + r || iLen < 2 + r)
// 		return FALSE;
// 	
// 	if( !IsNumber(str.Right(r)) || !IsNumber(str.Left(iLen-1-r)) || _T('.') != str.Right(1+r)[0] )
// 		return FALSE;
// 	
// 	return TRUE;
// }

/*
 *	过滤字符
 */
CString& EMFC::Trim(CString &str, const CString &sf/* =_T */)
{
	if(sf.IsEmpty())
		return str;

	for(int i = 0; i < sf.GetLength(); ++i)
		str.Replace( sf.GetAt(i), _T(' '));
	
	str.Replace(_T(" "), _T(""));
	return str;
}

/*
 *  单个整转字符串
 */
CString EMFC::Int2Str(int iNum, const LPCTSTR &szFmt/* = _T("%d")*/)
{
	CString str;
	str.Format(szFmt, iNum);
	return str;
}

/*
 *  多个整转字符串
 */
CString EMFC::Int2Str(const LPCTSTR &szFmt, int iCount, ... )
{
	TCHAR buf[BUF_LEN] = { _T('\0') };

	if( iCount > 0 )
	{
		va_list pArgList;
		va_start( pArgList, iCount );

		for(int i = 0; i < iCount; ++i)
			sprintf( buf + strlen(buf), szFmt, va_arg(pArgList, int) );
		
		va_end( pArgList );
	}
	return CString(buf);
}

/*
CString EMFC::FormatStr(const char *szFmt, ... )
{
	TCHAR buf[BUF_LEN];
	va_list pArgList;

	va_start(pArgList, szFmt);

	_vsnprintf (buf, BUF_LEN, szFmt, pArgList );

	return CString(buf);
}
*/

/*
 * 过滤重复字符
 */
int EMFC::ExcludeSame(CString &str)
{
	int i = str.GetLength() - 1;
	int n = -1;

	while(i > 0)
	{
		n = str.Find(str[i]);
		if( n >=0 && n < i )
			str.Delete(i);
		
		--i;
	}
	
	return str.GetLength();
}

/*
 *	过滤数组中的重复项
 */
int EMFC::ExcludeSame(CUIntArray &arr, BOOL bOrder)
{
	int i = 0;
	if( bOrder )
	{
		SortArray(arr);
		
		while(i+1 < arr.GetSize())
		{
			if(arr[i] == arr[i+1])
				arr.RemoveAt(i);
			else
				++i;
		}
	}
	else
	{
		i = arr.GetSize() - 1;

		while( i - 1 > 0 )
		{
			if( EMFC::IsInArray(arr[i], arr, i - 1) )
				arr.RemoveAt(i);

			--i;
		}
	}

	return arr.GetSize();
}

/*
 *	过滤数组中的空串
 */
int EMFC::ExcludeNull(CStringArray &arr)
{
	int i = arr.GetSize() - 1;
	
	while(i > 0)
	{
		if( arr[i].IsEmpty() )
			arr.RemoveAt(i);
		
		--i;
	}
	
	return arr.GetSize();
}

/*
 *	交集 arr1 = arr1 * arr2
 */
int EMFC::IntersectArray(CUIntArray &arr1, const CUIntArray &arr2)
{
	if(arr1.GetSize() > 0 && arr2.GetSize() > 0)
	{
		int i = 0;
		while( i < arr1.GetSize() )
		{
			if( !IsInArray(arr1[i], arr2) )
				arr1.RemoveAt(i);
			else
				++i;
		}
	}
	else if(0 == arr2.GetSize())
		arr1.RemoveAll();

	return arr1.GetSize();
}

/*
 *	差集 arr1 = arr1 - arr2
 */
int EMFC::ExcludeArray(CUIntArray &arr1, const CUIntArray &arr2)
{
	if(arr1.GetSize() > 0 && arr2.GetSize() > 0)
	{
		int i = 0;
		while( i < arr1.GetSize() )
		{		
			if( IsInArray(arr1[i], arr2) )
				arr1.RemoveAt(i);
			else
				++i;
		}
	}

	return arr1.GetSize();
}

/*
 *	并集 arr1 = arr1 + arr2
 */
int EMFC::UnionArray(CUIntArray &arr1, const CUIntArray &arr2)
{
	if(arr1.GetSize() > 0 || arr2.GetSize() > 0)
	{
		for(int i = 0; i < arr2.GetSize(); ++i)
		{
			if( !EMFC::IsInArray(arr2[i], arr1) )
				arr1.InsertAt(0, arr2[i]);
		}		
	}

	return arr1.GetSize();
}

/*
 * 打印数组前面 [iSt - iEnd) 之间的元素
 */
CString EMFC::PrintArray(const CUIntArray &arr,
						 int iEnd /*= -1*/, int iSt /* = 0 */,
						 int iLineCnt/* = -1*/,
						 const CString strFmt/*=_T("%d")*/,
						 const CString strCRLF/*=_T("\n")*/ )
{
	CString str = _T("");
	int iCount = arr.GetSize();
	if(0 == iCount)
		return str;

	if(iEnd < 0 || iEnd > iCount)
		iEnd = iCount;
	
	iCount = 0;
	CString ss;
	for(int i = iSt; i < iEnd; ++i)
	{
		ss.Format(strFmt, arr[i]);
		str += ss;

		// 换行
		if(iLineCnt > 0 && (++iCount) == iLineCnt)
		{
			iCount = 0;
			str += strCRLF;
		}
	}
	return str;
}

/*
 *	数组排序，bAsc = TRUE 为升序、FALSE 为降序
 */
VOID EMFC::SortArray(CUIntArray &arr, BOOL bAsc /* = TRUE */, int iSt/*=0*/, int iEnd/*=-1*/)
{
	if(iEnd < 0 || iEnd > arr.GetSize())
		iEnd = arr.GetSize();
	
	int k, num;
	for(int i = iSt; i < (iEnd - 1); ++i)
	{
		k = i;
		for(int j = i + 1; j < iEnd; ++j)
		{
			if( bAsc && arr[k]>arr[j] || !bAsc && arr[k]<arr[j] )
				k = j;
		}
		
		if(k > i)
		{
			num = arr[i];
			arr[i] = arr[k];
			arr[k] = num;
		}
	}
}

int EMFC::FindArray(const UINT e, const CUIntArray &arr, int iEnd/*=-1*/, int iSt/*=0*/)
{
	if(iEnd < 0 || iEnd > arr.GetSize())
		iEnd = arr.GetSize();
	
	for(int i = iSt; i < iEnd; ++i)
	{
		if( e == arr[i] )
			return i;
	}
	return -1;
}

int EMFC::FindArray(const CString &e, const CStringArray &arr, int iEnd/*=-1*/, int iSt/*=0*/)
{
	if(iEnd < 0 || iEnd > arr.GetSize())
		iEnd = arr.GetSize();
	
	for(int i = iSt; i < iEnd; ++i)
	{
		if( e == arr[i] )
			return i;
	}
	return -1;
}

/*
 * 在 str 中查找数字 n
 * 找到返回索引，否则返回 -1
 */
int EMFC::FindArray(TCHAR ch, const CString &str, int iEnd/* = -1*/, int iSt /*= 0*/)
{
	int len = str.Find(ch, iSt);
	return (-1 != len) && (len >= iSt && len < iEnd || -1 == iEnd) ? len : -1;
}
