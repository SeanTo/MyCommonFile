#ifndef _TXT_H_
#define _TXT_H_

////////////////////////////////////////////////////////////////////////////////

class TXT
{
	// �����������ռ��ã���ֹʵ����
private:
	TXT() {}
	~TXT() {}

public:
	// ���ļ�������ÿ�м��� strCRLF ����
	static CString Txt2String(const CString &strFile, const CString &strCRLF=_T(" "));

	// д���������ļ���strFmt ���Ƹ�ʽ��ÿ�� iLineCnt ��Ԫ��
	static BOOL Array2Txt(const CString strFileName, const CUIntArray &arr,
		const CString &strFmt=_T("%d"), const CString &strCRLF=_T("\n"),
		int iLineCnt=-1, BOOL bAdd=FALSE);
};

////////////////////////////////////////////////////////////////////////////////
#endif // _TXT_H_
