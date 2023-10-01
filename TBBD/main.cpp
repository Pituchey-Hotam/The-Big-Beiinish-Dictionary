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
	return status;
}
