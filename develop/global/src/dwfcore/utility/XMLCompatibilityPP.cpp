//
//  Copyright (c) 2004-2006 by Autodesk, Inc.  All rights reserved.
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted,
// provided that the above copyright notice appears in all copies and
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM 'AS IS' AND WITH ALL FAULTS.
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//
//  $Header: //DWF/Development/Components/Internal/DWF Core/v1.7/develop/global/src/dwfcore/utility/XMLCompatibilityPP.cpp#1 $
//  $DateTime: 2011/02/14 01:23:46 $
//  $Author: caos $
//  $Change: 197965 $
//  $Revision: #1 $
//
#include "dwfcore/utility/XMLCompatibilityPP.h"

#include "dwfcore/utility/XMLNamespaceCollection.h"
#include "dwfcore/utility/XMLQualifiedNameCollection.h"

namespace DWFCore
{

_DWFCORE_API
DWFXMLCompatibilityPP::DWFXMLCompatibilityPP( DWFXMLCallback& rTarget )
    throw()
{
    (void)rTarget;
}


_DWFCORE_API
DWFXMLCompatibilityPP::~DWFXMLCompatibilityPP() throw()
{
}


_DWFCORE_API
void 
DWFXMLCompatibilityPP::signalError() throw( DWFXMLCompatibilityException )
{
}


_DWFCORE_API
void 
DWFXMLCompatibilityPP::addSupportedNS( const DWFCore::DWFString& zUri,
                                       const tStringIt* pStrings )
    throw()
{
    (void)zUri;
    (void)pStrings;
}


_DWFCORE_API
void 
DWFXMLCompatibilityPP::addExtensionElementName( const DWFCore::DWFString& zQualName )
    throw()
{
    (void)zQualName;
}


_DWFCORE_API
void 
DWFXMLCompatibilityPP::notifyStartElement( const char*   zName,
                                           const char**  ppAttributeList )
    throw()
{
    (void)zName;
    (void)ppAttributeList;
}


_DWFCORE_API
void 
DWFXMLCompatibilityPP::notifyEndElement( const char*     zName )
    throw()
{
    (void) zName;
}

}
