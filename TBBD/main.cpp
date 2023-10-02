/*****************************************************************//**
 * \file   main.cpp
 * \brief  TBBD project
 *
 * \author Yehuda Eisenberg
 * \date   September 2023
 *********************************************************************/

#include <Windows.h>
#include "tbbd.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	WCHAR ErrorMessage[50] = { 0 };
	TBBD& tbbd = TBBD::getInstance();

	status = tbbd.init();
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	status = tbbd.run(hInstance, nCmdShow);
	if (TBBD_STATUS_SUCCESS != status) {
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;
l_cleanup:
	if (TBBD_STATUS_SUCCESS != status) {
		if (TBBD_STATUS_WAMIN_WINHTTPSENDREQUEST_FAILED == status) {
			MessageBoxW(NULL, L"תוכנת מילון הבייניש הגדול דורשת חיבור לאינטרנט כדי לפעול", L"שגיאה!", MB_OK | MB_ICONERROR | MB_RTLREADING);
		}
		else {
			wsprintfW(ErrorMessage, L"מספר שגיאה: %d\nאנא פנה לתמיכה.", (int)status);
			MessageBoxW(NULL, ErrorMessage, L"שגיאה!", MB_OK | MB_ICONERROR | MB_RTLREADING);
		}
	}

	return status;
}
