/*****************************************************************//**
 * \file   const.h
 * \brief  Constants
 * 
 * \author Yehuda Eisenberg.
 * \date   September 2023
 *********************************************************************/

#pragma once

#include <Windows.h>

#define SERVER_DOMAIN L"raw.githubusercontent.com"
#define SERVER_VERSION_PATH L"/Pituchey-Hotam/The-Big-Beiinish-Dictionary/data/version.txt"
#define LAST_UPDATE_HEBREW_DATE_PATH L"/Pituchey-Hotam/The-Big-Beiinish-Dictionary/data/last-update-hebrew-date.txt"
#define THE_DICTIONARY_URL L"https://raw.githubusercontent.com/Pituchey-Hotam/The-Big-Beiinish-Dictionary/data/TBBD.dic"

constexpr LPCWCHAR STATISTICS_SERVER_DOMAIN = L"yehudae.net";
constexpr LPCWCHAR STATISTICS_SERVER_PATH_INSTALL = L"/PitucheyHotam/tbbd.php?install";
constexpr LPCWCHAR STATISTICS_SERVER_PATH_UNINSTALL = L"/PitucheyHotam/tbbd.php?uninstall";
constexpr LPCWCHAR STATISTICS_SERVER_PATH_MANUAL_UPDATE = L"/PitucheyHotam/tbbd.php?manual-update";
constexpr LPCWCHAR STATISTICS_SERVER_PATH_AUTO_UPDATE = L"/PitucheyHotam/tbbd.php?auto-update";

#define MASTER_REGISTRY_PATH L"SOFTWARE\\Microsoft\\Shared Tools\\Proofing Tools\\1.0\\Custom Dictionaries"
#define CONFIG_DIR_PATH L"\\PitucheyHotam"
#define THE_BBD_FILE_PATH L"\\PitucheyHotam\\מילון הבייניש הגדול - מאת פיתוחי חותם.dic"
#define CONFIG_FILE_PATH L"\\PitucheyHotam\\config.TBBD.ini"
