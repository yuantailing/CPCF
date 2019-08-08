#pragma once
#include "../../core/inet/tinyhttpd.h"
#include "../../core/inet/tinyhttpd_fileserv.h"
#include "../../core/os/high_level.h"


class Scheduler;

enum _tagServiceNames
{
	SERVICE_CORE = 0,
	SERVICE_ECHO,
	SERVICE_VPATH,
	SERVICE_SCHEDULER,

	SERVICE_MAXNAME
};

class Pylon: protected inet::HttpHandler<Pylon>,
			 public os::Daemon
{
	static const int DELAYED_RELEASE_TTL = 5000;
protected:
	inet::TinyHttpd	m_Httpd;
	bool			m_IsRunning;
	os::Event		m_EndOfRun;
	bool			m_WantRestart;
	os::Timestamp	m_StartTime;

protected:
	void			_UpdateServiceEndpoints();
	rt::String		_PylonConfigFilename;
	os::CriticalSection	m_ServiceStateCS;

	// vpath
	rt::Buffer<inet::HttpVirtualPath*>	m_pVPaths;

	//// scheduler
	Scheduler*	m_pScheduler;

	// echo
	rt::BufferEx<inet::HttpRequestEcho*>	m_EchoRequests;

public:
	Pylon();
	~Pylon();
	bool	OnRequest(inet::HttpResponse& resp);

	void	OnDaemonControl(DWORD dwControl);

	bool	LoadAllServices(LPCSTR config_fn);
	bool	ReloadService(_tagServiceNames svc, const rt::String_Ref& config);	// config_xml is empty for unload, false for nothing changed

	bool	Start(DWORD port, LPCSTR binding = NULL);	// NULL binding for "127.0.0.1"

	void	Stop();
	bool	IsRunning() const { return m_IsRunning; }
	bool	WaitForRestart();

	inet::InetAddr	GetBindedAddress() const { return m_Httpd.GetBindedAddress(); }
	void			GetBindedAddress(LPSTR p) const { m_Httpd.GetBindedAddress().GetDottedDecimalAddress(p); }
	DWORD			GetBindedPort() const { return m_Httpd.GetBindedPort(); }

};

extern int RunPylon(os::CommandLine& cmdline);

