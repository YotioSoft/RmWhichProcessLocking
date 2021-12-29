#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include <Siv3D/Windows/Windows.hpp>
#include <string>

#include <winternl.h>
#include <ntstatus.h>

#include <RestartManager.h>
#include <winerror.h>

#pragma comment(lib, "Rstrtmgr.lib")

void Main()
{
	// 背景の色を設定 | Set background color
	Scene::SetBackground(ColorF{ 0.3, 0.3, 0.3 });

	Print << U"Drop File Here!";

	while (System::Update())
	{
		// ファイルがドラッグアンドドロップされたら
		if (DragDrop::HasNewFilePaths()) {
			// 文字をクリア
			ClearPrint();

			// ファイルの総数（1つ）
			DroppedFilePath file = DragDrop::GetDroppedFilePaths()[0];
			int files_n = 1;

			// RestartManagerセッションの開始
			DWORD dw_session, dw_error;
			WCHAR sz_session_key[CCH_RM_SESSION_KEY + 1]{};

			dw_error = RmStartSession(&dw_session, 0, sz_session_key);
			if (dw_error != ERROR_SUCCESS) {
				throw std::runtime_error("fail to start restart manager.");
			}

			// ファイルをセッションに登録
			//wchar_t* buf;
			//file_path.toWstr().assign(buf);

			std::wstring wstr = file.path.toWstr();
			PCWSTR pcwstr = wstr.c_str();
			Print << Unicode::FromWstring(pcwstr);

			dw_error = RmRegisterResources(dw_session, files_n, &pcwstr, 0, NULL, 0, NULL);
			if (dw_error != ERROR_SUCCESS) {
				Console << U"Err";
				throw std::runtime_error("fail to register target files.");
			}

			// そのファイルを使用しているプロセスの一覧を取得
			UINT n_proc_info_needed = 0;
			UINT n_proc_info = 1;
			RM_PROCESS_INFO* rgpi = new RM_PROCESS_INFO[n_proc_info];
			DWORD dw_reason;

			dw_error = RmGetList(dw_session, &n_proc_info_needed, &n_proc_info, rgpi, &dw_reason);

			if (dw_error == ERROR_MORE_DATA) {		// rgpiの足りない分を追加
				delete[] rgpi;
				n_proc_info = n_proc_info_needed;
				rgpi = new RM_PROCESS_INFO[n_proc_info];
				dw_error = RmGetList(dw_session, &n_proc_info_needed, &n_proc_info, rgpi, &dw_reason);	// もう一度取得
				Print << U"this file is opened by " << (int)n_proc_info_needed << U" processes.";
			}
			else if (dw_error == ERROR_SUCCESS) {
				Print << U"this file is opened by " << (int)n_proc_info_needed << U" processes.";
			}
			else {
				Print << U"Error";
				break;
			}
			if (dw_error != ERROR_SUCCESS) {
				throw std::runtime_error("fail to get process list.");
			}

			for (int i = 0; i < n_proc_info_needed; i++) {
				Print << U"---------------------------------------------------------------";
				Print << U"プロセスID: " << rgpi[i].Process.dwProcessId;
				Print << U"アプリ名: " << Unicode::FromWstring((std::wstring)rgpi[i].strAppName);
				Print << U"アプリのタイプ: " << rgpi[i].ApplicationType;
				Print << U"---------------------------------------------------------------";
			}

			// セッション終了
			RmEndSession(dw_session);
		}
	}
}
