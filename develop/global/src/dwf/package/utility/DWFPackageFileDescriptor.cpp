//
//  Copyright (c) 2004-2006 by Autodesk, Inc. 
// 
//  By using this code, you are agreeing to the terms and conditions of 
//  the License Agreement included in the documentation for this code. 
// 
//  AUTODESK MAKES NO WARRANTIES, EXPRESSED OR IMPLIED, 
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


#ifndef DWFTK_READ_ONLY

#include "dwf/Version.h"
#include "dwf/package/utility/DWFPackageFileDescriptor.h"
using namespace DWFToolkit;


_DWFTK_API
DWFPackageFileDescriptor::DWFPackageFileDescriptor( const DWFFile&                   rFile,
                                                    DWFZipFileDescriptor::teFileMode eMode,
                                                    unsigned short                   nVersionMajor,
                                                    unsigned short                   nVersionMinor )
throw()
                        : DWFZipFileDescriptor( rFile, eMode )
                        , _nMajor( nVersionMajor )
                        , _nMinor( nVersionMinor )
{
    ;
}

_DWFTK_API
DWFPackageFileDescriptor::~DWFPackageFileDescriptor()
throw()
{
    ;
}

_DWFTK_API
void
DWFPackageFileDescriptor::open()
throw( DWFException )
{
    if (_eMode != eUnzip)
    {
        if (_pZipStream)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Zip archive already open for zipping" );
        }

        _pZipStream = dwfOpen( _oFile.name(), _nMajor, _nMinor );

        if (_pZipStream == NULL)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Failed to open DWF package for writing" );
        }
    }
}

#endif
