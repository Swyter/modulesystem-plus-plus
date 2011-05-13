#include <Windows.h>
#include "Python.h"
#include <iostream>
#include "ModuleSystem.h"

int main(int argc, char **argv)
{
	ModuleSystem ms("D:\\Dev\\test");

	ms.Compile();
	
	system("pause");
}
