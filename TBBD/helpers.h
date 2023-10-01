/*****************************************************************//**
 * \file   helpers.h
 * \brief  Helpers.
 *
 * \author Yehuda Eisenberg
 * \date   September 2023
 *********************************************************************/

#include <Windows.h>

#pragma once

 /**
  * @brief Create a button.
  */
HWND MyCreateButton(HINSTANCE instance, HWND handle, LPCWSTR value, int x, int y, int width, int height, bool show = true);


/**
 * @brief Create a lable.
 */
HWND MyCreateLable(HINSTANCE instance, HWND handle, LPCWSTR value, int x, int y, int width, int height, bool show = true);

/**
 * @brief Compare two version strings.
 */
INT compareVersionStrings(LPCWCHAR version1, LPCWCHAR version2);

