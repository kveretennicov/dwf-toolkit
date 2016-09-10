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
#include "dwf/package/writer/extensions/6.0/PackageVersionExtension.h"
using namespace DWFToolkit;


_DWFTK_API
DWFPackageVersion60Extension::DWFPackageVersion60Extension()
throw()
{
    ;
}

_DWFTK_API
DWFPackageVersion60Extension::~DWFPackageVersion60Extension()
throw()
{
    ;
}

_DWFTK_API
bool
DWFPackageVersion60Extension::addSection( DWFSection*   /*pSection*/,
                                                DWFInterface* /*pInterface*/ )
throw( DWFException )
{
    return true;
}

_DWFTK_API
bool
DWFPackageVersion60Extension::addGlobalSection( DWFGlobalSection* /*pSection*/ )
throw( DWFException )
{
    return true;
}

_DWFTK_API
bool
DWFPackageVersion60Extension::write( const DWFString& /*zSourceProductVendor*/,
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
DWFPackageVersion60Extension::prewriteManifest( DWF6PackageWriter&          /*rPackageWriter*/, 
                                                DWFPackageFileDescriptor&  /*rPackageDescriptor*/,
                                                DWFXMLSerializer&          /*rXMLSerializer*/,
                                                const DWFString&           /*rPackagePassword*/ )
throw( DWFException )
{
    ;
}

_DWFTK_API
void
DWFPackageVersion60Extension::postwriteManifest( DWF6PackageWriter&         /*rPackageWriter*/,
                                                 DWFPackageFileDescriptor& /*rPackageDescriptor*/,
                                                 DWFXMLSerializer&         /*rXMLSerializer*/,
                                                 const DWFString&          /*rPackagePassword*/ )
throw( DWFException )
{
    ;
}

_DWFTK_API
void
DWFPackageVersion60Extension::postwriteSections( DWF6PackageWriter&         /*rPackageWriter*/, 
                                                 DWFPackageFileDescriptor& /*rPackageDescriptor*/,
                                                 DWFXMLSerializer&         /*rXMLSerializer*/,
                                                 const DWFString&          /*rPackagePassword*/ )
throw( DWFException )
{
    ;
}


#endif

