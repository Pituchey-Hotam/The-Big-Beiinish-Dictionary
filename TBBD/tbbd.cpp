/*****************************************************************//**
 * \file   tbbd.cpp
 * \brief  TBBD Program.
 *
 * \author Yehuda Eisenberg.
 * \date   September 2023
 *********************************************************************/

#include <Windows.h>
#include <strsafe.h>
#include <shlobj.h>
#include <urlmon.h>
#include <regex>
#include <string>
#include "tbbd.h"
#include "const.h"
#include "resource.h"
#include "TBBD_status.h"
#include "helpers.h"
#include "web_helpers.h"

#pragma comment(lib, "urlmon.lib")


LRESULT CALLBACK GlobalCallbackHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TBBD& tbbd = TBBD::getInstance();
	return tbbd.CallbackHandler(hwnd, msg, wParam, lParam);
}

TBBD::TBBD() :
	ChangeStatusBotton(NULL), RefrashBotton(NULL), versionLabel(NULL), lastUpdateDateLabel(NULL), statusLabel(NULL),
	ServerVersion(L""), CurrentVersion(L""), ServerLastUpdateDate(L""), CurrentLastUpdateDate(L""),
	ConfigFilePath(L""), TBBDFilePath(L""), softwareInstalled(FALSE)
{}

tbbd_status_t TBBD::init() {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	size_t existingPathLen = 0;
	size_t filenameLen = 0;
	WCHAR configDirPath[MAX_PATH] = { 0 };

	if (!SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, configDirPath))) {
		status = TBBD_STATUS_SHGETFOLDERPATHW_FAILED;
		goto l_cleanup;
	}
	if (!SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, this->ConfigFilePath))) {
		status = TBBD_STATUS_SHGETFOLDERPATHW_FAILED;
		goto l_cleanup;
	}
	if (!SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, this->TBBDFilePath))) {
		status = TBBD_STATUS_SHGETFOLDERPATHW_FAILED;
		goto l_cleanup;
	}

	existingPathLen = wcslen(configDirPath);

	filenameLen = sizeof(CONFIG_DIR_PATH);
	if (existingPathLen + filenameLen + 1 <= MAX_PATH) {
		wcscat_s(configDirPath, MAX_PATH, CONFIG_DIR_PATH);
	}
	else {
		status = TBBD_STATUS_CONFIG_DIR_PATH_IS_TOO_BIG;
		goto l_cleanup;
	}

	if (!CreateDirectoryW(configDirPath, nullptr)) {
		if (ERROR_ALREADY_EXISTS != GetLastError()) {
			status = TBBD_STATUS_CREATEDIRECTORYW_FAILED;
			goto l_cleanup;
		}
	}

	filenameLen = sizeof(THE_BBD_FILE_PATH);
	if (existingPathLen + filenameLen + 1 <= MAX_PATH) {
		wcscat_s(this->TBBDFilePath, MAX_PATH, THE_BBD_FILE_PATH);
	}
	else {
		status = TBBD_STATUS_TBBD_FILE_PATH_IS_TOO_BIG;
		goto l_cleanup;
	}


	filenameLen = sizeof(CONFIG_FILE_PATH);
	if (existingPathLen + filenameLen + 1 <= MAX_PATH) {
		wcscat_s(this->ConfigFilePath, MAX_PATH, CONFIG_FILE_PATH);
	}
	else {
		status = TBBD_STATUS_CONFIG_FILE_PATH_IS_TOO_BIG;
		goto l_cleanup;
	}

	status = GetServerVersion(this->ServerVersion, VERSION_STRING_LENGTH);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}
	status = GetCurrentVersion(this->CurrentVersion, VERSION_STRING_LENGTH);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}
	status = GetServerLastUpdateHebDate(this->ServerLastUpdateDate, LAST_UPDATE_DATE_STRING_LENGTH);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}
	status = GetLastUpdateHebDate(this->CurrentLastUpdateDate, LAST_UPDATE_DATE_STRING_LENGTH);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	status = CheckIfInstalled(softwareInstalled);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	return status;
}

tbbd_status_t TBBD::run(HINSTANCE hInstance, int nCmdShow) {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	WNDCLASS wc = { 0 };
	HWND MainWindowHandle = NULL;
	HMENU hMenu = NULL;
	MSG msg = { 0 };
	HANDLE singlton = NULL;

	singlton = CreateMutexW(NULL, TRUE, L"TBBD_SINGLTON");
	if (NULL == singlton) {
		status = TBBD_STATUS_CREATE_MUTEX_ERROR;
		goto l_cleanup;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		status = TBBD_STATUS_THE_PROGRAM_ALREADY_OPEN;
		goto l_cleanup;
	}

	wc.lpfnWndProc = GlobalCallbackHandler;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wc.lpszClassName = L"TBBDClass";
	wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(MAINICON));

	if (NULL == wc.hIcon) {
		status = TBBD_STATUS_LOAD_ICON_ERROR;
		goto l_cleanup;
	}

	if (0 == RegisterClassW(&wc)) {
		status = TBBD_STATUS_REGISTERCLASS_ERROR;
		goto l_cleanup;
	}

	MainWindowHandle = CreateWindowExW(
		WS_EX_LAYOUTRTL | WS_EX_NOINHERITLAYOUT,
		L"TBBDClass",
		L"מילון הבייניש הגדול - מאת פיתוחי חותם",
		WS_OVERLAPPEDWINDOW & ~(WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME),
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		400, 180,
		NULL, NULL, hInstance, NULL);
	if (NULL == MainWindowHandle) {
		status = TBBD_STATUS_CREATE_WINDOW_ERROR;
		goto l_cleanup;
	}

	hMenu = LoadMenuW(hInstance, MAKEINTRESOURCE(MAIN_MENU));
	if (NULL == hMenu) {
		status = TBBD_STATUS_LOAD_MENU_ERROR;
		goto l_cleanup;
	}
	SetMenu(MainWindowHandle, hMenu);


	ChangeStatusBotton = MyCreateButton(hInstance, MainWindowHandle, L"התקן את המילון", 120, 25, 130, 50);
	RefrashBotton = MyCreateButton(hInstance, MainWindowHandle, L"עדכון את המילון", 250, 44, 130, 30, false);
	versionLabel = MyCreateLabel(hInstance, MainWindowHandle, L"0.0.0", 290, 85, 300, 20);
	lastUpdateDateLabel = MyCreateLabel(hInstance, MainWindowHandle, L"<התוכנה לא\nהותקנה>", 10, 45, 110, 40);
	statusLabel = MyCreateLabel(hInstance, MainWindowHandle, L"סטטוס: לא מותקן", 135, 2, 100, 20);
	(void)MyCreateLabel(hInstance, MainWindowHandle, L"בס\"ד", 5, 2, 30, 20);
	(void)MyCreateLabel(hInstance, MainWindowHandle, L"תאריך העדכון:", 10, 25, 100, 20);
	(void)MyCreateLabel(hInstance, MainWindowHandle, L"מילון הבייניש הגדול מאת פיתוחי חותם - גרסה", 30, 85, 260, 20);
	(void)MyCreateLabel(hInstance, MainWindowHandle, L"מייל תמיכה: pituchey-hotam@yehudae.net", 50, 100, 300, 20);

	if (softwareInstalled) {
		SetWindowTextW(statusLabel, L"סטטוס: מותקן");
		SetWindowTextW(ChangeStatusBotton, L"הסר את המילון");
		SetWindowTextW(versionLabel, this->CurrentVersion);
		SetWindowTextW(lastUpdateDateLabel, this->CurrentLastUpdateDate);
		ShowWindow(RefrashBotton, SW_SHOW);
	}

	ShowWindow(MainWindowHandle, nCmdShow);
	UpdateWindow(MainWindowHandle);

	while (GetMessageW(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	if (NULL != wc.hInstance) {
		UnregisterClassW(wc.lpszClassName, wc.hInstance);
	}

	if (NULL != singlton) {
		CloseHandle(singlton);
	}

	return status;
}

LRESULT CALLBACK TBBD::CallbackHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	WCHAR ErrorMessage[50] = { 0 };

	switch (uMsg) {
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 1:
			if (softwareInstalled) {
				status = UnInstall();
				if (TBBD_STATUS_SUCCESS != status) {
					goto l_cleanup;
				}
				SetWindowTextW(statusLabel, L"סטטוס: לא מותקן");
				SetWindowTextW(ChangeStatusBotton, L"התקן את המילון");
				SetWindowTextW(versionLabel, L"0.0.0");
				SetWindowTextW(lastUpdateDateLabel, L"<התוכנה לא\nהותקנה>");
				ShowWindow(RefrashBotton, SW_HIDE);

				softwareInstalled = FALSE;
			}
			else {
				status = Install();
				if (TBBD_STATUS_SUCCESS != status) {
					goto l_cleanup;
				}
				SetWindowTextW(statusLabel, L"סטטוס: מותקן");
				SetWindowTextW(ChangeStatusBotton, L"הסר את המילון");
				SetWindowTextW(versionLabel, this->CurrentVersion);
				SetWindowTextW(lastUpdateDateLabel, this->CurrentLastUpdateDate);
				ShowWindow(RefrashBotton, SW_SHOW);

				softwareInstalled = TRUE;
			}
			break;
		case 2:
			if (compareVersionStrings(this->ServerVersion, this->CurrentVersion) == CSTR_GREATER_THAN) {
				status = Update();
				if (TBBD_STATUS_SUCCESS != status) {
					goto l_cleanup;
				}
				SetWindowTextW(versionLabel, this->CurrentVersion);
				SetWindowTextW(lastUpdateDateLabel, this->CurrentLastUpdateDate);
				MessageBoxW(hwnd, L"המילון עודכן בהצלחה!", L"מילון הבייניש הגדול", MB_OK | MB_ICONINFORMATION | MB_RTLREADING);
			}
			else {
				MessageBoxW(hwnd, L"אין עדכונים חדשים...", L"מילון הבייניש הגדול", MB_OK | MB_ICONINFORMATION | MB_RTLREADING);
			}
			break;
		case MENU_ADD_WORDS:
			ShellExecuteW(NULL, L"OPEN", L"https://l.yehudae.net/6mPtxy", NULL, NULL, SW_SHOWNORMAL);
			break;
		case MENU_SITE_LINK:
			ShellExecuteW(NULL, L"OPEN", L"https://l.yehudae.net/6mPtxy", NULL, NULL, SW_SHOWNORMAL);
			break;
		case MENU_UPDATE_LINK:
			ShellExecuteW(NULL, L"OPEN", L"https://l.yehudae.net/W0BdXG", NULL, NULL, SW_SHOWNORMAL);
			break;
		case MENU_JOINUS_LINK:
			ShellExecuteW(NULL, L"OPEN", L"https://l.yehudae.net/IOpKTA", NULL, NULL, SW_SHOWNORMAL);
			break;
		case MENU_ABOUT:
			MessageBoxW(NULL, L"התוכנה פותחה ע\"י פיתוחי חותם", L"מילון הבייניש הגדול", MB_OK | MB_ICONINFORMATION | MB_RTLREADING);
			break;
		case MENU_EXIT:
			PostQuitMessage(0);
			break;
		default:
			break;
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			PostQuitMessage(0);
		}
		break;
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	if (TBBD_STATUS_SUCCESS != status) {
		if (TBBD_STATUS_URLDOWNLOADTOFILEW_FAILED == status) {
			MessageBoxW(NULL, L"תוכנת מילון הבייניש הגדול דורשת חיבור לאינטרנט כדי לפעול", L"שגיאה!", MB_OK | MB_ICONERROR | MB_RTLREADING);
		}
		else {
			wsprintfW(ErrorMessage, L"מספר שגיאה: %d\nאנא פנה לתמיכה.", (int)status);
			MessageBoxW(NULL, ErrorMessage, L"שגיאה!", MB_OK | MB_ICONERROR | MB_RTLREADING);
		}
		PostQuitMessage((int)status);
	}

	return 0;
}

tbbd_status_t TBBD::GetServerVersion(LPWSTR CurrentVersion, DWORD size) {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	PCHAR tmpServerVersion = NULL;

	tmpServerVersion = (PCHAR)HeapAlloc(GetProcessHeap(), 0, size);
	if (NULL == tmpServerVersion)
	{
		status = TBBD_STATUS_HEAPALLOC_FAILED;
		goto l_cleanup;
	}

	ZeroMemory(tmpServerVersion, size);

	status = http_get(SERVER_DOMAIN, SERVER_VERSION_PATH, (PBYTE)tmpServerVersion, size);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	MultiByteToWideChar(CP_UTF8, 0, tmpServerVersion, -1, &CurrentVersion[0], size);

l_cleanup:
	if (NULL != tmpServerVersion)
	{
		HeapFree(GetProcessHeap(), 0, tmpServerVersion);
		tmpServerVersion = NULL;
	}
	return status;
}

tbbd_status_t TBBD::GetCurrentVersion(LPWSTR CurrentVersion, DWORD size) {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	DWORD readedBytes = 0;

	readedBytes = GetPrivateProfileStringW(L"General", L"current_version", L"0.0.0", CurrentVersion, size, this->ConfigFilePath);
	if (0 == readedBytes) {
		status = TBBD_STATUS_FAILED_READ_VERSION;
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	return status;
}

tbbd_status_t TBBD::SetCurrentVersion(LPCWSTR CurrentVersion) {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;

	if (TRUE != WritePrivateProfileStringW(L"General", L"current_version", CurrentVersion, this->ConfigFilePath)) {
		status = TBBD_STATUS_FAILED_WRITE_VERSION;
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	return status;
}

tbbd_status_t TBBD::GetLastUpdateHebDate(LPWSTR LastUpdateHebDate, DWORD size) {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	DWORD readedBytes = 0;

	readedBytes = GetPrivateProfileStringW(L"General", L"last_update_date", L"------", LastUpdateHebDate, size, this->ConfigFilePath);
	if (0 == readedBytes) {
		status = TBBD_STATUS_FAILED_READ_LAST_UPDATE_DATE;
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	return status;
}

tbbd_status_t TBBD::SetLastUpdateHebDate(LPCWSTR LastUpdateHebDate) {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;

	if (TRUE != WritePrivateProfileStringW(L"General", L"last_update_date", LastUpdateHebDate, this->ConfigFilePath)) {
		status = TBBD_STATUS_FAILED_WRITE_LAST_UPDATE_DATE;
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	return status;
}

tbbd_status_t TBBD::GetServerLastUpdateHebDate(LPWSTR LastUpdateHebDate, DWORD size) {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	PCHAR tmpLastUpdateHebDate = NULL;

	tmpLastUpdateHebDate = (PCHAR)HeapAlloc(GetProcessHeap(), 0, size);
	if (NULL == tmpLastUpdateHebDate)
	{
		status = TBBD_STATUS_HEAPALLOC_FAILED;
		goto l_cleanup;
	}

	ZeroMemory(tmpLastUpdateHebDate, size);

	status = http_get(SERVER_DOMAIN, LAST_UPDATE_HEBREW_DATE_PATH, (PBYTE)tmpLastUpdateHebDate, size);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	MultiByteToWideChar(CP_UTF8, 0, tmpLastUpdateHebDate, -1, &LastUpdateHebDate[0], size);

l_cleanup:
	if (NULL != tmpLastUpdateHebDate)
	{
		HeapFree(GetProcessHeap(), 0, tmpLastUpdateHebDate);
		tmpLastUpdateHebDate = NULL;
	}
	return status;
}

tbbd_status_t TBBD::Install() {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	BOOL adalRegexNotFound = FALSE;

	if (!SUCCEEDED(URLDownloadToFileW(nullptr, THE_DICTIONARY_URL, this->TBBDFilePath, 0, nullptr))) {
		status = TBBD_STATUS_URLDOWNLOADTOFILEW_FAILED;
		goto l_cleanup;
	}

	status = TryToCreateRegistrys(L"^(\\d+)_(.+)_ADAL_state$", L"_ADAL", L"PitucheyHotem_TBBD_ADAL_Prefix");
	if (TBBD_STATUS_REGEX_NOT_FOUND == status) {
		adalRegexNotFound = TRUE;
	}
	else if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	status = TryToCreateRegistrys(L"^(\\d+)_(.+)_LiveId_state$", L"_LiveId", L"PitucheyHotem_TBBD_LiveId_Prefix");
	if (TBBD_STATUS_REGEX_NOT_FOUND == status) {
		if (adalRegexNotFound) {
			status = TBBD_STATUS_ADAL_AND_LIVEID_REGEX_NOT_FOUND;
			goto l_cleanup;
		}
	}
	else if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	status = InstallExeToAppdata();
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}
	status = InstallScheduleTask();
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	if (!SUCCEEDED(StringCchCopyExW(this->CurrentVersion, (VERSION_STRING_LENGTH / sizeof(this->CurrentVersion[0])), this->ServerVersion, NULL, NULL, 0))) {
		status = TBBD_STATUS_STRINGCCHCOPYEXW_FAILED;
		goto l_cleanup;
	}

	if (!SUCCEEDED(StringCchCopyExW(this->CurrentLastUpdateDate, (LAST_UPDATE_DATE_STRING_LENGTH / sizeof(this->CurrentLastUpdateDate[0])), this->ServerLastUpdateDate, NULL, NULL, 0))) {
		status = TBBD_STATUS_STRINGCCHCOPYEXW_FAILED;
		goto l_cleanup;
	}

	status = SetCurrentVersion(this->CurrentVersion);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}
	status = SetLastUpdateHebDate(this->CurrentLastUpdateDate);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	status = http_get(STATISTICS_SERVER_DOMAIN, STATISTICS_SERVER_PATH_INSTALL, NULL, 0);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	return status;
}

tbbd_status_t TBBD::TryToCreateRegistrys(LPCWCHAR regex, LPCWCHAR prefix, LPCWCHAR prefixRegistryName) {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	HKEY hKey = NULL;
	DWORD index = 0;
	WCHAR keyName[MAX_PATH] = { 0 };
	DWORD keyNameSize = MAX_PATH;
	std::wstring s = L"";
	std::wregex words_regex;
	std::wsregex_iterator words_begin;
	std::wsregex_iterator words_end;
	std::wsmatch match;
	LSTATUS win_status = -1;
	BOOL regex_found = FALSE;
	INT maxId = 0;
	std::wstring registryName = L"";
	std::wstring registryPrefix = L"";
	BYTE tmpValue = 0;
	const WCHAR* tmpPointer = NULL;

	words_regex = std::wregex(regex);

	if (ERROR_SUCCESS != RegOpenKeyExW(HKEY_CURRENT_USER, MASTER_REGISTRY_PATH, 0, KEY_READ | KEY_WRITE, &hKey)) {
		status = TBBD_STATUS_REGOPENKEYEXW_ERROR;
		goto l_cleanup;
	}

	while (TRUE) {
		keyNameSize = MAX_PATH;
		win_status = RegEnumValueW(hKey, index, keyName, &keyNameSize, NULL, NULL, NULL, NULL);
		if (win_status == ERROR_NO_MORE_ITEMS) {
			break;
		}
		else if (win_status != ERROR_SUCCESS) {
			status = TBBD_STATUS_REGENUMKEYEXW_ERROR;
			goto l_cleanup;
		}

		s = keyName;
		words_begin = std::wsregex_iterator(s.begin(), s.end(), words_regex);
		words_end = std::wsregex_iterator();

		if (0 < std::distance(words_begin, words_end)) {
			match = *words_begin;

			if (std::stoi(match[1]) > maxId) {
				try {
					maxId = std::stoi(match[1]);
				}
				catch (const std::invalid_argument& e) {
					(void)e;
					status = TBBD_STATUS_STOI_INVALID_ARGUMENT;
					goto l_cleanup;
				}
				catch (const std::out_of_range& e) {
					(void)e;
					status = TBBD_STATUS_STOI_OUT_OF_RANGE;
					goto l_cleanup;
				}
			}

			registryName = match[2].str();

			regex_found = TRUE;
		}

		index++;
	}

	if (regex_found) {
		++maxId;
		registryPrefix = std::to_wstring(maxId) + L"_" + registryName;
		registryPrefix += prefix;

		if (ERROR_SUCCESS != RegSetValueExW(hKey, (registryPrefix).c_str(), 0, REG_SZ, (const BYTE*)this->TBBDFilePath, sizeof(this->TBBDFilePath))) {
			status = TBBD_STATUS_REGSETVALUEEXW_ERROR;
			goto l_cleanup;
		}
		tmpValue = 1;
		if (ERROR_SUCCESS != RegSetValueExW(hKey, (registryPrefix + L"_external").c_str(), 0, REG_BINARY, &tmpValue, sizeof(tmpValue))) {
			status = TBBD_STATUS_REGSETVALUEEXW_ERROR;
			goto l_cleanup;
		}
		tmpValue = 0;
		if (ERROR_SUCCESS != RegSetValueExW(hKey, (registryPrefix + L"_roamed").c_str(), 0, REG_BINARY, &tmpValue, sizeof(tmpValue))) {
			status = TBBD_STATUS_REGSETVALUEEXW_ERROR;
			goto l_cleanup;
		}
		tmpValue = 1;
		if (ERROR_SUCCESS != RegSetValueExW(hKey, (registryPrefix + L"_state").c_str(), 0, REG_BINARY, &tmpValue, sizeof(tmpValue))) {
			status = TBBD_STATUS_REGSETVALUEEXW_ERROR;
			goto l_cleanup;
		}
		s = L"he-IL";
		tmpPointer = s.c_str();
		if (ERROR_SUCCESS != RegSetValueExW(hKey, (registryPrefix + L"_culturetag").c_str(), 0, REG_SZ, (const BYTE*)(tmpPointer), sizeof(L"he-IL"))) {
			status = TBBD_STATUS_REGSETVALUEEXW_ERROR;
			goto l_cleanup;
		}
		s = registryPrefix;
		tmpPointer = s.c_str();
		if (ERROR_SUCCESS != RegSetValueExW(hKey, prefixRegistryName, 0, REG_SZ, (const BYTE*)(tmpPointer), (DWORD)((s.length() + 1) * sizeof(WCHAR)))) {
			status = TBBD_STATUS_REGSETVALUEEXW_ERROR;
			goto l_cleanup;
		}
	}
	else {
		status = TBBD_STATUS_REGEX_NOT_FOUND;
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	if (NULL != hKey) {
		RegCloseKey(hKey);
	}

	return status;
}

tbbd_status_t TBBD::InstallScheduleTask() {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;

	HMODULE hModule = NULL;
	HRSRC hResource = NULL;
	HGLOBAL hResourceData = NULL;
	PWCHAR pData = NULL;
	DWORD dwSize = 0;
	WCHAR tempPath[MAX_PATH] = { 0 };
	DWORD tempPathLength = 0;
	std::wstring outputFileFullPath = L"";
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD bytesWritten;
	std::wstring command = L"schtasks.exe /create /tn \"PitucheyHotam_TBBD_Update\" /xml ";
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };

	hModule = GetModuleHandleW(NULL);
	if (NULL == hModule) {
		status = TBBD_STATUS_GETMODULEHANDLEW_FAILED;
		goto l_cleanup;
	}

	hResource = FindResourceW(hModule, MAKEINTRESOURCE(IDR_UPDATE_TASK_XML), RT_RCDATA);
	if (NULL == hResource) {
		status = TBBD_STATUS_FINDRESOURCEW_FAILED;
		goto l_cleanup;
	}

	hResourceData = LoadResource(hModule, hResource);
	if (NULL == hResourceData) {
		status = TBBD_STATUS_LOADRESOURCE_FAILED;
		goto l_cleanup;
	}

	pData = (PWCHAR)LockResource(hResourceData);
	dwSize = SizeofResource(hModule, hResource);

	tempPathLength = GetTempPathW(MAX_PATH, tempPath);
	if (0 == tempPathLength || MAX_PATH < tempPathLength) {
		status = TBBD_STATUS_BAD_TEMP_PATH;
		goto l_cleanup;
	}

	outputFileFullPath = tempPath;
	outputFileFullPath += L"TBBD-UpdateTaskTempFile.xml";

	hFile = CreateFileW(outputFileFullPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile) {
		status = TBBD_STATUS_CREATEFILEW_FAILED;
		goto l_cleanup;
	}

	if (!WriteFile(hFile, (PVOID)pData, dwSize, &bytesWritten, NULL)) {
		status = TBBD_STATUS_WRITEFILE_FAILED;
		goto l_cleanup;
	}

	if (dwSize != bytesWritten) {
		status = TBBD_STATUS_ERROR_ON_WRITE_TO_THE_FILE;
		goto l_cleanup;
	}

	CloseHandle(hFile);
	hFile = INVALID_HANDLE_VALUE;


	command += L"\"" + outputFileFullPath + L"\"";
	si.cb = sizeof(STARTUPINFO);

	if (CreateProcessW(NULL, (LPWSTR)command.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
		WaitForSingleObject(pi.hProcess, INFINITE);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		status = TBBD_STATUS_CREATEPROCESSW_FAILED;
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	if (INVALID_HANDLE_VALUE != hFile) {
		CloseHandle(hFile);
	}

	return status;
}

tbbd_status_t TBBD::InstallExeToAppdata() {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	wchar_t exePath[MAX_PATH] = { 0 };
	wchar_t appDataPath[MAX_PATH] = { 0 };

	if (0 == GetModuleFileNameW(NULL, exePath, MAX_PATH)) {
		status = TBBD_STATUS_GETMODULEFILENAMEW_FAILED;
		goto l_cleanup;
	}

	if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath) != S_OK) {
		status = TBBD_STATUS_SHGETFOLDERPATHW_FAILED;
		goto l_cleanup;
	}

	wcscat_s(appDataPath, MAX_PATH, L"\\PitucheyHotam\\tbbd.exe");

	if (!CopyFileW(exePath, appDataPath, FALSE)) {
		status = TBBD_STATUS_COPYFILEW_FAILED;
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	return status;
}

tbbd_status_t TBBD::UnInstall() {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;

	status = TryToDeleteRegistrys(L"PitucheyHotem_TBBD_ADAL_Prefix");
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	status = TryToDeleteRegistrys(L"PitucheyHotem_TBBD_LiveId_Prefix");
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	status = SetCurrentVersion(L"0.0.0");
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}
	status = SetLastUpdateHebDate(L"-------");
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}
	status = GetCurrentVersion(this->CurrentVersion, VERSION_STRING_LENGTH);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}
	status = GetLastUpdateHebDate(this->CurrentLastUpdateDate, LAST_UPDATE_DATE_STRING_LENGTH);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	if (!DeleteFileW(this->TBBDFilePath)) {
		if (ERROR_FILE_NOT_FOUND != GetLastError()) {
			status = TBBD_STATUS_DELETEFILEW_FAILED;
			goto l_cleanup;
		}
	}
	if (!DeleteFileW(this->ConfigFilePath)) {
		if (ERROR_FILE_NOT_FOUND != GetLastError()) {
			status = TBBD_STATUS_DELETEFILEW_FAILED;
			goto l_cleanup;
		}
	}

	status = UnInstallScheduleTask();
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}
	status = UnInstallExeFromAppdata();
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	status = http_get(STATISTICS_SERVER_DOMAIN, STATISTICS_SERVER_PATH_UNINSTALL, NULL, 0);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	return status;
}

tbbd_status_t TBBD::TryToDeleteRegistrys(LPCWCHAR registryName) {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	HKEY hKey = NULL;
	WCHAR registryValus[MAX_PATH] = { 0 };
	DWORD registryValusSize = MAX_PATH;
	std::wstring registryPrefix = L"";
	LSTATUS resStatus = -1;

	if (ERROR_SUCCESS != RegOpenKeyExW(HKEY_CURRENT_USER, MASTER_REGISTRY_PATH, 0, KEY_READ | KEY_WRITE, &hKey)) {
		status = TBBD_STATUS_REGOPENKEYEXW_ERROR;
		goto l_cleanup;
	}

	registryValusSize = MAX_PATH;
	resStatus = RegQueryValueExW(hKey, registryName, nullptr, nullptr, reinterpret_cast<LPBYTE>(registryValus), &registryValusSize);
	if (ERROR_SUCCESS == resStatus) {
		registryPrefix = registryValus;

		if (ERROR_SUCCESS != RegDeleteValueW(hKey, (registryPrefix).c_str())) {
			status = TBBD_STATUS_REGDELETEKEYW_ERROR;
			goto l_cleanup;
		}
		if (ERROR_SUCCESS != RegDeleteValueW(hKey, (registryPrefix + L"_external").c_str())) {
			status = TBBD_STATUS_REGDELETEKEYW_ERROR;
			goto l_cleanup;
		}
		if (ERROR_SUCCESS != RegDeleteValueW(hKey, (registryPrefix + L"_roamed").c_str())) {
			status = TBBD_STATUS_REGDELETEKEYW_ERROR;
			goto l_cleanup;
		}
		if (ERROR_SUCCESS != RegDeleteValueW(hKey, (registryPrefix + L"_state").c_str())) {
			status = TBBD_STATUS_REGDELETEKEYW_ERROR;
			goto l_cleanup;
		}
		if (ERROR_SUCCESS != RegDeleteValueW(hKey, (registryPrefix + L"_culturetag").c_str())) {
			status = TBBD_STATUS_REGDELETEKEYW_ERROR;
			goto l_cleanup;
		}

		if (ERROR_SUCCESS != RegDeleteValueW(hKey, registryName)) {
			status = TBBD_STATUS_REGDELETEKEYW_ERROR;
			goto l_cleanup;
		}
	}
	else if (ERROR_FILE_NOT_FOUND != resStatus) {
		status = TBBD_STATUS_REGQUERYVALUEEXW_ERROR;
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	if (NULL != hKey) {
		RegCloseKey(hKey);
	}

	return status;
}

tbbd_status_t TBBD::UnInstallScheduleTask() {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	std::wstring command = L"schtasks.exe /delete /tn \"PitucheyHotam_TBBD_Update\" /f";

	si.cb = sizeof(STARTUPINFO);

	if (CreateProcessW(NULL, (LPWSTR)command.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
		WaitForSingleObject(pi.hProcess, INFINITE);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else {
		status = TBBD_STATUS_CREATEPROCESSW_FAILED;
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	return status;
}

tbbd_status_t TBBD::UnInstallExeFromAppdata() {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	wchar_t exePath[MAX_PATH] = { 0 };
	wchar_t appDataPath[MAX_PATH] = { 0 };

	if (0 == GetModuleFileNameW(NULL, exePath, MAX_PATH)) {
		status = TBBD_STATUS_GETMODULEFILENAMEW_FAILED;
		goto l_cleanup;
	}

	if (SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath) != S_OK) {
		status = TBBD_STATUS_SHGETFOLDERPATHW_FAILED;
		goto l_cleanup;
	}

	wcscat_s(appDataPath, MAX_PATH, L"\\PitucheyHotam\\tbbd.exe");

	if (!DeleteFileW(appDataPath)) {
		status = TBBD_STATUS_DELETEFILEW_FAILED;
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	return status;
}

tbbd_status_t TBBD::Update() {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;

	if (!DeleteFileW(this->TBBDFilePath)) {
		if (ERROR_FILE_NOT_FOUND != GetLastError()) {
			status = TBBD_STATUS_DELETEFILEW_FAILED;
			goto l_cleanup;
		}
	}

	if (!SUCCEEDED(URLDownloadToFileW(nullptr, THE_DICTIONARY_URL, this->TBBDFilePath, 0, nullptr))) {
		status = TBBD_STATUS_URLDOWNLOADTOFILEW_FAILED;
		goto l_cleanup;
	}

	if (NULL == this->RefrashBotton) {
		status = http_get(STATISTICS_SERVER_DOMAIN, STATISTICS_SERVER_PATH_AUTO_UPDATE, NULL, 0);
		if (TBBD_STATUS_SUCCESS != status) {
			goto l_cleanup;
		}
	}
	else {
		status = http_get(STATISTICS_SERVER_DOMAIN, STATISTICS_SERVER_PATH_MANUAL_UPDATE, NULL, 0);
		if (TBBD_STATUS_SUCCESS != status) {
			goto l_cleanup;
		}
	}

	if (!SUCCEEDED(StringCchCopyExW(this->CurrentVersion, (VERSION_STRING_LENGTH / sizeof(this->CurrentVersion[0])), this->ServerVersion, NULL, NULL, 0))) {
		status = TBBD_STATUS_STRINGCCHCOPYEXW_FAILED;
		goto l_cleanup;
	}

	if (!SUCCEEDED(StringCchCopyExW(this->CurrentLastUpdateDate, (LAST_UPDATE_DATE_STRING_LENGTH / sizeof(this->CurrentLastUpdateDate[0])), this->ServerLastUpdateDate, NULL, NULL, 0))) {
		status = TBBD_STATUS_STRINGCCHCOPYEXW_FAILED;
		goto l_cleanup;
	}

	status = SetCurrentVersion(this->CurrentVersion);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}
	status = SetLastUpdateHebDate(this->CurrentLastUpdateDate);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	return status;
}

tbbd_status_t TBBD::CheckIfInstalled(BOOL& installed) {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	HKEY hKey = NULL;
	CHAR id[MAX_PATH] = { 0 };
	DWORD size = sizeof(id);
	LSTATUS res = 0;

	if (ERROR_SUCCESS != RegOpenKeyExW(HKEY_CURRENT_USER, MASTER_REGISTRY_PATH, 0, KEY_READ, &hKey)) {
		status = TBBD_STATUS_REGOPENKEYEXW_ERROR;
		goto l_cleanup;
	}

	res = RegQueryValueExW(hKey, L"PitucheyHotem_TBBD_ADAL_Prefix", nullptr, nullptr, reinterpret_cast<LPBYTE>(id), &size);
	if (ERROR_SUCCESS == res) {
		installed = TRUE;
	}
	else {
		installed = FALSE;

		if (ERROR_FILE_NOT_FOUND != res) {
			status = TBBD_STATUS_REGQUERYVALUEEXW_ERROR;
			goto l_cleanup;
		}
	}

	if (!installed) {
		res = RegQueryValueExW(hKey, L"PitucheyHotem_TBBD_LiveId_Prefix", nullptr, nullptr, reinterpret_cast<LPBYTE>(id), &size);
		if (ERROR_SUCCESS == res) {
			installed = TRUE;
		}
		else {
			installed = FALSE;

			if (ERROR_FILE_NOT_FOUND != res) {
				status = TBBD_STATUS_REGQUERYVALUEEXW_ERROR;
				goto l_cleanup;
			}
		}
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	if (NULL != hKey) {
		RegCloseKey(hKey);
	}

	return status;
}

tbbd_status_t TBBD::checkForUpdates() {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;

	if (softwareInstalled) {
		if (compareVersionStrings(this->ServerVersion, this->CurrentVersion) == CSTR_GREATER_THAN) {
			status = Update();
			if (TBBD_STATUS_SUCCESS != status) {
				goto l_cleanup;
			}
			// MessageBoxW(NULL, L"המילון עודכן בהצלחה!", L"מילון הבייניש הגדול", MB_OK | MB_ICONINFORMATION | MB_RTLREADING);
		}
	}
	else {
		status = UnInstallScheduleTask();
		if (TBBD_STATUS_SUCCESS != status) {
			goto l_cleanup;
		}
		status = UnInstallExeFromAppdata();
		if (TBBD_STATUS_SUCCESS != status) {
			goto l_cleanup;
		}
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:

	return status;
}
