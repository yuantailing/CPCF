#include <stdio.h>
#include "../../essentials.h"
#include "../../core/rt/json.h"


int main(int argc, char **argv)
{
	rt::String s = 
	((
		J(a) = 	JA(1,2,3,"hello",
					JA('a',2.3,4.5f),
					(
						J(b) = JA('a',2.3),
						J(c) = 3
					)
				)
	));
	
	_LOG(s);
	return 0;
}
