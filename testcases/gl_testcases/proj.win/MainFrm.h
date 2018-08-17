#pragma once

#include "resource.h"       // main symbols
#include <WinSDKVer.h>
#include <afxwin.h>
#include "../tests/render_core.h"

// CglTestApp:
// See glTest.cpp for the implementation of this class
//

class CglTestApp : public CWinApp
{
public:
	CglTestApp();


// Overrides
public:
	virtual bool InitInstance();
	virtual int ExitInstance();
	void    CreateConsole();

// Implementation

public:
	DECLARE_MESSAGE_MAP()
	virtual int Run();
};

extern CglTestApp theApp;


class CMainFrame : public CFrameWnd
{

protected: 
	DECLARE_DYNAMIC(CMainFrame)

public:
	virtual bool PreCreateWindow(CREATESTRUCT& cs);
	virtual bool OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);

public:
	CMainFrame();
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	HCURSOR		m_hCursor;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg bool OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};


