//
//  Copyright (c) 2007 by Autodesk, Inc.
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

#include "XAML/pch.h"

WT_Result WT_XAML_DWF_Header::serialize( WT_File& ) const
{
    return WT_Result::Toolkit_Usage_Error;
}

WT_Result
WT_XAML_DWF_Header::parseAttributeList( XamlXML::tAttributeMap& rMap, WT_XAML_File& rFile )
{
    if ( rMap.size() == 0 )
    {
        return WT_Result::Corrupt_File_Error;
    }

    const char** ppValue = rMap.find( XamlXML::kpzVersion_Major_Attribute );
    WD_Assert( ppValue != NULL && *ppValue != NULL );
    if ( ppValue == NULL || *ppValue == NULL )
    {
        return WT_Result::Corrupt_File_Error;
    }
    else
    {
        set_major_revision( rFile, atol( *ppValue ) );
    }

    ppValue = rMap.find( XamlXML::kpzVersion_Minor_Attribute );
    WD_Assert( ppValue != NULL && *ppValue != NULL );
    if ( ppValue == NULL || *ppValue == NULL )
    {
        return WT_Result::Corrupt_File_Error;
    }
    else
    {
        set_minor_revision( rFile, atol( *ppValue ) );
    }

    if ( rFile.rendition().drawing_info().major_revision() > rFile.toolkit_major_revision() )
    {
        return WT_Result::DWF_Version_Higher_Than_Toolkit;
    }
    else if ( rFile.rendition().drawing_info().decimal_revision() < REVISION_WHEN_DWFX_FORMAT_BEGINS )
    {
        return WT_Result::Toolkit_Usage_Error;
    }
    else if ( rFile.rendition().drawing_info().minor_revision() > rFile.toolkit_minor_revision() )
    {
        return WT_Result::Minor_Version_Warning;
    }

    materialized() = WD_True;
    return WT_Result::Success;
}