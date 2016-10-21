// xGSIOC.h : main header file for the XGSIOC application
//

#if !defined(AFX_XGSIOC_H__174BC265_EC5E_11D2_BDEC_00E02921253C__INCLUDED_)
#define AFX_XGSIOC_H__174BC265_EC5E_11D2_BDEC_00E02921253C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CXGSIOCApp:
// See xGSIOC.cpp for the implementation of this class
//

class CXGSIOCApp : public CWinApp
{
public:
	CXGSIOCApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXGSIOCApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CXGSIOCApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XGSIOC_H__174BC265_EC5E_11D2_BDEC_00E02921253C__INCLUDED_)
