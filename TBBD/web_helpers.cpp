/*****************************************************************//**
 * \file   web_helpers.cpp
 * \brief  Web Helpers.
 * 
 * \author Yehuda Eisenberg
 * \date   September 2023
 *********************************************************************/


 /*
 * @project
 * @brief Remote control
 * @author Yehuda
 * @date 16/01/2023
 */


#include <stdio.h>
#include <Windows.h>
#include <WinHttp.h>
#include "tbbd_status.h"

#pragma comment(lib, "winhttp.lib")

tbbd_status_t http_get(LPCWCHAR domain, LPCWCHAR path, PBYTE response, DWORD response_size)
{
	tbbd_status_t status = TBBD_STATUS_UNINITIALIZED;
	HINTERNET session = NULL;
	HINTERNET connection = NULL;
	HINTERNET request = NULL;

	session = WinHttpOpen(L"TBBD-PH-YE/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
	if (NULL == session)
	{
		status = TBBD_STATUS_WAMIN_WINHTTPOPEN_FAILED;
		goto l_cleanup;
	}

	connection = WinHttpConnect(session, domain, INTERNET_DEFAULT_HTTPS_PORT, 0);
	if (NULL == connection)
	{
		status = TBBD_STATUS_WAMIN_WINHTTPCONNECT_FAILED;
		goto l_cleanup;
	}

	request = WinHttpOpenRequest(connection, L"GET", path, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
	if (NULL == request)
	{
		status = TBBD_STATUS_WAMIN_WINHTTPOPENREQUEST_FAILED;
		goto l_cleanup;
	}

	if (FALSE == WinHttpSendRequest(request, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
	{
		status = TBBD_STATUS_WAMIN_WINHTTPSENDREQUEST_FAILED;
		goto l_cleanup;
	}

	if (FALSE == WinHttpReceiveResponse(request, NULL))
	{
		status = TBBD_STATUS_WAMIN_WINHTTPRECEIVERESPONSE_FAILED;
		goto l_cleanup;
	}

	/*if (FALSE == WinHttpQueryDataAvailable(request, &response_size))
	{
		status = TBBD_STATUS_WAMIN_WINHTTPQUERYDATAAVAILABLE_FAILED;
		goto l_cleanup;
	}

	response = (PBYTE)HeapAlloc(GetProcessHeap(), 0, (response_size + 1));
	if (NULL == response)
	{
		status = TBBD_STATUS_WAMIN_HEAPALLOC_FAILED;
		goto l_cleanup;
	}*/

	if (FALSE == WinHttpReadData(request, (PVOID)response, response_size, NULL))
	{
		status = TBBD_STATUS_WAMIN_WINHTTPREADDATA_FAILED;
		goto l_cleanup;
	}

	status = TBBD_STATUS_SUCCESS;

l_cleanup:
	/*if (NULL != response)
	{
		HeapFree(GetProcessHeap(), 0, response);
		response = NULL;
	}*/
	if (NULL != request)
	{
		WinHttpCloseHandle(request);
		request = NULL;
	}
	if (NULL != connection)
	{
		WinHttpCloseHandle(connection);
		connection = NULL;
	}
	if (NULL != session)
	{
		WinHttpCloseHandle(session);
		session = NULL;
	}
	return status;
}