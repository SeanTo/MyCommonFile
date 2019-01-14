// TabSheet.cpp : implementation file
//

#include "stdafx.h"
//#include "Property5.h"
#include "TabSheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabSheet

CTabSheet::CTabSheet()
{
	m_nNumOfPages = 0;
	m_nCurrentPage = 0;
}

CTabSheet::~CTabSheet()
{
}

BEGIN_MESSAGE_MAP(CTabSheet, CTabCtrl)
	//{{AFX_MSG_MAP(CTabSheet)
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabSheet message handlers

BOOL CTabSheet::AddPage(LPCTSTR title, CDialog *pDialog, UINT ID)
{
	if( MAXPAGE == m_nNumOfPages )
		return FALSE;

	m_Title[m_nNumOfPages] = title;
	m_pPages[m_nNumOfPages] = pDialog;
	m_IDD[m_nNumOfPages] = ID;

	m_nNumOfPages++;

	return TRUE;
}

void CTabSheet::Show()
{
	for( int i = 0; i < m_nNumOfPages; i++ )
	{
		m_pPages[i]->Create( m_IDD[i], this );
		InsertItem( i, m_Title[i] );
	}

	m_pPages[0]->ShowWindow(SW_SHOW);
	for(int i = 1; i < m_nNumOfPages; i++)
		m_pPages[i]->ShowWindow(SW_HIDE);

	SetRect();
}

void CTabSheet::SetRect()
{
	CRect tabRect, itemRect;
	int x, y, cx, cy;

	GetClientRect(&tabRect);
	GetItemRect(0, &itemRect);

//	tabRect.NormalizeRect();
//	itemRect.NormalizeRect();

	DWORD dwStyle = GetStyle();
	if((dwStyle & TCS_VERTICAL) == 0) // 水平
	{
		if((dwStyle & TCS_BOTTOM) == 0) // 标签在顶部
		{
			x = itemRect.left;
			y = itemRect.Height() + x + 2;
			cx = tabRect.right - x * 2;
			cy = tabRect.bottom - itemRect.Height() - x * 2 - 2;
		}
		else  // 标签在底部
		{
			x = y = itemRect.left; // 左上角余边等宽
			cx = tabRect.right - x * 2;
			cy = tabRect.bottom - itemRect.Height() - y * 2 - 2;
		}
	}
	else // 垂直
	{
		if((dwStyle & TCS_RIGHT) == 0) // 标签在左边
		{
			y = itemRect.top;
			x = itemRect.Width() + y + 2;
			cx = tabRect.right - itemRect.Width() - y * 2 - 2;
			cy = tabRect.bottom - y * 2;
		}
		else // 标签在右边
		{
			x = y = itemRect.top; // 左上角余边等宽
			cx = tabRect.right - itemRect.Width() - y * 2 - 2;
			cy = tabRect.bottom - y * 2;
		}
	}

	m_pPages[0]->SetWindowPos(&wndTop, x, y, cx, cy, SWP_SHOWWINDOW);
	for( int nCount = 1; nCount < m_nNumOfPages; nCount++ )
		m_pPages[nCount]->SetWindowPos(&wndTop, x, y, cx, cy, SWP_HIDEWINDOW);

}

void CTabSheet::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CTabCtrl::OnLButtonDown(nFlags, point);

	if(m_nCurrentPage != GetCurFocus())
	{
		m_pPages[m_nCurrentPage]->ShowWindow(SW_HIDE);
		m_nCurrentPage = GetCurFocus();
		m_pPages[m_nCurrentPage]->ShowWindow(SW_SHOW);
//		m_pPages[m_nCurrentPage]->SetFocus();
	}
}

int CTabSheet::SetCurSel(int nItem)
{
	if( nItem < 0 || nItem >= m_nNumOfPages)
		return -1;

	int ret = m_nCurrentPage;

	if(m_nCurrentPage != nItem )
	{
		m_pPages[m_nCurrentPage]->ShowWindow(SW_HIDE);
		m_nCurrentPage = nItem;
		m_pPages[m_nCurrentPage]->ShowWindow(SW_SHOW);
//		m_pPages[m_nCurrentPage]->SetFocus();
		CTabCtrl::SetCurSel(nItem);
	}

	return ret;
}

int CTabSheet::GetCurSel()
{
	return CTabCtrl::GetCurSel();
}
