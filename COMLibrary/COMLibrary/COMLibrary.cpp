// COMLibrary.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "COMLibrary.h"
#include <map>

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
using namespace std;

BEGIN_MESSAGE_MAP(CCOMLibraryApp, CWinApp)
END_MESSAGE_MAP()

#define BUFFER_SIZE		(1024 * 1024 * 1024)

#define  UM_QUIT	(WM_USER + 1)

DWORD WINAPI ReciveProc(LPVOID lpParameter);

//HANDLE g_hCOM;
HANDLE hThread;

//BYTE g_buffer[BUFFER_SIZE];
DWORD g_size;

//RecvCallback g_Callback;
//static map<int, HANDLE> g_mapCom;//当打开多个COM， COM的handle存放之处

typedef struct _tagComParameter
{
	HANDLE hCOM;
	HANDLE hThread;
	HANDLE hEvent;
	DWORD dwThreadID;
	DWORD dwReadSize;
	RecvCallback pCallback;
	BYTE szDataBuff[BUFFER_SIZE];
}ComParameter, *pComParameter;

map<int, pComParameter> g_mapComPara;
HANDLE g_mutex;
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

pComParameter FindComParam(int nID)
{
	auto it = g_mapComPara.find(nID);
	if (it == g_mapComPara.end())
	{
		return nullptr;
	}
	return it->second;
}

void UninitCom(int nComNum)
{
	//WaitForSingleObject(g_mutex, INFINITE);

	pComParameter pComPara;
	pComPara = FindComParam(nComNum);
	if (pComPara == nullptr)
	{
		return;
	}
	PostThreadMessage(pComPara->dwThreadID, UM_QUIT, 0, 0);
	CloseHandle(pComPara->hCOM);

	delete pComPara;
	g_mapComPara.erase(nComNum);

	//ReleaseMutex(g_mutex);
}

ExportC BOOL OpenCOM(int nCom, DWORD baudrate, UCHAR bytesize, UCHAR parity, UCHAR stopbits, RecvCallback pCallback)
{
	CString strCom;
	strCom.Format(L"COM%d", nCom);
	//g_mutex = CreateMutex(NULL, FALSE, NULL);

	pComParameter pComPara = FindComParam(nCom);
	if (pComPara != nullptr) return false;
	pComPara = new ComParameter;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	pComPara->hCOM = CreateFileA(CT2A(strCom), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (pComPara->hCOM == INVALID_HANDLE_VALUE)
		return FALSE;
	
	pComPara->pCallback = pCallback;
	g_mapComPara.insert(pair<int, pComParameter>(nCom, pComPara));

	SetupComm(pComPara->hCOM, 1024, 1024);
	DCB dcb;
	GetCommState(pComPara->hCOM, &dcb);
	dcb.DCBlength = sizeof(DCB);
	dcb.BaudRate = baudrate;
	dcb.ByteSize = bytesize;
	dcb.Parity = parity;
	dcb.StopBits = stopbits;

	COMMTIMEOUTS TimeOuts;
	TimeOuts.ReadIntervalTimeout = INFINITE;
	TimeOuts.ReadTotalTimeoutMultiplier = 0;
	TimeOuts.ReadTotalTimeoutConstant = 0;

	SetCommTimeouts(pComPara->hCOM, &TimeOuts);
	SetCommState(pComPara->hCOM, &dcb);
	PurgeComm(pComPara->hCOM, PURGE_TXCLEAR | PURGE_RXCLEAR);//清除一下缓冲区

	hThread = CreateThread(NULL, 0, ReciveProc, (LPVOID)nCom, 0, &pComPara->dwThreadID);

	WaitForSingleObject(pComPara->hEvent, INFINITE);
	ResetEvent(pComPara->hEvent);

	return TRUE;
	// normal function body here
}

DWORD WINAPI ReciveProc(LPVOID lpParameter)
{
	g_size = 0;
	char recv_buf[1024] = {0};
	pComParameter pComPara = FindComParam((int)lpParameter);
	if (pComPara == nullptr)
		return FALSE;

	MSG msg;
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
	SetEvent(pComPara->hEvent);

	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == UM_QUIT)
			{
				UninitCom((int)lpParameter);
				AfxMessageBox(L"Stop Thread");
				return TRUE;
			}
		}
		DWORD dwReadSize = 0;
		int bReadStatus;
		while (!dwReadSize)
		{
			bReadStatus = ReadFile(pComPara->hCOM, recv_buf, 1024, &dwReadSize, NULL);
		}
		if (bReadStatus)
		{
			if (g_size + dwReadSize < BUFFER_SIZE)
			{
				memcpy(&pComPara->szDataBuff[g_size], recv_buf, dwReadSize);
				pComPara->dwReadSize = dwReadSize;
				if (pComPara->pCallback != NULL)
					pComPara->pCallback((int)lpParameter, recv_buf, dwReadSize);
				g_size += dwReadSize;
				memset(recv_buf, 0, 1024);
			}
			bReadStatus = FALSE;
		}

	}
	return 0;
}

ExportC void CloseData(int nCom)
{
	//DWORD ExitCode;
	//GetExitCodeThread(hThread, &ExitCode);
	//TerminateThread(hThread, ExitCode);
	//CloseHandle(hThread);
	//CloseHandle(g_mapCom[nCom]);
}