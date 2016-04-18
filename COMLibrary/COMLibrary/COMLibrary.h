// COMLibrary.h : main header file for the COMLibrary DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#define ExportC extern "C" __declspec(dllexport)
// CCOMLibraryApp
// See COMLibrary.cpp for the implementation of this class
//

class CCOMLibraryApp : public CWinApp
{
public:
	CCOMLibraryApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
