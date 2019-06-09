#pragma once
#include "../../essentials.h"

void test_string();
void test_string_conv();
void test_rt();
void test_buffer();
void test_xml();
void test_html();
void test_json();
void test_file();
void test_timedate();
void test_Httpclient();
void test_socket();
void test_pfw();
void test_sockettimed();
void test_multithread();
void test_download();
void test_inet_encoding();
void test_inet_encoding_custom();
void test_delayed_deletion();
void test_sysinfo();
void test_socket_io(bool recv);
void test_filelist();
void test_smallmath();
void test_vm();
void test_pcqueue();
void test_BinarySearch();
void test_plog();
void test_Precompiler();
void test_commandline();
void test_sortedpush();

void test_httpd();

void test_botan_hash();
void test_botan_cipher();
void test_ipp_zlib();
void test_ipp_canvas();
void test_ipp_image();
void test_ipp_major_color();
void test_express_tk();
void test_ipp_zip();

void callback_to_member_function();
void hash_func();

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

