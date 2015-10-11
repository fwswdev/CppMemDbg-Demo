// memleakdetect.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
using namespace std;
#include "cppMemDbg.h"


#define SHARED_PTR  boost::shared_ptr // TODO: use typedef on this one

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


class Child; // forward reference

class Parent
{
public:
	Parent(string name, SHARED_PTR<Child> child)
	{
		this->child = child;
		this->name = name;
	}

private:
	SHARED_PTR<Child> child;
	string name;
};

class Child
{
public:
	Child(string name,SHARED_PTR<Parent> mom, SHARED_PTR<Parent> dad)
	{
		this->mom = mom;
		this->dad = dad;
		this->name = name;
		cout << "Child " << name << " constructor";
	}

	~Child()
	{
		cout << "Child " << name << " destructor";
	}

private:
	SHARED_PTR<Parent> mom;
	SHARED_PTR<Parent> dad;
	string name;
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

