#include <Windows.h>
#include <fileapi.h>
#include <string>

#include <winternl.h>
#include <ntstatus.h>

#include <RestartManager.h>
#include <winerror.h>

#include <stdexcept>
#include <iostream>
#include <cstdlib>

#include <wchar.h>
#include <Shlwapi.h>

#include <locale.h>

#pragma comment(lib, "Rstrtmgr.lib")	// Restart Manager用
#pragma comment(lib, "Shlwapi.lib")		// ファイルの存在確認用

int main() {
	setlocale(LC_CTYPE, "ja_JP.UTF-8");

	DWORD dw_session, dw_error;
	WCHAR sz_session_key[CCH_RM_SESSION_KEY + 1]{};

	dw_error = RmStartSession(&dw_session, 0, sz_session_key);
	if (dw_error != ERROR_SUCCESS) {
		throw std::runtime_error("fail to start restart manager.");
	}

	LPCWSTR lpcwstr = L"D:\\test.docx";			// ここでファイルパスを指定
	printf("Target file : %ls\n", lpcwstr);

	// ファイルの存在をチェック
	if (!PathFileExists(lpcwstr)) {
		printf("The file doesn't exit.\n");
		return 0;
	}
	
	const int files_n = 1;
	dw_error = RmRegisterResources(dw_session, files_n, &lpcwstr, 0, NULL, 0, NULL);
	if (dw_error != ERROR_SUCCESS) {
		printf("fail to register target files.");
	}

	// そのファイルを使用しているプロセスの一覧を取得
	UINT n_proc_info_needed = 0;
	UINT n_proc_info = 256;
	RM_PROCESS_INFO* rgpi = new RM_PROCESS_INFO[n_proc_info];
	DWORD dw_reason;

	dw_error = RmGetList(dw_session, &n_proc_info_needed, &n_proc_info, rgpi, &dw_reason);

	if (dw_error == ERROR_MORE_DATA) {
		delete[] rgpi;
		n_proc_info = n_proc_info_needed;
		rgpi = new RM_PROCESS_INFO[n_proc_info];
		dw_error = RmGetList(dw_session, &n_proc_info_needed, &n_proc_info, rgpi, &dw_reason);
	}
	else if (dw_error == ERROR_SUCCESS) {
		printf("this file is opened by %d processes.\n", (int)n_proc_info_needed);
	}
	else {
		printf("Couldn't get process list: %d\n", dw_error);
	}

	RmEndSession(dw_session);

	for (int i = 0; i < n_proc_info_needed; i++) {
		printf("Process ID:\t\t%d\n", rgpi[i].Process.dwProcessId);
		printf("Application Name:\t%ls\n", rgpi[i].strAppName);
		printf("Application Type:\t%d\n", rgpi[i].ApplicationType);
	}
}
