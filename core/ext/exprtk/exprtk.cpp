#include "exprtk.h"

namespace rt
{

namespace _details
{

void _translate_code(const rt::String_Ref& code, std::string& out)
{
	rt::String temp = code;
	temp.Replace('\t', ' ');

	// .length -> []
	temp.Replace(".length", "[]");
	temp.Replace("++", "+=1");
	temp.Replace("--", "-=1");
	temp.Replace("||", " or ");
	temp.Replace("&&", " and ");

	
	{	rt::String_Ref symbol;
		rt::CharacterSet_Symbol		symbol_set(".");
		rt::CharacterSet_Alphabet	alpha_set("_");
	
		//  xx.yy -> xx_yy
		while(temp.GetNextToken(symbol_set, symbol))
		{
			if(alpha_set.Has(symbol[0]))
				symbol.Replace('.', '_');
		}
	}

	// = -> :=
	// ! -> not
	{
		rt::CharacterSet	op_set("/-=+!<>:%");
		for(UINT i=0;i<temp.GetLength();i++)
		{
			if(i && temp[i] == '=' && !op_set.Has(temp[i-1]) && temp[i+1] != '=')
			{	
				temp.Insert(i, ':');
				i++;
				continue;
			}
			
			if(temp[i] == '!' && temp[i+1] != '=')
			{
				temp.Insert(i, rt::SS(" true nand"));
				temp[i+10] = ' ';
				i+=10;
				continue;
			}
		}
	}

	out = temp.GetString();
}

void _translate_symbol(const rt::String_Ref& code, std::string& out)
{
	STACK_STRING_BEGIN(x) = code;
	STACK_STRING_END;

	x.Replace('.', '_');
	out = x.GetString();
}

} // namespace _details
} // namespace rt

