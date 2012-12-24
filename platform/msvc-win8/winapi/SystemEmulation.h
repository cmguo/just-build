// SystemEmulation.h

#pragma once

#define FORMAT_MESSAGE_ALLOCATE_BUFFER	0x00000100

namespace SystemEmulation
{

	HLOCAL WINAPI_DECL LocalAlloc(
		_In_  UINT uFlags,
		_In_  SIZE_T uBytes
		);

	HLOCAL WINAPI_DECL LocalFree(
		_In_  HLOCAL hMem
		);

	DWORD WINAPI_DECL FormatMessage2A(
		_In_      DWORD dwFlags,
		_In_opt_  LPCVOID lpSource,
		_In_      DWORD dwMessageId,
		_In_      DWORD dwLanguageId,
		_Out_     LPSTR lpBuffer,
		_In_      DWORD nSize,
		_In_opt_  va_list *Arguments
		);

#define FormatMessageA FormatMessage2A

	void WINAPI_DECL OutputDebugString2A(
		_In_opt_  LPCSTR lpOutputString
		);

#define OutputDebugStringA OutputDebugString2A

	DWORD WINAPI_DECL GetTickCount(void);

	void WINAPI_DECL GetSystemInfo(
		_Out_  LPSYSTEM_INFO lpSystemInfo
		);

	LPCH WINAPI_DECL GetEnvironmentStringsA(void);

#define GetEnvironmentStrings GetEnvironmentStringsA

	DWORD WINAPI_DECL GetEnvironmentVariableA(
		_In_opt_   LPCSTR lpName,
		_Out_opt_  LPSTR lpBuffer,
		_In_       DWORD nSize
		);

#define GetEnvironmentVariable GetEnvironmentVariableA

	BOOL WINAPI_DECL SetEnvironmentVariableA(
		_In_      LPCSTR lpName,
		_In_opt_  LPCSTR lpValue
		);

#define SetEnvironmentVariable SetEnvironmentVariableA

	LCID WINAPI_DECL GetUserDefaultLCID(void);

	int WINAPI_DECL LCMapStringA(
		_In_       LCID Locale,
		_In_       DWORD dwMapFlags,
		_In_       LPCSTR lpSrcStr,
		_In_       int cchSrc,
		_Out_opt_  LPSTR lpDestStr,
		_In_       int cchDest
		);

	int WINAPI_DECL LCMapStringW(
		_In_       LCID Locale,
		_In_       DWORD dwMapFlags,
		_In_       LPCWSTR lpSrcStr,
		_In_       int cchSrc,
		_Out_opt_  LPWSTR lpDestStr,
		_In_       int cchDest
		);

	BOOL WINAPI_DECL GetStringTypeExA(
		_In_   LCID Locale,
		_In_   DWORD dwInfoType,
		_In_   LPCSTR lpSrcStr,
		_In_   int cchSrc,
		_Out_  LPWORD lpCharType
		);

	int WINAPI_DECL LoadStringA(
		_In_opt_  HINSTANCE hInstance,
		_In_      UINT uID,
		_Out_     LPSTR lpBuffer,
		_In_      int nBufferMax
		);

	int WINAPI_DECL LoadStringW(
		_In_opt_  HINSTANCE hInstance,
		_In_      UINT uID,
		_Out_     LPWSTR lpBuffer,
		_In_      int nBufferMax
		);

	LPVOID WINAPI_DECL VirtualAlloc(
		_In_opt_  LPVOID lpAddress,
		_In_      SIZE_T dwSize,
		_In_      DWORD flAllocationType,
		_In_      DWORD flProtect
		);

	BOOL WINAPI_DECL VirtualFree(
		_In_  LPVOID lpAddress,
		_In_  SIZE_T dwSize,
		_In_  DWORD dwFreeType
		);

	HANDLE WINAPI_DECL GetStdHandle(
		_In_  DWORD nStdHandle
		);

	BOOL WINAPI_DECL SetConsoleTextAttribute(
		_In_  HANDLE hConsoleOutput,
		_In_  WORD wAttributes
		);

}
