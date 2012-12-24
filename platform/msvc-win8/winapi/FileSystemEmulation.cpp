// FileSystemEmulation.cpp

#include <Windows.h>

#define WINAPI_DECL	 __declspec(dllexport)

#include "FileSystemEmulation.h"

#include <memoryapi.h>

#include <assert.h>

namespace FileSystemEmulation
{

	BOOL WINAPI_DECL CreateDirectory2A(
		_In_      LPCSTR lpPathName,
		_In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes
		)
	{
		int cchWideChar = strlen(lpPathName) + 1;
		LPWSTR lpWideCharStr = (LPWSTR)new WCHAR[cchWideChar];
		if (::MultiByteToWideChar(CP_ACP, 0, lpPathName, -1, lpWideCharStr, cchWideChar) == 0) {
			delete [] lpWideCharStr;
			return FALSE;
		}
		BOOL b = CreateDirectoryW(
			lpWideCharStr, 
			lpSecurityAttributes);
		delete [] lpWideCharStr;
		return b;
	}

	BOOL WINAPI_DECL RemoveDirectory2A(
		_In_  LPCSTR lpPathName
		)
	{
		int cchWideChar = strlen(lpPathName) + 1;
		LPWSTR lpWideCharStr = (LPWSTR)new WCHAR[cchWideChar];
		if (::MultiByteToWideChar(CP_ACP, 0, lpPathName, -1, lpWideCharStr, cchWideChar) == 0) {
			delete [] lpWideCharStr;
			return FALSE;
		}
		BOOL b = RemoveDirectoryW(
			lpWideCharStr);
		delete [] lpWideCharStr;
		return b;
	}

	DWORD WINAPI_DECL GetFileAttributesA(
		_In_  LPCSTR lpFileName
		)
	{
		int cchWideChar = strlen(lpFileName) + 1;
		LPWSTR lpWideCharStr = (LPWSTR)new WCHAR[cchWideChar];
		if (::MultiByteToWideChar(CP_ACP, 0, lpFileName, -1, lpWideCharStr, cchWideChar) == 0) {
			delete [] lpWideCharStr;
			return INVALID_FILE_ATTRIBUTES;
		}
		WIN32_FILE_ATTRIBUTE_DATA fileInformation;
		BOOL b = GetFileAttributesExW(
			lpWideCharStr, 
			GetFileExInfoStandard, 
			&fileInformation);
		delete [] lpWideCharStr;
		return b ? fileInformation.dwFileAttributes : INVALID_FILE_ATTRIBUTES;
	}

	DWORD WINAPI_DECL GetFileAttributesW(
		_In_  LPCWSTR lpFileName
		)
	{
		WIN32_FILE_ATTRIBUTE_DATA fileInformation;
		BOOL b = GetFileAttributesExW(
			lpFileName, 
			GetFileExInfoStandard, 
			&fileInformation);
		return b ? fileInformation.dwFileAttributes : INVALID_FILE_ATTRIBUTES;
	}

	HANDLE WINAPI_DECL CreateFileA(
		_In_      LPCSTR lpFileName,
		_In_      DWORD dwDesiredAccess,
		_In_      DWORD dwShareMode,
		_In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_      DWORD dwCreationDisposition,
		_In_      DWORD dwFlagsAndAttributes,
		_In_opt_  HANDLE hTemplateFile
		)
	{
		int cchWideChar = strlen(lpFileName) + 1;
		LPWSTR lpWideCharStr = (LPWSTR)new WCHAR[cchWideChar];
		if (::MultiByteToWideChar(CP_ACP, 0, lpFileName, -1, lpWideCharStr, cchWideChar) == 0) {
			delete [] lpWideCharStr;
			return INVALID_HANDLE_VALUE;
		}
		HANDLE hFile = CreateFileW(
			lpWideCharStr, 
			dwDesiredAccess, 
			dwShareMode, 
			lpSecurityAttributes, 
			dwCreationDisposition, 
			dwFlagsAndAttributes, 
			hTemplateFile);
		delete [] lpWideCharStr;
		return hFile;
	}

	HANDLE WINAPI_DECL CreateFileW(
		_In_      LPCWSTR lpFileName,
		_In_      DWORD dwDesiredAccess,
		_In_      DWORD dwShareMode,
		_In_opt_  LPSECURITY_ATTRIBUTES lpSecurityAttributes,
		_In_      DWORD dwCreationDisposition,
		_In_      DWORD dwFlagsAndAttributes,
		_In_opt_  HANDLE hTemplateFile
		)
	{
		CREATEFILE2_EXTENDED_PARAMETERS createExParams;
		createExParams.dwSize = sizeof(createExParams);
		createExParams.dwFileAttributes = dwFlagsAndAttributes;
		createExParams.dwFileFlags = 0;
		createExParams.dwSecurityQosFlags = 0;
		createExParams.lpSecurityAttributes = lpSecurityAttributes;
		createExParams.hTemplateFile = hTemplateFile;
		HANDLE hFile = CreateFile2(
			lpFileName, 
			dwDesiredAccess, 
			dwShareMode, 
			dwCreationDisposition, 
			&createExParams);
		return hFile;
	}

	BOOL WINAPI_DECL DeleteFile2A(
		_In_  LPCSTR lpFileName
		)
	{
		int cchWideChar = strlen(lpFileName) + 1;
		LPWSTR lpWideCharStr = (LPWSTR)new WCHAR[cchWideChar];
		if (::MultiByteToWideChar(CP_ACP, 0, lpFileName, -1, lpWideCharStr, cchWideChar) == 0) {
			delete [] lpWideCharStr;
			return FALSE;
		}
		BOOL b = DeleteFileW(
			lpWideCharStr);
		delete [] lpWideCharStr;
		return b;
	}

	BOOL WINAPI_DECL MoveFileEx2A(
		_In_      LPCSTR lpExistingFileName,
		_In_opt_  LPCSTR lpNewFileName,
		_In_      DWORD dwFlags
		)
	{
		int cchWideChar1 = strlen(lpExistingFileName) + 1;
		int cchWideChar2 = strlen(lpNewFileName) + 1;
		LPWSTR lpWideCharStr1 = (LPWSTR)new WCHAR[cchWideChar1];
		LPWSTR lpWideCharStr2 = (LPWSTR)new WCHAR[cchWideChar2];
		if (::MultiByteToWideChar(CP_ACP, 0, lpExistingFileName, -1, lpWideCharStr1, cchWideChar1) == 0) {
			delete [] lpWideCharStr1;
			delete [] lpWideCharStr2;
			return FALSE;
		}
		if (::MultiByteToWideChar(CP_ACP, 0, lpNewFileName, -1, lpWideCharStr2, cchWideChar2) == 0) {
			delete [] lpWideCharStr1;
			delete [] lpWideCharStr2;
			return FALSE;
		}
		BOOL b = MoveFileExW(
			lpWideCharStr1, 
			lpWideCharStr2, 
			dwFlags);
		delete [] lpWideCharStr1;
		delete [] lpWideCharStr2;
		return b;
	}

	DWORD WINAPI_DECL SetFilePointer(
		_In_         HANDLE hFile,
		_In_         LONG lDistanceToMove,
		_Inout_opt_  PLONG lpDistanceToMoveHigh,
		_In_         DWORD dwMoveMethod
		)
	{
		LARGE_INTEGER liDistanceToMove;
		LARGE_INTEGER liNewFilePointer;
		liDistanceToMove.LowPart = lDistanceToMove;
		if (lpDistanceToMoveHigh)
			liDistanceToMove.HighPart = *lpDistanceToMoveHigh;
		else
			liDistanceToMove.HighPart = 0;
		BOOL b = SetFilePointerEx(
			hFile, 
			liDistanceToMove, 
			&liNewFilePointer, 
			dwMoveMethod);
		if (b == FALSE) {
			return  INVALID_SET_FILE_POINTER;
		}
		if (lpDistanceToMoveHigh)
			*lpDistanceToMoveHigh = liDistanceToMove.HighPart;
		return liDistanceToMove.LowPart;
	}

	DWORD WINAPI_DECL GetFileSize(
		_In_       HANDLE hFile,
		_Out_opt_  LPDWORD lpFileSizeHigh
		)
	{
		FILE_STANDARD_INFO standardInfo;
		BOOL b = GetFileInformationByHandleEx(
			hFile, 
			FileStandardInfo, 
			&standardInfo, 
			sizeof(standardInfo));
		if (b == FALSE)
			return INVALID_FILE_SIZE;
		return standardInfo.EndOfFile.LowPart;	
	}

	BOOL WINAPI_DECL GetFileSizeEx(
		_In_   HANDLE hFile,
		_Out_  PLARGE_INTEGER lpFileSize
		)
	{
		FILE_STANDARD_INFO standardInfo;
		BOOL b = GetFileInformationByHandleEx(
			hFile, 
			FileStandardInfo, 
			&standardInfo, 
			sizeof(standardInfo));
		if (b == FALSE)
			return FALSE;
		*lpFileSize = standardInfo.EndOfFile;
		return TRUE;
	}

	BOOL WINAPI_DECL FileTimeToLocalFileTime(
		_In_   const FILETIME *lpFileTime,
		_Out_  LPFILETIME lpLocalFileTime
		)
	{
		SYSTEMTIME SystemTime;
		SYSTEMTIME LocalSystemTime;
		if (FALSE == FileTimeToSystemTime(
			lpFileTime, 
			&SystemTime))
			return FALSE;
		if (FALSE == SystemTimeToTzSpecificLocalTime(
			NULL, 
			&SystemTime, 
			&LocalSystemTime))
			return FALSE;
		if (FALSE == SystemTimeToFileTime(
			&LocalSystemTime, 
			lpLocalFileTime))
			return FALSE;
		return TRUE;
	}

	HANDLE WINAPI_DECL CreateFileMappingA(
		_In_      HANDLE hFile,
		_In_opt_  LPSECURITY_ATTRIBUTES lpAttributes,
		_In_      DWORD flProtect,
		_In_      DWORD dwMaximumSizeHigh,
		_In_      DWORD dwMaximumSizeLow,
		_In_opt_  LPCSTR lpName
		)
	{
		LPWSTR lpWideCharStr = NULL;
		if (lpName) {
			int cchWideChar = strlen(lpName) + 1;
			lpWideCharStr = (LPWSTR)new WCHAR[cchWideChar];
			if (::MultiByteToWideChar(CP_ACP, 0, lpName, -1, lpWideCharStr, cchWideChar) == 0) {
				delete [] lpWideCharStr;
				return NULL;
			}
		}
		ULONG64 ulMaximumSize = (ULONG64)dwMaximumSizeHigh << 32 | dwMaximumSizeLow;
		HANDLE hFileMapping = CreateFileMappingFromApp(
			hFile, 
			lpAttributes, 
			flProtect, 
			ulMaximumSize , 
			lpWideCharStr);
		if (lpWideCharStr)
			delete [] lpWideCharStr;
		return hFileMapping;
	}

	HANDLE WINAPI_DECL OpenFileMappingA(
		_In_ DWORD dwDesiredAccess,
		_In_ BOOL bInheritHandle,
		_In_ LPCSTR lpName
		)
	{
		HANDLE hFileMapping = CreateFileMappingA(
			INVALID_HANDLE_VALUE, 
			NULL, 
			dwDesiredAccess, 
			0, 
			0, 
			lpName);
		if (hFileMapping != NULL && GetLastError() == ERROR_ALREADY_EXISTS) {
			SetLastError(0);
		}
		return hFileMapping;
	}

	LPVOID WINAPI_DECL MapViewOfFile(
		_In_  HANDLE hFileMappingObject,
		_In_  DWORD dwDesiredAccess,
		_In_  DWORD dwFileOffsetHigh,
		_In_  DWORD dwFileOffsetLow,
		_In_  SIZE_T dwNumberOfBytesToMap
		)
	{
		ULONG64 ulFileOffset = (ULONG64)dwFileOffsetHigh << 32 | dwFileOffsetLow;
		return MapViewOfFileFromApp(
			hFileMappingObject, 
			dwDesiredAccess, 
			ulFileOffset, 
			dwNumberOfBytesToMap);
	}

	LPVOID WINAPI_DECL MapViewOfFileEx(
		_In_      HANDLE hFileMappingObject,
		_In_      DWORD dwDesiredAccess,
		_In_      DWORD dwFileOffsetHigh,
		_In_      DWORD dwFileOffsetLow,
		_In_      SIZE_T dwNumberOfBytesToMap,
		_In_opt_  LPVOID lpBaseAddress
		)
	{
		assert(lpBaseAddress == NULL);
		ULONG64 ulFileOffset = (ULONG64)dwFileOffsetHigh << 32 | dwFileOffsetLow;
		return MapViewOfFileFromApp(
			hFileMappingObject, 
			dwDesiredAccess, 
			ulFileOffset, 
			dwNumberOfBytesToMap);
	}

	DWORD WINAPI_DECL GetCurrentDirectoryA(
		_In_   DWORD nBufferLength,
		_Out_  LPSTR lpBuffer
		)
	{
		return 0;
	}

	DWORD WINAPI_DECL GetCurrentDirectoryW(
		_In_   DWORD nBufferLength,
		_Out_  LPWSTR lpBuffer
		)
	{
		assert(false);
		return 0;
	}

	BOOL WINAPI_DECL SetCurrentDirectoryA(
		_In_  LPCSTR lpPathName
		)
	{
		assert(false);
		return FALSE;
	}

	BOOL WINAPI_DECL SetCurrentDirectoryW(
		_In_  LPCWSTR lpPathName
		)
	{
		assert(false);
		return FALSE;
	}

	BOOL WINAPI_DECL CreateHardLinkA(
		_In_        LPCSTR lpFileName,
		_In_        LPCSTR lpExistingFileName,
		_Reserved_  LPSECURITY_ATTRIBUTES lpSecurityAttributes
		)
	{
		assert(false);
		return FALSE;
	}

	BOOL WINAPI_DECL CreateHardLinkW(
		_In_        LPCWSTR lpFileName,
		_In_        LPCWSTR lpExistingFileName,
		_Reserved_  LPSECURITY_ATTRIBUTES lpSecurityAttributes
		)
	{
		assert(false);
		return FALSE;
	}

	DWORD WINAPI_DECL GetFullPathNameA(
		_In_   LPCSTR lpFileName,
		_In_   DWORD nBufferLength,
		_Out_  LPSTR lpBuffer,
		_Out_  LPSTR *lpFilePart
		)
	{
		assert(false);
		return 0;
	}

	DWORD WINAPI_DECL GetFullPathNameW(
		_In_   LPCWSTR lpFileName,
		_In_   DWORD nBufferLength,
		_Out_  LPWSTR lpBuffer,
		_Out_  LPWSTR *lpFilePart
		)
	{
		assert(false);
		return 0;
	}

	DWORD WINAPI_DECL GetShortPathNameA(
		_In_   LPCSTR lpszLongPath,
		_Out_  LPSTR lpszShortPath,
		_In_   DWORD cchBuffer
		)
	{
		assert(false);
		return 0;
	}

	DWORD WINAPI_DECL GetShortPathNameW(
		_In_   LPCWSTR lpszLongPath,
		_Out_  LPWSTR lpszShortPath,
		_In_   DWORD cchBuffer
		)
	{
		assert(false);
		return 0;
	}

	BOOL WINAPI_DECL MoveFileA(
		_In_  LPCSTR lpExistingFileName,
		_In_  LPCSTR lpNewFileName
		)
	{
		return MoveFileExA(
			lpExistingFileName, 
			lpNewFileName, 
			0);
	}

	BOOL WINAPI_DECL MoveFileW(
		_In_  LPCWSTR lpExistingFileName,
		_In_  LPCWSTR lpNewFileName
		)
	{
		return MoveFileExW(
			lpExistingFileName, 
			lpNewFileName, 
			0);
	}

	BOOL WINAPI_DECL CopyFileA(
		_In_  LPCSTR lpExistingFileName,
		_In_  LPCSTR lpNewFileName,
		_In_  BOOL bFailIfExists
		)
	{
		int cchWideChar1 = strlen(lpExistingFileName) + 1;
		int cchWideChar2 = strlen(lpNewFileName) + 1;
		LPWSTR lpWideCharStr1 = (LPWSTR)new WCHAR[cchWideChar1];
		LPWSTR lpWideCharStr2 = (LPWSTR)new WCHAR[cchWideChar2];
		if (::MultiByteToWideChar(CP_ACP, 0, lpExistingFileName, -1, lpWideCharStr1, cchWideChar1) == 0) {
			delete [] lpWideCharStr1;
			delete [] lpWideCharStr2;
			return FALSE;
		}
		if (::MultiByteToWideChar(CP_ACP, 0, lpNewFileName, -1, lpWideCharStr2, cchWideChar2) == 0) {
			delete [] lpWideCharStr1;
			delete [] lpWideCharStr2;
			return FALSE;
		}
		BOOL b = CopyFileW(
			lpWideCharStr1, 
			lpWideCharStr2, 
			bFailIfExists);
		delete [] lpWideCharStr1;
		delete [] lpWideCharStr2;
		return b;
	}

	BOOL WINAPI_DECL CopyFileW(
		_In_  LPCWSTR lpExistingFileName,
		_In_  LPCWSTR lpNewFileName,
		_In_  BOOL bFailIfExists
		)
	{
		COPYFILE2_EXTENDED_PARAMETERS extendedParameters;
		extendedParameters.dwSize = sizeof(extendedParameters);
		extendedParameters.dwCopyFlags = bFailIfExists ? COPY_FILE_FAIL_IF_EXISTS : 0;
		extendedParameters.pfCancel = FALSE;
		extendedParameters.pProgressRoutine = NULL;
		extendedParameters.pvCallbackContext = NULL;
		HRESULT hr = CopyFile2(
			lpExistingFileName, 
			lpNewFileName, 
			&extendedParameters);
		return SUCCEEDED(hr);
	}

	HANDLE WINAPI_DECL FindFirstFileA(
		_In_   LPCSTR lpFileName,
		_Out_  LPWIN32_FIND_DATAA lpFindFileData
		)
	{
		HANDLE hFindFile = FindFirstFileExA(
			lpFileName, 
			FindExInfoStandard, 
			lpFindFileData, 
			FindExSearchNameMatch, 
			NULL, 
			0);
		return hFindFile;
	}

	HANDLE WINAPI_DECL FindFirstFileW(
		_In_   LPCWSTR lpFileName,
		_Out_  LPWIN32_FIND_DATAW lpFindFileData
		)
	{
		HANDLE hFindFile = FindFirstFileExW(
			lpFileName, 
			FindExInfoStandard, 
			lpFindFileData, 
			FindExSearchNameMatch, 
			NULL, 
			0);
		return hFindFile;
	}

	BOOL WINAPI_DECL GetFileInformationByHandle(
		_In_   HANDLE hFile,
		_Out_  LPBY_HANDLE_FILE_INFORMATION lpFileInformation
		)
	{
		FILE_BASIC_INFO basicInfo;
		FILE_STANDARD_INFO standardInfo;
		BOOL b = GetFileInformationByHandleEx(
			hFile, 
			FileBasicInfo, 
			&basicInfo, 
			sizeof(basicInfo));
		if (b == FALSE)
			return FALSE;
		b = GetFileInformationByHandleEx(
			hFile, 
			FileStandardInfo, 
			&standardInfo, 
			sizeof(standardInfo));
		if (b == FALSE)
			return FALSE;
		lpFileInformation->dwFileAttributes = basicInfo.FileAttributes;
		lpFileInformation->ftCreationTime = (FILETIME const &)basicInfo.CreationTime;
		lpFileInformation->ftLastAccessTime = (FILETIME const &)basicInfo.LastAccessTime;
		lpFileInformation->ftLastWriteTime = (FILETIME const &)basicInfo.LastWriteTime;
		lpFileInformation->dwVolumeSerialNumber = 0;
		lpFileInformation->nFileSizeHigh = standardInfo.EndOfFile.HighPart;
		lpFileInformation->nFileSizeLow = standardInfo.EndOfFile.LowPart;
		lpFileInformation->nNumberOfLinks = standardInfo.NumberOfLinks;
		lpFileInformation->nFileIndexHigh = 0;
		lpFileInformation->nFileIndexLow = 0;
		return TRUE;
	}

	BOOL WINAPI_DECL GetFileTime(
		_In_       HANDLE hFile,
		_Out_opt_  LPFILETIME lpCreationTime,
		_Out_opt_  LPFILETIME lpLastAccessTime,
		_Out_opt_  LPFILETIME lpLastWriteTime
		)
	{
		FILE_BASIC_INFO basicInfo;
		BOOL b = GetFileInformationByHandleEx(
			hFile, 
			FileBasicInfo, 
			&basicInfo, 
			sizeof(basicInfo));
		if (b == FALSE)
			return FALSE;
		if (lpCreationTime)
			*lpCreationTime = (FILETIME const &)basicInfo.CreationTime;
		if (lpLastAccessTime)
			*lpLastAccessTime = (FILETIME const &)basicInfo.LastAccessTime;
		if (lpLastWriteTime)
			*lpLastWriteTime = (FILETIME const &)basicInfo.LastWriteTime;
		return TRUE;
	}

	BOOL WINAPI_DECL SetFileTime(
		_In_      HANDLE hFile,
		_In_opt_  const FILETIME *lpCreationTime,
		_In_opt_  const FILETIME *lpLastAccessTime,
		_In_opt_  const FILETIME *lpLastWriteTime
		)
	{
		FILE_BASIC_INFO basicInfo;
		BOOL b = GetFileInformationByHandleEx(
			hFile, 
			FileBasicInfo, 
			&basicInfo, 
			sizeof(basicInfo));
		if (b == FALSE)
			return FALSE;
		if (lpCreationTime)
			basicInfo.CreationTime = *(LARGE_INTEGER const *)lpCreationTime;
		if (lpLastAccessTime)
			basicInfo.LastAccessTime = *(LARGE_INTEGER const *)lpLastAccessTime;
		if (lpLastWriteTime)
			basicInfo.LastWriteTime = *(LARGE_INTEGER const *)lpLastAccessTime;
		b = SetFileInformationByHandle(
			hFile, 
			FileBasicInfo, 
			&basicInfo, 
			sizeof(basicInfo));
		return b;
	}

	HMODULE WINAPI_DECL GetModuleHandleA(
		_In_opt_  LPCSTR lpModuleName
		)
	{
		int cchWideChar = strlen(lpModuleName) + 1;
		LPWSTR lpWideCharStr = (LPWSTR)new WCHAR[cchWideChar];
		if (::MultiByteToWideChar(CP_ACP, 0, lpModuleName, -1, lpWideCharStr, cchWideChar) == 0) {
			delete [] lpWideCharStr;
			return FALSE;
		}
		HMODULE hModule = LoadPackagedLibrary(
			lpWideCharStr, 
			0);
		delete [] lpWideCharStr;
		return hModule;
	}

	HMODULE WINAPI_DECL LoadLibraryA(
		_In_  LPCSTR lpFileName
		)
	{
		int cchWideChar = strlen(lpFileName) + 1;
		LPWSTR lpWideCharStr = (LPWSTR)new WCHAR[cchWideChar];
		if (::MultiByteToWideChar(CP_ACP, 0, lpFileName, -1, lpWideCharStr, cchWideChar) == 0) {
			delete [] lpWideCharStr;
			return FALSE;
		}
		HMODULE hModule = LoadPackagedLibrary(
			lpWideCharStr, 
			0);
		delete [] lpWideCharStr;
		return hModule;
	}

	DWORD WINAPI_DECL GetModuleFileNameA(
	  _In_opt_  HMODULE hModule,
	  _Out_     LPSTR lpFilename,
	  _In_      DWORD nSize
	)
	{
		Platform::String ^ path = 
			Windows::ApplicationModel::Package::Current->InstalledLocation->Path;
		Platform::String ^ name = 
			Windows::ApplicationModel::Package::Current->Id->Name;
		DWORD nBufferLength = ::WideCharToMultiByte(
			CP_ACP, 0, 
			path->Data(), path->Length(), 
			lpFilename, nSize - 1, 
			NULL, NULL);
		if (nBufferLength == 0)
			return 0;
		if (nBufferLength + 1 < nSize) {
			lpFilename[nBufferLength++] = '\\';
		}
		nBufferLength += ::WideCharToMultiByte(
			CP_ACP, 0, 
			name->Data(), name->Length(), 
			lpFilename + nBufferLength, nSize - nBufferLength - 1, 
			NULL, NULL);
		if (nBufferLength < nSize) {
			lpFilename[nBufferLength] = '\0';
		}
		return nBufferLength;
	}

	DWORD WINAPI_DECL GetTempPathA(
		_In_   DWORD nBufferLength,
		_Out_  LPSTR lpBuffer
		)
	{
		int cchWideChar = nBufferLength;
		LPWSTR lpWideCharStr = (LPWSTR)new WCHAR[cchWideChar];
		cchWideChar = GetTempPathW(
			cchWideChar, 
			lpWideCharStr);
		nBufferLength = ::WideCharToMultiByte(
			CP_ACP, 0, 
			lpWideCharStr, cchWideChar, 
			lpBuffer, nBufferLength - 1, 
			NULL, NULL);
		delete [] lpWideCharStr;
		lpBuffer[nBufferLength] = '\0';
		return nBufferLength;
	}

	DWORD WINAPI_DECL GetTempPathW(
		_In_   DWORD nBufferLength,
		_Out_  LPWSTR lpBuffer
		)
	{
		Platform::String ^ temp = Windows::Storage::ApplicationData::Current->LocalFolder->Path;
		wcsncpy_s(lpBuffer, nBufferLength, temp->Data(), temp->Length());
		return temp->Length();
	}
}
