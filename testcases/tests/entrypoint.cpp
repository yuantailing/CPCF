#include "test.h"

void exp_tracking_proc_ip();
#pragma comment(lib,"Mpr.lib")


#if defined (PLATFORM_ANDROID) || defined (PLATFORM_IOS)
extern "C" 
#endif
void TestMain()
{
	if(!os::CommandLine::Get().HasOption("verify"))
	{
		test_rt();
		//callback_to_member_function();
		return;
	}
	else
	{
		LPCSTR logfile = "../testcases.log";
		os::SetLogFile(logfile, false);
		os::SetLogPrefix(os::LogPrefix());

		rt::String fn;
		os::File::ResolveRelativePath(logfile, fn);
		_LOGC("Log: "<<fn);
				

		rt::String dir;
	/*
	#ifndef PLATFORM_WIN
		os::GetAppSandbox(dir, "CPF_Tester");
		_LOG(dir);
		os::File::SetCurrentDirectory(dir);
	#endif
	*/
		os::File::GetCurrentDirectory(dir);
		_LOGC(dir);

		test_rt();
		test_buffer();
		test_sortedpush();
		test_string_conv();
		test_string();
		test_json();
		test_express_tk();
		test_xml();
		test_html();
		test_file();
		test_Precompiler();
		test_vm();
		test_timedate();
		test_smallmath();
		test_filelist();
		test_multithread();
		test_inet_encoding();
		test_inet_encoding_custom();
		test_sysinfo();
		test_BinarySearch();
		test_botan_cipher();
		test_botan_hash();
	
		if(0) // non-static test
		{
			test_pfw();
			test_plog();
			test_pcqueue();
			//test_commandline();
			//test_socket();
			//test_sockettimed();
			//test_delayed_deletion();
			//test_socket_io(1);

			//test_Httpclient();
			//test_download();

			//test_HttpNav();
			//test_httpd();
	
			//for(int i=0; i<21; i++)
			//	test_ipp_Saliency(rt::String_Ref("sai/img") + i + ".jpg");

			//test_ipp_canvas();
			//test_ipp_matting();
			//test_ipp_imageproc();
			//test_ipp_image();
			//test_ipp_image_apps();
			//test_ipp_zlib();
			//test_ipp_zip();
			//test_mkl_vector();
		}
	}
}



