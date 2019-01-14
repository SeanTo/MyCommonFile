#ifndef _EMFC_H_
#define _EMFC_H_

//////////////////////////////////////////////////////////////////////////

// #include	"stdarg.h"

#include	"afxtempl.h"
#include	"string"
using		std::string;

#define		MsgBox	AfxMessageBox

//////////////////////////////////////////////////////////////////////////

class EMFC
{
public:
	// MFC-STL 串转换
	static CString STL2MFC(string ss);
	static string  MFC2STL(CString &str);

	// 数字判断
	static BOOL IsNumber(const CString &str, const CString &ss = _T(""));
	static BOOL IsFloat(const CString &str, int *pNum=NULL);

	// 过滤字符
	static CString& Trim(CString &str, const CString &sf=_T("^　 "));
	
	// 整数转串
	static CString Int2Str(int iNum, const LPCTSTR &szFmt = _T("%d"));
	static CString Int2Str(const LPCTSTR &szFmt, int iCount, ... );
	//static CString FormatStr(const char *szFmt, ... );

	// 数字转字符
	static const TCHAR I2C(int i) { ASSERT(0<=i && i<=9); return m_ch[i]; }

public:
	// 过滤重复字符
	static int ExcludeSame(CString &str);
	// 排除字符串数组中的空串
	static int ExcludeNull(CStringArray &arr);
	// 排除数组重复项
	static int ExcludeSame(CUIntArray &arr, BOOL bOrder = TRUE);

	// 数组集合运算
public:
	static int IntersectArray(CUIntArray &arr1, const CUIntArray &arr2);// arr1 = arr1 * arr2
	static int ExcludeArray(CUIntArray &arr1, const CUIntArray &arr2);	// arr1 = arr1 - arr2
	static int UnionArray(CUIntArray &arr1, const CUIntArray &arr2);	// arr1 = arr1 + arr2
	
public:
	// 打印数组 [iSt, iEnd) 之间的元素
	static CString PrintArray(const CUIntArray &arr, int iEnd=-1, int iSt=0, int iLineCnt = -1,
		const CString strFmt=_T("%d"), const CString strCRLF=_T("\n"));

	// 排序数组 [iSt, iEnd) 之间的元素
public:
	template <typename T>
	static void SortArray(T arr[], int iEnd, BOOL bUp=TRUE, int iSt = 0)
	{
		if(iEnd <= 1)
			return;

		T k, num;
		for(int i = iSt; i < iEnd-1; ++i)
		{
			k = i;
			for(int j = i + 1; j < iEnd; ++j)
			{
				if( bUp && arr[k]>arr[j] || !bUp && arr[k]<arr[j] )
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

	static void SortArray(CUIntArray &arr, BOOL bAsc=TRUE, int iSt=0, int iEnd=-1);

	// 在 [iSt, iEnd) 间查找元素，找到返回索引，否则返回 -1
public:
	// 非模板函数调用时如查找元素类型不匹配则需要强制转换

	template <typename T>
	static int  FindArray(const T e, const T arr[], int iEnd, int iSt=0);
	template <typename T>
	static int  FindArray(const T e, const CArray<T, T> &arr, int iEnd=-1, int iSt=0);

	static int  FindArray(const UINT e, const CUIntArray &arr, int iEnd=-1, int iSt=0);
	static int  FindArray(const CString &e, const CStringArray &arr, int iEnd=-1, int iSt=0);
	static int  FindArray(TCHAR ch, const CString &str, int iEnd = -1, int iSt = 0);
	static int  FindArray(int n, const CString &str, int iEnd = -1, int iSt = 0) { return FindArray(I2C(n), str, iEnd, iSt); }
	
	template <typename T>
	static BOOL IsInArray(const T e, const T arr[], int iEnd, int iSt=0) { return -1 != FindArray(e, arr, iEnd, iSt); }
	template <typename T>
	static BOOL IsInArray(const T e, const CArray<T, T> &arr, int iEnd=-1, int iSt=0) { return -1 != FindArray(e, arr, iEnd, iSt); }
	
	static BOOL IsInArray(const UINT e, const CUIntArray &arr, int iEnd=-1, int iSt=0) { return -1 != FindArray((UINT)e, arr, iEnd, iSt); }
	static BOOL IsInArray(const CString &e, const CStringArray &arr, int iEnd=-1, int iSt=0) { return -1 != FindArray(e, arr, iEnd, iSt); }
	static BOOL IsInArray(TCHAR ch, const CString &str, int iEnd = -1, int iSt = 0) { return -1 != FindArray(ch, str, iEnd, iSt); }
	static BOOL IsInArray(int n, const CString &str, int iEnd = -1, int iSt = 0) { return -1 != FindArray(I2C(n), str, iEnd, iSt); }

	// 仅当作域名空间用，禁止实例化
private:
	EMFC() {}
	~EMFC() {}

	enum { BUF_LEN = 1024 };
	static TCHAR m_ch[];
};

//////////////////////////////////////////////////////////////////////////

template <typename T>
int  EMFC::FindArray(const T e, const T arr[], int iEnd, int iSt/*=0*/)
{
	for(int i = iSt; i < iEnd; ++i)
	{
		if( e == arr[i] )
			return i;
	}
	return -1;
}

template <typename T>
static int  FindArray(const T e, const CArray<T, T> &arr, int iEnd=-1, int iSt=0)
{
	if(iEnd < 0)	
		iEnd = arr.GetSize();

	for(int i = iSt; i < iEnd; ++i)
	{
		if( e == arr[i] )
			return i;
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////////////
#endif // _EMFC_H_
