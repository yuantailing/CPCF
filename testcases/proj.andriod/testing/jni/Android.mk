LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := shared_testing
LOCAL_SRC_FILES :=	\
../../../../shared_api/rt/xml_xhtml.cpp \
../../../../shared_api/os/kernel.cpp \
../../../../shared_api/os/file_zip.cpp \
../../../../shared_api/os/file_dir.cpp \
../../../../shared_api/os/multi_thread.cpp \
../../../../shared_api/inet/inet.cpp \
../../../../shared_api/inet/tinyhttpd.cpp \
../../../../shared_api/inet/tinyhttpd_websocket.cpp \
../../../../shared_library/botan/botan.cpp \
../../../../shared_library/lib_inc.c \
../../../../testcases/shared_tests/entrypoint.cpp \
../../../../testcases/shared_tests/test_api.cpp \
../../../../testcases/shared_tests/test_http.cpp \
../../../../testcases/shared_tests/test_botan.cpp \
test.c \

LOCAL_LDLIBS    := -lm -llog
LOCAL_CPP_FEATURES += exceptions
LOCAL_CPP_FEATURES += rtti

APP_OPTIM := debug
LOCAL_CFLAGS := -D_DEBUG
LOCAL_CFLAGS += -D_GLIBCXX_PERMIT_BACKWARD_HASH


include $(BUILD_SHARED_LIBRARY)


