#pragma once
#include "../../essentials.h"

namespace rt
{

struct UnitTests
{
	static void encoding();
	static void string();
	static void string_conv();
	static void rt();
	static void buffer();
	static void xml();
	static void html();
	static void json();
	static void file();
	static void timedate();
	static void http_client();
	static void http_nav();
	static void socket();
	static void pfw();
	static void sockettimed();
	static void multithread();
	static void download();
	static void inet_encoding();
	static void delayed_deletion();
	static void sysinfo();
	static void socket_io();
	static void socket_socket_event();
	static void filelist();
	static void smallmath();
	static void vm();
	static void pcqueue();
	static void binary_search();
	static void plog();
	static void precompiler();
	static void commandline();
	static void sortedpush();

	static void httpd();

	static void botan_hash();
	static void botan_cipher();
	static void ipp_zlib();
	static void ipp_canvas();
	static void ipp_image();
	static void ipp_major_color();
	static void express_tk();
	static void ipp_zip();

	static void callback_to_member_function();
	static void hash_func();

	static void image_to_text();


	#if defined(PLATFORM_INTEL_IPP_SUPPORT)
	static void ipp_matting();
	static void ipp_image_apps();
	static void ipp_imageproc();
	#endif

	#if defined(PLATFORM_INTEL_MKL_SUPPORT)
	static void mkl_vector();
	static void mkl_linequ();
	#endif
};

} // namespace rt


