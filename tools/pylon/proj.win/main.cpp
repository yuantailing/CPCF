#include "../../../essentials.h"
#include "../pylon.h"


int _tmain(int argc, _TCHAR* argv[])
{
	os::CommandLine cmd;
	cmd.Parse(argc, argv);
	return RunPylon(cmd);
}

