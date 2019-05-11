#include "../../essentials.h"


struct _test_section
{	LPCSTR	_func_name;
	_test_section(LPCSTR func)
	{	_LOG("/===== BEGIN: "<<func<<" =====\\");
		_func_name = func;
	}
	~_test_section()
	{	_LOG("\\===== END:   "<<_func_name<<" =====/");
		_LOG(' ');
		_LOG(' ');
	}
};

#define DEF_TEST_SECTION	_test_section __test_s(__FUNCTION__);

struct vFun
{
	virtual void func(){}
};

struct non_vFun
{
	__declspec(noinline) void func(){}
};

template<typename T>
struct cb_t
{	static void func(LPVOID cookie)
	{	(*((T*)cookie))();
	}
};

void callback_to_member_function()
{
	static const size_t t = 1000000000;

	vFun		vf;
	non_vFun	nvf;

	vFun*		pVF = &vf;

	// Call by cast to uni-class with function of same signature
	LPVOID		obj = &nvf;
	auto		funptr = &non_vFun::func;
	LPVOID		memfunc = (LPVOID&)funptr;
	struct CallWithSameFunctionSignature
	{	__declspec(noinline) void func(){}
	};
	typedef	void (__thiscall CallWithSameFunctionSignature::* t_func)();

	// Call by callback function
	LPVOID cookie = &nvf;
	struct cb
	{	static void func(LPVOID cookie)
		{	((non_vFun*)cookie)->func();
		}
	};
	typedef	void (*t_callback_func)(LPVOID cookie);
	t_callback_func cbfunc = &cb::func;


	auto lambda = [&nvf]()
	{	nvf.func();
	};

	LPVOID cookie_t = &lambda;
	typedef cb_t<decltype(lambda)> cb_t_func;
	t_callback_func cbfunc_t = &cb_t_func::func;
	   	
	os::HighPerformanceCounter tm;

	tm.Restart();
	for(size_t i=0; i<t; i++)nvf.func();
	_LOG("Reference: "<<t*1000000/tm.TimeLapse()<<" cps");

	tm.Restart();
	for(size_t i=0; i<t; i++)pVF->func();
	_LOG("Virtual Function: "<<t*1000000/tm.TimeLapse()<<" cps");

	tm.Restart();
	for(size_t i=0; i<t; i++)
	{
		(((CallWithSameFunctionSignature*)obj)->*(t_func&)memfunc)();
	}
	_LOG("Member Function Pointer: "<<t*1000000/tm.TimeLapse()<<" cps");

	tm.Restart();
	for(size_t i=0; i<t; i++)
	{
		cbfunc(cookie);
	}
	_LOG("Callback Function: "<<t*1000000/tm.TimeLapse()<<" cps");

	tm.Restart();
	for(size_t i=0; i<t; i++)
	{
		cbfunc_t(cookie_t);
	}
	_LOG("Callback Lambda Function: "<<t*1000000/tm.TimeLapse()<<" cps");

}