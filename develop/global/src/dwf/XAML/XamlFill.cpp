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

#include "XAML/pch.h"
#include "dwfcore/DWFXMLSerializer.h"

WT_Result WT_XAML_Fill::serialize(WT_File& file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Fill::serialize( *rFile.w2dContentFile() );
    }

    return WT_Result::Success;
}

WT_Result WT_XAML_Fill::provideFill(XamlDrawableAttributes::Fill*& rpFill)
{
    if (_pSerializeFile == NULL)
    {
        return WT_Result::Internal_Error;
    }

    if ( fill() == WD_False )
    {
        // Fill mode is off
        return WT_Result::Success;
    }

    if (rpFill == NULL)
    {
        rpFill = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::Fill() );
        if (rpFill == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    XamlBrush::SolidColor* pSolidBrush = DWFCORE_ALLOC_OBJECT( XamlBrush::SolidColor() );
	pSolidBrush->set( _pSerializeFile->rendition().color().rgba() );

	rpFill->brush() = pSolidBrush;
    return WT_Result::Success;
}

WT_Result WT_XAML_Fill::providePathAttributes( XamlDrawableAttributes::PathAttributeConsumer* pConsumer, WT_XAML_File& rFile ) const
{
    const_cast<WT_XAML_Fill*>(this)->_pSerializeFile = &rFile;
    return pConsumer->consumeFill( const_cast<WT_XAML_Fill*>(this) );
}
