// FileSystemEmulation.h

#pragma once

#include <windows.h>

namespace FileSystemEmulation
{

	BOOL WINAPI CreateDirectory2A(
		_In_      LPCSTR lpPathName,
		_In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes
		);

#define CreateDirectoryA CreateDirectory2A

	BOOL WINAPI RemoveDirectory2A(
		_In_  LPCSTR lpPathName
		);

#define RemoveDirectoryA RemoveDirectory2A


	DWORD WINAPI GetFileAttributesA(
		_In_  LPCSTR lpFileName
		);

	DWORD WINAPI GetFileAttributesW(
		_In_  LPCWSTR lpFileName
		);

	HANDLE WINAPI CreateFileA(
		_In_      LPCSTR lpFileName,
		_In_      DWORD dwDesiredAccess,
		_In_      DWORD dwShareMode,
		_In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_      DWORD dwCreationDisposition,
		_In_      DWORD dwFlagsAndAttributes,
		_In_opt_  HANDLE hTemplateFile
		);

	HANDLE WINAPI CreateFileW(
		_In_      LPCWSTR lpFileName,
		_In_      DWORD dwDesiredAccess,
		_In_      DWORD dwShareMode,
		_In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_      DWORD dwCreationDisposition,
		_In_      DWORD dwFlagsAndAttributes,
		_In_opt_  HANDLE hTemplateFile
		);

	BOOL WINAPI DeleteFile2A(
		_In_  LPCSTR lpFileName
		);

#define DeleteFileA DeleteFile2A

	BOOL WINAPI MoveFileEx2A(
		_In_      LPCSTR lpExistingFileName,
		_In_opt_  LPCSTR lpNewFileName,
		_In_      DWORD dwFlags
		);

#define MoveFileExA MoveFileEx2A

	DWORD WINAPI SetFilePointer(
		_In_         HANDLE hFile,
		_In_         LONG lDistanceToMove,
		_Inout_opt_  PLONG lpDistanceToMoveHigh,
		_In_         DWORD dwMoveMethod
		);

	DWORD WINAPI GetFileSize(
		_In_       HANDLE hFile,
		_Out_opt_  LPDWORD lpFileSizeHigh
		);

	BOOL WINAPI GetFileSizeEx(
		_In_   HANDLE hFile,
		_Out_  PLARGE_INTEGER lpFileSize
		);

	BOOL WINAPI FileTimeToLocalFileTime(
		_In_   const FILETIME *lpFileTime,
		_Out_  LPFILETIME lpLocalFileTime
		);

	HANDLE WINAPI CreateFileMappingA(
		_In_      HANDLE hFile,
		_In_opt_  LPSECURITY_ATTRIBUTES lpAttributes,
		_In_      DWORD flProtect,
		_In_      DWORD dwMaximumSizeHigh,
		_In_      DWORD dwMaximumSizeLow,
		_In_opt_  LPCSTR lpName
		);

	HANDLE WINAPI OpenFileMappingA(
		_In_ DWORD dwDesiredAccess,
		_In_ BOOL bInheritHandle,
		_In_ LPCSTR lpName
		);

	LPVOID WINAPI MapViewOfFile(
		_In_  HANDLE hFileMappingObject,
		_In_  DWORD dwDesiredAccess,
		_In_  DWORD dwFileOffsetHigh,
		_In_  DWORD dwFileOffsetLow,
		_In_  SIZE_T dwNumberOfBytesToMap
		);

	LPVOID WINAPI MapViewOfFileEx(
		_In_      HANDLE hFileMappingObject,
		_In_      DWORD dwDesiredAccess,
		_In_      DWORD dwFileOffsetHigh,
		_In_      DWORD dwFileOffsetLow,
		_In_      SIZE_T dwNumberOfBytesToMap,
		_In_opt_  LPVOID lpBaseAddress
		);

	DWORD WINAPI GetCurrentDirectoryA(
		_In_   DWORD nBufferLength,
		_Out_  LPSTR lpBuffer
		);

	DWORD WINAPI GetCurrentDirectoryW(
		_In_   DWORD nBufferLength,
		_Out_  LPWSTR lpBuffer
		);

	BOOL WINAPI SetCurrentDirectoryA(
		_In_  LPCSTR lpPathName
		);

	BOOL WINAPI SetCurrentDirectoryW(
		_In_  LPCWSTR lpPathName
		);

	BOOL WINAPI CreateHardLinkA(
		_In_        LPCSTR lpFileName,
		_In_        LPCSTR lpExistingFileName,
		_Reserved_  LPSECURITY_ATTRIBUTES lpSecurityAttributes
		);

	BOOL WINAPI CreateHardLinkW(
		_In_        LPCWSTR lpFileName,
		_In_        LPCWSTR lpExistingFileName,
		_Reserved_  LPSECURITY_ATTRIBUTES lpSecurityAttributes
		);

	DWORD WINAPI GetFullPathNameA(
		_In_   LPCSTR lpFileName,
		_In_   DWORD nBufferLength,
		_Out_  LPSTR lpBuffer,
		_Out_  LPSTR *lpFilePart
		);

	DWORD WINAPI GetFullPathNameW(
		_In_   LPCWSTR lpFileName,
		_In_   DWORD nBufferLength,
		_Out_  LPWSTR lpBuffer,
		_Out_  LPWSTR *lpFilePart
		);

	DWORD WINAPI GetShortPathNameA(
		_In_   LPCSTR lpszLongPath,
		_Out_  LPSTR lpszShortPath,
		_In_   DWORD cchBuffer
		);

	DWORD WINAPI GetShortPathNameW(
		_In_   LPCWSTR lpszLongPath,
		_Out_  LPWSTR lpszShortPath,
		_In_   DWORD cchBuffer
		);

	BOOL WINAPI MoveFileA(
		_In_  LPCSTR lpExistingFileName,
		_In_  LPCSTR lpNewFileName
		);

	BOOL WINAPI MoveFileW(
		_In_  LPCWSTR lpExistingFileName,
		_In_  LPCWSTR lpNewFileName
		);

	BOOL WINAPI CopyFileA(
		_In_  LPCSTR lpExistingFileName,
		_In_  LPCSTR lpNewFileName,
		_In_  BOOL bFailIfExists
		);

	BOOL WINAPI CopyFileW(
		_In_  LPCWSTR lpExistingFileName,
		_In_  LPCWSTR lpNewFileName,
		_In_  BOOL bFailIfExists
		);

	HANDLE WINAPI FindFirstFileA(
		_In_   LPCSTR lpFileName,
		_Out_  LPWIN32_FIND_DATAA lpFindFileData
		);

	HANDLE WINAPI FindFirstFileW(
		_In_   LPCWSTR lpFileName,
		_Out_  LPWIN32_FIND_DATAW lpFindFileData
		);

	typedef struct _BY_HANDLE_FILE_INFORMATION {
		DWORD    dwFileAttributes;
		FILETIME ftCreationTime;
		FILETIME ftLastAccessTime;
		FILETIME ftLastWriteTime;
		DWORD    dwVolumeSerialNumber;
		DWORD    nFileSizeHigh;
		DWORD    nFileSizeLow;
		DWORD    nNumberOfLinks;
		DWORD    nFileIndexHigh;
		DWORD    nFileIndexLow;
	} BY_HANDLE_FILE_INFORMATION, *LPBY_HANDLE_FILE_INFORMATION;

	BOOL WINAPI GetFileInformationByHandle(
		_In_   HANDLE hFile,
		_Out_  LPBY_HANDLE_FILE_INFORMATION lpFileInformation
		);

	BOOL WINAPI GetFileTime(
		_In_       HANDLE hFile,
		_Out_opt_  LPFILETIME lpCreationTime,
		_Out_opt_  LPFILETIME lpLastAccessTime,
		_Out_opt_  LPFILETIME lpLastWriteTime
		);

	BOOL WINAPI SetFileTime(
		_In_      HANDLE hFile,
		_In_opt_  const FILETIME *lpCreationTime,
		_In_opt_  const FILETIME *lpLastAccessTime,
		_In_opt_  const FILETIME *lpLastWriteTime
		);

	HMODULE WINAPI GetModuleHandleA(
		_In_opt_  LPCSTR lpModuleName
		);

	HMODULE WINAPI LoadLibraryA(
		_In_  LPCSTR lpFileName
		);

	DWORD WINAPI GetModuleFileNameA(
	  _In_opt_  HMODULE hModule,
	  _Out_     LPSTR lpFilename,
	  _In_      DWORD nSize
	);

	DWORD WINAPI GetModuleFileNameW(
	  _In_opt_  HMODULE hModule,
	  _Out_     LPWSTR lpFilename,
	  _In_      DWORD nSize
	);

	DWORD WINAPI GetTempPathA(
		_In_   DWORD nBufferLength,
		_Out_  LPSTR lpBuffer
		);

	DWORD WINAPI GetTempPathW(
		_In_   DWORD nBufferLength,
		_Out_  LPWSTR lpBuffer
		);

}
