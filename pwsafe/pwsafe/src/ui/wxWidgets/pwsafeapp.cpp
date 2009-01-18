/////////////////////////////////////////////////////////////////////////////
// Name:        pwsafeapp.cpp
// Purpose:     
// Author:      Rony Shapiro
// Modified by: 
// Created:     Wed 14 Jan 2009 10:11:38 PM IST
// RCS-ID:      
// Copyright:   Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "pwsafeapp.h"

////@begin XPM images
////@end XPM images


/*!
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( PwsafeApp )
////@end implement app


/*!
 * PwsafeApp type definition
 */

IMPLEMENT_CLASS( PwsafeApp, wxApp )


/*!
 * PwsafeApp event table definition
 */

BEGIN_EVENT_TABLE( PwsafeApp, wxApp )

////@begin PwsafeApp event table entries
////@end PwsafeApp event table entries

END_EVENT_TABLE()


/*!
 * Constructor for PwsafeApp
 */

PwsafeApp::PwsafeApp()
{
    Init();
}


/*!
 * Member initialisation
 */

void PwsafeApp::Init()
{
////@begin PwsafeApp member initialisation
////@end PwsafeApp member initialisation
}

/*!
 * Initialisation for PwsafeApp
 */

bool PwsafeApp::OnInit()
{    
////@begin PwsafeApp initialisation
	// Remove the comment markers above and below this block
	// to make permanent changes to the code.

#if wxUSE_XPM
	wxImage::AddHandler(new wxXPMHandler);
#endif
#if wxUSE_LIBPNG
	wxImage::AddHandler(new wxPNGHandler);
#endif
#if wxUSE_LIBJPEG
	wxImage::AddHandler(new wxJPEGHandler);
#endif
#if wxUSE_GIF
	wxImage::AddHandler(new wxGIFHandler);
#endif
	CSafeCombinationEntry* mainWindow = new CSafeCombinationEntry(NULL);
	/* int returnValue = */ mainWindow->ShowModal();

	mainWindow->Destroy();
	// A modal dialog application should return false to terminate the app.
	return false;
////@end PwsafeApp initialisation

    return true;
}


/*!
 * Cleanup for PwsafeApp
 */

int PwsafeApp::OnExit()
{    
////@begin PwsafeApp cleanup
	return wxApp::OnExit();
////@end PwsafeApp cleanup
}

