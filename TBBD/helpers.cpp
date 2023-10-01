/*****************************************************************//**
 * \file   helpers.cpp
 * \brief  Helpers.
 *
 * \author Yehuda Eisenberg.
 * \date   September 2023
 *********************************************************************/


#include <fstream>
#include <string>
#include <vector>
#include "helpers.h"


HWND MyCreateButton(HINSTANCE instance, HWND handle, LPCWSTR value, int x, int y, int width, int height, bool show) {
	static DWORD id = 1;

	return CreateWindowExW(WS_EX_LAYOUTRTL | WS_EX_NOINHERITLAYOUT, L"BUTTON", value, WS_CHILD | (show ? WS_VISIBLE : 0) | BS_DEFPUSHBUTTON, x, y, width, height, handle, (HMENU)(id++), instance, NULL);
}

HWND MyCreateLable(HINSTANCE instance, HWND handle, LPCWSTR value, int x, int y, int width, int height, bool show) {
	static DWORD id = 1;

	return CreateWindowExW(WS_EX_LAYOUTRTL | WS_EX_NOINHERITLAYOUT, L"STATIC", value, WS_CHILD | (show ? WS_VISIBLE : 0), x, y, width, height, handle, (HMENU)(id++), instance, NULL);
}

INT compareVersionStrings(LPCWCHAR version1, LPCWCHAR version2) {
	return (INT)CompareStringW(LOCALE_USER_DEFAULT, 0, version1, -1, version2, -1);
}

