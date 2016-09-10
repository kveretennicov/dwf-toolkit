//
//  Copyright (c) 2003-2006 by Autodesk, Inc.
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

#include "dwfcore/XML.h"


//////////

_DWFCORE_API
void
DWFXMLBuildable::parseAttributeList( const char** /*ppAttributeList*/, 
                                     tUnresolvedList& /*rUnresolved*/ )
throw( DWFException )
{
    ;
}

//////////

DWFXMLSerializableBase::DWFXMLSerializableBase( const DWFString& zDefaultNamespace )
throw()
                  : _zDefaultNamespace( zDefaultNamespace )
{
    ;
}

///////////

_DWFCORE_API
DWFXMLNamespaceBase::DWFXMLNamespaceBase( const DWFString& zNamespace, const DWFString& zXMLNS )
throw( DWFException )
                   : _zPrefix( zNamespace )
                   , _zXMLNS( zXMLNS )
{
        //
        // all parameters are required
        //
    if ((_zPrefix.chars() == 0) || (_zXMLNS.chars() == 0))
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"The full namespace definition must be provided." );
    }
}

