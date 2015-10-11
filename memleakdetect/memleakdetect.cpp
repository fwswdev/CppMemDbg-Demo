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
#define WEAK_PTR  boost::weak_ptr // TODO: use typedef on this one

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
	Parent(string name)
	{
		this->name = name;
		cout << "Parent constructor " << name;
	}

	~Parent()
	{
		cout << "Parent destructor " << name;
	}

	void SetChild(SHARED_PTR<Child> child)
	{
		this->child = child;
	}

private:
	//SHARED_PTR<Child> child; // this will cause memory leak
	WEAK_PTR<Child> child; // this will not cause memory leak
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


void BoostSharedPtrExample()
{
	SHARED_PTR<Parent> mom(new Parent("mom"));
	SHARED_PTR<Parent> dad(new Parent("dad"));

	SHARED_PTR<Child> child(new Child("myChild", mom, dad));
	mom->SetChild(child);
	dad->SetChild(child);

}


int _tmain(int argc, _TCHAR* argv[])
{
	{

		TestClass tc;
		cout << tc.getString();
	}

	BoostSharedPtrExample();

	Sleep(100);
	PrintMemoryLeakInfo();
	_gettch();
	return 0;
}

