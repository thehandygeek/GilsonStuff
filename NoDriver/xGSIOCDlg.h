// xGSIOCDlg.h : header file
//

#if !defined(AFX_XGSIOCDLG_H__174BC267_EC5E_11D2_BDEC_00E02921253C__INCLUDED_)
#define AFX_XGSIOCDLG_H__174BC267_EC5E_11D2_BDEC_00E02921253C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "gsiocport.h"

/////////////////////////////////////////////////////////////////////////////
// CXGSIOCDlg dialog

class CXGSIOCDlg : public CDialog
{
// Construction
public:
	virtual void UpdLogView(int unit, unsigned long tnow, CString& cmd, CString& rsp);
	CXGSIOCDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CXGSIOCDlg)
	enum { IDD = IDD_XGSIOC_DIALOG };
	CComboBox	m_cbxCommand;
	CComboBox	m_cbxDevice;
	CEdit	m_edView;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXGSIOCDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	
	virtual void CXGSIOCDlg::DoSend(BOOL IsImmediateCommand);

	CGsiocPort m_gcom;
	CFont m_cfNewLook;

	// Generated message map functions
	//{{AFX_MSG(CXGSIOCDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDeviceScan();
	afx_msg void OnBufferedSend();
	afx_msg void OnImmediateSend();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XGSIOCDLG_H__174BC267_EC5E_11D2_BDEC_00E02921253C__INCLUDED_)
