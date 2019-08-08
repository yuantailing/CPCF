#pragma once
#include "../../essentials.h"


extern int  pylon_copy(const rt::String_Ref& dest, const rt::String_Ref& src);
extern int  pylon_launch(const rt::String_Ref& endpoint, const rt::String_Ref& cmdline, const rt::String_Ref& workdir, bool wait, bool dump_log);
extern int  pylon_jobstat(const rt::String_Ref& endpoint, const rt::String_Ref& job_id, bool wait, bool dump_log);

class Endpoint: public rt::String
{
public:
	bool Parse(const rt::String_Ref& ep);
};