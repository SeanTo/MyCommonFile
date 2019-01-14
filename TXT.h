#ifndef _TXT_H_
#define _TXT_H_

////////////////////////////////////////////////////////////////////////////////

class TXT
{
	// 仅当作域名空间用，禁止实例化
private:
	TXT() {}
	~TXT() {}

public:
	// 读文件到串，每行间用 strCRLF 隔开
	static CString Txt2String(const CString &strFile, const CString &strCRLF=_T(" "));

	// 写数组内容文件，strFmt 控制格式，每行 iLineCnt 个元素
	static BOOL Array2Txt(const CString strFileName, const CUIntArray &arr,
		const CString &strFmt=_T("%d"), const CString &strCRLF=_T("\n"),
		int iLineCnt=-1, BOOL bAdd=FALSE);
};

////////////////////////////////////////////////////////////////////////////////
#endif // _TXT_H_
