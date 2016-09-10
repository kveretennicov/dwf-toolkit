//
//  Copyright (c) 2004-2006 by Autodesk, Inc.
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



#ifndef DWFTK_READ_ONLY


#include "dwfcore/MIME.h"
using namespace DWFCore;

#include "dwf/Version.h"
#include "dwf/package/XML.h"
#include "dwf/package/Constants.h"
#include "dwf/package/Interface.h"
#include "dwf/package/EPlotSection.h"
#include "dwf/package/EModelSection.h"
#include "dwf/package/GlobalSection.h"
#include "dwf/package/writer/extensions/6.20/PackageVersionExtension.h"
using namespace DWFToolkit;


_DWFTK_API
DWFPackageVersion620Extension::DWFPackageVersion620Extension( const DWFString& zDocumentType )
throw()
                                  : _zDocumentType( zDocumentType )
{
    ;
}

_DWFTK_API
DWFPackageVersion620Extension::~DWFPackageVersion620Extension()
throw()
{
    ;
}

_DWFTK_API
bool
DWFPackageVersion620Extension::addSection( DWFSection*   /*pSection*/,
                                                DWFInterface* /*pInterface*/ )
throw( DWFException )
{
    return true;
}

_DWFTK_API
bool
DWFPackageVersion620Extension::addGlobalSection( DWFGlobalSection* /*pSection*/ )
throw( DWFException )
{
    return true;
}

_DWFTK_API
bool
DWFPackageVersion620Extension::write( const DWFString& /*zSourceProductVendor*/,
                                      const DWFString& /*zSourceProductName*/,
                                      const DWFString& /*zSourceProductVersion*/,
                                      const DWFString& /*zDWFProductVendor*/,
                                      const DWFString& /*zDWFProductVersion*/,
                                      DWFZipFileDescriptor::teFileMode /*eCompressionMode*/ )
throw( DWFException )
{
    return true;
}

_DWFTK_API
void
DWFPackageVersion620Extension::prewriteManifest( DWF6PackageWriter&          /*rPackageWriter*/, 
                                                 DWFPackageFileDescriptor&  rPackageDescriptor,
                                                 DWFXMLSerializer&          /*rXMLSerializer*/,
                                                 const DWFString&           rPackagePassword )
throw( DWFException )
{
    //
    // organize the zip archive in an optimized order:
    //
    //  *.TYPEINFO
    //      - This indicates that any files with the extension of .TYPEINFO should be placed first in the archive.
    //  manifest.xml
    //      - This indicates that the manifest.xml file is to be placed next.
    //  com.autodesk.dwf.ePlotGlobal, com.autodesk.dwf.ePlot, *
    //      - This indicates that the remaining files are to be ordered by section type,
    //          where files in eplot sections will come before files in any other sections.
    //      - This orders resources by plot order,
    //          for those resources belonging to a section contianing a descriptor which specifies a "Page" element contining a "plotOrder".
    //          Descriptors will still appear before thumbnails, but this will order the files within their role groups based on plot Order
    //  descriptor, thumbnail, font, 2d streaming graphics, RML markup, *
    //      - This indicates that resources which have the role "font",
    //          then "2d streaming graphics" should be placed before other files [in the eplot section group]
    //          (but after the descriptors and thumbnails)
    //  Any files which otherwise match in all respects will be ordered so that the smaller files come first.
    //

        //
        // If a document type has been specified, create a typeinfo of that type.
        //
    if (_zDocumentType.bytes() != 0)
    {
        //
        // The document type is the name of the TYPEINFO file
        //
        _zDocumentType.append( /*NOXLATE*/L".TYPEINFO" );

        //
        // create the file
        //
        DWFOutputStream* pFilestream = rPackageDescriptor.zip( _zDocumentType, rPackagePassword );

        //
        // this file is empty, so just delete the stream (and close the file)
        //
        DWFCORE_FREE_OBJECT( pFilestream );
    }
}

_DWFTK_API
void
DWFPackageVersion620Extension::postwriteManifest( DWF6PackageWriter&         /*rPackageWriter*/,
                                                  DWFPackageFileDescriptor& /*rPackageDescriptor*/,
                                                  DWFXMLSerializer&         /*rXMLSerializer*/,
                                                  const DWFString&          /*rPackagePassword*/ )
throw( DWFException )
{
    ;
}

_DWFTK_API
void
DWFPackageVersion620Extension::postwriteSections( DWF6PackageWriter&         /*rPackageWriter*/, 
                                                  DWFPackageFileDescriptor& /*rPackageDescriptor*/,
                                                  DWFXMLSerializer&         /*rXMLSerializer*/,
                                                  const DWFString&          /*rPackagePassword*/ )
throw( DWFException )
{
    ;
}


#endif

