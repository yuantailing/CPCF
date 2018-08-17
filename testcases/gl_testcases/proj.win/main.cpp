#include "main.h"


#ifndef _CONSOLE
	int APIENTRY wWinMain(HINSTANCE hInstance,
		                 HINSTANCE hPrevInstance,
		                 LPTSTR    lpCmdLine,
		                 int       nCmdShow
	)
	{
		os::CommandLine	cmd;
		cmd.Parse(lpCmdLine);
#else
	int _tmain(int argc, _TCHAR* argv[])
	{	os::CommandLine	cmd;
		cmd.Parse(argc, argv);
#endif

	cmd.LoadEnvironmentVariablesAsOptions();

	//if(cmd.HasOption("log"))
	//{	os::SetLogFile(rt::String(cmd.GetOption("log", "viscw.log")));
	//}

	MainWnd wnd;
	if(wnd.Create())
	{
		wnd.SetWindowText(L"OpenGL Demo");
		wnd.SetAsMainWnd();
		wnd.ResizeWindowByClientArea(800,600);
		wnd.CenterWindow();
		wnd.ShowWindow();
	}

	return os::UserInputEventPump::Get()->MainLoop();
}


bool MainWnd::OnInitWnd()
{
	return _core.Init(*this) && os::WndBase::OnInitWnd();
}