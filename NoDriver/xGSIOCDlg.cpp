// xGSIOCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "xGSIOC.h"
#include "xGSIOCDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXGSIOCDlg dialog

CXGSIOCDlg::CXGSIOCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CXGSIOCDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CXGSIOCDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CXGSIOCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CXGSIOCDlg)
	DDX_Control(pDX, IDC_CMBCMD, m_cbxCommand);
	DDX_Control(pDX, IDC_CMBDEV, m_cbxDevice);
	DDX_Control(pDX, IDC_EDIT1, m_edView);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CXGSIOCDlg, CDialog)
	//{{AFX_MSG_MAP(CXGSIOCDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTNSCAN, OnDeviceScan)
	ON_BN_CLICKED(IDC_BTNSEND_B, OnBufferedSend)
	ON_BN_CLICKED(IDC_BTNSEND_I, OnImmediateSend)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXGSIOCDlg message handlers

BOOL CXGSIOCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// Preset the GSIOC port to COM1 for this example.
	m_gcom.SetCommID(CString("COM1"));

	// Change the trace/log view's font to fit more information into a
	// smaller space
	LOGFONT fontspec = {	-12, 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
							ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY, VARIABLE_PITCH, "Times New Roman" };

	m_cfNewLook.CreateFontIndirect(&fontspec);
	m_edView.SetFont(&m_cfNewLook);

	// Set the trace/log view's tab position for convenient formatting
	int ts[3] = { 12, 30, 65};
	m_edView.SetTabStops(3,&(ts[0]));

	// Label the trace/log view's columns
	m_edView.ReplaceSel("ID\tTime\tCmd\t\"Response\"");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CXGSIOCDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CXGSIOCDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CXGSIOCDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CXGSIOCDlg::OnDeviceScan() 
{
	unsigned long ulTime;
	int nChar;
	CString csResponse,csTmp, csDevName,csDevVer;
	CString csGSIOCversion("%");
	CString csLogStr("[I] %");

	m_cbxDevice.ResetContent();		// Clear the current device list

	// Poll each device ID with a version request and see if anything responds
	for (int n=63; n >= 0; n--)
	{
		// Send Ver request (and write it to the trace view so user has something to see)
		ulTime = m_gcom.SendImmediate(n, csGSIOCversion, csResponse);
		UpdLogView(n, ulTime, csLogStr, csResponse);

		// On a valid response ( not failed)
		if ((csResponse.Find("failed") == -1))
		{
			// Seperate the device name and revision information
			nChar = csResponse.Find("V");
			csDevName = csResponse.Left(nChar);
			csDevVer = csResponse.Mid(nChar+1);

			// Create an entry for the list-box of the form "ID -- DeviceName vVersion"
			csTmp.Format("%d -- %s v%s", n, csDevName, csDevVer);
			m_cbxDevice.AddString(csTmp);
		}

	}

	// If the list is not empty
	// then set the current-ID to the first device located.
	if (m_cbxDevice.GetCount() > 0)
		m_cbxDevice.SetCurSel(0);
}

void CXGSIOCDlg::OnBufferedSend()
{
	DoSend(FALSE);
}

void CXGSIOCDlg::OnImmediateSend()
{
	CString csCmd;

	// Trim command to a single character (in case user accidentally typed
	// more than the single character required) and update the GUI to show the
	// actual command sent..
	m_cbxCommand.GetWindowText(csCmd);
	csCmd = csCmd.Left(1);
	m_cbxCommand.SetWindowText(csCmd);

	DoSend(TRUE);
}

void CXGSIOCDlg::DoSend(BOOL IsImmediateCommand)
{
	CString csResponse, csCmd, csDevice;
	unsigned long ulTime;
	int nUnit;

	// -------------------------- Extract and Validate the command information

	// UpdateData(TRUE);
	// ... UpdateData() can't be used for this since the member variables of the
	// ... dialog are "controls" rather than "values".  This was done so that the
	// ... list portions of the ComboBox controls could be altered dynamically by
	// ... the program (for example, to create a history list of commands issued).

	m_cbxDevice.GetWindowText(csDevice);	// Retrieve the Device ID string from the list-box
	nUnit = atoi(csDevice);					// Convert to INT (assuming that a numberic value
											// preceeds any other Device ID text

	//	Notify User and Abandon send if Unit ID is out-of-range
	if ( (nUnit > 63) || (nUnit < 0) )
	{
		CString csMsg;
		csMsg.Format(	"Sorry, the ID value %d is invalid\r\n"
						"ID values must be in the range 0 <= id <= 63", nUnit );
		AfxMessageBox(csMsg);
		return;
	}

	m_cbxCommand.GetWindowText(csCmd);		// Retrieve the cmd from the list-box
	m_cbxCommand.InsertString(0,csCmd);		// Add cmd string to history list-box

	// -------------------------- Issue the command and display it in the trace/log window

	if ( IsImmediateCommand )
	{
		ulTime = m_gcom.SendImmediate(nUnit, csCmd, csResponse);
		csCmd = "[I] " + csCmd;
	}
	else
	{
		ulTime = m_gcom.SendBuffered(nUnit, csCmd, csResponse);
		csCmd = "[B] " + csCmd;
	}

	UpdLogView(nUnit,ulTime,csCmd, csResponse);
}

void CXGSIOCDlg::UpdLogView(int unit, unsigned long tnow, CString &cmd, CString &rsp)
{
	static int nLineCount = 0;
	CString csLogLine;

	csLogLine.Format("\r\n%2d\t%5d\t%s\t\"%s\"", unit, tnow, cmd, rsp);

	// Update the log view trimming lines from the top to keep the size at
	// no more than 150 

	if (++nLineCount < 150)
		m_edView.ReplaceSel(csLogLine);
	else
	{
		m_edView.ReplaceSel(csLogLine);

		int x = m_edView.LineIndex(10);

		m_edView.SetSel(0,x,TRUE);		
		m_edView.Clear();
		int iPos = m_edView.GetWindowTextLength();
		m_edView.SetSel(iPos,iPos,FALSE);

		nLineCount -= 10;
	}
}
