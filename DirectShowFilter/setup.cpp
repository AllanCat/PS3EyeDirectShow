//------------------------------------------------------------------------------
// File: Setup.cpp
//
// Desc: DirectShow sample code - implementation of PushSource sample filters
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>
#include <initguid.h>

#include "ps3eye.hpp"
#include "PS3EyeSourceFilter.h"
#include "PS3EyeGuids.h"

// Note: It is better to register no media types than to register a partial 
// media type (subtype == GUID_NULL) because that can slow down intelligent connect 
// for everyone else.

// For a specialized source filter like this, it is best to leave out the 
// AMOVIESETUP_FILTER altogether, so that the filter is not available for 
// intelligent connect. Instead, use the CLSID to create the filter or just 
// use 'new' in your application.

#define MAX_DEVICE_COUNT 8

// Filter setup data
const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
	&MEDIATYPE_Video,       // Major type
	&MEDIASUBTYPE_RGB24      // Minor type
};

const AMOVIESETUP_MEDIATYPE sudOpPinTypesA =
{
	&MEDIATYPE_Audio,       // Major type
	&MEDIASUBTYPE_PCM      // Minor type
};

const AMOVIESETUP_PIN sudOutputPinPS3Eye =
{
	L"Output",      // Obsolete, not used.
	FALSE,          // Is this pin rendered?
	TRUE,           // Is it an output pin?
	FALSE,          // Can the filter create zero instances?
	FALSE,          // Does the filter create multiple instances?
	&CLSID_NULL,    // Obsolete.
	NULL,           // Obsolete.
	1,              // Number of media types.
	&sudOpPinTypes  // Pointer to media types.
};

/*
const AMOVIESETUP_FILTER sudPushSourcePS3Eye =
{
	&CLSID_PS3EyeSource,    // Filter CLSID
	g_ps3PS3EyeSource,      // String name
	MERIT_NORMAL,       // Filter merit
	1,                      // Number pins
	&sudOutputPinPS3Eye     // Pin details
};
*/

CFactoryTemplate g_Templates[MAX_DEVICE_COUNT + 1] =
{
	{
	  g_ps3PS3EyeSourceA,               // Name
	  &CLSID_PS3EyeSourceA,             // CLSID
	  PS3EyeMicSource::CreateInstance,  // Method to create an instance of MyComponent
	  NULL,                             // Initialization function
	  NULL                              // Set-up information (for filters)
	},
	{
	  g_ps3PS3EyeSource,                // Name
	  &CLSID_PS3EyeSource,              // CLSID
	  PS3EyeSource::CreateInstance,     // Method to create an instance of MyComponent
	  NULL,                             // Initialization function
	  NULL                              // Set-up information (for filters)
      //&sudPushSourcePS3Eye,           // Set-up information (for filters)
	},
	{
	  g_ps3PS3EyeSource2,               // Name
	  &CLSID_PS3EyeSource2,             // CLSID
	  PS3EyeSource::CreateInstance2,    // Method to create an instance of MyComponent
	  NULL,                             // Initialization function
	  NULL                              // Set-up information (for filters)
	},
	{
	  g_ps3PS3EyeSource3,               // Name
	  &CLSID_PS3EyeSource3,             // CLSID
	  PS3EyeSource::CreateInstance3,    // Method to create an instance of MyComponent
	  NULL,                             // Initialization function
	  NULL                              // Set-up information (for filters)
	},
	{
	  g_ps3PS3EyeSource4,               // Name
	  &CLSID_PS3EyeSource4,             // CLSID
	  PS3EyeSource::CreateInstance4,    // Method to create an instance of MyComponent
	  NULL,                             // Initialization function
	  NULL                              // Set-up information (for filters)
	},
	{
	  g_ps3PS3EyeSource5,               // Name
	  &CLSID_PS3EyeSource5,             // CLSID
	  PS3EyeSource::CreateInstance5,    // Method to create an instance of MyComponent
	  NULL,                             // Initialization function
	  NULL                              // Set-up information (for filters)
	},
	{
	  g_ps3PS3EyeSource6,               // Name
	  &CLSID_PS3EyeSource6,             // CLSID
	  PS3EyeSource::CreateInstance6,    // Method to create an instance of MyComponent
	  NULL,                             // Initialization function
	  NULL                              // Set-up information (for filters)
	},
	{
	  g_ps3PS3EyeSource7,               // Name
	  &CLSID_PS3EyeSource7,             // CLSID
	  PS3EyeSource::CreateInstance7,    // Method to create an instance of MyComponent
	  NULL,                             // Initialization function
	  NULL                              // Set-up information (for filters)
	},
	{
	  g_ps3PS3EyeSource8,               // Name
	  &CLSID_PS3EyeSource8,             // CLSID
	  PS3EyeSource::CreateInstance8,    // Method to create an instance of MyComponent
	  NULL,                             // Initialization function
	  NULL                              // Set-up information (for filters)
	}	
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

const REGFILTERPINS2 sudOutputPinPS3Eye2 =
{
	REG_PINFLAG_B_OUTPUT,
	1,
	1,
	&sudOpPinTypes,
	0,
	NULL,
	&PIN_CATEGORY_CAPTURE
};

const REGFILTERPINS2 sudOutputPinPS3EyeA2 =
{
	REG_PINFLAG_B_OUTPUT,
	1,
	1,
	&sudOpPinTypesA,
	0,
	NULL,
	&PIN_CATEGORY_CAPTURE
};

/*
const REGFILTER2 sudPushSourcePS3Eye = {
	1,
	MERIT_NORMAL,
	1,
	&sudOutputPinPS3Eye
};
*/

////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

STDAPI RegisterFilters(BOOL bRegister, int filterCount)
{
	HRESULT hr;
	IFilterMapper2 *pFM2 = NULL;
	if (bRegister)
	{
		hr = AMovieDllRegisterServer2(TRUE);
		if (FAILED(hr))
			return hr;

		hr = CoCreateInstance(CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER,
			IID_IFilterMapper2, (void **)&pFM2);

		if (FAILED(hr))
			return hr;

		REGFILTER2 sudPushSourcePS3Eye;
		sudPushSourcePS3Eye.dwVersion = 2;
		sudPushSourcePS3Eye.dwMerit = MERIT_NORMAL;
		sudPushSourcePS3Eye.cPins2 = 1;
		sudPushSourcePS3Eye.rgPins2 = &sudOutputPinPS3EyeA2;

		hr = pFM2->RegisterFilter(
			*(g_Templates[0].m_ClsID),           // Filter CLSID. 
			g_Templates[0].m_Name,               // Filter name.
			NULL,								 // Device moniker. 
			&CLSID_AudioInputDeviceCategory,     // Input device category.
			g_Templates[0].m_Name,               // Instance data.
			&sudPushSourcePS3Eye                 // Pointer to filter information.
		);

		sudPushSourcePS3Eye;
		sudPushSourcePS3Eye.dwVersion = 2;
		sudPushSourcePS3Eye.dwMerit = MERIT_NORMAL;
		sudPushSourcePS3Eye.cPins2 = 1;
		sudPushSourcePS3Eye.rgPins2 = &sudOutputPinPS3Eye2;

		for (int i = 1; i < filterCount + 1; i++)
		{
			hr = pFM2->RegisterFilter(
				*(g_Templates[i].m_ClsID),           // Filter CLSID. 
				g_Templates[i].m_Name,               // Filter name.
				NULL,								 // Device moniker. 
				&CLSID_VideoInputDeviceCategory,     // Input device category.
				g_Templates[i].m_Name,               // Instance data.
				&sudPushSourcePS3Eye                 // Pointer to filter information.
			);
		}
	}
	else
	{
		hr = AMovieDllRegisterServer2(FALSE);
		if (FAILED(hr))
			return hr;

		hr = CoCreateInstance(CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER,
			IID_IFilterMapper2, (void **)&pFM2);

		if (FAILED(hr))
			return hr;

		hr = pFM2->UnregisterFilter(&CLSID_AudioInputDeviceCategory, g_Templates[0].m_Name, *(g_Templates[0].m_ClsID));

		for (int i = 1; i < filterCount + 1; i++)
		{
			hr = pFM2->UnregisterFilter(&CLSID_VideoInputDeviceCategory, g_Templates[i].m_Name, *(g_Templates[i].m_ClsID));
		}
	}

	pFM2->Release();
	return hr;
}

STDAPI DllRegisterServer()
{
	return RegisterFilters(TRUE, 1);
}

STDAPI DllUnregisterServer()
{
	return RegisterFilters(FALSE, MAX_DEVICE_COUNT);
}

STDAPI DllInstall(BOOL bInstall, _In_opt_ LPCWSTR pszCmdLine)
{
	if (!bInstall)
		return RegisterFilters(FALSE, MAX_DEVICE_COUNT);
	else if (lstrcmpW(pszCmdLine, L"1") == 0)
		return RegisterFilters(TRUE, 1);
	else if (lstrcmpW(pszCmdLine, L"2") == 0)
		return RegisterFilters(TRUE, 2);
	else if (lstrcmpW(pszCmdLine, L"3") == 0)
		return RegisterFilters(TRUE, 3);
	else if (lstrcmpW(pszCmdLine, L"4") == 0)
		return RegisterFilters(TRUE, 4);
	else if (lstrcmpW(pszCmdLine, L"5") == 0)
		return RegisterFilters(TRUE, 5);
	else if (lstrcmpW(pszCmdLine, L"6") == 0)
		return RegisterFilters(TRUE, 6);
	else if (lstrcmpW(pszCmdLine, L"7") == 0)
		return RegisterFilters(TRUE, 7);
	else
		return RegisterFilters(TRUE, MAX_DEVICE_COUNT);
}

//
// DllEntryPoint
//

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule,
	DWORD  dwReason,
	LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}