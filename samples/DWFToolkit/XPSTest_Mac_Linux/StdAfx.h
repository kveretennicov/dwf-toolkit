//
//  Copyright (c) 2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED,
//  AS TO THE CORRECTNESS OF THIS CODE OR ANY DERIVATIVE
//  WORKS WHICH INCORPORATE IT.
//
//  AUTODESK PROVIDES THE CODE ON AN "AS-IS" BASIS
//  AND EXPLICITLY DISCLAIMS ANY LIABILITY, INCLUDING
//  CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__8688457C_8424_45D2_B161_C3AA30FC279B__INCLUDED_)
#define AFX_STDAFX_H__8688457C_8424_45D2_B161_C3AA30FC279B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <stdio.h>
#if defined(_DWFCORE_WIN32_SYSTEM)
#include <io.h>
#include <direct.h>
#endif
#include <iostream>

#include "dwfcore/File.h"
#include "dwfcore/StreamFileDescriptor.h"
#include "dwfcore/FileOutputStream.h"
#include "dwfcore/FileInputStream.h"
#include "dwfcore/BufferInputStream.h"
#include "dwfcore/BufferOutputStream.h"
#include "dwfcore/String.h"
#include "dwfcore/MIME.h"
#include "dwfcore/BufferOutputStream.h"
#include "dwfcore/StreamFileDescriptor.h"
#include "dwfcore/FileInputStream.h"

#include "dwf/whiptk/whip_toolkit.h"

#include "XAML/XamlFile.h"


#include "dwf/Version.h"
#include "dwf/package/Constants.h"
#include "dwf/package/EPlotSection.h"
#include "dwf/package/DefinedObject.h"
#include "dwf/package/writer/DWFXPackageWriter.h"
#include "dwf/package/writer/DWF6PackageWriter.h"

#define ERR_CHECK(X) if ( (X) != WT_Result::Success ) { WD_Assert( false ); return; }
#define ERR_NULLCHECK(X) if ( (X) == NULL ) { WD_Assert( false ); return; }
#define WD_NULLCHECK(X) if ( (X) == WD_Null ) { WD_Assert( false ); return WT_Result::Out_Of_Memory_Error; }

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__8688457C_8424_45D2_B161_C3AA30FC279B__INCLUDED_)
