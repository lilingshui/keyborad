// ChatRoomSrvDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Wireless.h"
#include "WirelessControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


CWirelessControl::CWirelessControl(CWnd* pParent /*=NULL*/)
: CDialog(CWirelessControl::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hThread = NULL;
	m_bLunch = FALSE;
}

CWirelessControl::~CWirelessControl()
{
	closesocket(m_soketSrv);
	closesocket(m_soketCon);
	WSACleanup();
}

void CWirelessControl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SHOW, m_ListMsg);
}

BEGIN_MESSAGE_MAP(CWirelessControl, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_TRAYCALLMSG, OnTrayIconNotify)
	ON_COMMAND(ID_EXIT_DLG, &CWirelessControl::OnExitDlg)
END_MESSAGE_MAP()

// CWirelessControl message handlers

BOOL CWirelessControl::OnInitDialog()
{
	CDialog::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	if(!InitWinsock())
	{
		return FALSE;
	}

	HICON	hIcon = AfxGetApp()->LoadIcon(IDI_KEY_BOARD);
	m_TrayIcon.CreateIcon(this, ID_TRAYICON, hIcon, _T("KeyBoard"));

	ModifyStyleEx(WS_EX_APPWINDOW,WS_EX_TOOLWINDOW);

	SetTimer(ID_HIDEDIALOG, 50, NULL);
	StartSrv();

	return TRUE;
}

BOOL CWirelessControl::InitWinsock()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD( 2, 2 );

	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		return FALSE;
	}

	if ( LOBYTE( wsaData.wVersion ) != 2 ||
		HIBYTE( wsaData.wVersion ) != 2 ) 
	{
		WSACleanup( );
		return FALSE; 
	}

	return TRUE;
}

void CWirelessControl::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

void CWirelessControl::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CWirelessControl::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CWirelessControl::StartSrv()
{
	m_bLunch = TRUE;
	m_hThread = CreateThread(NULL, 0, SRVThreadProc, this, 0, NULL);
	m_TrayIcon.SetStartState(TRUE);
}

LRESULT CWirelessControl::OnTrayIconNotify(WPARAM wParam, LPARAM lParam)
{
	m_TrayIcon.OnNotify(wParam,lParam);
	return 1;
}

void CWirelessControl::OnTimer(UINT_PTR nIDEvent)
{
	if(ID_HIDEDIALOG == nIDEvent)
	{
		ShowWindow(SW_HIDE);
		KillTimer(ID_HIDEDIALOG);
	}

	CDialog::OnTimer(nIDEvent);
}

void CWirelessControl::OnExitDlg()
{
	m_bLunch = FALSE;
	PostMessage(WM_QUIT, NULL, NULL);
}

DWORD WINAPI SRVThreadProc(LPVOID pParam)
{
	int nPort = 2000;
	sockaddr_in addrclt;
	CWirelessControl* pDlg = (CWirelessControl*)pParam;
	ASSERT(pDlg != NULL);

	if(!CreateSrvSocket(pDlg))
	{
		return FALSE;
	}

	if(!BindAddress(pDlg->m_soketSrv,nPort))
	{
		return FALSE;
	}

	if(!ListenSocket(pDlg->m_soketSrv,pDlg->m_soketCon))
	{
		return FALSE;
	}

	CString str;
	str.LoadString(IDS_START_SUCCESS);
	pDlg->m_ListMsg.AddString(str);

	char szMsg = -1;
	int nCount = 0;

	while(pDlg->m_bLunch)
	{
		if(!AcceptSocket(pDlg->m_soketSrv,pDlg->m_soketCon,addrclt))
		{
			continue;
		}

		str.LoadString(IDS_LINK_OK);
		pDlg->m_ListMsg.AddString(str);
		nCount = pDlg->m_ListMsg.GetCount();
		pDlg->m_ListMsg.SetCurSel(nCount - 1);
		
		while(pDlg->m_bLunch)
		{
			szMsg = ReceiveMessage(pDlg->m_soketSrv,pDlg->m_soketCon,addrclt);

			if(szMsg != -1 && szMsg != 0)
			{
				str.Format(_T("%x"),szMsg);
				pDlg->m_ListMsg.AddString(str);
				nCount = pDlg->m_ListMsg.GetCount();
				pDlg->m_ListMsg.SetCurSel(nCount - 1);

				if(szMsg >= 97 && szMsg <= 122)
				{
					szMsg -= 32;
				}

				//keybd_event(szMsg,
				//	0,
				//	0,
				//	0 );

				//keybd_event(szMsg,
				//	0,
				//	KEYEVENTF_KEYUP,
				//	0);

				//keybd_event(VK_LWIN, 0, 0 ,0);
				//keybd_event(VK_LWIN, 0, KEYEVENTF_KEYUP,0);


				//keybd_event(VK_MENU, (BYTE)0, 0 ,0);
				//keybd_event(VK_TAB,(BYTE)0, 0 ,0); //此处可以用   'A', (BYTE)65, 用'a'不起作用.
				//keybd_event(VK_TAB, (BYTE)0, KEYEVENTF_KEYUP,0);
				//keybd_event(VK_MENU, (BYTE)0, KEYEVENTF_KEYUP,0);

				INPUT keyInput;
				keyInput.type = INPUT_KEYBOARD;

				KEYBDINPUT key;
				key.wVk = szMsg;
				key.wScan = ::VkKeyScan(szMsg);

				key.dwFlags = 0;
				keyInput.ki = key;
				::SendInput(1,&keyInput,sizeof(INPUT));

				key.dwFlags = KEYEVENTF_KEYUP;
				keyInput.ki = key;
				::SendInput(1,&keyInput,sizeof(INPUT));
			}
			else
			{
				str.LoadString(IDS_LINK_FAIL);
				pDlg->m_ListMsg.AddString(str);
				nCount = pDlg->m_ListMsg.GetCount();
				pDlg->m_ListMsg.SetCurSel(nCount - 1);
				break;
			}
		}
	}

	return TRUE;
}

BOOL CreateSrvSocket(CWirelessControl* pDlg)
{
	ASSERT(pDlg != NULL);
	pDlg->m_soketSrv = socket(AF_INET , SOCK_STREAM , IPPROTO_TCP);
	if ( pDlg->m_soketSrv == INVALID_SOCKET ) 
	{
		AfxMessageBox(IDS_CREATE_FAIL);
		return FALSE;
	}
	return TRUE;
}

BOOL BindAddress(SOCKET& socketSrv,int nPort)
{
	sockaddr_in addrSrv;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_addr.s_addr = INADDR_ANY;
	addrSrv.sin_port = htons(nPort);
	if (bind(socketSrv, (sockaddr*)&addrSrv, sizeof(sockaddr_in)) == SOCKET_ERROR )
	{
		AfxMessageBox(IDS_BIND_FAIL);
		closesocket(socketSrv);
		return FALSE;
	}
	return TRUE;
}

BOOL ListenSocket(SOCKET& socketSrv,SOCKET& socketCon)
{
	if(listen(socketSrv, 5) == SOCKET_ERROR )
	{
		AfxMessageBox(IDS_LISTEN_FAIL);
		closesocket(socketSrv);
		closesocket(socketCon);
		return FALSE;
	}

	return TRUE;
}

BOOL AcceptSocket(SOCKET& socketSrv,SOCKET& socketCon,sockaddr_in& addrclt)
{
	int nLen = sizeof(sockaddr_in);
	socketCon = accept(socketSrv, (struct sockaddr *)&addrclt , &nLen);

	if(socketCon == INVALID_SOCKET) 
	{
		closesocket(socketSrv);
		closesocket(socketCon);
		//AfxMessageBox(_T("连接失败!"));
		return FALSE;
	}

	return TRUE;
}

char ReceiveMessage(SOCKET& socketSrv,SOCKET& socketCon,sockaddr_in& addrclt)
{
	char szBuf[100] = {0};

	int bytesRecv = SOCKET_ERROR;
	bytesRecv = recv( socketCon, szBuf, 100, 0 );

	if ( bytesRecv == 0 || bytesRecv == WSAECONNRESET )
	{
		return -1;
	}

	return szBuf[0];
}

