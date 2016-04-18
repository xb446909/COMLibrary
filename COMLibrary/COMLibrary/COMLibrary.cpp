// COMLibrary.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "COMLibrary.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CCOMLibraryApp

BEGIN_MESSAGE_MAP(CCOMLibraryApp, CWinApp)
END_MESSAGE_MAP()

DWORD WINAPI ReciveProc(LPVOID lpParameter);

HANDLE hCOM;
HANDLE hThread;

BYTE g_buffer[1024 * 1024 * 1024];
DWORD g_size;


// CCOMLibraryApp construction

CCOMLibraryApp::CCOMLibraryApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CCOMLibraryApp object

CCOMLibraryApp theApp;


// CCOMLibraryApp initialization

BOOL CCOMLibraryApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}


ExportC BOOL OpenCOM(LPCSTR com, DWORD baudrate, UCHAR bytesize, UCHAR parity, UCHAR stopbits)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	hCOM = CreateFileA(com, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hCOM == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	SetupComm(hCOM, 1024, 1024);
	DCB dcb;
	GetCommState(hCOM, &dcb);
	dcb.DCBlength = sizeof(DCB);
	dcb.BaudRate = baudrate;
	dcb.ByteSize = bytesize;
	dcb.Parity = parity;
	dcb.StopBits = stopbits;

	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = INFINITE;
	TimeOuts.ReadTotalTimeoutMultiplier = 0;
	TimeOuts.ReadTotalTimeoutConstant = 0;

	SetCommTimeouts(hCOM, &TimeOuts);
	SetCommState(hCOM, &dcb);
	PurgeComm(hCOM, PURGE_TXCLEAR | PURGE_RXCLEAR);
	return TRUE;
	// normal function body here
}

DWORD WINAPI ReciveProc(LPVOID lpParameter)
{
	g_size = 0;

	while (1)
	{
		DWORD dwReadSize = 0;
		int bReadStatus;
		while (!dwReadSize)
		{
			bReadStatus = ReadFile(hCOM, recv_buf, 1024, &dwReadSize, NULL);
		}
		if (bReadStatus)
		{
			EditBoxAppendText(hEditBox, CA2W(recv_buf));
			bReadStatus = FALSE;
		}

	}
	return 0;
}