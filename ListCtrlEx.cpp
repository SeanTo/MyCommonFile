// EditListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "ListCtrlEx.h"
#include <windowsx.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define IDC_EDIT		0X01
#define PROPERTY_ITEM	0x02
#define PROPERTY_SUB	0x03

/////////////////////////////////////////////////////////////////////////////
// CListCtrlCombo
#ifdef _USE_COMBOBOX_

CListCtrlCombo::CListCtrlCombo()
{
}

CListCtrlCombo::~CListCtrlCombo()
{
}

BEGIN_MESSAGE_MAP(CListCtrlEdit,  CEdit)
	//{{AFX_MSG_MAP(CListCtrlEdit)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CListCtrlCombo::OnSetFocus(CWnd* pOldWnd) 
{
	CComboBox::OnSetFocus(pOldWnd);
    //m_bExchange = TRUE;	
}

void CListCtrlCombo::OnKillFocus(CWnd* pNewWnd) 
{
	CComboBox::OnKillFocus(pNewWnd);
	CWnd* pParent = this->GetParent();
// 	::PostMessage(pParent->GetSafeHwnd(), WM_USER_COMB_END, m_bExchange, 0);
}

// BOOL CListCtrlCombo::PreTranslateMessage(MSG* pMsg) 
// {
// 	if(pMsg->message == WM_KEYDOWN)
// 	{
// 		if(pMsg->wParam == VK_RETURN)
// 		{
// 			CWnd* pParent = this->GetParent();
// 			m_bExchange = TRUE;
// 			::PostMessage(pParent->GetSafeHwnd(), WM_USER_COMB_END, m_bExchange, 0);
// 		}
// 		else if(pMsg->wParam == VK_ESCAPE)
// 		{
// 			CWnd* pParent = this->GetParent();
// 			m_bExchange = FALSE;
// 			::PostMessage(pParent->GetSafeHwnd(), WM_USER_COMB_END, m_bExchange, 0);
// 		}
// 	}
// 	
// 	return CComboBox::PreTranslateMessage(pMsg);
// }

#endif // _USE_COMBOBOX_
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// CListCtrlEdit

CListCtrlEdit::CListCtrlEdit()
{
	m_strValidChars.Empty();
	m_nLen = 0;
}

CListCtrlEdit::~CListCtrlEdit()
{
}

BEGIN_MESSAGE_MAP(CListCtrlEdit,  CEdit)
	//{{AFX_MSG_MAP(CListCtrlEdit)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListCtrlEdit message handlers

void CListCtrlEdit::SetCtrlData(DWORD dwData)
{
	m_dwData = dwData;
}

DWORD CListCtrlEdit::GetCtrlData()
{
	return m_dwData;
}

void CListCtrlEdit::OnSetFocus(CWnd* pOldWnd) 
{
	CEdit::OnSetFocus(pOldWnd);
    m_bExchange = TRUE;	
}

void CListCtrlEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);
	CWnd* pParent = this->GetParent();
	::PostMessage(pParent->GetSafeHwnd(), WM_USER_EDIT_END, m_bExchange, 0);
}

void CListCtrlEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
 	int iBeg = -1, iEnd = -1;
 	GetSel(iBeg, iEnd);

	UINT nLen = (UINT)GetWindowTextLength();

	if ( nChar == VK_BACK || nChar == CTRL_C || nChar == CTRL_V || nChar == CTRL_X
		|| (nLen < m_nLen || 0 == m_nLen || iBeg != iEnd) && IsValidChar((TCHAR)nChar) )
	{
		CEdit::OnChar(nChar, nRepCnt, nFlags);
	}
	else
	{
		MessageBeep(MB_ICONEXCLAMATION);
	}

// 	CEdit::OnChar(nChar, nRepCnt, nFlags);
}

BOOL CListCtrlEdit::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_RETURN)
		{
			CWnd* pParent = this->GetParent();
			m_bExchange = TRUE;
			::PostMessage(pParent->GetSafeHwnd(), WM_USER_EDIT_END, m_bExchange, 0);
		}
		else if(pMsg->wParam == VK_ESCAPE)
		{
			CWnd* pParent = this->GetParent();
			m_bExchange = FALSE;
			::PostMessage(pParent->GetSafeHwnd(), WM_USER_EDIT_END, m_bExchange, 0);
		}
	}
	
	return CEdit::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// CListCtrlEx

CListCtrlEx::CListCtrlEx()
{
	m_edit.m_hWnd = NULL;
	ZeroMemory(m_bReadOnlyColumns, MAX_RESTRICT_COLUMNS * sizeof(BOOL) );
	ZeroMemory(m_nValidLength, MAX_RESTRICT_COLUMNS * sizeof(UINT) );

	for(int i = 0; i < MAX_RESTRICT_COLUMNS; ++i)
		m_strValidChars[i].Empty();
}

CListCtrlEx::~CListCtrlEx()
{
}

BEGIN_MESSAGE_MAP(CListCtrlEx,  CListCtrl)
	//{{AFX_MSG_MAP(CListCtrlEx)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PARENTNOTIFY()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_USER_EDIT_END, OnEditEnd)
END_MESSAGE_MAP()

int CListCtrlEx::GetCurSel() const
{
	int iSel = -1;
	POSITION pos = GetFirstSelectedItemPosition();
	if(NULL != pos)
	{
		while(pos)
		{
			iSel = GetNextSelectedItem(pos);
			break;
		}
	}
	return iSel;
}

BOOL CListCtrlEx::HitTestEx(CPoint &pt, int &iRow, int &iCol, BOOL bScreen/* = FALSE*/)
{
	if( bScreen )
		ScreenToClient(&pt);

	LVHITTESTINFO lvhti;
    lvhti.pt = pt;
	
	iRow = CListCtrl::SubItemHitTest(&lvhti);
	iCol = (-1 == iRow) ? 0 : lvhti.iSubItem;
	
	return  (-1 != iRow) && !m_bReadOnlyColumns[iCol];
}

void CListCtrlEx::OnLButtonDown(UINT nFlags,  CPoint point) 
{
	int iRow, iCol, iSel = GetCurSel();;
	HitTestEx(point, iRow, iCol);

	BOOL bVisible = FALSE;
	if( m_edit.m_hWnd != NULL )
	{
		DWORD dwStyle = m_edit.GetStyle();
		bVisible = (dwStyle & WS_VISIBLE) == WS_VISIBLE;
	}

	// 两次单击同一行时不响应
	if( iRow == iSel && !bVisible && (iCol >= 0 && iCol < MAX_RESTRICT_COLUMNS && !m_bReadOnlyColumns[iCol]) )
		return;

	CListCtrl::OnLButtonDown(nFlags, point);
}

void CListCtrlEx::OnLButtonDblClk(UINT nFlags,  CPoint point) 
{
	int nItem = -1, nSubItem = -1;
	
	if( HitTestEx(point, nItem, nSubItem) )
	{
		CRect rcCtrl;
		CListCtrl::GetSubItemRect(nItem, nSubItem, LVIR_LABEL, rcCtrl); 
		ShowEdit(TRUE, nItem, nSubItem, rcCtrl);
	}

	CListCtrl::OnLButtonDblClk(nFlags,  point);
}

void CListCtrlEx::ShowEdit(BOOL bShow, int nItem, int nIndex, const CRect &rcCtrl)
{
    if(NULL == m_edit.m_hWnd)
	{
		m_edit.Create(ES_AUTOHSCROLL | WS_CHILD | ES_LEFT | ES_WANTRETURN | WS_BORDER,
			CRect(0, 0, 0, 0), this, IDC_EDIT);
		m_edit.ShowWindow(SW_HIDE);

		CFont tpFont;
		tpFont.CreateStockObject(DEFAULT_GUI_FONT);
		m_edit.SetFont(&tpFont);
		tpFont.DeleteObject();
	}

	m_edit.SetLimitChars( m_strValidChars[nIndex], m_nValidLength[nIndex] );

	if(bShow == TRUE)
	{
		CString strItem = CListCtrl::GetItemText(nItem, nIndex);
		m_edit.MoveWindow(rcCtrl);
		m_edit.ShowWindow(SW_SHOW);
		m_edit.SetWindowText(strItem);

		::SetFocus(m_edit.GetSafeHwnd());
		m_edit.SetSel(0, -1);
		m_edit.SetCtrlData(MAKEWPARAM(nIndex, nItem));	
	}
	else
		m_edit.ShowWindow(SW_HIDE);
}

// #ifdef  _USE_COMBOBOX_
// void CListCtrlEx::ShowComb(BOOL bShow, int nItem, int nIndex,
// 						   CRect rcCtrl/*=CRect(0,0,0,0)*/,
// 						   CStringArray *arr/*=NULL*/)
// {
// 	if(NULL == m_cmb.m_hWnd)
// 	{
// 		
// 		CFont tpFont;
// 		tpFont.CreateStockObject(DEFAULT_GUI_FONT);
// 		m_cmb.SetFont(&tpFont);
// 		tpFont.DeleteObject();
// 	}
// 
// 	m_cmb.ResetContent();
// 	if(NULL != arr)
// 	{
// 		for(int i = 0; i < arr->GetSize(); ++i)
// 			m_cmb.AddString(arr->GetAt(i));
// 	}
// 
// 	if(bShow)
// 	{
// 		m_cmb.MoveWindow(rcCtrl);
// 		m_cmb.ShowWindow(SW_HIDE);
// 
// 		CString strItem = CListCtrl::GetItemText(nItem, nIndex);
// 		m_cmb.SelectString(0, strItem);
// 		
// 		::SetFocus(m_cmb.GetSafeHwnd());
// 	}
// 	else
// 		m_cmb.ShowWindow(SW_HIDE);
// }
// #endif

void CListCtrlEx::OnEditEnd(WPARAM wParam, LPARAM lParam)
{
	if(wParam == TRUE)
	{
		CString strText(_T(""));
		m_edit.GetWindowText(strText);
		DWORD dwData = m_edit.GetCtrlData();
		int nItem= dwData >> 16;
		int nIndex = dwData & 0x0000ffff;
		CListCtrl::SetItemText(nItem, nIndex, strText);
	}

    if(lParam == FALSE)
	    m_edit.ShowWindow(SW_HIDE);
}

void CListCtrlEx::OnParentNotify(UINT message,  LPARAM lParam) 
{
	CListCtrl::OnParentNotify(message,  lParam);

	//////////////////////////////////////////////////////////////////////////
	CHeaderCtrl *pHeaderCtrl = CListCtrl::GetHeaderCtrl();
	if(pHeaderCtrl == NULL)
		return;

	CRect rcHeader;
	pHeaderCtrl->GetWindowRect(rcHeader);
	ScreenToClient(rcHeader);

	//The x coordinate is in the low-order word and the y coordinate is in the high-order word.
	CPoint pt;
	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);

	if(rcHeader.PtInRect(pt) && message == WM_LBUTTONDOWN)
	{
		if(m_edit.m_hWnd != NULL)
		{
			DWORD dwStyle = m_edit.GetStyle();
			if((dwStyle & WS_VISIBLE) == WS_VISIBLE)
			{
				m_edit.ShowWindow(SW_HIDE);
			}
		}	
	}	
}

BOOL CListCtrlEx::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message == WM_KEYDOWN)
	{
		if(pMsg->wParam == VK_TAB && m_edit.m_hWnd != NULL)
		{
		 	DWORD dwStyle = m_edit.GetStyle();
			if((dwStyle & WS_VISIBLE) == WS_VISIBLE)
			{
			     OnEditEnd(TRUE, TRUE);

				 CRect rcCtrl;		 
				 int nItem, nSub;

				 if( FALSE == Key_Ctrl(nItem, nSub) )
                    Key_Shift(nItem, nSub);

                 CListCtrl::GetSubItemRect(nItem, nSub, LVIR_LABEL, rcCtrl); 
				 CPoint pt(rcCtrl.left+1, rcCtrl.top+1);
				 OnLButtonDblClk(0, pt);
				 POSITION pos = CListCtrl::GetFirstSelectedItemPosition();
				 if (pos == NULL)
				 {
				 }
				 else
				 {
					 while (pos)
					 {
						 int ntpItem = CListCtrl::GetNextSelectedItem(pos);
						 CListCtrl::SetItemState(ntpItem, 0, LVIS_SELECTED);
					 }
				 }
				 CListCtrl::SetItemState(nItem,   LVIS_SELECTED,   LVIS_SELECTED);
				 return TRUE;
			}
		}
	}
	
	return CListCtrl::PreTranslateMessage(pMsg);
}

BOOL CListCtrlEx::Key_Shift(int& nItem,  int& nSub)
{
	int nItemCount = CListCtrl::GetItemCount();
	DWORD dwData = m_edit.GetCtrlData();
	nItem= dwData >> 16;
	nSub = dwData & 0x0000ffff;
	
	CHeaderCtrl* pHeader = CListCtrl::GetHeaderCtrl();
	if(pHeader == NULL)
		return FALSE;
	
	short sRet = GetKeyState(VK_SHIFT);
	int nSubcCount = pHeader->GetItemCount();
	sRet = sRet >> 15;

	BOOL bIsShift = (0 != sRet);

	int iAddCol = bIsShift ? (nSubcCount - 1) : 1;
	int iAddRow = bIsShift ? (nItemCount - 1) : 1;

	do 
	{
		nSub = (nSub + iAddCol) % nSubcCount;
		if( !bIsShift && 0 == nSub
			|| bIsShift && nSubcCount - 1 == nSub )
		{
			nItem = (nItem + iAddRow) % nItemCount;
		}

	} while( m_bReadOnlyColumns[nSub] );

	return bIsShift;
}

BOOL CListCtrlEx::Key_Ctrl(int& nItem, int &nSub)
{
    short sRet = GetKeyState(VK_CONTROL);
	DWORD dwData = m_edit.GetCtrlData();
	nItem= dwData >> 16;
	nSub = dwData & 0x0000ffff;
	
	sRet = sRet >> 15;
	int nItemCount = CListCtrl::GetItemCount();
	if(sRet != 0)
	{
		nItem = nItem >=nItemCount-1? 0:nItem+=1;
		return TRUE;
	}
	
	return FALSE;
}

void CListCtrlEx::SetValidCharsColumn(const CString str, UINT nLen /*= 0*/, int iCol /* = -1 */)
{
	if( iCol > MAX_RESTRICT_COLUMNS - 1)
		return;
	
	if( iCol >= 0 )
	{
		m_strValidChars[iCol] = str;
		m_nValidLength[iCol] = nLen;
	}
	else
	{
		for(int i = 0; i < MAX_RESTRICT_COLUMNS; ++i)
		{
			m_strValidChars[i] = str;
			m_nValidLength[i] = nLen;
		}
	}
}

void CListCtrlEx::SetReadOnlyColumn(int iCol/*=-1*/, BOOL bReadOnly/* = TRUE*/)
{
	if(iCol > MAX_RESTRICT_COLUMNS - 1)
		return;
	
	if( iCol >= 0 )
	{
		m_bReadOnlyColumns[iCol] = bReadOnly;
	}
	else
	{
		for(int i = 0; i < MAX_RESTRICT_COLUMNS ; ++i)
		{
			m_bReadOnlyColumns[i] = bReadOnly;
		}
	}
}

BOOL CListCtrlEx::SetCheckAll(BOOL bChk/*=TRUE*/)
{
	ASSERT( IsCheck() );
	const int iList = GetItemCount();

	for(int i = 0; i < iList ; ++i)
	{
		if( !SetCheck(i, bChk) )
			return FALSE;
	}
	return TRUE;
}

BOOL CListCtrlEx::ReverseCheck(int iRow/*=-1*/)
{
	const int iList = GetItemCount();
	ASSERT( IsCheck() && iRow < iList);
	
	if(iRow < 0)
	{
		for(int i = 0; i < iList ; ++i)
		{
			if( !SetCheck(i, !GetCheck(i)) )
				return FALSE;
		}
	}
	else
	{
		if( !SetCheck(iRow, !GetCheck(iRow)) )
			return FALSE;
	}

	return TRUE;
}

int CListCtrlEx::GetCheckCount()
{
	ASSERT( IsCheck() );
	const iList = GetItemCount();
	
	int iSelCnt = 0;
	for(int i = 0; i < iList ; ++i)
	{
		iSelCnt += GetCheck(i);
	}
	return iSelCnt;
}
//////////////////////////////////////////////////////////////////////////

