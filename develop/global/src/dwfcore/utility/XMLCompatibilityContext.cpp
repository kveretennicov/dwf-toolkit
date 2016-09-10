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
//  $Header: //DWF/Development/Components/Internal/DWF Core/v1.7/develop/global/src/dwfcore/utility/XMLCompatibilityContext.cpp#1 $
//  $DateTime: 2011/02/14 01:23:46 $
//  $Author: caos $
//  $Change: 197965 $
//  $Revision: #1 $
//
#include "XMLCompatibilityContext.h"

namespace DWFCore
{

DWFXMLNamespaceCollection 
DWFXMLCompatibilityContext::koNullNamespaces;

DWFXMLQualifiedNameCollection
DWFXMLCompatibilityContext::koNullNames(DWFXMLCompatibilityContext::koNullNamespaces);


DWFXMLCompatibilityContext::DWFXMLCompatibilityContext( DWFXMLCompatibilityProcessTransferable& rProcess )
    throw()
    : _rProcess( rProcess )
    , _rIgnoredNs( koNullNamespaces )
    , _rProcessContent( koNullNames )
{
    (void)rProcess;
}


DWFXMLCompatibilityContext::DWFXMLCompatibilityContext( DWFXMLCompatibilityProcessTransferable& rProcess,
                                                        DWFXMLNamespaceCollection& rIgnoredNs,
                                                        DWFXMLQualifiedNameCollection& rProcessContent )
    throw()
    : _rProcess( rProcess )
    , _rIgnoredNs( rIgnoredNs )
    , _rProcessContent( rProcessContent )
{
    (void)rProcess;
}


DWFXMLCompatibilityContext::~DWFXMLCompatibilityContext()
{
}


DWFXMLCompatibilityProcessTransferable& 
DWFXMLCompatibilityContext::process()
    throw()
{
    return _rProcess;
}


DWFXMLNamespaceCollection&  
DWFXMLCompatibilityContext::ignoredNamespaces()
    throw()
{
    return _rIgnoredNs;
}


DWFXMLQualifiedNameCollection& 
DWFXMLCompatibilityContext::processedElements()
    throw()
{
    return _rProcessContent;
}


}
