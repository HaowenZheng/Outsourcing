
// Demo_com.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CDemo_comApp:
// See Demo_com.cpp for the implementation of this class
//

class CDemo_comApp : public CWinApp
{
public:
	CDemo_comApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CDemo_comApp theApp;