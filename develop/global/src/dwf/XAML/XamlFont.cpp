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
WT_XAML_Font::serialize (WT_File & file) const
{
	//Serialize in w2x
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Font::serialize( *rFile.w2dContentFile() );
    }

    WD_CHECK( rFile.dump_delayed_drawable() );

	//Serialize RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncStartElement() );

    file.desired_rendition().font_extension();
    WD_CHECK(file.desired_rendition().sync(file, WT_Rendition::Font_Extension_Bit));

    // BPM: this could be improved when time allows...
    if(file.heuristics().apply_transform() && !m_rotation_checked && file.heuristics().transform().rotation())
    {
        // The first time we start up the file, if the application wants an unrotated
        // font, and we need to flip landscape to portrait, then the above test will think
        // that the rotation doesn't need to be output the first time.  Here we force it...
        ((WT_XAML_Font &)(*this)).m_fields_defined |= FONT_ROTATION_BIT;
        ((WT_XAML_Font *)this)->m_rotation_checked = WD_True;  // Cast from const to alterable.
    }

	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;

    pW2XSerializer->startElement( XamlXML::kpzFont_Element );

    if (m_fields_defined & FONT_NAME_BIT)
    {
        const wchar_t *zFontName = WT_String::to_wchar( font_name().name().length(), font_name().name().unicode() );
        const wchar_t *zResolvedFontName = rFile.resolveFontUri( zFontName );
        pW2XSerializer->addAttribute( XamlXML::kpzName_Attribute, zFontName );
        pW2XSerializer->addAttribute( XamlXML::kpzFontUri_Attribute, zResolvedFontName );
        delete [] zFontName; //not allocated using DWFCORE_ALLOC_MEMORY;
    }

    if (m_fields_defined & FONT_STYLE_BIT)
    {
        pW2XSerializer->addAttribute( XamlXML::kpzBold_Attribute, style().bold()==WD_True );
        pW2XSerializer->addAttribute( XamlXML::kpzItalic_Attribute, style().italic()==WD_True );
        pW2XSerializer->addAttribute( XamlXML::kpzUnderline_Attribute, style().underlined()==WD_True );
    }
    if (m_fields_defined & FONT_CHARSET_BIT)
    {
        pW2XSerializer->addAttribute( XamlXML::kpzCharset_Attribute, (int) charset().charset() );
    }
    if (m_fields_defined & FONT_PITCH_BIT)
    {
        pW2XSerializer->addAttribute( XamlXML::kpzPitch_Attribute, (int) pitch().pitch() );
    }
    if (m_fields_defined & FONT_FAMILY_BIT)
    {
        pW2XSerializer->addAttribute( XamlXML::kpzFamily_Attribute, (int) family().family() );
    }
    if (m_fields_defined & FONT_HEIGHT_BIT)
    {
        pW2XSerializer->addAttribute( XamlXML::kpzHeight_Attribute, (int) height().height() );
    }
    if (m_fields_defined & FONT_ROTATION_BIT)
    {
        WT_Unsigned_Integer16 nRotation = rotation().rotation();

        //Massage for heuristic transform
        if (rFile.heuristics().apply_transform())
        {
            static const double kToDeg = 360. / 65536.;

            double fRotation = kToDeg * (double)nRotation;

            fRotation += rFile.heuristics().transform().rotation();
            while (fRotation < 0) //make sure we have a positive rotation
                fRotation += 360; 
            while (fRotation > 360) //make sure it's 0 => 360
                fRotation -= 360;

            nRotation = (WT_Unsigned_Integer16) floor( 0.5 + (fRotation / kToDeg));//back to WT rotation units
        }

        pW2XSerializer->addAttribute( XamlXML::kpzRotation_Attribute, nRotation );
    }
    if (m_fields_defined & FONT_WIDTH_SCALE_BIT)
    {
        pW2XSerializer->addAttribute( XamlXML::kpzWidthScale_Attribute, width_scale().width_scale() );
    }
    if (m_fields_defined & FONT_SPACING_BIT)
    {
        pW2XSerializer->addAttribute( XamlXML::kpzSpacing_Attribute, spacing().spacing() );
    }
    if (m_fields_defined & FONT_OBLIQUE_BIT)
    {
        pW2XSerializer->addAttribute( XamlXML::kpzOblique_Attribute, oblique().oblique() );
    }
    if (m_fields_defined & FONT_FLAGS_BIT)
    {
        pW2XSerializer->addAttribute( XamlXML::kpzFlags_Attribute, (int) flags().flags() );
    }

	pW2XSerializer->endElement();

	return WT_Result::Success;
}

WT_Result 
WT_XAML_Font::provideGlyphsAttributes( XamlDrawableAttributes::GlyphsAttributeConsumer* p, WT_XAML_File& rFile) const
{
    const_cast<WT_XAML_Font*>(this)->_pSerializeFile = &rFile;
    WD_CHECK( p->consumeFontRenderingEmSize( const_cast<WT_XAML_Font*>(this) ) );
    WD_CHECK( p->consumeFontUri( const_cast<WT_XAML_Font*>(this) ) );
	WD_CHECK( p->consumeIsSideways( const_cast<WT_XAML_Font*>(this) ) );
    WD_CHECK( p->consumeStyleSimulations( const_cast<WT_XAML_Font*>(this) ) );
    return WT_Result::Success;
}

WT_Result 
WT_XAML_Font::provideFontRenderingEmSize( XamlDrawableAttributes::FontRenderingEmSize*& rpFontRenderingEmSize )
{
    if (rpFontRenderingEmSize == NULL)
    {
        rpFontRenderingEmSize = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::FontRenderingEmSize() );
        if (rpFontRenderingEmSize == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    if(_pSerializeFile == NULL)
    {
        return WT_Result::Internal_Error;
    }

    //
    // explanation : the viewer is very sensitive to the final font size, that is
    // after transformation. The emSize is a logical quantity which gets scaled by
    // the top-level transform. If ever it's too small or too big, drop the glyph

    // This font's height may be undefined.  In which case, we need to use the current
    // rendition font.  JK
    WT_Integer32 nHeight = height().height();
    if (! (m_fields_defined | FONT_HEIGHT_BIT) )
    {
        nHeight = _pSerializeFile->rendition().font().height();
    }

    //Note: For now, I am commenting this code below. MS had a bug in rendering
    //small sized text. To avoid that this estimation of projected height was 
    //calculated to see if it was < 1. It looks like this bug has been fixed
    //recently so dont need to do this. Keeping it here might help for future
    //references just in case.... Archana

    ////
    //// the height is converted to page units (x96) in convertToPaperSpace()
    ////
    //float fProjected = _pSerializeFile->convertToPaperSpace((float)nHeight);

    ////
    //// 1.0 seems to be the smallest possible font size we can display
    ////
    //if(fProjected < 1.)
    //{
    //    return WT_Result::Internal_Error;        
    //}

    rpFontRenderingEmSize->value() = (float)nHeight; 

    return WT_Result::Success;
}


WT_Result 
WT_XAML_Font::provideFontUri( XamlDrawableAttributes::FontUri*& rpFontUri )
{
    if (rpFontUri == NULL)
    {
        rpFontUri = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::FontUri() );
        if (rpFontUri == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    if ( _pSerializeFile == NULL )
    {
        return WT_Result::Internal_Error;
    }

    const wchar_t *zFontName = WT_String::to_wchar( font_name().name().length(), font_name().name().unicode() );
    const wchar_t *zResolvedFontName = _pSerializeFile->resolveFontUri( zFontName );

    //
    // Do not return an error here if there has been no
    // resolution of the font uri.  We need to give the font 
    // extension a chance to provide the uri.
    //

    rpFontUri->uri() = zResolvedFontName;

    delete [] zFontName; //not allocated using DWFCORE_ALLOC_MEMORY;

    return WT_Result::Success;
}

WT_Result
WT_XAML_Font::provideIsSideways( XamlDrawableAttributes::IsSideways*& rpIsSideWays)
{
    if (rpIsSideWays == NULL)
    {
		rpIsSideWays = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::IsSideways() );
        if (rpIsSideWays == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    if ( _pSerializeFile == NULL )
    {
        return WT_Result::Internal_Error;
    }

    const wchar_t *zFontName = WT_String::to_wchar( font_name().name().length(), font_name().name().unicode() );

	// the true-SideWays tells the Glyph to rotate 90 degree Counter-Clockwise
	if ( wcslen(zFontName) > 0 )
	{
		if ( zFontName[0] == '@' )
			rpIsSideWays->value() = true; 
	}
	delete [] zFontName; //not allocated using DWFCORE_ALLOC_MEMORY;

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Font::provideStyleSimulations( XamlDrawableAttributes::StyleSimulations*& rpStyleSimulations )
{
    if (rpStyleSimulations == NULL)
    {
        rpStyleSimulations = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::StyleSimulations() );
        if (rpStyleSimulations == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    if (style().bold() && style().italic())
    {
        rpStyleSimulations->style() = XamlDrawableAttributes::StyleSimulations::BoldItalic;
    }
    else if (style().bold())
    {
        rpStyleSimulations->style() = XamlDrawableAttributes::StyleSimulations::Bold;
    }
    else if (style().italic())
    {
        rpStyleSimulations->style() = XamlDrawableAttributes::StyleSimulations::Italic;
    }

    return WT_Result::Success;
}

WT_Result
WT_XAML_Font::parseKey( char* keyOut, const wchar_t* zFontUriIn)
{
	/*
		The Font URI mist end with a GUID of the form "B03B02B01B00-B11B10-B21B20-B30B31-B32B33B34B35B36B37" or 
		"03B02B01B00-B11B10-B21B20-B30B31-B32B33B34B35B36B37.ext" where each Bx represents a 
		placeholder for one byte of the GUID, represented as two hex digits, and .ext is an arbitrary
        optional extension [M2.54].		
	*/

	/*
		Key: B37B36B35B34B33B32B31B30B20B21B10B11B00B01B02B03
	*/

    DWFString zFontGUID = zFontUriIn;
    off_t nOffset = zFontGUID.findLast('/');
    if (nOffset != -1)
    {
        zFontGUID = zFontGUID.substring(nOffset+1);
    }
    nOffset = zFontGUID.findLast('\\');
    if (nOffset != -1)
    {
        zFontGUID = zFontGUID.substring(nOffset+1);
    }
    nOffset = zFontGUID.findLast('.');
    if (nOffset != -1)
    {
        zFontGUID = zFontGUID.substring(0,nOffset);
    }

    WD_Assert(keyOut != NULL);
    if (!keyOut)
    {
        return WT_Result::Toolkit_Usage_Error; 
    }

    int i, idx=15;
	size_t len = zFontGUID.chars();
	memset(keyOut, 0, 16*sizeof(char));
    bool highNibble = true;

	for (i=0; i<(int)len && idx>=0; i++)
	{
        bool bValid = false;
		char ch = 0;
		if ( zFontGUID[i] >= '0' && zFontGUID[i] <= '9' )
		{
			ch = (char) (0xFF & (zFontGUID[i] - '0'));
            bValid = true;
		}
		else if ( zFontGUID[i] >= 'A' && zFontGUID[i] <= 'F' )
		{
			ch = (char) (0xFF & (10 + (zFontGUID[i] - 'A')));
            bValid = true;
		}
		else if ( zFontGUID[i] >= 'a' && zFontGUID[i] <= 'f' )
		{
			ch = (char) (0xFF & (10 + (zFontGUID[i] - 'a')));
            bValid = true;
		}

        if (bValid)
        {
            if (highNibble)
            {
		        keyOut[idx] = ch<<4;
                highNibble = false;
            }
            else
            {
		        keyOut[idx] = (char)(keyOut[idx] + ch );
                highNibble = true;
	    	    idx--;
            }
        }
	}

    if (idx != -1)
    {
        return WT_Result::Internal_Error;
    }

    return WT_Result::Success;
}

//It does both obfuscation and deobfuscation
WT_Result
WT_XAML_Font::obfuscate(DWFInputStream* in, DWFOutputStream* out, const char* key)
{
	if (in == NULL || out == NULL)
	{
        return WT_Result::Toolkit_Usage_Error;
	}

	char buf[32];
	bool done = false;
	bool xorComplete = false;
	while( !done )
	{
        size_t bytes = in->read(buf, 32);
        if (bytes != 0)
        {
            if ( !xorComplete )
            {
                int i,j;
                for( j=0; j<32; j+=16 )
                {
                    for( i=0; i<16; i++ )
                    {
                        buf[i+j] ^= key[i];
                    }
                }
                xorComplete = true;
            }
            out->write(buf, bytes);
        }

		if ( bytes == 0 && in->available() == 0)
		{
			done = true;
		}
	}

    out->flush();

    return WT_Result::Success;
}

/// partial materialization of shell from W2X
WT_Result 
WT_XAML_Font::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& rFile)
{
	if(!rMap.size())
		return WT_Result::Success;

    DWFString zFontName;
    m_fields_defined = 0;

    const char **ppValue = rMap.find( XamlXML::kpzName_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        zFontName = *ppValue;
        WT_String zString;
        WD_CHECK( zString.set( (int) zFontName.chars(),zFontName ) );
        font_name().set( zString );
        m_fields_defined |= FONT_NAME_BIT;
    }
    ppValue = rMap.find( XamlXML::kpzFontUri_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        DWFString zFontUri = *ppValue;
        if ( zFontName.chars() && rFile.resolveFontUri( zFontName ) == NULL )
        {
            rFile.registerFontUri( zFontName,  zFontUri );
        }
    }
    ppValue = rMap.find( XamlXML::kpzBold_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        style().set_bold( atoi(*ppValue) ? WD_True : WD_False );
        m_fields_defined |= FONT_STYLE_BIT;
    }
    ppValue = rMap.find( XamlXML::kpzItalic_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        style().set_italic( atoi(*ppValue) ? WD_True : WD_False );
        m_fields_defined |= FONT_STYLE_BIT;
    }
    ppValue = rMap.find( XamlXML::kpzUnderline_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        style().set_underlined( atoi(*ppValue) ? WD_True : WD_False );
        m_fields_defined |= FONT_STYLE_BIT;
    }
    ppValue = rMap.find( XamlXML::kpzCharset_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        charset().set( (WT_Byte) atoi(*ppValue) );
        m_fields_defined |= FONT_CHARSET_BIT;
    }
    ppValue = rMap.find( XamlXML::kpzPitch_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        pitch().set( (WT_Byte) atoi(*ppValue) );
        m_fields_defined |= FONT_PITCH_BIT;
    }
    ppValue = rMap.find( XamlXML::kpzFamily_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        family().set( (WT_Byte) atoi(*ppValue) );
        m_fields_defined |= FONT_FAMILY_BIT;
    }
    ppValue = rMap.find( XamlXML::kpzHeight_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        height().set( (WT_Unsigned_Integer32) atoi(*ppValue) );
        m_fields_defined |= FONT_HEIGHT_BIT;
    }
    ppValue = rMap.find( XamlXML::kpzRotation_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        rotation().set( (WT_Unsigned_Integer16) atoi(*ppValue) );
        m_fields_defined |= FONT_ROTATION_BIT;
    }
    ppValue = rMap.find( XamlXML::kpzWidthScale_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        width_scale().set( (WT_Unsigned_Integer16) atoi(*ppValue) );
        m_fields_defined |= FONT_WIDTH_SCALE_BIT;
    }
    ppValue = rMap.find( XamlXML::kpzSpacing_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        spacing().set( (WT_Unsigned_Integer16) atoi(*ppValue) );
        m_fields_defined |= FONT_SPACING_BIT;
    }
    ppValue = rMap.find( XamlXML::kpzOblique_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        oblique().set( (WT_Unsigned_Integer16) atoi(*ppValue) );
        m_fields_defined |= FONT_OBLIQUE_BIT;
    }
    ppValue = rMap.find( XamlXML::kpzFlags_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        flags().set( atoi(*ppValue) );
        m_fields_defined |= FONT_FLAGS_BIT;
    }

    materialized() = WD_True;

	return WT_Result::Success;

}
