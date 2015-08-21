
/*************************************************************************************************
	This file is part of cppMemDbg - Easy to use C++ memory leak detection library

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
File: cppMemDbg.cpp
Version: 1.0
Last modification: 8/7/2009
Author: Ezequiel Gastón Miravalles
Website: http://www.neoegm.com/software/cppmemdbg/
License: GNU GPL v3 (attached, read above)

Notes: Read cppMemDbg.h notes.
*************************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

FILE *g_fFile = stdout;

//Configuration Constants ***************
#define	PRINT_OPERATIONS	1
#define MAX_ALLOC			256
#define PRINT_OUTPUT		g_fFile
#define	MAX_DELETE_STACK	64
//***************************************

enum EAllocType
{
	Type_Malloc,
	Type_Calloc,
	Type_Realloc,
	Type_New,
	Type_New_Array
};

enum EFreeType
{
	Type_Free,
	Type_Delete,
	Type_Delete_Array
};

int g_nAllocMem = 0;
struct DataAlloc
{
	void *pData;
	int nBytes;
	const char *pszFile;
	int nLine;
	EAllocType nType;
} g_pAllocData[MAX_ALLOC] = {0};

struct DeleteData
{
	const char *pszFile;
	int nLine;
} g_pDeleteStack[MAX_DELETE_STACK] = {0};

char g_nDeleteStackPtr = 0;

void InitCPPMemDbg(const char *pszOutputPath)
{
	if (pszOutputPath)
	{
		FILE *fFile = fopen(pszOutputPath, "w");
		
		if (fFile)
			g_fFile = fFile;
	}
}

void PrintTotalAllocatedMemory()
{
	fprintf(PRINT_OUTPUT, ">INFO\tTotal allocated memory at this moment: %d\n", g_nAllocMem);
}

void PrintMemoryReservedByCPPMemDbgLibrary()
{
	fprintf(PRINT_OUTPUT, ">INFO\tMemory reserved by cppMemDbg Library (constant): %d bytes\n", sizeof(g_nAllocMem) + sizeof(g_pAllocData) + sizeof(g_pDeleteStack) + sizeof(g_nDeleteStackPtr));
}

EFreeType GetCorrespondingFreeType(EAllocType eType)
{
	switch (eType)
	{
	case Type_Malloc:
	case Type_Calloc:
	case Type_Realloc:
		return Type_Free;
	case Type_New:
		return Type_Delete;
	case Type_New_Array:
		return Type_Delete_Array;
	default:
		fprintf(PRINT_OUTPUT, ">INTERNAL_ERROR\tUnknown allocation type %d\n", eType);
		return Type_Delete;
	}
}

const char * GetFreeTypeString(EFreeType eType)
{
	switch (eType)
	{
	case Type_Delete:
		return "delete";
	case Type_Delete_Array:
		return "delete []";
	case Type_Free:
		return "free";
	default:
		return "UNKNOWN";
	}
}

int FreeAllocDataBytes(void *pPointer, const char **ppszFile, int *pnLine, EFreeType eType)
{
	int i;

	if (!pPointer)
		return 0;

	for (i = 0; i < MAX_ALLOC; i++)
		if (g_pAllocData[i].pData == pPointer)
		{
			g_pAllocData[i].pData = 0;		//Free position

			g_nAllocMem -= g_pAllocData[i].nBytes;

			if (ppszFile)
				*ppszFile = g_pAllocData[i].pszFile;

			if (pnLine)
				*pnLine = g_pAllocData[i].nLine;
				
			EFreeType eCorrespondingType = GetCorrespondingFreeType(g_pAllocData[i].nType);
			
			if (eCorrespondingType != eType)
				fprintf(PRINT_OUTPUT, ">ERROR\tBad free type\t%s => %s\t%p\t%d\t(%s:%d)\n", GetFreeTypeString(eType), GetFreeTypeString(eCorrespondingType), pPointer, g_pAllocData[i].nBytes, g_pAllocData[i].pszFile, g_pAllocData[i].nLine);

			return g_pAllocData[i].nBytes;
		}

	return 0;
}

void AddAllocDataBytes(void *pPointer, size_t size, const char *pszFile, int nLine, EAllocType eType)
{
	int i;

	if (!pPointer)
		fprintf(PRINT_OUTPUT, ">MEM_ERROR\tCould not allocate %d bytes [%s:%d]\n", size, pszFile, nLine);

	for (i = 0; i < MAX_ALLOC; i++)
		if (!g_pAllocData[i].pData)
		{
			g_pAllocData[i].pData = pPointer;
			g_pAllocData[i].nBytes = size;
			g_pAllocData[i].pszFile = pszFile;
			g_pAllocData[i].nLine = nLine;
			g_pAllocData[i].nType = eType;
			g_nAllocMem += size;
			return;
		}

	fprintf(PRINT_OUTPUT, ">INTERNAL_ERROR\tAllocation stack overflow, please increase MAX_ALLOC\n");
}

void DumpUnfreedBlocks()
{
	int i;

	for (i = 0; i < MAX_ALLOC; i++)
		if (g_pAllocData[i].pData)
			fprintf(PRINT_OUTPUT, ">INFO\tUnfreed block\t%p\t%d\t\t[%s:%d]\n", g_pAllocData[i].pData, g_pAllocData[i].nBytes, g_pAllocData[i].pszFile, g_pAllocData[i].nLine);
}

void * mallocb(size_t size, const char *pszFile, int nLine)
{
	void * pRet = malloc(size);

	if (PRINT_OPERATIONS)
		fprintf(PRINT_OUTPUT, ">malloc\t%p\t%d\t[%s:%d]\n", pRet, size, pszFile, nLine);

	AddAllocDataBytes(pRet, size, pszFile, nLine, Type_Malloc);

	return pRet;
}

void * callocb(size_t num, size_t size, const char *pszFile, int nLine)
{
	void * pRet = calloc(num, size);

	if (PRINT_OPERATIONS)
		fprintf(PRINT_OUTPUT, ">calloc\t%p\t%d\t(%d x %d)\t[%s:%d]\n", pRet, num*size, num, size, pszFile, nLine);

	AddAllocDataBytes(pRet, num*size, pszFile, nLine, Type_Calloc);

	return pRet;
}

void freeb(void *memblock, const char *pszFile, int nLine)
{
	int nBytes;
	const char * pszOldFile = NULL;
	int nOldLine = 0;

	if (memblock)
	{
		nBytes = FreeAllocDataBytes(memblock, &pszOldFile, &nOldLine, Type_Free);

		if (PRINT_OPERATIONS)
			if (nBytes)
				fprintf(PRINT_OUTPUT, ">free\t%p\t%d\t(%s:%d)\t[%s:%d]\n", memblock, nBytes, pszOldFile, nOldLine, pszFile, nLine);
			else
				fprintf(PRINT_OUTPUT, ">free\t%p\t%d\t\t[%s:%d]\n", memblock, nBytes, pszFile, nLine);

		if (!nBytes)
			fprintf(PRINT_OUTPUT, ">ERROR\tTrying to free unallocated memory: %p [%s:%d]\n", memblock, pszFile, nLine);
	}

	free(memblock);
}

void * reallocb(void *memblock, size_t size, const char *pszFile, int nLine)
{
	void * pRet;
	int nBytes = FreeAllocDataBytes(memblock, NULL, NULL, Type_Free);

	if (memblock && !nBytes)
		fprintf(PRINT_OUTPUT, ">ERROR\tTrying to free unallocated memory while reallocating: %p [%s:%d]\n", memblock, pszFile, nLine);

	pRet = realloc(memblock, size);

	AddAllocDataBytes(pRet, size, pszFile, nLine, Type_Realloc);

	if (PRINT_OPERATIONS)
		if (memblock)
			fprintf(PRINT_OUTPUT, ">realloc\t%p\t=>\t%p\t%d\t=>\t%d\t[%s:%d]\n", memblock, pRet, nBytes, size, pszFile, nLine);
		else
			fprintf(PRINT_OUTPUT, ">realloc(A)\t%p\t%d\t[%s:%d]\n", pRet, size, pszFile, nLine);

	return pRet;
}

void PrintMemoryLeakInfo()
{
	if (g_nAllocMem == 0)
		fprintf(PRINT_OUTPUT, ">INFO\tNo memory leaks detected\n");
	else if (g_nAllocMem < 0)
		fprintf(PRINT_OUTPUT, ">INFO\tPROBLEM: There was more freed memory than allocated (%d bytes) [This shouldn't happen, memory corruption?]\n", g_nAllocMem);
	else
		fprintf(PRINT_OUTPUT, ">INFO\tPROBLEM: Memory leak found (%d bytes)\n", g_nAllocMem);

	if (g_nDeleteStackPtr)
		fprintf(PRINT_OUTPUT, ">INFO\tPROBLEM: Delete stack not empty (%d items)\n", g_nDeleteStackPtr);

	DumpUnfreedBlocks();
}

void * operator new(size_t size, const char *pszFile, int nLine)
{
	void * pRet = malloc(size);

	if (PRINT_OPERATIONS)
		fprintf(PRINT_OUTPUT, ">new\t%p\t%d\t[%s:%d]\n", pRet, size, pszFile, nLine);

	AddAllocDataBytes(pRet, size, pszFile, nLine, Type_New);

	return pRet;
}

void * operator new[](size_t size, const char *pszFile, int nLine)
{
	void * pRet = malloc(size);

	if (PRINT_OPERATIONS)
		fprintf(PRINT_OUTPUT, ">new[]\t%p\t%d\t[%s:%d]\n", pRet, size, pszFile, nLine);

	AddAllocDataBytes(pRet, size, pszFile, nLine, Type_New_Array);

	return pRet;
}

#include <new>

void operator delete(void *memblock) throw ()
{
	int nBytes;
	const char * pszOldFile = NULL;
	int nOldLine = 0;
	
	const char *pszFile = NULL;
	int nLine = 0;
	
	if (g_nDeleteStackPtr > 0)
	{
		pszFile = g_pDeleteStack[g_nDeleteStackPtr-1].pszFile;
		nLine = g_pDeleteStack[g_nDeleteStackPtr-1].nLine;
		g_nDeleteStackPtr--;
	}
	else
		fprintf(PRINT_OUTPUT, ">ERROR\tDelete stack underflow (THIS SHOULDN'T HAPPEN... MULTIPLE THREADS?)\n");

	if (memblock)
	{
		nBytes = FreeAllocDataBytes(memblock, &pszOldFile, &nOldLine, Type_Delete);

		if (PRINT_OPERATIONS)
			if (nBytes)
				fprintf(PRINT_OUTPUT, ">delete\t%p\t%d\t(%s:%d)\t[%s:%d]\n", memblock, nBytes, pszOldFile, nOldLine, pszFile, nLine);
			else
				fprintf(PRINT_OUTPUT, ">delete\t%p\t%d\t\t[%s:%d]\n", memblock, nBytes, pszFile, nLine);

		if (!nBytes)
			fprintf(PRINT_OUTPUT, ">ERROR\tTrying to free unallocated memory: %p [%s:%d]\n", memblock, pszFile, nLine);
	}

	free(memblock);
}

void operator delete[](void *memblock) throw()
{
	int nBytes;
	const char * pszOldFile = NULL;
	int nOldLine = 0;
	
	const char *pszFile = NULL;
	int nLine = 0;

	if (g_nDeleteStackPtr > 0)
	{
		pszFile = g_pDeleteStack[g_nDeleteStackPtr-1].pszFile;
		nLine = g_pDeleteStack[g_nDeleteStackPtr-1].nLine;
		g_nDeleteStackPtr--;
	}
	else
		fprintf(PRINT_OUTPUT, ">ERROR\tDelete stack underflow (THIS SHOULDN'T HAPPEN... MULTIPLE THREADS?)\n");

	if (memblock)
	{
		nBytes = FreeAllocDataBytes(memblock, &pszOldFile, &nOldLine, Type_Delete_Array);

		if (PRINT_OPERATIONS)
			if (nBytes)
				fprintf(PRINT_OUTPUT, ">delete[]\t%p\t%d\t(%s:%d)\t[%s:%d]\n", memblock, nBytes, pszOldFile, nOldLine, pszFile, nLine);
			else
				fprintf(PRINT_OUTPUT, ">delete[]\t%p\t%d\t\t[%s:%d]\n", memblock, nBytes, pszFile, nLine);

		if (!nBytes)
			fprintf(PRINT_OUTPUT, ">ERROR\tTrying to free unallocated memory: %p [%s:%d]\n", memblock, pszFile, nLine);
	}

	free(memblock);
}

void pre_delete(const char *pszFile, int nLine)
{
	if (g_nDeleteStackPtr < MAX_DELETE_STACK-1)
	{
		g_pDeleteStack[g_nDeleteStackPtr].pszFile = pszFile;
		g_pDeleteStack[g_nDeleteStackPtr].nLine = nLine;
		g_nDeleteStackPtr++;
	}
	else
		fprintf(PRINT_OUTPUT, ">INTERNAL_ERROR\tDelete stack overflow, please increase MAX_DELETE_STACK\n");
}
