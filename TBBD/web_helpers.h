/*****************************************************************//**
 * \file   web_helpers.h
 * \brief  Web Helpers.
 * 
 * \author Yehuda Eisenberg
 * \date   September 2023
 *********************************************************************/

#pragma once


tbbd_status_t http_get(LPCWCHAR domain, LPCWCHAR path, PBYTE response, DWORD response_size);

