#pragma once
#include <Winsock2.h>
#include "afxwin.h"
#include "TrayIcon.h"

// CWirelessControl dialog
class CWirelessControl : public CDialog
{
// Construction
public:
	CWirelessControl(CWnd* pParent = NULL);	// standard constructor
	~CWirelessControl();
	enum { IDD = IDD_CHATROOMSRV_DIALOG };

public:
	void StartSrv();
	BOOL InitWinsock();

public:
	SOCKET		m_soketSrv;
	SOCKET		m_soketCon;
	HANDLE		m_hThread;
	CListBox	m_ListMsg;

	BOOL		m_bLunch;
	CTrayIcon	m_TrayIcon;

	enum ID_TIMER
	{
		ID_HIDEDIALOG = 1
	};

// Implementation
protected:
	HICON m_hIcon;
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	afx_msg void OnExitDlg();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg LRESULT OnTrayIconNotify(WPARAM, LPARAM);
	
};

	DWORD WINAPI SRVThreadProc(LPVOID pParam);
	BOOL CreateSrvSocket(CWirelessControl* pDlg);
	BOOL BindAddress(SOCKET& socketSrv,int nPort);
	BOOL ListenSocket(SOCKET& socketSrv,SOCKET& socketCon);
	BOOL AcceptSocket(SOCKET& socketSrv,SOCKET& socketCon,sockaddr_in& addrclt);
	char ReceiveMessage(SOCKET& socketSrv,SOCKET& socketCon,sockaddr_in& addrclt);
