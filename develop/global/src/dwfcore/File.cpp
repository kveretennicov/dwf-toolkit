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

#include "dwfcore/File.h"
#include "dwfcore/CountedObject.h"
using namespace DWFCore;


_DWFCORE_API
DWFFile::DWFFile( const ASCII_char_t* zName )
throw()
        : _zName( zName )
{
}

_DWFCORE_API
DWFFile::DWFFile( const DWFString& rName )
throw()
        : _zName( rName )
{
    ;
}

_DWFCORE_API
DWFFile::DWFFile( const DWFFile& rFile )
throw()
        : _zName( rFile._zName )
{
    ;
}

_DWFCORE_API
DWFFile&
DWFFile::operator=( const DWFFile& rFile )
throw()
{
    _zName = rFile._zName;

    return *this;
}

_DWFCORE_API
DWFFile::~DWFFile()
throw()
{
    ;
}

_DWFCORE_API
const DWFString&
DWFFile::name() const
throw( DWFException )
{
    return _zName;
}
