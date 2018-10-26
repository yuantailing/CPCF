#include "../../core/os/kernel.h"
#include "../../core/ext/exprtk/exprtk.h"



void test_express_tk()
{
	typedef double T;

	rt::Expression<T> Expr;
	T x[100];

	Expr.BindVariable("x", x, 100);
	
	for(UINT i=0;i<sizeofArray(x);i++)x[i] = 0.1f;
	
	rt::String_Ref source = __STRING(
		for(var i=1;i<100;i++)
		{	x[i-1] = x[i] - 0.1;
		}
	);

	if(Expr.Compile(source))
	{
		os::Timestamp tm;
		tm.LoadCurrentTime();

		for(UINT i=0;i<100000;i++)
			Expr.Compute();

		_LOGC("ExprTk: "<<rt::tos::TimeSpan<false>(tm.TimeLapse()));
		_LOG(x[0]);

		for(UINT i=0;i<sizeofArray(x);i++)x[i] = 0.1f;
		tm.LoadCurrentTime();
		
		for(UINT i=0;i<100000;i++)
			for(UINT i=1;i<sizeofArray(x);i++)
			{	x[i-1] = x[i] - 0.1;
			}

		_LOGC("Native: "<<rt::tos::TimeSpan<false>(tm.TimeLapse()));
		_LOG(x[0]);
	}
	else
	{	_LOG_ERROR(Expr.GetLastError());
	}
}
