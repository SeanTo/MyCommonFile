#ifndef _MY_DATE_H_
#define _MY_DATE_H_

////////////////////////////////////////////////////////////////////////////////
class MyDT
{
	// 仅当作域名空间用，禁止实例化
private:
	MyDT() {}
	~MyDT() {}
	
public:
	static void SetNow() { day = CTime::GetCurrentTime(); }
	
public:
	static int Year()	{ return day.GetYear(); }
	static int Mon()	{ return day.GetMonth(); }
	static int Day()	{ return day.GetDay(); }
	static int Week()	{ return day.GetDayOfWeek() - 1; }
	
	static int Hour()	{ return day.GetHour(); }
	static int Min()	{ return day.GetMinute(); }
	static int Sec()	{ return day.GetSecond(); }
	
	static CString Week(LPCTSTR szWeek[]) { return NULL!=szWeek ? szWeek[Week()] : m_WEEK[Week()]; }
	
public:
	// 闰年
	static BOOL IsLeapYear(int nYear)
	{
		ASSERT(nYear > 0);
		return 0 == (nYear%4) && 0 != (nYear%100) || 0 == (nYear%400);
	}
	
	static CString Today(LPCTSTR ss=_T(""))
	{
		CString str;
		str.Format( _T("%04d%s%02d%s%02d"), day.GetYear(), ss, day.GetMonth(), ss, day.GetDay() );
		return str;
	}
	
	static CString Now(LPCTSTR ss=_T(""))
	{
		CString str;
		str.Format( _T("%02d%s%02d%s%02d"), day.GetHour(), ss, day.GetMinute(), ss, day.GetSecond() );
		return str;
	}
	
public:
	static CTime day;
	
public:
	static const CString m_WEEK[];
};

////////////////////////////////////////////////////////////////////////////////
#endif // _MY_DATE_H_
