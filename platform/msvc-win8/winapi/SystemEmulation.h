// SystemEmulation.h

#pragma once

#define FORMAT_MESSAGE_ALLOCATE_BUFFER	0x00000100

namespace SystemEmulation
{

	HLOCAL WINAPI LocalAlloc(
		_In_  UINT uFlags,
		_In_  SIZE_T uBytes
		);

	HLOCAL WINAPI LocalFree(
		_In_  HLOCAL hMem
		);

	DWORD WINAPI FormatMessage2A(
		_In_      DWORD dwFlags,
		_In_opt_  LPCVOID lpSource,
		_In_      DWORD dwMessageId,
		_In_      DWORD dwLanguageId,
		_Out_     LPSTR lpBuffer,
		_In_      DWORD nSize,
		_In_opt_  va_list *Arguments
		);

#define FormatMessageA FormatMessage2A

	void WINAPI OutputDebugString2A(
		_In_opt_  LPCSTR lpOutputString
		);

#define OutputDebugStringA OutputDebugString2A

	DWORD WINAPI GetTickCount(void);

	void WINAPI GetSystemInfo(
		_Out_  LPSYSTEM_INFO lpSystemInfo
		);

	LPCH WINAPI GetEnvironmentStringsA(void);

#define GetEnvironmentStrings GetEnvironmentStringsA

	DWORD WINAPI GetEnvironmentVariableA(
		_In_opt_   LPCSTR lpName,
		_Out_opt_  LPSTR lpBuffer,
		_In_       DWORD nSize
		);

#define GetEnvironmentVariable GetEnvironmentVariableA

	BOOL WINAPI SetEnvironmentVariableA(
		_In_      LPCSTR lpName,
		_In_opt_  LPCSTR lpValue
		);

#define SetEnvironmentVariable SetEnvironmentVariableA

	LCID GetUserDefaultLCID(void);

	int LCMapStringA(
		_In_       LCID Locale,
		_In_       DWORD dwMapFlags,
		_In_       LPCSTR lpSrcStr,
		_In_       int cchSrc,
		_Out_opt_  LPSTR lpDestStr,
		_In_       int cchDest
		);

	int LCMapStringW(
		_In_       LCID Locale,
		_In_       DWORD dwMapFlags,
		_In_       LPCWSTR lpSrcStr,
		_In_       int cchSrc,
		_Out_opt_  LPWSTR lpDestStr,
		_In_       int cchDest
		);

	BOOL GetStringTypeExA(
		_In_   LCID Locale,
		_In_   DWORD dwInfoType,
		_In_   LPCSTR lpSrcStr,
		_In_   int cchSrc,
		_Out_  LPWORD lpCharType
		);

	int WINAPI LoadStringA(
		_In_opt_  HINSTANCE hInstance,
		_In_      UINT uID,
		_Out_     LPSTR lpBuffer,
		_In_      int nBufferMax
		);

	int WINAPI LoadStringW(
		_In_opt_  HINSTANCE hInstance,
		_In_      UINT uID,
		_Out_     LPWSTR lpBuffer,
		_In_      int nBufferMax
		);

	LPVOID WINAPI VirtualAlloc(
		_In_opt_  LPVOID lpAddress,
		_In_      SIZE_T dwSize,
		_In_      DWORD flAllocationType,
		_In_      DWORD flProtect
		);

	BOOL WINAPI VirtualFree(
		_In_  LPVOID lpAddress,
		_In_  SIZE_T dwSize,
		_In_  DWORD dwFreeType
		);

	HANDLE WINAPI GetStdHandle(
		_In_  DWORD nStdHandle
		);

	BOOL WINAPI SetConsoleTextAttribute(
		_In_  HANDLE hConsoleOutput,
		_In_  WORD wAttributes
		);

}
