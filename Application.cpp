// Application.cpp: implementation of the Application class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CS580HW.h"
#include "Application.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Application::Application()
{
	int index = 0;

	for(index = 0; index < AAKERNEL_SIZE; index++){
		m_pDisplay[index] = NULL;		// the display
		m_pRender[index] = NULL;		// the renderer
	}
	m_pFinalDisplay = NULL;
	m_pUserInput = NULL;
	m_pFrameBuffer = NULL;
}

Application::~Application()
{
	if(m_pFrameBuffer != NULL)
		delete m_pFrameBuffer;
}

