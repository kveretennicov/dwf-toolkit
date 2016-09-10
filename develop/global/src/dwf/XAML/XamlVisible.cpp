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

//
// from XamlDrawableAttributes::PathAttributeProvider
//
WT_Result WT_XAML_Visibility::providePathAttributes(
    XamlDrawableAttributes::PathAttributeConsumer *pConsumer,
    WT_XAML_File &) const
{
    if(!pConsumer)
    {
        return WT_Result::Internal_Error;
    }

    return pConsumer->consumeOpacity( const_cast<WT_XAML_Visibility*>(this) );
}

//
// from XamlDrawableAttributes::GlyphsAttributeProvider
//
WT_Result WT_XAML_Visibility::provideGlyphsAttributes(
    XamlDrawableAttributes::GlyphsAttributeConsumer *pConsumer,
    WT_XAML_File &) const
{
    if(!pConsumer)
    {
        return WT_Result::Internal_Error;
    }

    return pConsumer->consumeOpacity( const_cast<WT_XAML_Visibility*>(this) );
}

//
// from XamlDrawableAttributes::Fill::Provider
//
WT_Result WT_XAML_Visibility::provideOpacity(
    XamlDrawableAttributes::Opacity *&rpOpacity)
{
    if(!rpOpacity)
    {
        rpOpacity = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::Opacity );
        if(!rpOpacity)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }
    
    if(visible() == WD_False)
    {
        rpOpacity->value() = 0.;
    }

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Visibility::consumeOpacity( XamlDrawableAttributes::Opacity::Provider *pProvider )
{
    if ( pProvider == NULL )
    {
        return WT_Result::Toolkit_Usage_Error;
    }

    XamlDrawableAttributes::Opacity oOpacity, *p = &oOpacity;
    
    WD_CHECK( pProvider->provideOpacity( p ) );
    
    visible() = (oOpacity.value() == 0) ? WD_False : WD_True;

    return WT_Result::Success;
}

WT_Result WT_XAML_Visibility::serialize(WT_File &file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Visibility::serialize( *rFile.w2dContentFile() );
    }

    return WT_Result::Success;
}
