/*****************************************************************//**
 * \file   tbbd.h
 * \brief  TBBD Program.
 *
 * \author Yehuda Eisenber.
 * \date   September 2023
 *********************************************************************/

#pragma once

#include <Windows.h>
#include "TBBD_status.h"

#define VERSION_STRING_LENGTH (15)
#define LAST_UPDATE_DATE_STRING_LENGTH (50)

class TBBD {
public:
	static TBBD& getInstance() {
		static TBBD instance;
		return instance;
	}

	LRESULT CALLBACK CallbackHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	tbbd_status_t init();
	tbbd_status_t run(HINSTANCE hInstance, int nCmdShow);
	tbbd_status_t GetServerVersion(LPWSTR CurrentVersion, DWORD size);
	tbbd_status_t GetCurrentVersion(LPWSTR CurrentVersion, DWORD size);
	tbbd_status_t SetCurrentVersion(LPCWSTR CurrentVersion);
	tbbd_status_t GetServerLastUpdateHebDate(LPWSTR LastUpdateHebDate, DWORD size);
	tbbd_status_t GetLastUpdateHebDate(LPWSTR LastUpdateHebDate, DWORD size);
	tbbd_status_t SetLastUpdateHebDate(LPCWSTR LastUpdateHebDate);
	tbbd_status_t CheckIfInstalled(BOOL& installed);
	tbbd_status_t Install();
	tbbd_status_t UnInstall();
	tbbd_status_t Update();

	TBBD(const TBBD&) = delete;
	TBBD& operator=(const TBBD&) = delete;
private:
	TBBD();

	WCHAR ServerVersion[VERSION_STRING_LENGTH];
	WCHAR CurrentVersion[VERSION_STRING_LENGTH];
	WCHAR ServerLastUpdateDate[LAST_UPDATE_DATE_STRING_LENGTH];
	WCHAR CurrentLastUpdateDate[LAST_UPDATE_DATE_STRING_LENGTH];

	WCHAR ConfigFilePath[MAX_PATH];
	WCHAR TBBDFilePath[MAX_PATH];

	BOOL softwareInstalled;

	HWND ChangeStatusBotton;
	HWND RefrashBotton;
	HWND versionLable;
	HWND lastUpdateDateLable;
	HWND statusLable;
};
