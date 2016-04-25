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

#define BUFFER_SIZE		(1024 * 1024 * 1024)

DWORD WINAPI ReciveProc(LPVOID lpParameter);

HANDLE hCOM;
HANDLE hThread;

BYTE g_buffer[BUFFER_SIZE];
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

	hThread = CreateThread(NULL, 0, ReciveProc, NULL, 0, NULL);

	return TRUE;
	// normal function body here
}

DWORD WINAPI ReciveProc(LPVOID lpParameter)
{
	g_size = 0;
	char recv_buf[1024] = {0};

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
			if (g_size + dwReadSize < BUFFER_SIZE)
			{
				memcpy(&g_buffer[g_size], recv_buf, dwReadSize);
				g_size += dwReadSize;
				memset(recv_buf, 0, 1024);
			}
			bReadStatus = FALSE;
		}

	}
	return 0;
}

ExportC void ReadData(char* str, int &len)
{
	DWORD ExitCode;
	GetExitCodeThread(hThread, &ExitCode);
	TerminateThread(hThread, ExitCode);
	CloseHandle(hThread);
	CloseHandle(hCOM);

	memcpy(str, g_buffer, g_size);
	len = g_size;
}