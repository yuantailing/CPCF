#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include "../../core/os/file_dir.h"
#include "../../core/os/kernel.h"

void TestMain();

os::CommandLine	g_Cmdline;

int _tmain(int argc, _TCHAR* argv[])
{
	g_Cmdline.Parse(argc, argv);
	TestMain();

	return 0;
}

