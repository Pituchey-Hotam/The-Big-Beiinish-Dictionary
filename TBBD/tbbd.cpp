/*****************************************************************//**
 * \file   tbbd.cpp
 * \brief  TBBD Program.
 *
 * \author Yehuda Eisenberg.
 * \date   September 2023
 *********************************************************************/

#include <Windows.h>
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
	ChangeStatusBotton(NULL), RefrashBotton(NULL), versionLable(NULL), lastUpdateDateLable(NULL), statusLable(NULL),
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


	ChangeStatusBotton = MyCreateButton(hInstance, MainWindowHandle, L"התקן את התוכנה", 120, 25, 130, 50);
	RefrashBotton = MyCreateButton(hInstance, MainWindowHandle, L"רענן את התוכנה", 250, 44, 130, 30, false);
	versionLable = MyCreateLable(hInstance, MainWindowHandle, L"0.0.0", 290, 85, 300, 20);
	lastUpdateDateLable = MyCreateLable(hInstance, MainWindowHandle, L"<התוכנה לא\nהותקנה>", 10, 45, 110, 40);
	statusLable = MyCreateLable(hInstance, MainWindowHandle, L"סטטוס: לא מותקן", 135, 2, 100, 20);
	(void)MyCreateLable(hInstance, MainWindowHandle, L"בס\"ד", 5, 2, 30, 20);
	(void)MyCreateLable(hInstance, MainWindowHandle, L"תאריך העדכון:", 10, 25, 100, 20);
	(void)MyCreateLable(hInstance, MainWindowHandle, L"מילון הבייניש הגדול מאת פיתוחי חותם - גרסה", 30, 85, 260, 20);
	(void)MyCreateLable(hInstance, MainWindowHandle, L"מייל תמיכה: pituchey-hotam@yehudae.net", 50, 100, 300, 20);

	if (softwareInstalled) {
		SetWindowTextW(statusLable, L"סטטוס: מותקן");
		SetWindowTextW(ChangeStatusBotton, L"הסר את התוכנה");
		SetWindowTextW(versionLable, this->CurrentVersion);
		SetWindowTextW(lastUpdateDateLable, this->CurrentLastUpdateDate);
		ShowWindow(RefrashBotton, SW_SHOW);
	}

	ShowWindow(MainWindowHandle, nCmdShow);

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
				SetWindowTextW(statusLable, L"סטטוס: לא מותקן");
				SetWindowTextW(ChangeStatusBotton, L"התקן את התוכנה");
				SetWindowTextW(versionLable, L"0.0.0");
				SetWindowTextW(lastUpdateDateLable, L"<התוכנה לא\nהותקנה>");
				ShowWindow(RefrashBotton, SW_HIDE);

				softwareInstalled = FALSE;
			}
			else {
				status = Install();
				if (TBBD_STATUS_SUCCESS != status) {
					goto l_cleanup;
				}
				SetWindowTextW(statusLable, L"סטטוס: מותקן");
				SetWindowTextW(ChangeStatusBotton, L"הסר את התוכנה");
				SetWindowTextW(versionLable, this->CurrentVersion);
				SetWindowTextW(lastUpdateDateLable, this->CurrentLastUpdateDate);
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
				SetWindowTextW(versionLable, this->CurrentVersion);
				SetWindowTextW(lastUpdateDateLable, this->CurrentLastUpdateDate);
				MessageBoxW(hwnd, L"המילון עודכן בהצלחה!", L"מילון הבייניש הגדול", MB_OK | MB_ICONINFORMATION);
			}
			else {
				MessageBoxW(hwnd, L"אין עדכונים חדשים...", L"מילון הבייניש הגדול", MB_OK | MB_ICONINFORMATION);
			}
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
			MessageBoxW(NULL, L"התוכנה נוצרה באמצעות פרוייקט פיתוחי חותם", L"מילון הבייניש הגדול", MB_OK | MB_ICONINFORMATION | MB_RTLREADING);
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

	tmpServerVersion = (PCHAR)HeapAlloc(GetProcessHeap(), 0, (size + 1));
	if (NULL == tmpServerVersion)
	{
		status = TBBD_STATUS_HEAPALLOC_FAILED;
		goto l_cleanup;
	}

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

	tmpLastUpdateHebDate = (PCHAR)HeapAlloc(GetProcessHeap(), 0, (size + 1));
	if (NULL == tmpLastUpdateHebDate)
	{
		status = TBBD_STATUS_HEAPALLOC_FAILED;
		goto l_cleanup;
	}

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
	HKEY hKey = NULL;
	DWORD index = 0;
	WCHAR keyName[MAX_PATH] = { 0 };
	DWORD keyNameSize = MAX_PATH;
	std::wstring s = L"";
	std::wregex words_regex(L"^(\\d+)_(.+)_state$");
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

	if (!SUCCEEDED(URLDownloadToFileW(nullptr, THE_DICTIONARY_URL, this->TBBDFilePath, 0, nullptr))) {
		status = TBBD_STATUS_URLDOWNLOADTOFILEW_FAILED;
		goto l_cleanup;
	}

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
		s = std::to_wstring(maxId);
		tmpPointer = s.c_str();
		if (ERROR_SUCCESS != RegSetValueExW(hKey, L"PitucheyHotem_TBBD_ID", 0, REG_SZ, (const BYTE*)(tmpPointer), (DWORD)(s.length() + 1))) {
			status = TBBD_STATUS_REGSETVALUEEXW_ERROR;
			goto l_cleanup;
		}

		memcpy(this->CurrentVersion, this->ServerVersion, (size_t)((VERSION_STRING_LENGTH) / sizeof(WCHAR)) + 1);
		memcpy(this->CurrentLastUpdateDate, this->ServerLastUpdateDate, (size_t)((LAST_UPDATE_DATE_STRING_LENGTH) / sizeof(WCHAR)) + 1);

		status = SetCurrentVersion(this->CurrentVersion);
		if (TBBD_STATUS_SUCCESS != status) {
			goto l_cleanup;
		}
		status = SetLastUpdateHebDate(this->CurrentLastUpdateDate);
		if (TBBD_STATUS_SUCCESS != status) {
			goto l_cleanup;
		}
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	if (NULL != hKey) {
		RegCloseKey(hKey);
	}

	return status;
}

tbbd_status_t TBBD::UnInstall() {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	HKEY hKey = NULL;
	DWORD index = 0;
	WCHAR registryValus[MAX_PATH] = { 0 };
	DWORD registryValusSize = MAX_PATH;
	std::wstring s = L"";
	std::wregex words_regex(L"^(\\d+)_(.+)_state$");
	std::wsregex_iterator words_begin;
	std::wsregex_iterator words_end;
	std::wsmatch match;
	LSTATUS win_status = -1;
	DWORD TBBD_id = 0;
	std::wstring registryName = L"";
	std::wstring registryPrefix = L"";

	if (ERROR_SUCCESS != RegOpenKeyExW(HKEY_CURRENT_USER, MASTER_REGISTRY_PATH, 0, KEY_READ | KEY_WRITE, &hKey)) {
		status = TBBD_STATUS_REGOPENKEYEXW_ERROR;
		goto l_cleanup;
	}

	while (TRUE) {
		registryValusSize = MAX_PATH;
		win_status = RegEnumValueW(hKey, index, registryValus, &registryValusSize, NULL, NULL, NULL, NULL);
		if (win_status == ERROR_NO_MORE_ITEMS) {
			break;
		}
		else if (win_status != ERROR_SUCCESS) {
			status = TBBD_STATUS_REGENUMKEYEXW_ERROR;
			goto l_cleanup;
		}

		s = registryValus;
		words_begin = std::wsregex_iterator(s.begin(), s.end(), words_regex);
		words_end = std::wsregex_iterator();

		if (0 < std::distance(words_begin, words_end)) {
			match = *words_begin;
			registryName = match[2].str();
			break;
		}

		index++;
	}

	if (ERROR_SUCCESS != RegQueryValueExW(hKey, L"PitucheyHotem_TBBD_ID", nullptr, nullptr, reinterpret_cast<LPBYTE>(registryValus), &registryValusSize)) {
		status = TBBD_STATUS_REGQUERYVALUEEXW_ERROR;
		goto l_cleanup;
	}

	try {
		TBBD_id = std::stoi(registryValus);
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
	
	registryPrefix = std::to_wstring(TBBD_id) + L"_" + registryName;

	win_status = RegDeleteValueW(hKey, (registryPrefix).c_str());
	if (ERROR_SUCCESS != win_status) {
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

	if (ERROR_SUCCESS != RegDeleteValueW(hKey, L"PitucheyHotem_TBBD_ID")) {
		status = TBBD_STATUS_REGDELETEKEYW_ERROR;
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

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	if (NULL != hKey) {
		RegCloseKey(hKey);
	}

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

	memcpy(this->CurrentVersion, this->ServerVersion, (size_t)((VERSION_STRING_LENGTH) / sizeof(WCHAR)) + 1);
	memcpy(this->CurrentLastUpdateDate, this->ServerLastUpdateDate, (size_t)((LAST_UPDATE_DATE_STRING_LENGTH) / sizeof(WCHAR)) + 1);

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
	CHAR id[10] = { 0 };
	DWORD size = sizeof(id);
	LSTATUS res = 0;

	if (ERROR_SUCCESS != RegOpenKeyExW(HKEY_CURRENT_USER, MASTER_REGISTRY_PATH, 0, KEY_READ, &hKey)) {
		status = TBBD_STATUS_REGOPENKEYEXW_ERROR;
		goto l_cleanup;
	}

	res = RegQueryValueExW(hKey, L"PitucheyHotem_TBBD_ID", nullptr, nullptr, reinterpret_cast<LPBYTE>(id), &size);

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

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	if (NULL != hKey) {
		RegCloseKey(hKey);
	}

	return status;
}
