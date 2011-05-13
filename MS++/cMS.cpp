#include <Windows.h>
#include "Python.h"
#include <iostream>
#include "ModuleSystem.h"

int main(int argc, char **argv)
{
	Py_Initialize();
	
	ModuleSystem ms;
	LARGE_INTEGER frequency, t1, t2;
	
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&t1);

	try
	{
		ms.Compile();
	}
	catch (CompileException e)
	{
		std::cout << "Exception: " << e.GetText() << std::endl;
	}

	QueryPerformanceCounter(&t2);
	std::cout << "Compile time: " << ((t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart) << "ms" << std::endl;
	
	system("pause");
}
