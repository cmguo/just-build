// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include <Windows.h>

#include "SystemEmulation.h"

#include <assert.h>
#include <stdio.h>
#include <io.h>

namespace SystemEmulation
{

	HLOCAL WINAPI LocalAlloc(
		_In_  UINT uFlags,
		_In_  SIZE_T uBytes
		)
	{
		assert(uFlags == LMEM_FIXED);
		return new char [uBytes];
	}

	HLOCAL WINAPI LocalFree(
		_In_  HLOCAL hMem
		)
	{
		delete [] (char *)hMem;
		return NULL;
	}

#undef FormatMessageA

	DWORD WINAPI FormatMessage2A(
		_In_      DWORD dwFlags,
		_In_opt_  LPCVOID lpSource,
		_In_      DWORD dwMessageId,
		_In_      DWORD dwLanguageId,
		_Out_     LPSTR lpBuffer,
		_In_      DWORD nSize,
		_In_opt_  va_list *Arguments
		)
	{
		LPSTR lpBuffer2 = lpBuffer;
		if (dwFlags & FORMAT_MESSAGE_ALLOCATE_BUFFER) {
			dwFlags &= ~FORMAT_MESSAGE_ALLOCATE_BUFFER;
			lpBuffer2 = *(LPSTR *)lpBuffer = (LPSTR)new char [512];
			nSize = 512;
		}
		return ::FormatMessageA(
			dwFlags, 
			lpSource, 
			dwMessageId, 
			dwLanguageId, 
			lpBuffer2, 
			nSize, 
			Arguments);
	}

	DWORD WINAPI GetTickCount(void)
	{
		return (DWORD)GetTickCount64();
	}

	void WINAPI GetSystemInfo(
		_Out_  LPSYSTEM_INFO lpSystemInfo
		)
	{
		GetNativeSystemInfo(lpSystemInfo);
	}

	LPCH WINAPI GetEnvironmentStringsA(void)
	{
		return "";
	}

	DWORD WINAPI GetEnvironmentVariableA(
		_In_opt_   LPCSTR lpName,
		_Out_opt_  LPSTR lpBuffer,
		_In_       DWORD nSize
		)
	{
		//assert(0);
		lpBuffer[0] = '0';
		return 0;
	}

	BOOL WINAPI SetEnvironmentVariableA(
		_In_      LPCSTR lpName,
		_In_opt_  LPCSTR lpValue
		)
	{
		assert(0);
		return FALSE;
	}

	LCID GetUserDefaultLCID(void)
	{
		return LOCALE_USER_DEFAULT;
	}

	int LCMapStringA(
		_In_       LCID Locale,
		_In_       DWORD dwMapFlags,
		_In_       LPCSTR lpSrcStr,
		_In_       int cchSrc,
		_Out_opt_  LPSTR lpDestStr,
		_In_       int cchDest
		)
	{
		int cchWideChar = cchSrc;
		LPWSTR lpWideCharStr = (LPWSTR)new WCHAR[cchWideChar];
		if ((cchWideChar = ::MultiByteToWideChar(CP_ACP, 0, lpSrcStr, cchSrc, lpWideCharStr, cchWideChar)) == 0) {
			delete [] lpWideCharStr;
			return 0;
		}
		LPWSTR lpWideCharStr2 = (LPWSTR)new WCHAR[cchWideChar];
		int cchWideChar2 = LCMapStringEx(
			LOCALE_NAME_SYSTEM_DEFAULT, 
			dwMapFlags, 
			lpWideCharStr, 
			cchWideChar, 
			lpWideCharStr2, 
			cchWideChar, 
			NULL, 
			NULL, 
			NULL);
		delete [] lpWideCharStr;
		if (cchWideChar2 == 0) {
			delete [] lpWideCharStr2;
			return 0;
		}
		cchDest = ::WideCharToMultiByte(CP_ACP, 0, lpWideCharStr2, cchWideChar2, lpDestStr, cchDest, 0, FALSE);
		return cchDest;
	}

	int LCMapStringW(
		_In_       LCID Locale,
		_In_       DWORD dwMapFlags,
		_In_       LPCWSTR lpSrcStr,
		_In_       int cchSrc,
		_Out_opt_  LPWSTR lpDestStr,
		_In_       int cchDest
		)
	{
		return LCMapStringEx(
			LOCALE_NAME_SYSTEM_DEFAULT, 
			dwMapFlags, 
			lpSrcStr, 
			cchSrc, 
			lpDestStr, 
			cchDest, 
			NULL, 
			NULL, 
			NULL);
	}

	BOOL GetStringTypeExA(
		_In_   LCID Locale,
		_In_   DWORD dwInfoType,
		_In_   LPCSTR lpSrcStr,
		_In_   int cchSrc,
		_Out_  LPWORD lpCharType
		)
	{
		int cchWideChar = cchSrc;
		LPWSTR lpWideCharStr = (LPWSTR)new WCHAR[cchWideChar];
		if ((cchWideChar = ::MultiByteToWideChar(CP_ACP, 0, lpSrcStr, cchSrc, lpWideCharStr, cchWideChar)) == 0) {
			delete [] lpWideCharStr;
			return FALSE;
		}
		BOOL b = GetStringTypeExW(
			Locale, 
			dwInfoType, 
			lpWideCharStr, 
			cchWideChar, 
			lpCharType);
		delete [] lpWideCharStr;;
		return b;
	}

	int WINAPI LoadStringA(
		_In_opt_  HINSTANCE hInstance,
		_In_      UINT uID,
		_Out_     LPSTR lpBuffer,
		_In_      int nBufferMax
		)
	{
		assert(false);
		return 0;
	}

	int WINAPI LoadStringW(
		_In_opt_  HINSTANCE hInstance,
		_In_      UINT uID,
		_Out_     LPWSTR lpBuffer,
		_In_      int nBufferMax
		)
	{
		assert(false);
		return 0;
	}

	LPVOID WINAPI VirtualAlloc(
		_In_opt_  LPVOID lpAddress,
		_In_      SIZE_T dwSize,
		_In_      DWORD flAllocationType,
		_In_      DWORD flProtect
		)
	{
		assert(lpAddress == NULL);
		assert(flAllocationType == MEM_COMMIT);
		HANDLE hFileMapping = CreateFileMappingFromApp(
			INVALID_HANDLE_VALUE, 
			NULL, 
			flProtect, 
			dwSize, 
			NULL);
		if (hFileMapping == NULL)
			return NULL;
		PVOID addr = MapViewOfFileFromApp(
			hFileMapping, 
			FILE_MAP_ALL_ACCESS, 
			0, 
			dwSize);
		CloseHandle(hFileMapping);
		return addr;
	}

	BOOL WINAPI VirtualFree(
		_In_  LPVOID lpAddress,
		_In_  SIZE_T dwSize,
		_In_  DWORD dwFreeType
		)
	{
		assert(dwSize == 0);
		assert(dwFreeType == MEM_RELEASE);
		return UnmapViewOfFile(
			lpAddress);
	}

	HANDLE WINAPI GetStdHandle(
		_In_  DWORD nStdHandle
		)
	{
		switch (STD_OUTPUT_HANDLE)
		{
		case STD_INPUT_HANDLE:
			return (HANDLE)_get_osfhandle(_fileno(stdin));
		case STD_OUTPUT_HANDLE:
			return (HANDLE)_get_osfhandle(_fileno(stdout));
		case STD_ERROR_HANDLE:
			return (HANDLE)_get_osfhandle(_fileno(stderr));
		default:
			assert(false);
			return NULL;
		}
	}

	BOOL WINAPI SetConsoleTextAttribute(
		_In_  HANDLE hConsoleOutput,
		_In_  WORD wAttributes
		)
	{
		return FALSE;
	}

	char * environ[] = {0};
}
