#include	<windows.h>
#include "resource.h"

#include	"..\..\Util\gsioc32.h"

HINSTANCE	static	hInstance;

bool			static	Terminating;		// flag to terminate outstanding thread(s) when closing
int			static	Threads;

char			static	CurCmd[256];
int			static	CurUnitID = -1;	// -1 means no unit

static void UpdateButtons(HWND hWnd)
{
	// Enable buttons if id and command are valid.
	char txt[80];
	GetDlgItemText(hWnd, IDC_ID, txt, sizeof txt);

	CurUnitID= -1;
	for (int i=0; txt[i]>='0'&& txt[i]<='9'; i++){
		if ( CurUnitID == -1 )
			CurUnitID=0;
		CurUnitID = CurUnitID*10 + txt[i] - '0';
		}

	bool enable =
			GetDlgItemText(hWnd, IDC_CMDSTR, CurCmd, sizeof CurCmd) > 0
		&& CurUnitID >= 0
		&& CurUnitID <= 63;
	
	EnableWindow( GetDlgItem(hWnd, IDC_IMDBUTTON), enable );
	EnableWindow( GetDlgItem(hWnd, IDC_BUFBUTTON), enable );
	}

static BOOL CALLBACK AboutDlg(
	  HWND		hWnd
	, UINT		message
	, WPARAM		wParam
	, LPARAM		//lParam
	)
{
	switch( message ){
	case WM_INITDIALOG:
		break;

	case WM_COMMAND:
		EndDialog(hWnd, 0);
		break;

	default:
		return false;
		}

	return true;
	}

static DWORD __stdcall ScanThread(void *p)
{
	Threads++;
	HWND hWnd =(HWND)p;

	// disable scan menu item
	HMENU hMenu = GetMenu(hWnd);
	EnableMenuItem(hMenu, ID_SCAN, MF_BYCOMMAND | MF_GRAYED);
	DrawMenuBar(hWnd);

	// disable buttons
	EnableWindow( GetDlgItem(hWnd, IDC_IMDBUTTON), false );
	EnableWindow( GetDlgItem(hWnd, IDC_BUFBUTTON), false );

	// save current contents of id box
	char	curtxt[80];
	HWND	hCB = GetDlgItem(hWnd, IDC_ID);
	GetWindowText(hCB, curtxt, sizeof curtxt);

	SendMessage(hCB, CB_RESETCONTENT, 0, 0);

	for ( int i=63; i>=0; i-- ){

		if ( Terminating ){
			Threads--;
			return 0;
			}

		// keep a nice count going
		char	msg[80];
		wsprintf(msg, "*Scanning* %02d", i);
		SetWindowText(hCB, msg);
		UpdateWindow(hCB);

		char	rsp[256];
		ICmd( i, "%", rsp, sizeof rsp );				// find out type of unit

		if ( rsp[0]=='#' )
			if ( rsp[1]=='4' )
				wsprintf( msg, "%02d 302|303", i );
			else
				continue;
		else
			wsprintf( msg, "%02d %s", i, rsp );

		SendMessage(hCB, CB_SHOWDROPDOWN, false, 0);
		SendMessage(hCB, CB_INSERTSTRING, 0, (LPARAM)msg);
		SendMessage(hCB, CB_SHOWDROPDOWN, true, 0);
		}
			
	// show list
	SetForegroundWindow(hWnd);
	SetFocus(hCB);
	SendMessage(hCB, CB_SHOWDROPDOWN, true, 0);

	// restore old text, button states
	SetWindowText(hCB, curtxt);
	UpdateButtons(hWnd);

	EnableMenuItem(hMenu, ID_SCAN, MF_ENABLED);
	DrawMenuBar(hWnd);

	Threads--;
	return 0;
	}

static BOOL CALLBACK MainDlg( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	HICON static	hIcon;

	char	msg[256], rsp[256];

	switch( message ){
	case WM_INITDIALOG:

		hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_MAIN));
		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM) hIcon);
		SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

		GetDriverVersion(rsp, sizeof rsp);
		wsprintf( msg, "Driver:  %s", rsp );
		SetDlgItemText( hWnd, IDC_VERSTR, msg );

		GetPort(rsp, sizeof rsp );
		wsprintf(msg, "COM%s", rsp);
		SetDlgItemText(hWnd, IDC_PORT, msg);
		
 		GetIRQ(rsp, sizeof rsp);
 		SetDlgItemText(hWnd, IDC_IRQ, rsp);
 		
		GetBaud(rsp, sizeof rsp);
		SetDlgItemText(hWnd, IDC_BAUD, rsp);
		
		break;

	case WM_QUERYDRAGICON:
		return (BOOL)hIcon;

	case WM_ERASEBKGND:
		return IsIconic(hWnd);

	case WM_PAINT:{

		PAINTSTRUCT ps;

		BeginPaint(hWnd, &ps);

		if ( IsIconic(hWnd) ){
			DefWindowProc(hWnd, WM_ICONERASEBKGND, (WPARAM)ps.hdc, 0);
			DrawIcon(ps.hdc, 0,0, hIcon);
			}

		EndPaint(hWnd, &ps);

		break;
		}

	// Diagnostic aid: Right clicking on IDC_IMDBUTTON (when disabled) causes command to be
	// sent directly to the driver.

	case WM_RBUTTONUP:
	case WM_RBUTTONDOWN:{
		POINT	p		= {LOWORD(lParam), HIWORD(lParam)};
		bool	hit	=
				ChildWindowFromPoint(hWnd, p)==GetDlgItem(hWnd, IDC_IMDBUTTON)
			&& CurUnitID == -1;

		if ( hit ){
			rsp[0]=0;
			if ( message==WM_RBUTTONDOWN ){
				char	cmd[256];
				int	i, j;
				for (i=j=0; CurCmd[i]; i++)
					if ( CurCmd[i]=='`' ){
						cmd[j++] = 
							  (CurCmd[i+1]-'0')*100
							+ (CurCmd[i+2]-'0')*10
							+ (CurCmd[i+3]-'0')*1;
						i+=3;
						}
					else
						cmd[j++] = CurCmd[i];

				cmd[j] = 0;

				DoCmd( cmd, rsp, sizeof rsp );
				}

			SetFocus( GetDlgItem( hWnd, IDC_CMDSTR ) );
			SetDlgItemText(hWnd, IDC_RSPSTR, rsp);
			}
		
		break;
		}

	case WM_COMMAND:
		switch( LOWORD(wParam) ){
      case ID_ABOUT:
			DialogBox(
				  hInstance
				, MAKEINTRESOURCE(IDD_ABOUT)
				, hWnd
				, AboutDlg
				);
         break;
         
		case ID_SCAN:{
			OSVERSIONINFO vi;
			vi.dwOSVersionInfoSize = sizeof vi;
			GetVersionEx(&vi);
			if ( vi.dwPlatformId==VER_PLATFORM_WIN32s )
				ScanThread(hWnd);
			else{
				DWORD	tid;
				CreateThread(NULL, 4096, ScanThread, hWnd, 0, &tid);
				}
				
			break;
			}

		case IDC_ID:		// watch for changes in editable fields
		case IDC_CMDSTR:

			if ( HIWORD(wParam)==CBN_SELENDOK ){
				HWND hCB = (HWND)lParam;
				LRESULT i = SendMessage(hCB, CB_GETCURSEL, 0, 0);
				if ( i!=CB_ERR ){
					char	txt[80];
					SendMessage(hCB, CB_GETLBTEXT, i, (LPARAM)txt);
					SetWindowText(hCB, txt);
					}
				}
			UpdateButtons(hWnd);
					
			break;

		case IDC_IMDBUTTON:
		case IDC_BUFBUTTON:{
			// execute command

			HMAILBOX hMB = CreateMailBox();

			(wParam==IDC_IMDBUTTON ? PostICmd : PostBCmd)
				( hMB, CurUnitID, CurCmd, rsp, sizeof rsp );

			do{
				Sleep(5);
				ReadMailBox(hMB, rsp, sizeof rsp);
				}
			while ( rsp[0]=='T' && rsp[1]==0 );

			SetDlgItemText( hWnd, IDC_RSPSTR, rsp+6 );

			CloseMailBox(hMB);

			// look for command in recent list
			int idx =
				(int)SendDlgItemMessage(
					hWnd, IDC_CMDSTR, CB_FINDSTRINGEXACT, 0, (LPARAM)CurCmd
					);

			// if found, delete it
			if ( idx!=CB_ERR ){
				SendDlgItemMessage(hWnd, IDC_CMDSTR, CB_DELETESTRING, idx, 0);
				SetDlgItemText(hWnd, IDC_CMDSTR, CurCmd);	// 2.01p. restore text in box
				}

			// add to top of list
			SendDlgItemMessage(hWnd, IDC_CMDSTR, CB_INSERTSTRING, 0, (LPARAM)CurCmd);

			// make next command easy to enter
			SetFocus(GetDlgItem(hWnd, IDC_CMDSTR));
			break;
			}

		default:
			return false;
			}
		break;

	case WM_CLOSE:
		Terminating = true;

		while ( Threads )
			Sleep(5);
		
		DestroyIcon(hIcon);
		EndDialog(hWnd, 0);
		break;

	default:
		return false;
		}

	return true;
	}

// entry routine: short circuit CRT
extern "C" void WinMainCRTStartup()
{
	// staticize instance
	hInstance = GetModuleHandle(NULL);
	
	// standard gilson finder aid
	char exepath[256];
	GetModuleFileName(NULL, exepath, sizeof exepath);
	WritePrivateProfileString("PathInfo", "EXE", exepath, "GSUTIL.INI");

	// Main window is a dialog
	DialogBox(hInstance, MAKEINTRESOURCE(IDR_MAIN), NULL, MainDlg);
	}
