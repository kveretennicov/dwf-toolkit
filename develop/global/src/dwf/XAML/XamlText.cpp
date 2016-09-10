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

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include "XAML/pch.h"

bool
WT_XAML_Text::_needCanvasWrapper( WT_XAML_File &rFile ) const
{
    if ((rFile.rendition().text_background().background() == WT_Text_Background::Ghosted && 
        rFile.rendition().text_background().offset() > 0 )
        ||  (rFile.rendition().text_background().background() == WT_Text_Background::Solid)
        ||  (rFile.rendition().font().style().underlined() == WD_True))
    {
        return true;
    }
    else
    {
        return false;
    }
}

WT_Result
WT_XAML_Text::serialize(WT_File & file) const
{
    //
    // we won't be able to render a dwfx file if a glyph has
    // no string content - if ever the WT_Text opcode has no
    // content, just drop it
    //
    if(string().length() == 0)
    {
        return WT_Result::Success;
    }

    WT_XAML_File &rFile = static_cast<WT_XAML_File&>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Text::serialize( *rFile.w2dContentFile() );
    }

    const_cast<WT_XAML_Text*>(this)->_pSerializeFile = &rFile;

    WD_CHECK( rFile.dump_delayed_drawable() );

    //Must have the attributes serialized before we do any glyph work, because we need
    //them to be on the active rendition, ready to go.
    WT_Integer32    parts_to_sync =     WT_Rendition::Color_Bit           |
        WT_Rendition::Color_Map_Bit       |
        //  WT_Rendition::Fill_Bit            |
        //  WT_Rendition::Fill_Pattern_Bit    |
        WT_Rendition::Merge_Control_Bit   |
        WT_Rendition::Visibility_Bit      |
        //  WT_Rendition::Line_Weight_Bit     |
        WT_Rendition::Pen_Pattern_Bit     |
        //  WT_Rendition::Line_Pattern_Bit    |
        //  WT_Rendition::Line_Caps_Bit       |
        //  WT_Rendition::Line_Join_Bit       |
        //  WT_Rendition::Marker_Size_Bit     |
        //  WT_Rendition::Marker_Symbol_Bit   |
        WT_Rendition::URL_Bit             |
        WT_Rendition::Layer_Bit           |
        WT_Rendition::Viewport_Bit        |
        WT_Rendition::Font_Extension_Bit  |
        WT_Rendition::Font_Bit            |
        WT_Rendition::Object_Node_Bit     |
        WT_Rendition::Text_Background_Bit |
        WT_Rendition::Text_HAlign_Bit     |
        WT_Rendition::Text_VAlign_Bit     |
        WT_Rendition::Contrast_Color_Bit;
    //  WT_Rendition::Delineate_Bit            

    WD_CHECK (rFile.desired_rendition().sync(file, parts_to_sync));

    bool bNeedCanvasWrapper = _needCanvasWrapper( rFile );

    if (rFile.heuristics().apply_transform())
    {
        const_cast<WT_XAML_Text*>(this)->transform (file.heuristics().transform());
    }

    //Serialize in w2x
    if (!bNeedCanvasWrapper || (bNeedCanvasWrapper && _eGhostPosition==None))
    {
        WD_CHECK( rFile.serializeRenditionSyncEndElement() );

        //now write whipx hint to w2xserializer
        DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
        if (!pW2XSerializer)
            return WT_Result::Internal_Error;

        pW2XSerializer->startElement(XamlXML::kpzText_Element);

        rFile.w2xSerializer()->addAttribute( XamlXML::kpzRefName_Attribute, rFile.nameIndexString());

        WT_Point2D oPosition( position().m_x, position().m_y );
        rFile.w2xSerializer()->addAttribute( XamlXML::kpzStartPoint_Attribute, oPosition.toString(10) );

        if (bounds().bounds() != NULL)
        {
            int nBounds[8] = { bounds().bounds()[0].m_x, bounds().bounds()[0].m_y,
                               bounds().bounds()[1].m_x, bounds().bounds()[1].m_y,
                               bounds().bounds()[2].m_x, bounds().bounds()[2].m_y,
                               bounds().bounds()[3].m_x, bounds().bounds()[3].m_y };

            rFile.w2xSerializer()->addAttribute( XamlXML::kpzBounds_Attribute, nBounds, 8 );
        }

        if ( overscore().count() > 0 )
        {
            int *pOverscorePositions = DWFCORE_ALLOC_MEMORY( int, sizeof(int)*overscore().count() );
            if ( pOverscorePositions == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
            int i;
            for( i=0; i<overscore().count(); i++ )
            {
                pOverscorePositions[i] = overscore().positions()[i];
            }

            pW2XSerializer->addAttribute( XamlXML::kpzOverscore_Attribute, pOverscorePositions, overscore().count() );
            DWFCORE_FREE_MEMORY( pOverscorePositions );
        }

        if ( this->underscore().positions() != NULL )
        {
            int *pUnderscorePositions = DWFCORE_ALLOC_MEMORY( int, sizeof(int)*underscore().count() );
            if ( pUnderscorePositions == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
            int i;
            for( i=0; i<underscore().count(); i++ )
            {
                pUnderscorePositions[i] = underscore().positions()[i];
            }

            pW2XSerializer->addAttribute( XamlXML::kpzUnderscore_Attribute, pUnderscorePositions, underscore().count() );
            DWFCORE_FREE_MEMORY( pUnderscorePositions );
        }

        if (rFile.rendition().visibility().visible() == WD_False)
        {
            //special case, we don't create 'invisible glyphs'
            wchar_t* zStr = WT_String::to_wchar( string().length(), string() );
            pW2XSerializer->addAttribute( XamlXML::kpzUnicodeString_Attribute, zStr );
            delete [] zStr; //not allocated using DWFCORE_ALLOC_MEMORY;

            //Do nothing else if it's invisible
            pW2XSerializer->endElement();

            rFile.nameIndex()++; //bump the index

            return WT_Result::Success;
        }

        pW2XSerializer->endElement();
    }

    // Send out the canvas if we need one
    if (bNeedCanvasWrapper)
    {
        if (_eGhostPosition==None)
        {
            //Serialize RenditionSync (top element) if not yet done
            XamlCanvas oCanvas;
            WD_CHECK( ((XamlDrawableAttributes::RenderTransform::Consumer*)&oCanvas)->consumeRenderTransform( const_cast<WT_XAML_Text*>(this) ) );
            WD_CHECK( oCanvas.serialize( file ) );
        }
    }

    // Send out background stuff if we need it
    if (rFile.rendition().text_background().background() == WT_Text_Background::Solid)
    {
        //create a background polygon and serialize it.
        WT_Point2D point[4];

        //
        // layout : y-mirror by hand all the vertices to make sure
        // the final rendering is not reversed
        //
        WD_CHECK( _getFontMetrics( rFile ) );

        double nOffset = rFile.rendition().text_background().offset();
        point[0].m_x = -nOffset;
        point[0].m_y = _fBase + nOffset;
        point[1].m_x = _fWidth + nOffset;
        point[1].m_y = _fBase + nOffset;
        point[2].m_x = _fWidth + nOffset;
        point[2].m_y = -(_fAscent + nOffset);
        point[3].m_x = -nOffset;
        point[3].m_y = -(_fAscent + nOffset);

        //These values are real world units (inches), so we make them XAML units here.
        WT_XAML_Point_Set_Data points(4, point, false);

        XamlPath *pBackground = DWFCORE_ALLOC_OBJECT( XamlPath );
        if ( pBackground==NULL )
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        XamlPolylineSegment* pXamlPolylineSegment = DWFCORE_ALLOC_OBJECT( XamlPolylineSegment(points) );
        if ( pXamlPolylineSegment==NULL )
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        pBackground->addDrawable(pXamlPolylineSegment);

        WT_XAML_Rendition& rXamlRendition = static_cast<WT_XAML_Rendition&>(rFile.rendition());
        WD_CHECK( ((XamlDrawableAttributes::PathAttributeConsumer*)pBackground)->consumeFill( (WT_XAML_Contrast_Color*) &rXamlRendition.contrast_color() ) );
        WD_CHECK( pBackground->serialize(file) );
        WD_CHECK( rFile.dump_delayed_drawable() );
        DWFCORE_FREE_OBJECT( pBackground );
    }

    //We only beget ghosties if we're corporeal
    if (rFile.rendition().text_background().background() == WT_Text_Background::Ghosted && 
        rFile.rendition().text_background().offset() > 0 &&
        _eGhostPosition == None)
    {
        //create four ghost Text objects to underlay this one.
        WT_XAML_Text oLeft(*this); 
        oLeft._eGhostPosition = Left;
        WT_XAML_Text oTop(*this); 
        oTop._eGhostPosition = Top;
        WT_XAML_Text oRight(*this); 
        oRight._eGhostPosition = Right;
        WT_XAML_Text oBottom(*this); 
        oBottom._eGhostPosition = Bottom;

        WD_CHECK( oLeft.serialize( file ) );
        WD_CHECK( oTop.serialize( file ) );
        WD_CHECK( oRight.serialize( file ) );
        WD_CHECK( oBottom.serialize( file ) );
    }

    //Create XamlGlyphs
    XamlGlyphs *pGlyphs = DWFCORE_ALLOC_OBJECT( XamlGlyphs );
    if (pGlyphs == NULL)
        return WT_Result::Out_Of_Memory_Error;
    WD_CHECK( provideGlyphsAttributes( pGlyphs, rFile ) );
    WD_CHECK( pGlyphs->serialize( file ) );
    DWFCORE_FREE_OBJECT( pGlyphs );

	//Send out underline if we need it
    if (rFile.rendition().font().style().underlined() == WD_True)
    {
        //create an underline path and serialize it.
        WT_Point2D point[2];

        //
        // layout : y-mirror by hand all the vertices to make sure
        // the final rendering is not reversed
        //

        WD_CHECK( _getFontMetrics( rFile ) );

        //These values are real world units (inches), so we make them XAML units here.
        point[0].m_x = 0;
        point[0].m_y = _fBase;
        point[1].m_x = _fWidth;
        point[1].m_y = point[0].m_y;

        WT_XAML_Point_Set_Data points(2, point, false);

        XamlPath *pUnderline = DWFCORE_ALLOC_OBJECT( XamlPath );
        if ( pUnderline==NULL )
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        XamlPolylineSegment* pXamlPolylineSegment = DWFCORE_ALLOC_OBJECT( XamlPolylineSegment(points) );
        if ( pXamlPolylineSegment==NULL )
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        pUnderline->addDrawable(pXamlPolylineSegment);

        WT_XAML_Rendition& rXamlRendition = static_cast<WT_XAML_Rendition&>(rFile.rendition());
        WD_CHECK( ((XamlDrawableAttributes::PathAttributeConsumer*)pUnderline)->consumeStroke((WT_XAML_Color*)&rXamlRendition.color() ) );
        WD_CHECK( pUnderline->serialize(file) );
        WD_CHECK( rFile.dump_delayed_drawable() );
        DWFCORE_FREE_OBJECT( pUnderline );
    }

    if (bNeedCanvasWrapper && _eGhostPosition==None)
    {
        WD_CHECK( rFile.serializeCanvasEndElement() );
    }

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Text::provideGlyphsAttributes( XamlDrawableAttributes::GlyphsAttributeConsumer* p, WT_XAML_File& r) const
{
    const_cast<WT_XAML_Text*>(this)->_pSerializeFile = &r;
    bool bNeedCanvasWrapper = _needCanvasWrapper( *_pSerializeFile );
    if ( !bNeedCanvasWrapper ) //only put the render transform on non-canvas-wrapped text
    {
        WD_CHECK( p->consumeRenderTransform( const_cast<WT_XAML_Text*>(this) ) );
    }
    WD_CHECK( p->consumeOriginX( const_cast<WT_XAML_Text*>(this) ) );
    WD_CHECK( p->consumeOriginY( const_cast<WT_XAML_Text*>(this) ) );
    WD_CHECK( p->consumeIndices( const_cast<WT_XAML_Text*>(this) ) );
    WD_CHECK( p->consumeUnicodeString( const_cast<WT_XAML_Text*>(this) ) );
    if ( _eGhostPosition == None ) 
    {
        WD_CHECK( static_cast<XamlDrawableAttributes::GlyphsAttributeProvider&>(
            static_cast<WT_XAML_Color&>(r.rendition().color()) ).provideGlyphsAttributes(p, r) );

        if (!bNeedCanvasWrapper) //canvas-wrapped text has a name on the canvas
        {
            // Only non-ghosted text items get the current element name from the file
            WD_CHECK( p->consumeName( &r ));
            r.nameIndex()++;
        }
    }
    else
    {
        WD_CHECK( static_cast<XamlDrawableAttributes::GlyphsAttributeProvider&>(
            static_cast<WT_XAML_Contrast_Color&>(r.rendition().contrast_color()) ).provideGlyphsAttributes(p, r) );
    }


    return WT_Result::Success;
}

WT_Result 
WT_XAML_Text::provideUnicodeString( XamlDrawableAttributes::UnicodeString *& rpUnicodeString )
{
    if (rpUnicodeString == NULL)
    {
        rpUnicodeString = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::UnicodeString() );
        if (rpUnicodeString == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    wchar_t* zStr = WT_String::to_wchar( string().length(), string() );
    rpUnicodeString->string() = zStr;
    delete [] zStr; //not allocated using DWFCORE_ALLOC_MEMORY;

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Text::provideIndices( XamlDrawableAttributes::Indices *& rpIndices )
{
    if (rpIndices == NULL)
    {
        rpIndices = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::Indices() );
        if (rpIndices == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    WT_Integer32 nSpacing = _pSerializeFile->rendition().font().spacing().spacing();
    if (nSpacing != 1024)
    {
        wchar_t buf[64];
        size_t i, nLen = string().length();
        DWFString zIndices( (size_t)(nLen * 64) );
		zIndices.assign(L"\0");

        double fSpaceFactor = (double)nSpacing / 1024.0;
        double fDesiredOffset = (fSpaceFactor * 50.0 ) - 50.0; //note, no layout transform scaling needed

        for (i=1;i<=nLen;i++)
        {
            _DWFCORE_SWPRINTF( buf, 64, L";,,%ls", (const wchar_t*)DWFString::DoubleToString((fDesiredOffset * i),7) );
            zIndices.append( buf );
        }            
        rpIndices->string() = zIndices;
    }

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Text::provideOriginX( XamlDrawableAttributes::OriginX *& rpOriginX )
{
    if (rpOriginX == NULL)
    {
        rpOriginX = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::OriginX() );
        if (rpOriginX == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    WD_CHECK( relativizePosition() );
    rpOriginX->value() = _fOriginX;
    return WT_Result::Success;
}

WT_Result 
WT_XAML_Text::provideOriginY( XamlDrawableAttributes::OriginY *& rpOriginY )
{
    if (rpOriginY == NULL)
    {
        rpOriginY = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::OriginY() );
        if (rpOriginY == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    WD_CHECK( relativizePosition() );
    rpOriginY->value() = _fOriginY;
    return WT_Result::Success;
}

WT_Result 
WT_XAML_Text::provideRenderTransform( XamlDrawableAttributes::RenderTransform *& rpRenderTransform )
{
    if (rpRenderTransform == NULL)
    {
        rpRenderTransform = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::RenderTransform() );
        if (rpRenderTransform == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    //Here we check the text for a bounding box, and if we have one
    //Dig into the rendition() font and determine the width 
    //of the text (we already have the height).  Given the height and 
    //width, we construct a render transform that translates and scales 
    //the text into the given box

    if (!_pSerializeFile)
    {
        return WT_Result::Internal_Error;
    }

    static const double kToDeg = 360. / 65536.;
    static const double kToRads = M_PI / 180.;
    WT_Matrix2D oMatrix;

    int nRotation = _pSerializeFile->rendition().font().rotation().rotation();
    double fRotation = kToRads * kToDeg * (double)nRotation;

    //moved this above the bounds() check becuase that overrides the font rotation setting
    if (_pSerializeFile->heuristics().apply_transform())
    {
        fRotation += kToRads * _pSerializeFile->heuristics().transform().rotation();
        while (fRotation < 0) //make sure we have a positive rotation
            fRotation += (2*M_PI); 
        while (fRotation > (2*M_PI)) //make sure it's 0 => 2PI
            fRotation -= (2*M_PI); 
    }

    if (bounds().bounds() != NULL)
    {
        //A bounds box trumps the font rotation
        int dx = bounds().bounds()[1].m_x - bounds().bounds()[0].m_x;
        int dy = bounds().bounds()[1].m_y - bounds().bounds()[0].m_y;
        if (dx == 0) //infinite slope
        {
            if (dy>0)
            {
                fRotation = .5*M_PI;
            }
            else
            {
                fRotation = (1.5*M_PI);
            }
        }
        else
        {
            fRotation = atan2( (double) dy , (double)dx );
        }
    }

    WT_Integer32 nWidthScale = _pSerializeFile->rendition().font().width_scale().width_scale();
    double fEpsilon = 1e-5;
    WT_Unsigned_Integer16 nOblique = _pSerializeFile->rendition().font().oblique().oblique();
    WT_Text_VAlign::WT_VAlign eVAlign = _pSerializeFile->rendition().text_valign().vAlign();
    WT_Text_HAlign::WT_HAlign eHAlign = _pSerializeFile->rendition().text_halign().hAlign();
    double fObliqueRads = (double)nOblique * kToDeg * kToRads;
    double fWidthScale = (double)nWidthScale / 1024.0;

    if ( fRotation != 0. || nWidthScale != 1024 || nOblique != 0 
        || eVAlign != WT_Text_VAlign::Baseline || eHAlign != WT_Text_HAlign::Left 
        || bounds().bounds() != NULL)
    {
        WD_CHECK( relativizePosition() );
        //Reset the origin - we take care of that in the render transform.
        _fOriginX = _fOriginY = 0;

        WT_Point2D oPosition;
        double xp=0., yp=0.;

        if (bounds().bounds() != NULL)
        {
            WD_CHECK( _getFontMetrics( *_pSerializeFile ) );

            double dx = bounds().bounds()[0].m_x - bounds().bounds()[1].m_x;
            double dy = bounds().bounds()[0].m_y - bounds().bounds()[1].m_y;
            double fDesiredWidth = sqrt( (dx*dx) + (dy*dy) );

            dx = bounds().bounds()[0].m_x - bounds().bounds()[3].m_x;
            dy = bounds().bounds()[0].m_y - bounds().bounds()[3].m_y;
            double fDesiredHeight = sqrt( (dx*dx) + (dy*dy) );

            double fXScale = fDesiredWidth / _fWidth; 
            double fYScale = fDesiredHeight / _fHeight; 

            oPosition.m_x = bounds().bounds()[0].m_x;
            oPosition.m_y = bounds().bounds()[0].m_y;

            //rotate the position offset
            xp = 0;
            yp = fDesiredHeight * (_fBase / _fHeight);
            oPosition.m_x += (xp * cos(-fRotation)) - (yp * sin(-fRotation));
            oPosition.m_y += (yp * cos(-fRotation)) + (xp * sin(-fRotation));

            oMatrix(0,0) = fXScale;
            oMatrix(1,1) = fYScale;
        }
        else
        {
            oPosition.m_x = position().m_x;
            oPosition.m_y = position().m_y;
        }

        //Vertical alignment
        if (eVAlign != WT_Text_VAlign::Baseline)
        {
            WD_CHECK( _getFontMetrics( *_pSerializeFile ) );
            double fModifier = 0.0;
            switch (eVAlign)
            {
            case WT_Text_VAlign::Ascentline:
                fModifier = -_fHeight;
                break;
            case WT_Text_VAlign::Capline:
                fModifier = -_fAscent;
                break;
            case WT_Text_VAlign::Halfline:
                fModifier = -_fAscent/2;
                break;
            case WT_Text_VAlign::Descentline:
                fModifier = _fBase;
                break;
            default:
                break;
            }

            xp = 0;
            yp = fModifier;
            oPosition.m_x += (xp * cos(fRotation)) - (yp * sin(fRotation));
            oPosition.m_y += (yp * cos(fRotation)) + (xp * sin(fRotation));
        }

        //Horizontal alignment
        if (eHAlign != WT_Text_HAlign::Left)
        {
            WD_CHECK( _getFontMetrics( *_pSerializeFile ) );
            double fModifier = 0.0;
            switch(eHAlign)
            {
            case WT_Text_HAlign::Center:
                fModifier = -(_fWidth * fWidthScale)/2;
                break;
            case WT_Text_HAlign::Right:
                fModifier = -(_fWidth * fWidthScale);
                break;
            default:
                break;
            }

            xp = fModifier;
            yp = 0;
            oPosition.m_x += (xp * cos(-fRotation)) - (yp * sin(-fRotation));
            oPosition.m_y += (yp * cos(-fRotation)) + (xp * sin(-fRotation));
        }

        //Width scale
        if (bounds().bounds() == NULL && nWidthScale != 1024)
        {
            // the rotation is done in "Apply rotation" part below, only set the fWidthSacle in the matrix
            oMatrix(0,0) *= fWidthScale;
        }

        //Apply rotation
        if ( fabs(fRotation) > fEpsilon )
        {
            WT_Matrix2D oTemp(oMatrix);
            oTemp.rotate(oMatrix, -fRotation);
        }

        //Apply obliquing
        if ( nOblique != 0 )
        {
            WT_Matrix2D oTemp;
            oTemp(0,1) = -sin( fObliqueRads );
            oMatrix *= oTemp;
        }

        WD_CHECK( _pSerializeFile->flipPoint(oPosition) );
        oMatrix(2,0) = oPosition.m_x;
        oMatrix(2,1) = oPosition.m_y;

    }

    switch( _eGhostPosition )
    {
    case Left:
        oMatrix(2,0) += _pSerializeFile->convertToPaperSpace( (float)(_pSerializeFile->rendition().text_background().offset()/96.0) );
        break;
    case Right:
        oMatrix(2,0) -= _pSerializeFile->convertToPaperSpace( (float)(_pSerializeFile->rendition().text_background().offset()/96.0) );
        break;
    case Bottom:
        oMatrix(2,1) += _pSerializeFile->convertToPaperSpace( (float)(_pSerializeFile->rendition().text_background().offset()/96.0) );
        break;
    case Top:
        oMatrix(2,1) -= _pSerializeFile->convertToPaperSpace( (float)(_pSerializeFile->rendition().text_background().offset()/96.0) );
        break;
    default:
        break;
    }

    if ( _needCanvasWrapper( *_pSerializeFile ) && oMatrix == WT_Matrix2D()/*identity*/ )
    {
        WD_CHECK( relativizePosition() );
        oMatrix(2,0) = _fOriginX;
        oMatrix(2,1) = _fOriginY;
        _fOriginX = _fOriginY = 0.0;
    }

    if (oMatrix != WT_Matrix2D()/*identity*/)
        rpRenderTransform->matrix() = oMatrix;

    return WT_Result::Success;
}


WT_Result 
WT_XAML_Text::relativizePosition(void)
{
    if (! _bIsRelativized )
    {
        if (_pSerializeFile == NULL)
            return WT_Result::Internal_Error;

        //
        // Simple positioning
        //

        //
        // layout : y-mirror by hand all the vertices to make sure
        // the final rendering is not reversed
        //

        switch( _eGhostPosition )
        {
        case None:
            {
                WT_Point2D p;
                p.m_x = position().m_x;
                p.m_y = position().m_y;
                _pSerializeFile->flipPoint(p);
                _fOriginX = (float)p.m_x;
                _fOriginY = (float)p.m_y;
                break;
            }
        case Left:
            _fOriginX = _pSerializeFile->convertToPaperSpace( (float)(_pSerializeFile->rendition().text_background().offset()/96.0) );
            _fOriginY = 0.0f;
            break;
        case Right:
            _fOriginX = -_pSerializeFile->convertToPaperSpace( (float)(_pSerializeFile->rendition().text_background().offset()/96.0) );
            _fOriginY = 0.0f;
            break;
        case Bottom:
            _fOriginX = 0.0f;
            _fOriginY = _pSerializeFile->convertToPaperSpace( (float)(_pSerializeFile->rendition().text_background().offset()/96.0) );
            break;
        case Top:
            _fOriginX = 0.0f;
            _fOriginY = -_pSerializeFile->convertToPaperSpace( (float)(_pSerializeFile->rendition().text_background().offset()/96.0) );
            break;
        default:
            break;
        }

        _bIsRelativized = true;
    }
    return WT_Result::Success;
}

WT_Result
WT_XAML_Text::_getFontMetrics( const WT_XAML_File &rFile ) const
{
    if (!_bHaveFontMetrics)
    {
        WT_XAML_Text* pThis = const_cast<WT_XAML_Text*>(this);
        WD_CHECK( rFile.calculateDesiredFontStringSize( string(), pThis->_fWidth, pThis->_fHeight, pThis->_fBase, pThis->_fAscent ) );
        pThis->_bHaveFontMetrics = true;
    }
    return WT_Result::Success;
}

WT_Result
WT_XAML_Text::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& rFile)
{
    if(!rMap.size())
        return WT_Result::Internal_Error;

    if (rFile.rendition().text_background().background() == WT_Text_Background::Ghosted &&
        rFile.rendition().text_background().offset() > 0)
    {
        _eGhostPosition = Undefined;
    }

    //position
    const char** ppValue = rMap.find( XamlXML::kpzStartPoint_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        WT_Point2D oPt;
        char *pPtr = const_cast< char* >( *ppValue );
        WD_CHECK( XamlPathGeometry::XamlPathFigure::getPoint( NULL, pPtr, oPt, false ) );

        //TODO: unflip point ?
        position().m_x = (WT_Integer32) floor( 0.5 + oPt.m_x );
        position().m_y = (WT_Integer32) floor( 0.5 + oPt.m_y );
    }

    //bounds
    ppValue = rMap.find( XamlXML::kpzBounds_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        double f;
        WT_Logical_Point oBounds[4];
        char *pPtr = const_cast< char* >( *ppValue );
        for (int i=0;i<4;i++)
        {
            WD_CHECK( XamlPathGeometry::XamlPathFigure::getScalar( pPtr, f ) );
            oBounds[i].m_x = (WT_Integer32) floor( 0.5 + f );
            WD_CHECK( XamlPathGeometry::XamlPathFigure::getScalar( pPtr, f ) );
            oBounds[i].m_y = (WT_Integer32) floor( 0.5 + f );
        }

        bounds().set( oBounds );
    }

    //overscore
    ppValue = rMap.find( XamlXML::kpzOverscore_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        WT_Text_Option_Scoring::tPositionVector vInt;
        char *pSavePtr; 
        char *pToken = DWFCORE_ASCII_STRING_TOKENIZE( const_cast< char* >( *ppValue ), /*NOXLATE*/" ", &pSavePtr );
        while (pToken != NULL )
        {
            vInt.push_back( (WT_Unsigned_Integer16) atoi( pToken ) );
            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
        }
        overscore().set( vInt );
    }

    //underscore
    ppValue = rMap.find( XamlXML::kpzUnderscore_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        WT_Text_Option_Scoring::tPositionVector vInt;
        char *pSavePtr;
        char *pToken = DWFCORE_ASCII_STRING_TOKENIZE( const_cast< char* >( *ppValue ), /*NOXLATE*/" ", &pSavePtr );
        while (pToken != NULL )
        {
            vInt.push_back( (WT_Unsigned_Integer16) atoi( pToken ) );
            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
        }
        underscore().set( vInt );
    }

    ppValue = rMap.find( XamlXML::kpzUnicodeString_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        //Special case: when the string is stored in W2X, it can only mean that it is invisible text
        //which means we should emit a WT_Visibility opcode if needed

        if (rFile.desired_rendition().visibility().visible() == WD_True || rFile.rendition().visibility().visible() == WD_True)
        {
            WT_XAML_Class_Factory oClassFactory;
            WT_Boolean bVisible = WD_False;
            WT_Visibility *pVisibility = oClassFactory.Create_Visibility( bVisible );
            if (pVisibility == NULL)
                return WT_Result::Out_Of_Memory_Error;

            pVisibility->materialized() = WD_True;
            WD_CHECK( XamlObjectFactory::insertObject( rFile, pVisibility, oClassFactory ) );
        }

        size_t nSrcLen = DWFCORE_ASCII_STRING_LENGTH( *ppValue );
        wchar_t* pUtfBuffer = DWFCORE_ALLOC_MEMORY( wchar_t, nSrcLen+1 ); //UTF-8 decoding definitely won't grow the string
        if (pUtfBuffer == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
        size_t nUtfLen = DWFString::DecodeUTF8( *ppValue, nSrcLen, pUtfBuffer, (nSrcLen+1) * sizeof(wchar_t) );
        
        string().set( (int)(nUtfLen / sizeof(wchar_t)),pUtfBuffer );

        DWFCORE_FREE_MEMORY( pUtfBuffer );

        materialized() = WD_True;
    }

    return WT_Result::Success;
}
