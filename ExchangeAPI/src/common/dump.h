#pragma once

#include <Windows.h>
#include <dbghelp.h>
#define DUMP_FILE ".\\WindowsP.dmp"
#include <string>
using std::string;

void CreateDumpFile(LPCSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
{
	HANDLE hDumpFile = CreateFileA(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
	dumpInfo.ExceptionPointers = pException;
	dumpInfo.ThreadId = GetCurrentThreadId();
	dumpInfo.ClientPointers = TRUE;
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
	CloseHandle(hDumpFile);
}


LONG ApplicationCrashHandler(EXCEPTION_POINTERS *pException)
{
	char szPath[512];
	GetModuleFileNameA(NULL, szPath, 512);
	char *pChar = strrchr(szPath, '\\');
	*(pChar + 1) = 0;
	string strPath = szPath;
	SYSTEMTIME syst;
	GetLocalTime(&syst);
	char strCount[100];
	sprintf_s(strCount, 100, "%d.%.2d.%.2d.%.2d.%.2d.%.2d.%.3d.dmp", syst.wYear - 2000, syst.wMonth, syst.wDay, syst.wHour, syst.wMinute, syst.wSecond, syst.wMilliseconds);

	strPath += string(strCount);
	MakeSureDirectoryPathExists(strPath.c_str());
	CreateDumpFile(strPath.c_str(), pException);
	FatalAppExitA(0, "*** Unhandled Exception! ***");

	return EXCEPTION_EXECUTE_HANDLER;
}