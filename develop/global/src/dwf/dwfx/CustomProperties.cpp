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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/dwfx/CustomProperties.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/dwfx/CustomProperties.h"
#include "dwf/dwfx/Constants.h"
#include "dwf/package/Constants.h"
using namespace DWFToolkit;


//DNT_Start

#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)

const char* const DWFXCustomProperties::kzName  = "CustomProperties.xml";

#endif


_DWFTK_API
DWFXCustomProperties::DWFXCustomProperties()
throw( DWFException )
                    : _oPropertySet()
{
    ;
}

_DWFTK_API
DWFXCustomProperties::~DWFXCustomProperties()
throw()
{
    // As an ownable class, we need to notify observers that we are about to be deleted
    _notifyDelete();
}

_DWFTK_API
void
DWFXCustomProperties::addProperty( DWFProperty* pProperty,
                                   bool         bOwnProperty )
throw( DWFException )
{
    _oPropertySet.addProperty( pProperty, bOwnProperty );
}

_DWFTK_API
DWFProperty*
DWFXCustomProperties::addProperty( const DWFString& zName,
                                   const DWFString& zValue,
                                   const DWFString& zCategory,
                                   const DWFString& zType,
                                   const DWFString& zUnits )
throw( DWFException )
{
    return _oPropertySet.addProperty( zName, zValue, zCategory, zType, zUnits );
}

_DWFTK_API
void
DWFXCustomProperties::removeProperty( DWFProperty* pProperty,
                                      bool         bDeleteIfOwned )
throw( DWFException )
{
    _oPropertySet.removeProperty( pProperty, bDeleteIfOwned );
}

_DWFTK_API
const DWFProperty* const
DWFXCustomProperties::findProperty( const DWFString& zName,
                                    const DWFString& zCategory )
throw()
{
    return _oPropertySet.findProperty( zName, zCategory );
}

_DWFTK_API
DWFProperty::tMap::Iterator*
DWFXCustomProperties::getProperties( const DWFString& zCategory )
throw()
{
    return _oPropertySet.getProperties( zCategory );
}


#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFXCustomProperties::serializeXML( DWFXMLSerializer& rSerializer )
throw( DWFException )
{
    rSerializer.emitXMLHeader();

    _oPropertySet.getSerializable().serializeXML( rSerializer, 0 );
}

#endif

_DWFTK_API
DWFProperty*
DWFXCustomProperties::provideProperty( DWFProperty* pProperty )
throw()
{
    _oPropertySet.addProperty( pProperty, true );
    return pProperty;
}


//DNT_End

