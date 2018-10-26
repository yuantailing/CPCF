#include <stdio.h>
#include "../../essentials.h"
#include "../tests/test.h"

extern void TestMain();

int main(int argc, char **argv)
{
	os::CommandLine cmd;
	cmd.Parse(argc, argv);
	
	TestMain();

	return 0;
}
