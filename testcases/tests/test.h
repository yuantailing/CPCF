#pragma once
#include "../../essentials.h"

void testing_string();
void testing_string_conv();
void testing_rt();
void testing_buffer();
void testing_xml();
void testing_html();
void testing_json();
void testing_file();
void testing_timedate();
void testing_Httpclient();
void testing_socket();
void testing_pfw();
void testing_sockettimed();
void testing_multithread();
void testing_download();
void testing_inet_encoding();
void testing_inet_encoding_custom();
void testing_delayed_deletion();
void testing_sysinfo();
void testing_socket_io(bool recv);
void test_filelist();
void test_smallmath();
void testing_vm();
void testing_pcqueue();
void test_BinarySearch();
void testing_plog();
void test_Precompiler();
void testing_commandline();

void testing_httpd();

void test_botan_hash();
void test_botan_cipher();
void test_ipp_zlib();
void test_ipp_canvas();
void test_ipp_image();
void test_ipp_major_color();
void test_express_tk();
void test_ipp_zip();

void callback_to_member_function();

void image_to_text();


#if defined(PLATFORM_INTEL_IPP_SUPPORT)
void test_ipp_Saliency(LPCSTR);
void test_ipp_matting();
void test_ipp_image_apps();
void test_ipp_imageproc();
#endif

#if defined(PLATFORM_INTEL_MKL_SUPPORT)
void test_mkl_vector();
void test_mkl_linequ();
#endif


#define DEF_TEST_SECTION	_LOG(' '); _LOG("==== "<< __func__<<" ====");

