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

WT_Result WT_XAML_Line_Style::serialize(WT_File& file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Line_Style::serialize( *rFile.w2dContentFile() );
    }

    WT_XAML_Line_Style&     style   = static_cast<WT_XAML_Line_Style &>(rFile.rendition().line_style());
    WT_Unsigned_Integer32   fields  = 0;

    //Determine the deltas from the existing rendition line style
    if( m_adapt_patterns != style.adapt_patterns() && (m_fields_defined & ADAPT_PATTERNS_BIT) )
        fields |= ADAPT_PATTERNS_BIT;

    if( m_pattern_scale != style.pattern_scale() && (m_fields_defined & PATTERN_SCALE_BIT) )
        fields |= PATTERN_SCALE_BIT;

    if( m_line_join != style.line_join() && (m_fields_defined & LINE_JOIN_BIT) )
        fields |= LINE_JOIN_BIT;

    if( m_dash_start_cap != style.dash_start_cap() && (m_fields_defined & DASH_START_CAP_BIT) )
        fields |= DASH_START_CAP_BIT;

    if( m_dash_end_cap != style.dash_end_cap() && (m_fields_defined & DASH_END_CAP_BIT) )
        fields |= DASH_END_CAP_BIT;

    if( m_line_start_cap != style.line_start_cap() && (m_fields_defined & LINE_START_CAP_BIT) )
        fields |= LINE_START_CAP_BIT;

    if( m_line_end_cap != style.line_end_cap() && (m_fields_defined & LINE_END_CAP_BIT) )
        fields |= LINE_END_CAP_BIT;

    if( m_miter_angle != style.miter_angle() && (m_fields_defined & MITER_ANGLE_BIT) )
        fields |= MITER_ANGLE_BIT;

    if( m_miter_length != style.miter_length() && (m_fields_defined & MITER_LENGTH_BIT) )
        fields |= MITER_LENGTH_BIT;

    if( !fields )
        // there are no fields
        // that need to be serialized
        return WT_Result::Success;

	WD_CHECK( rFile.dump_delayed_drawable() ); //dumps any delayed drawable first

    // Serialize this pattern's attributes if different from its previous instance in
    // the file.

    //Serialize RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncStartElement() );
	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;	
	pW2XSerializer->startElement(XamlXML::kpzLine_Style_Element);	

    bool bForceSerializeToW2X = 
        file.rendition().fill_pattern() != file.desired_rendition().fill_pattern() ||
        file.rendition().user_hatch_pattern() != file.desired_rendition().user_hatch_pattern() ||
        file.rendition().user_fill_pattern() != file.desired_rendition().user_fill_pattern();

    //Only serialize out the deltas
    if( fields & ADAPT_PATTERNS_BIT )
	{
        //Serialize adapt_patterns
		pW2XSerializer->addAttribute( XamlXML::kpzAdapt_Patterns_Attribute, m_adapt_patterns == WD_True ? true : false );
        style.adapt_patterns() = m_adapt_patterns;
	}
    if( fields & PATTERN_SCALE_BIT )
	{
		//Serialize pattern_scale	
		pW2XSerializer->addAttribute( XamlXML::kpzScale_Attribute, (double)m_pattern_scale );
        style.pattern_scale() = m_pattern_scale;
	}
    if( fields & LINE_JOIN_BIT )
    {
        pW2XSerializer->addAttribute( XamlXML::kpzLine_Join_Attribute, (WT_Line_Style::WT_Joinstyle_ID)m_line_join );
        style.line_join() = m_line_join;
    }
    if( fields & DASH_START_CAP_BIT )
	{
		pW2XSerializer->addAttribute( XamlXML::kpzDash_Start_Cap_Attribute, (WT_Line_Style::WT_Capstyle_ID)m_dash_start_cap );
        style.dash_start_cap() = m_dash_start_cap;
	}
    if( fields & DASH_END_CAP_BIT )
    {
		if ( bForceSerializeToW2X )
        {
            pW2XSerializer->addAttribute( XamlXML::kpzDash_End_Cap_Attribute, (WT_Line_Style::WT_Capstyle_ID)m_dash_end_cap );
        }
        style.dash_end_cap() = m_dash_end_cap;
    }
    if( fields & LINE_START_CAP_BIT )
    {
		if ( bForceSerializeToW2X )
        {
            pW2XSerializer->addAttribute( XamlXML::kpzLine_Start_Cap_Attribute, (WT_Line_Style::WT_Capstyle_ID)m_line_start_cap );
        }
        style.line_start_cap() = m_line_start_cap;
    }
    if( fields & LINE_END_CAP_BIT )
    {
		if ( bForceSerializeToW2X )
        {
            pW2XSerializer->addAttribute( XamlXML::kpzLine_End_Cap_Attribute, (WT_Line_Style::WT_Capstyle_ID)m_line_end_cap );
        }
        style.line_end_cap() = m_line_end_cap;
    }
    if( fields & MITER_ANGLE_BIT )
	{
		pW2XSerializer->addAttribute( XamlXML::kpzMiter_Angle_Attribute, (WT_Unsigned_Integer16)m_miter_angle  );
        style.miter_angle() = m_miter_angle;
	}
    if( fields & MITER_LENGTH_BIT )
    {
		pW2XSerializer->addAttribute( XamlXML::kpzMiter_Length_Attribute, (WT_Unsigned_Integer16)m_miter_length );
        style.miter_length() = m_miter_length;
    }

    if ( bForceSerializeToW2X )
    {
        pW2XSerializer->addAttribute( XamlXML::kpzComplete_Attribute, true );
    }

	pW2XSerializer->endElement();
	//good to go
    return WT_Result::Success;
}

WT_Result WT_XAML_Line_Style::consumeStrokeEndLineCap(XamlDrawableAttributes::StrokeEndLineCap::Provider *pProvider)
{
    XamlDrawableAttributes::StrokeEndLineCap oStrokeEndLineCap, *pStrokeEndLineCap = &oStrokeEndLineCap;
    WD_CHECK( pProvider->provideStrokeEndLineCap( pStrokeEndLineCap ) );

    switch( oStrokeEndLineCap.style() )
    {
		case XamlDrawableAttributes::StrokeEndLineCap::Flat:
			line_end_cap() = Butt_Cap;
			break;
		case XamlDrawableAttributes::StrokeEndLineCap::Round:
			line_end_cap() = Round_Cap;
			break;
		case XamlDrawableAttributes::StrokeEndLineCap::Square:
			line_end_cap() = Square_Cap;
			 break;
		case XamlDrawableAttributes::StrokeEndLineCap::Triangle:
			line_end_cap() = Diamond_Cap;
			break;
		default:
			break;
    }

    return WT_Result::Success;
}

WT_Result WT_XAML_Line_Style::provideStrokeEndLineCap(XamlDrawableAttributes::StrokeEndLineCap* & rpStrokeEndLineCap)
{
	if (rpStrokeEndLineCap == NULL)
    {
        rpStrokeEndLineCap = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::StrokeEndLineCap() );
        if (rpStrokeEndLineCap == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

	WT_Line_Style::WT_Capstyle_ID eCap = line_end_cap(); 
	switch( eCap )
	{
		case Butt_Cap:
			rpStrokeEndLineCap->style() = XamlDrawableAttributes::StrokeEndLineCap::Flat;
			break;
		case Round_Cap:
			rpStrokeEndLineCap->style() = XamlDrawableAttributes::StrokeEndLineCap::Round;
			break;
		case Square_Cap:
			rpStrokeEndLineCap->style() = XamlDrawableAttributes::StrokeEndLineCap::Square;
			 break;
		case Diamond_Cap:
			rpStrokeEndLineCap->style() = XamlDrawableAttributes::StrokeEndLineCap::Triangle;
			break;
		default:
			break;

	};

	return WT_Result::Success;
}

WT_Result WT_XAML_Line_Style::consumeStrokeStartLineCap(XamlDrawableAttributes::StrokeStartLineCap::Provider *pProvider)
{
    XamlDrawableAttributes::StrokeStartLineCap oStrokeStartLineCap, *pStrokeStartLineCap = &oStrokeStartLineCap;
    WD_CHECK( pProvider->provideStrokeStartLineCap( pStrokeStartLineCap ) );

    switch( oStrokeStartLineCap.style() )
    {
		case XamlDrawableAttributes::StrokeStartLineCap::Flat:
			line_start_cap() = Butt_Cap;
			break;
		case XamlDrawableAttributes::StrokeStartLineCap::Round:
			line_start_cap() = Round_Cap;
			break;
		case XamlDrawableAttributes::StrokeStartLineCap::Square:
			line_start_cap() = Square_Cap;
			 break;
		case XamlDrawableAttributes::StrokeStartLineCap::Triangle:
			line_start_cap() = Diamond_Cap;
			break;
		default:
			break;
    }

    return WT_Result::Success;
}

WT_Result WT_XAML_Line_Style::provideStrokeStartLineCap(XamlDrawableAttributes::StrokeStartLineCap* & rpStrokeStartLineCap)
{
	if (rpStrokeStartLineCap == NULL)
    {
        rpStrokeStartLineCap = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::StrokeStartLineCap() );
        if (rpStrokeStartLineCap == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

	WT_Line_Style::WT_Capstyle_ID eCap = line_end_cap(); 
	switch( eCap )
	{
		case Butt_Cap:
			rpStrokeStartLineCap->style() = XamlDrawableAttributes::StrokeStartLineCap::Flat;
			break;
		case Round_Cap:
			rpStrokeStartLineCap->style() = XamlDrawableAttributes::StrokeStartLineCap::Round;
			break;
		case Square_Cap:
			rpStrokeStartLineCap->style() = XamlDrawableAttributes::StrokeStartLineCap::Square;
			 break;
		case Diamond_Cap:
			rpStrokeStartLineCap->style() = XamlDrawableAttributes::StrokeStartLineCap::Triangle;
			break;
		default:
			break;
	};

	return WT_Result::Success;
}

WT_Result WT_XAML_Line_Style::consumeStrokeDashCap(XamlDrawableAttributes::StrokeDashCap::Provider *pProvider)
{
    XamlDrawableAttributes::StrokeDashCap oStrokeDashCap, *pStrokeDashCap = &oStrokeDashCap;
    WD_CHECK( pProvider->provideStrokeDashCap( pStrokeDashCap ) );

    switch( oStrokeDashCap.style() )
    {
		case XamlDrawableAttributes::StrokeStartLineCap::Flat:
			dash_end_cap() = Butt_Cap;
			break;
		case XamlDrawableAttributes::StrokeStartLineCap::Round:
			dash_end_cap() = Round_Cap;
			break;
		case XamlDrawableAttributes::StrokeStartLineCap::Square:
			dash_end_cap() = Square_Cap;
			 break;
		case XamlDrawableAttributes::StrokeStartLineCap::Triangle:
			dash_end_cap() = Diamond_Cap;
			break;
		default:
			break;
    }

    return WT_Result::Success;
}

WT_Result WT_XAML_Line_Style::provideStrokeDashCap(XamlDrawableAttributes::StrokeDashCap* & rpStrokeDashCap)
{
	if (rpStrokeDashCap == NULL)
    {
        rpStrokeDashCap = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::StrokeDashCap() );
        if (rpStrokeDashCap == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

	WT_Line_Style::WT_Capstyle_ID eCap = dash_end_cap(); 
	switch( eCap )
	{
		case Butt_Cap:
			rpStrokeDashCap->style() = XamlDrawableAttributes::StrokeDashCap::Flat;
			break;
		case Round_Cap:
			rpStrokeDashCap->style() = XamlDrawableAttributes::StrokeDashCap::Round;
			break;
		case Square_Cap:
			rpStrokeDashCap->style() = XamlDrawableAttributes::StrokeDashCap::Square;
			 break;
		case Diamond_Cap:
			rpStrokeDashCap->style() = XamlDrawableAttributes::StrokeDashCap::Triangle;
			break;
		default:
			break;

	};

	return WT_Result::Success;
}

WT_Result WT_XAML_Line_Style::consumeStrokeLineJoin(XamlDrawableAttributes::StrokeLineJoin::Provider *pProvider)
{
    XamlDrawableAttributes::StrokeLineJoin oStrokeLineJoin, *pStrokeLineJoin = &oStrokeLineJoin;
    WD_CHECK( pProvider->provideStrokeLineJoin( pStrokeLineJoin ) );

    switch ( oStrokeLineJoin.style() )
    {
        case XamlDrawableAttributes::StrokeLineJoin::Miter:
            line_join() = Miter_Join;
			break;
        case XamlDrawableAttributes::StrokeLineJoin::Bevel:
            line_join() = Bevel_Join;
			break;
        case XamlDrawableAttributes::StrokeLineJoin::Round:
            line_join() = Round_Join;
			 break;
		default:
			break;

	};
	return WT_Result::Success;
}

WT_Result WT_XAML_Line_Style::provideStrokeLineJoin(XamlDrawableAttributes::StrokeLineJoin* & rpStrokeLineJoin)
{
	if (rpStrokeLineJoin == NULL)
    {
        rpStrokeLineJoin = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::StrokeLineJoin() );
        if (rpStrokeLineJoin == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

	WT_Line_Style::WT_Joinstyle_ID eLineJoin = line_join(); 
	switch( eLineJoin )
	{
		case Miter_Join:
		{
			if((const WT_Unsigned_Integer16 &)miter_length() == 6)
				rpStrokeLineJoin->style() = XamlDrawableAttributes::StrokeLineJoin::None;
			else
				rpStrokeLineJoin->style() = XamlDrawableAttributes::StrokeLineJoin::Miter;
			break;
		}
		case Bevel_Join:
			rpStrokeLineJoin->style() = XamlDrawableAttributes::StrokeLineJoin::Bevel;
			break;
		case Round_Join:
			rpStrokeLineJoin->style() = XamlDrawableAttributes::StrokeLineJoin::Round;
			 break;
		default:
			break;

	};

	return WT_Result::Success;
}

WT_Result WT_XAML_Line_Style::provideStrokeMiterLimit(XamlDrawableAttributes::StrokeMiterLimit* & rpStrokeMiterLimit)
{
	if (rpStrokeMiterLimit == NULL)
    {
        rpStrokeMiterLimit = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::StrokeMiterLimit() );
        if (rpStrokeMiterLimit == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

	if(!_pSerializeFile)
		return WT_Result::Internal_Error;

	int iLineWeight = _pSerializeFile->desired_rendition().line_weight().weight_value();
	if(!iLineWeight)
		rpStrokeMiterLimit->value() = 1.0f;
	else
		rpStrokeMiterLimit->value() = (float)( 2 * miter_length() / iLineWeight);
	
	return WT_Result::Success;
}

WT_Result WT_XAML_Line_Style::providePathAttributes( XamlDrawableAttributes::PathAttributeConsumer* pConsumer, WT_XAML_File& rFile) const
{
	const_cast<WT_XAML_Line_Style*>(this)->_pSerializeFile = &rFile;

    WD_CHECK( pConsumer->consumeStrokeEndLineCap( const_cast<WT_XAML_Line_Style*>(this) ) );
	WD_CHECK( pConsumer->consumeStrokeStartLineCap( const_cast<WT_XAML_Line_Style*>(this) ) );
	WD_CHECK( pConsumer->consumeStrokeDashCap( const_cast<WT_XAML_Line_Style*>(this) ) );
	WD_CHECK( pConsumer->consumeStrokeLineJoin( const_cast<WT_XAML_Line_Style*>(this) ) );
	WD_CHECK( pConsumer->consumeStrokeMiterLimit( const_cast<WT_XAML_Line_Style*>(this) ) );
	
	return WT_Result::Success;
}

WT_Result WT_XAML_Line_Style::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	WT_Result result = WT_Result::Success;

	//Start_Dash_Cap serialized in attributeMap
	if(!rMap.size())
		return WT_Result::Success;

    const char** ppValue = rMap.find( XamlXML::kpzAdapt_Patterns_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
	{
        adapt_patterns() = ( DWFCORE_COMPARE_ASCII_STRINGS("1", *ppValue ) == 0 );
	}
    
    ppValue = rMap.find( XamlXML::kpzScale_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
	{
		pattern_scale() = atof(*ppValue);
	}
    
	ppValue = rMap.find( XamlXML::kpzDash_Start_Cap_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
	{
		WT_Line_Style::WT_Capstyle_ID nValue = (WT_Line_Style::WT_Capstyle_ID) atoi( *ppValue );
		dash_start_cap() = nValue;
	}

	ppValue = rMap.find( XamlXML::kpzDash_End_Cap_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
	{
		WT_Line_Style::WT_Capstyle_ID nValue = (WT_Line_Style::WT_Capstyle_ID) atoi( *ppValue );
		dash_end_cap() = nValue;
	}

	ppValue = rMap.find( XamlXML::kpzLine_Start_Cap_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
	{
		WT_Line_Style::WT_Capstyle_ID nValue = (WT_Line_Style::WT_Capstyle_ID) atoi( *ppValue );
		line_start_cap() = nValue;
	}

	ppValue = rMap.find( XamlXML::kpzLine_End_Cap_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
	{
		WT_Line_Style::WT_Capstyle_ID nValue = (WT_Line_Style::WT_Capstyle_ID) atoi( *ppValue );
		line_end_cap() = nValue;
	}

	ppValue = rMap.find( XamlXML::kpzLine_Join_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
	{
		WT_Line_Style::WT_Joinstyle_ID nValue = (WT_Line_Style::WT_Joinstyle_ID) atoi( *ppValue );
		line_join() = nValue;
	}

    ppValue = rMap.find( XamlXML::kpzMiter_Angle_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
	{
		miter_angle() = (WT_Unsigned_Integer16)atoi( *ppValue );
	}
    
    ppValue = rMap.find( XamlXML::kpzMiter_Length_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
	{
		miter_length() = (WT_Unsigned_Integer16)atoi( *ppValue );
	}

    ppValue = rMap.find( XamlXML::kpzComplete_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
	{
        materialized() = WD_True;
    }

	return result;
}
