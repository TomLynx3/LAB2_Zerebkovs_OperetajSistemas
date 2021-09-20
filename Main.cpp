#undef UNICODE
#include <windows.h>
#include <stdio.h>
#include "resource.h"
#include <inttypes.h>
#include <process.h>


#pragma warning (disable:4996)


BOOL CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

HANDLE handle;


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, MainWndProc);
	return 0;
}


bool BrowseFileName(HWND hWnd, char* FileName) {
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "Executable Files (*.exe)\0*.exe\0"
		"All Files (*.*)\0*.*\0";
	ofn.lpstrFile = FileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = "exe";
	return GetOpenFileName(&ofn);
}

void GetProcessInfo(HWND hWnd) {
	DWORD exitCode;
	GetExitCodeProcess(handle, &exitCode);
	FILETIME creation_time, exit_time, kernel_time, user_time, local_creation_time;
	GetProcessTimes(handle, &creation_time, &exit_time, &kernel_time, &user_time);
	
	char buf[100];

	if (exitCode == 259) {
		SetDlgItemText(hWnd, IDC_STATE, "Running");

		
		SYSTEMTIME converted_time;
		

		FileTimeToLocalFileTime(&creation_time,&local_creation_time);

		FileTimeToSystemTime(&local_creation_time, &converted_time);


		sprintf(buf, "%02d:%02d:%02d\n",converted_time.wHour, converted_time.wMinute, converted_time.wSecond);

		SetDlgItemText(hWnd, IDC_STARTTIME, buf);
		EnableWindow(GetDlgItem(hWnd, IDC_TERMINATE), true);
		

	}else {
		if (handle != 0) {
			SYSTEMTIME converted_time;
			FileTimeToLocalFileTime(&exit_time, &local_creation_time);

			FileTimeToSystemTime(&local_creation_time, &converted_time);

			sprintf(buf, "%02d:%02d:%02d\n", converted_time.wHour, converted_time.wMinute, converted_time.wSecond);
			SetDlgItemText(hWnd, IDC_FINISHTIME, buf);

		}
		SetDlgItemText(hWnd, IDC_STATE, "Finished");
		EnableWindow(GetDlgItem(hWnd, IDC_TERMINATE),false);
		handle = 0;
	}
}


void HandleTerminate() {
	DWORD exitCode =0;
	if (handle != 0) {
		TerminateProcess(handle, exitCode);
		
	}

}


bool RunProcess(LPSTR command, HWND hWnd) {
	STARTUPINFO si; 
	ZeroMemory(&si, sizeof(si)); 
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi; 


	if (CreateProcess(
		NULL,
		command,
		NULL, 
		NULL, 
		FALSE, 
		0, 
		NULL,
		NULL, 
		&si, 
		&pi) 
		)
	{ 
	
		SetDlgItemInt(hWnd, IDC_PID, pi.dwProcessId,false);

		handle = pi.hProcess;
		CloseHandle(pi.hThread);

	
		return true;
	}
	return false;
}


void HandleBrowse(HWND hWnd) {
	char filename[MAX_PATH] = "";
	if (BrowseFileName(hWnd, filename)) {
		SetDlgItemText(hWnd, IDC_COMMANDLINE, filename);
		EnableWindow(GetDlgItem(hWnd, IDC_START),true);
	}

}


void HandleStart(HWND hWnd) {
	if (handle == 0) {
		char filename[MAX_PATH] = "";
		GetDlgItemText(hWnd, IDC_COMMANDLINE, filename, MAX_PATH);
		if (RunProcess(filename, hWnd)) {
			SetTimer(hWnd, NULL, 250, NULL);
		}
		
	}

}





BOOL CALLBACK MainWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {

	switch (Msg) {
	case WM_INITDIALOG:
		SetWindowTextA(hWnd, "Process Control");
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			DestroyWindow(hWnd);
			return TRUE;
		case IDC_BROWSE:
			HandleBrowse(hWnd);
			return TRUE;
		case IDC_EXIT:
			PostQuitMessage(0);
			return TRUE;
		case IDC_START:
			HandleStart(hWnd);
			return TRUE;
		case IDC_TERMINATE:
			HandleTerminate();
			return TRUE;
		}
		return FALSE;
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	case WM_TIMER:
		GetProcessInfo(hWnd);
		return true;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return TRUE;

	}
	return FALSE;
}