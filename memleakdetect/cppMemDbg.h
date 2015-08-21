
/*************************************************************************************************
	cppMemDbg - Easy to use C++ memory leak detection library

	Copyright (C) 2009 Ezequiel Gastón Miravalles

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************************************/


/*************************************************************************************************
Software: cppMemDbg
File: cppMemDbg.h
Version: 1.0
Last modification: 8/7/2009
Author: Ezequiel Gastón Miravalles
Website: http://www.neoegm.com/software/cppmemdbg/
License: GNU GPL v3 (attached, read above)

Notes: Include this file after the standard headers in every file in which you allocate or free
       memory.
	   
	   Supported C functions: malloc, realloc, calloc, free. Supported C++ operators: new,
	   new [], delete, delete [].
	   
	   To get a final dump of memory leaks, you should call PrintMemoryLeakInfo() just before
	   your program exits.
	   
	   You can call InitCPPMemDbg() to redirect the library output to a given file.

	   Other option, to simplify is to create a global object of the "cppMemDbg" type at the
	   beginning. You can pass a file path to the constructor to redirect the output. It will
	   call InitCPPMemDbg at construction and PrintMemoryLeakInfo() at destruction.
	   
	   IMPORTANT: Then only code modification needed is to never call the delete operator without
	   knowing if its argument is NULL. So: "delete a;" should become "if (a) delete a;". Otherwise,
	   the library may show incorrect messages.
	
	   You can tweak the macro preprocessor definitions on cppMemDbg.cpp to customize the behaviour:
			PRINT_OPERATIONS
				Function: output to stdout every operation (alloc or free) done.
				Default value: 1
				Valid values
					0   Just output errors and notifications
					1   Output every operation, errors and notifications
			MAX_ALLOC
				Function: memory tracking stack array element count.
				Default value: 256
				Valid values: integers higher than 0.
			PRINT_OUTPUT
				Function: location to print the library generated notifications
				Default value: stdout
				Valid values: any stream/file in which you could print using fprintf
			MAX_DELETE_STACK
				Function: delete nesting stack array element count.
				Default value: 64
				Valid values: integers higher than 0.
	
		There are tho more functions you can use to get further information:
			PrintTotalAllocatedMemory()
				Prints the accumulative amount of memory allocated at the moment of the call
			PrintMemoryReservedByCPPMemDbgLibrary()
				Prints the amount of memory reserved by the library (defined at compilation
				time by the MAX_ALLOC and MAX_DELETE_STACK constants)
*************************************************************************************************/

#pragma once

#include <cstdlib>

//Redefines
#define malloc(size) mallocb(size, __FILE__, __LINE__)
#define free(memblock) freeb(memblock, __FILE__, __LINE__)
#define realloc(memblock, size) reallocb(memblock, size, __FILE__, __LINE__)
#define calloc(num, size) callocb(num, size, __FILE__, __LINE__)

//Functions
void InitCPPMemDbg(const char *pszOutputPath = NULL);
void PrintMemoryLeakInfo();
void PrintTotalAllocatedMemory();
void PrintMemoryReservedByCPPMemDbgLibrary();

//Auto init struct
struct cppMemDbg
{
	cppMemDbg(const char *pszOutputPath = NULL) { InitCPPMemDbg(pszOutputPath); }
	~cppMemDbg() { PrintMemoryLeakInfo(); }
};

//Redefined functions
void * mallocb(size_t size, const char *pszFile, int nLine);
void freeb(void *memblock, const char *pszFile, int nLine);
void * reallocb(void *memblock, size_t size, const char *pszFile, int nLine);
void * callocb(size_t num, size_t size, const char *pszFile, int nLine);

//For C++

void * operator new(size_t size, const char *pszFile, int nLine);
void * operator new[](size_t size, const char *pszFile, int nLine);

void operator delete(void *pvMem) throw();
void operator delete[](void *pvMem) throw();

void pre_delete(const char *pszFile, int nLine);

//Redefine new and delete
#define new 	new(__FILE__, __LINE__)
#define delete	pre_delete(__FILE__, __LINE__),delete
