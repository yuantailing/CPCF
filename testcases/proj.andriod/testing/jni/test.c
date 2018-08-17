#include <jni.h>
#include <time.h>
#include <android/log.h>

void TestMain();

JNIEXPORT void JNICALL Java_com_cicada_testing_MainActivity_TestMain(JNIEnv * env, jobject  obj)
{
	TestMain();
}


