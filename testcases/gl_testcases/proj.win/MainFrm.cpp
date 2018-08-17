#include "MainFrm.h"
#include <fcntl.h>
#include <Wincon.h>
#include <io.h>
#include <iostream>

BEGIN_MESSAGE_MAP(CglTestApp, CWinApp)
END_MESSAGE_MAP()


// CglTestApp construction

CglTestApp::CglTestApp()
{
	// TODO: replace application ID string below with unique ID string; recommended
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	//SetAppID(_T("glTest.AppID.NoVersion"));

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

// The one and only CglTestApp object

CglTestApp theApp;


void CglTestApp::CreateConsole()
{
#pragma warning(disable:4311)

	HANDLE hConsole = NULL;

    int  hConHandle;
    long lStdHandle;
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    FILE   *fp;
    // allocate a console for this app
	AllocConsole();
	// set the screen buffer to be big enough to let us scroll text
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),	&coninfo);
    coninfo.dwSize.Y = 2000;
	// How many lines do you want to have in the console buffer
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),	coninfo.dwSize);
	// redirect unbuffered STDOUT to the console
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	lStdHandle = reinterpret_cast<long>( GetStdHandle(STD_OUTPUT_HANDLE) );
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stdout = *fp;
    setvbuf( stdout, NULL, _IONBF, 0 ); 
	// redirect unbuffered STDIN to the console
    lStdHandle = reinterpret_cast<long>(GetStdHandle(STD_INPUT_HANDLE)); 
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "r" ); 
	*stdin = *fp;
    setvbuf( stdin, NULL, _IONBF, 0 );
    // redirect unbuffered STDERR to the console
    lStdHandle = reinterpret_cast<long>(GetStdHandle(STD_ERROR_HANDLE)); 
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stderr = *fp;
    setvbuf( stderr, NULL, _IONBF, 0 );

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
	std::ios::sync_with_stdio();

	HWND hConsoleWnd = ::GetConsoleWindow();
	ASSERT(hConsoleWnd);

#pragma warning(default:4311)
}

bool CglTestApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	
	EnableTaskbarInteraction(false);

	CreateConsole();

	// To create the main window, this code creates a new frame window
	// object and then sets it as the application's main window object
	CMainFrame* pFrame = new CMainFrame;
	if (!pFrame)return false;
	m_pMainWnd = pFrame;
	// create and load the frame with its resources
	pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);
	
	pFrame->SetWindowPos(NULL,0,0, 800, 600, SWP_NOZORDER | SWP_NOMOVE);
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	return true;
}

int CglTestApp::ExitInstance()
{
	return CWinApp::ExitInstance();
}



IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_hCursor = ::LoadCursor(NULL, IDC_ARROW);
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	
	return 0;
}

bool CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return false;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0);
	return true;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG


// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	// forward focus to the view window
	//m_wndView.SetFocus();
}

bool CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}



bool CMainFrame::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	::SetCursor(m_hCursor);

	return CFrameWnd::OnSetCursor(pWnd, nHitTest, message);
}


int CglTestApp::Run()
{
	return 
	//return CWinApp::Run();
}
