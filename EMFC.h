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
	// MFC-STL ��ת��
	static CString STL2MFC(string ss);
	static string  MFC2STL(CString &str);

	// �����ж�
	static BOOL IsNumber(const CString &str, const CString &ss = _T(""));
	static BOOL IsFloat(const CString &str, int *pNum=NULL);

	// �����ַ�
	static CString& Trim(CString &str, const CString &sf=_T("^�� "));
	
	// ����ת��
	static CString Int2Str(int iNum, const LPCTSTR &szFmt = _T("%d"));
	static CString Int2Str(const LPCTSTR &szFmt, int iCount, ... );
	//static CString FormatStr(const char *szFmt, ... );

	// ����ת�ַ�
	static const TCHAR I2C(int i) { ASSERT(0<=i && i<=9); return m_ch[i]; }

public:
	// �����ظ��ַ�
	static int ExcludeSame(CString &str);
	// �ų��ַ��������еĿմ�
	static int ExcludeNull(CStringArray &arr);
	// �ų������ظ���
	static int ExcludeSame(CUIntArray &arr, BOOL bOrder = TRUE);

	// ���鼯������
public:
	static int IntersectArray(CUIntArray &arr1, const CUIntArray &arr2);// arr1 = arr1 * arr2
	static int ExcludeArray(CUIntArray &arr1, const CUIntArray &arr2);	// arr1 = arr1 - arr2
	static int UnionArray(CUIntArray &arr1, const CUIntArray &arr2);	// arr1 = arr1 + arr2
	
public:
	// ��ӡ���� [iSt, iEnd) ֮���Ԫ��
	static CString PrintArray(const CUIntArray &arr, int iEnd=-1, int iSt=0, int iLineCnt = -1,
		const CString strFmt=_T("%d"), const CString strCRLF=_T("\n"));

	// �������� [iSt, iEnd) ֮���Ԫ��
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

	// �� [iSt, iEnd) �����Ԫ�أ��ҵ��������������򷵻� -1
public:
	// ��ģ�庯������ʱ�����Ԫ�����Ͳ�ƥ������Ҫǿ��ת��

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

	// �����������ռ��ã���ֹʵ����
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
