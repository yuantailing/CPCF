#include "test.h"

void exp_tracking_proc_ip();
#pragma comment(lib,"Mpr.lib")


#if defined (PLATFORM_ANDROID) || defined (PLATFORM_IOS)
extern "C" 
#endif
void TestMain()
{
   test_ipp_imageproc();
	//testing_multithread();
	//testing_plog();
	//test_mkl_linequ();
	return;

	rt::String dir;
/*
#ifndef PLATFORM_WIN
	os::GetAppSandbox(dir, "CPF_Tester");
	_LOG(dir);
	os::File::SetCurrentDirectory(dir);
#endif
*/
	os::File::GetCurrentDirectory(dir);
	_LOG(dir);

	test_botan_hash();
	testing_rt();
	testing_buffer();
	testing_string_conv();
	testing_string();
	testing_json();
	test_express_tk();
	testing_xml();
	testing_html();
	testing_file();
	test_Precompiler();
	testing_vm();
	testing_timedate();
	test_smallmath();
	test_filelist();
	testing_multithread();
	testing_inet_encoding();
	testing_inet_encoding_custom();
	testing_sysinfo();
	test_BinarySearch();
	test_botan_cipher();
	
	if(0) // non-static test
	{
		testing_pfw();
		testing_plog();
		testing_pcqueue();
		//testing_commandline();
		//testing_socket();
		//testing_sockettimed();
		//testing_delayed_deletion();
		//testing_socket_io(1);

		//testing_Httpclient();
		//testing_download();

		//testing_HttpNav();
		//testing_httpd();
	
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



