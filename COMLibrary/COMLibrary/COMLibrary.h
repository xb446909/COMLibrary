// COMLibrary.h : main header file for the COMLibrary DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

#ifdef _USRDLL
#define ExportC extern "C" __declspec(dllexport)
#else
#define ExportC extern "C" __declspec(dllimport)
#endif
// CCOMLibraryApp
// See COMLibrary.cpp for the implementation of this class
//

#define NOPARITY            0
#define ODDPARITY           1
#define EVENPARITY          2
#define MARKPARITY          3
#define SPACEPARITY         4

#define ONESTOPBIT          0
#define ONE5STOPBITS        1
#define TWOSTOPBITS         2

ExportC BOOL OpenCOM(LPCSTR com, DWORD baudrate, UCHAR bytesize, UCHAR parity, UCHAR stopbits);
ExportC void ReadData(char* str, int &len);

class CCOMLibraryApp : public CWinApp
{
public:
	CCOMLibraryApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
