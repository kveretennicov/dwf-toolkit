//
//  Copyright (c) 2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED, AS TO THE CORRECTNESS
//  OF THIS CODE OR ANY DERIVATIVE WORKS WHICH INCORPORATE IT. AUTODESK
//  PROVIDES THE CODE ON AN "AS-IS" BASIS AND EXPLICITLY DISCLAIMS ANY
//  LIABILITY, INCLUDING CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/dwfx/ResourcePart.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwfcore/MIME.h"
using namespace DWFCore;

#include "dwf/dwfx/ResourcePart.h"
#include "dwf/package/Resource.h"
using namespace DWFToolkit;


_DWFTK_API
DWFXResourcePart::DWFXResourcePart( DWFResource* pResource )
throw()
                : _pResource( pResource )
{
	// <TODO> Remove or uncomment - Sriram's code - commented out by Basu 
	// since the name will be assigned later.
    //DWFString zResourceName = pResource->publishedIdentity();
    //off_t nOffset = zResourceName.find( L'\\', 0, true );
    //zResourceName = zResourceName.substring( (size_t)nOffset + 1, (size_t)-1 );
    //setName( zResourceName );                    
}

_DWFTK_API
DWFXResourcePart::~DWFXResourcePart()
throw()
{
    // As an ownable class, we need to notify observers that we are about to be deleted
    _notifyDelete();
}

_DWFTK_API
DWFInputStream*
DWFXResourcePart::getInputStream()
throw()
{
    return _pResource->getInputStream();
}



