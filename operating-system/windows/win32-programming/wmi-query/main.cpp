#include <iostream>
#include <string>
#include <windows.h>

#include "WmiQuery.h"





int main()
{
	try{
		WmiQuery quiery;
		std::string resultos = quiery.execQuery(WmiQuery::WmiObj::OperatingSystem, "Caption");
	}
	catch (WmiException& excep)
	{
		std::cout << excep.what(); 
	}

	
	int d;
	std::cin >> d;
	return 0;
}