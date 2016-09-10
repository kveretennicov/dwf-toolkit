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

WT_Result
WT_XAML_Font_Extension::serialize (WT_File & file) const
{
	//Serialize in w2x
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Font_Extension::serialize( *rFile.w2dContentFile() );
    }

    WD_CHECK( rFile.dump_delayed_drawable() );

	//Serialize RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncStartElement() );

	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;

    pW2XSerializer->startElement( XamlXML::kpzFont_Extension_Element );

	wchar_t *zStr = WT_String::to_wchar(logfont_name().length(), logfont_name() );
    if (zStr == NULL)
        return WT_Result::Out_Of_Memory_Error;
    pW2XSerializer->addAttribute( XamlXML::kpzLogfont_Attribute, zStr );
    delete [] zStr;  //This was not allocated with DWFCORE_ALLOC_MEMORY

    zStr = WT_String::to_wchar( cannonical_name().length(), cannonical_name() );
    if (zStr == NULL)
        return WT_Result::Out_Of_Memory_Error;
    pW2XSerializer->addAttribute( XamlXML::kpzCannonical_Attribute, zStr );
    delete [] zStr;  //This was not allocated with DWFCORE_ALLOC_MEMORY

	pW2XSerializer->endElement();

	return WT_Result::Success;
}

WT_Result
WT_XAML_Font_Extension::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(rMap.size())
    {
	    const char** pLogFontName = rMap.find(XamlXML::kpzLogfont_Attribute);
	    const char** pCanonicalName = rMap.find(XamlXML::kpzCannonical_Attribute);
	    if(pLogFontName != NULL && *pLogFontName != NULL)
	    {
		    set_logfont_name(*pLogFontName);
	    }

	    if(pCanonicalName != NULL && *pCanonicalName != NULL)
	    {
		    set_cannonical_name(*pCanonicalName);
	    }
    }

    materialized() = WD_True;
	return WT_Result::Success;
}

WT_Result 
WT_XAML_Font_Extension::provideFontUri( XamlDrawableAttributes::FontUri*&  rpFontUri )
{
    if ( _pSerializeFile == NULL )
    {
        return WT_Result::Internal_Error;
    }

    if (rpFontUri == NULL)
    {
        rpFontUri = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::FontUri() );
        if (rpFontUri == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }


    const wchar_t *zResolvedFontName( NULL );
    if ( cannonical_name().length() )
    {
        const wchar_t *zFontName = WT_String::to_wchar( 
            cannonical_name().length(), cannonical_name() );
        if ( zFontName )
        {
            zResolvedFontName = _pSerializeFile->resolveFontUri( zFontName );
            delete[] zFontName;
        }

    }

    if ( !zResolvedFontName && logfont_name().length())
    {
        const wchar_t *zFontName = WT_String::to_wchar( 
            logfont_name().length(), logfont_name() );
        if ( zFontName )
        {
            zResolvedFontName = _pSerializeFile->resolveFontUri( zFontName );
            delete[] zFontName;
        }
    }

    if(zResolvedFontName)
    {
        rpFontUri->uri() = zResolvedFontName;
    }

    if ( rpFontUri->uri() == "" )
    {
        return WT_Result::Internal_Error;
    }

    return WT_Result::Success;    
}

WT_Result 
WT_XAML_Font_Extension::provideGlyphsAttributes( XamlDrawableAttributes::GlyphsAttributeConsumer* p, WT_XAML_File& r) const
{
    const_cast<WT_XAML_Font_Extension*>(this)->_pSerializeFile = &r;
    WD_CHECK( p->consumeFontUri( const_cast<WT_XAML_Font_Extension*>(this)  ) );
    return WT_Result::Success;
}
