#include "stdafx.h"
#include "Wireless.h"
#include "TrayIcon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CTrayIcon::CTrayIcon()
{
	m_bStart = FALSE;
	m_bIconExit = FALSE;
}

CTrayIcon::~CTrayIcon()
{
	Shell_NotifyIcon(NIM_DELETE,&m_ntfData);
}

CWnd* CTrayIcon::GetNotifyWnd() const
{
	return m_pNtfWnd;
}

BOOL CTrayIcon::SetNotifyWnd(CWnd* pNtfWnd)
{
	if (!m_bIconExit)
	{
		return FALSE;
	}

	ASSERT(pNtfWnd && ::IsWindow(pNtfWnd->GetSafeHwnd()));

	m_pNtfWnd = pNtfWnd;

	m_ntfData.hWnd = m_pNtfWnd->GetSafeHwnd();

	m_ntfData.uFlags |= NIF_MESSAGE;

	return Shell_NotifyIcon(NIM_MODIFY, &m_ntfData);
}

BOOL CTrayIcon::CreateIcon(CWnd* pNtfWnd, UINT uID, HICON hIcon,
				LPCTSTR lpszTip)
{
	ASSERT(pNtfWnd && ::IsWindow(pNtfWnd->GetSafeHwnd()));
	ASSERT(lstrlen(lpszTip) <= 64);

	m_pNtfWnd = pNtfWnd;

	m_ntfData.hWnd = pNtfWnd->GetSafeHwnd();
	m_ntfData.uID = uID;
	m_ntfData.hIcon = hIcon;
	m_ntfData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	m_ntfData.uCallbackMessage = WM_TRAYCALLMSG;

	if (lpszTip)
	{
		lstrcpyn(m_ntfData.szTip, lpszTip, sizeof(m_ntfData.szTip));
	}
	else
	{
		m_ntfData.szTip[0] = _T('\0');
	}

	m_bIconExit = Shell_NotifyIcon(NIM_ADD, &m_ntfData);

	return m_bIconExit;
}

LRESULT CTrayIcon::OnNotify(WPARAM WParam, LPARAM LParam)
{
	if (WParam != m_ntfData.uID)
	{
		return 0;
	}

	if (LParam == WM_RBUTTONUP)
	{
		CMenu menu;
		if (!menu.LoadMenu(IDR_MENU_TRAY))
		{
			return 0;
		}

		CMenu* pSubMenu = menu.GetSubMenu(0);

		if (!pSubMenu)
		{
			return 0;
		}

		::SetMenuDefaultItem(pSubMenu->m_hMenu, 0, TRUE);
		CPoint pos;
		GetCursorPos(&pos);

		SetForegroundWindow(m_pNtfWnd->m_hWnd);

		if(m_bStart)
		{
			pSubMenu->EnableMenuItem(ID_START_SRV, MF_BYCOMMAND | MF_GRAYED);
		}

		pSubMenu->TrackPopupMenu(TPM_RIGHTALIGN|TPM_LEFTBUTTON
			|TPM_RIGHTBUTTON, pos.x, pos.y, m_pNtfWnd, NULL);
	}

	return 1;
}

BOOL CTrayIcon::SetTipText(UINT nID)
{
	CString szTip;
	VERIFY(szTip.LoadString(nID));
	return SetTipText(szTip);
}

BOOL CTrayIcon::SetTipText(LPCTSTR lpszTip)
{
	if (!m_bIconExit)
	{
		return FALSE;
	}

	_tcscpy(m_ntfData.szTip, lpszTip);

	m_ntfData.uFlags |= NIF_TIP;

	return Shell_NotifyIcon(NIM_MODIFY, &m_ntfData);
}

BOOL CTrayIcon::ChangeIcon(HICON hIcon)
{
	if (!m_bIconExit)
	{
		return FALSE;
	}

	m_ntfData.hIcon = hIcon;

	m_ntfData.uFlags |= NIF_ICON;

	return Shell_NotifyIcon(NIM_MODIFY, &m_ntfData);
}

BOOL CTrayIcon::ChangeIcon(UINT nID)
{
	HICON hIcon = AfxGetApp()->LoadIcon(nID);

	return ChangeIcon(hIcon);
}

BOOL CTrayIcon::ChangeStandardIcon(LPCTSTR lpszIconName)
{
	HICON hIcon = AfxGetApp()->LoadStandardIcon(lpszIconName);
	return ChangeIcon(hIcon);
}

BOOL CTrayIcon::DeleteIcon()
{
	if (!m_bIconExit)
	{
		return FALSE;
	}

	m_bIconExit = FALSE;

	return Shell_NotifyIcon(NIM_DELETE,&m_ntfData);
}

void CTrayIcon::SetStartState(BOOL bStart)
{
	m_bStart = bStart;
}