// memleakdetect.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <string>
using namespace std;
#include "cppMemDbg.h"

class TestClass
{
public:
	TestClass()
	{
		x = new int[5];
	};

	~TestClass()
	{
		delete[] x;
	};

	string getString()
	{
		return "Test";
	}

private:
	int *x;
};


int _tmain(int argc, _TCHAR* argv[])
{
	{

		TestClass tc;
		cout << tc.getString();
	}

	Sleep(100);
	PrintMemoryLeakInfo();
	_gettch();
	return 0;
}

