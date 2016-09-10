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
// DWF specific patterns
//
unsigned int kpDwfPattern1 [] = {1, 8, 4};
unsigned int kpDwfPattern2 [] = {2, 1, 5, 1, 5};
unsigned int kpDwfPattern3 [] = {2, 7, 2, 1, 2};
unsigned int kpDwfPattern4 [] = {1, 6, 10};
unsigned int kpDwfPattern5 [] = {1, 8, 8};
unsigned int kpDwfPattern6 [] = {1, 12, 4};
unsigned int kpDwfPattern7 [] = {1, 12, 20};
unsigned int kpDwfPattern8 [] = {1, 16, 16};
unsigned int kpDwfPattern9 [] = {1, 24, 8};
unsigned int kpDwfPattern10 [] = {1, 12, 4};
unsigned int kpDwfPattern11 [] = {3, 10, 4, 4, 4, 4, 4};
unsigned int kpDwfPattern12 [] = {2, 12, 4, 4, 4};
unsigned int kpDwfPattern13 [] = {3, 12, 2, 1, 2, 1, 2};
unsigned int kpDwfPattern14 [] = {2, 12, 2, 1, 2};
unsigned int kpDwfPattern15 [] = {4, 8, 2, 1, 2, 4, 2, 1, 2};
unsigned int kpDwfPattern16 [] = {1, 1, 16};

//
// ISO specific patterns
//
unsigned int kpIsoPattern1 [] = {1, 47, 12};
unsigned int kpIsoPattern2 [] = {1, 47, 71};
unsigned int kpIsoPattern3 [] = {2, 94, 12, 1, 12};
unsigned int kpIsoPattern4 [] = {3, 94, 12, 1, 12, 1, 12};
unsigned int kpIsoPattern5 [] = {4, 94, 12, 1, 12, 1, 12, 1, 12};
unsigned int kpIsoPattern6 [] = {1, 1, 79};
unsigned int kpIsoPattern7 [] = {2, 94, 12, 24, 12};
unsigned int kpIsoPattern8 [] = {3, 94, 12, 24, 12, 24, 12};
unsigned int kpIsoPattern9 [] = {2, 47, 12, 1, 12};
unsigned int kpIsoPattern10 [] = {3, 47, 12, 47, 12, 1, 12};
unsigned int kpIsoPattern11 [] = {3, 47, 12, 1, 12, 1, 12};
unsigned int kpIsoPattern12 [] = {4, 47, 12, 47, 12, 1, 12, 1, 12};
unsigned int kpIsoPattern13 [] = {4, 47, 12, 1, 12, 1, 12, 1, 12};
unsigned int kpIsoPattern14 [] = {5, 47, 12, 47, 12, 1, 12, 1, 12, 1, 12};

unsigned int *WT_XAML_Line_Pattern::_kpDwfPatterns [] =
{
    kpDwfPattern1,
    kpDwfPattern2,
    kpDwfPattern3,
    kpDwfPattern4,
    kpDwfPattern5,
    kpDwfPattern6,
    kpDwfPattern7,
    kpDwfPattern8,
    kpDwfPattern9,
    kpDwfPattern10,
    kpDwfPattern11,
    kpDwfPattern12,
    kpDwfPattern13,
    kpDwfPattern14,
    kpDwfPattern15,
    kpDwfPattern16
};

unsigned int *WT_XAML_Line_Pattern::_kpIsoPatterns [] =
{
    kpIsoPattern1,
    kpIsoPattern2,
    kpIsoPattern3,
    kpIsoPattern4,
    kpIsoPattern5,
    kpIsoPattern6,
    kpIsoPattern7,
    kpIsoPattern8,
    kpIsoPattern9,
    kpIsoPattern10,
    kpIsoPattern11,
    kpIsoPattern12,
    kpIsoPattern13,
    kpIsoPattern14
};

//
// from XamlDrawableAttributes::PathAttributeProvider
//
WT_Result WT_XAML_Line_Pattern::providePathAttributes(
    XamlDrawableAttributes::PathAttributeConsumer *pConsumer,
    WT_XAML_File& rFile ) const
{
    if(!pConsumer)
    {
        return WT_Result::Internal_Error;
    }

    const_cast<WT_XAML_Line_Pattern*>(this)->_pSerializeFile = &rFile;
    WD_CHECK( pConsumer->consumeStrokeDashArray( const_cast<WT_XAML_Line_Pattern*>(this) ) );
    return pConsumer->consumeStrokeDashOffset( const_cast<WT_XAML_Line_Pattern*>(this) );
}

//
// from XamlDrawableAttributes::Fill::Provider
//
WT_Result WT_XAML_Line_Pattern::provideStrokeDashArray(
    XamlDrawableAttributes::StrokeDashArray *& rpArray)
{
    if (_pSerializeFile == NULL)
    {
        return WT_Result::Internal_Error;
    }

    //
    // dash patterns override line patterns (see dashpat.h)
    // if the dash pattern is active (that is != kNull), abort gracefully
    // now
    //
    if(_pSerializeFile->rendition().dash_pattern() != WT_Dash_Pattern::kNull)
    {
        return WT_Result::Success;
    }

    //
    // - if the fill is null (what case would it be ?), alloc it
    // - alloc our XAML translator and set it in the fill
    //
    if(!rpArray)
    {
        rpArray = DWFCORE_ALLOC_OBJECT(XamlDrawableAttributes::StrokeDashArray);
        if(!rpArray)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    int lWeight = _pSerializeFile->rendition().line_weight().weight_value();

    switch(pattern_id())
    {
		case Solid :
			{
                if (lWeight <= 0)
                {
                    rpArray->dashGap().push_back( 
                        XamlDrawableAttributes::StrokeDashArray::tDashGapPair(1.0f, 0.0f) );
                }
			}
			break;
		case Dashed :
		case Dotted :
		case Dash_Dot :
		case Short_Dash :
		case Medium_Dash :
		case Long_Dash :
		case Short_Dash_X2 :
		case Medium_Dash_X2 :
		case Long_Dash_X2 :
		case Medium_Long_Dash :
		case Medium_Dash_Short_Dash_Short_Dash :
		case Long_Dash_Short_Dash :
		case Long_Dash_Dot_Dot :
		case Long_Dash_Dot :
		case Medium_Dash_Dot_Short_Dash_Dot :
		case Sparse_Dot :
			{
                unsigned int i;
				int idx = pattern_id() - WT_Line_Pattern::Dashed;

                if(lWeight == 0)
                {
                    lWeight = 1;
                }

                for(i = 0; i < _kpDwfPatterns[ idx ][ 0 ];i++) {
                    rpArray->dashGap().push_back(
                        XamlDrawableAttributes::StrokeDashArray::tDashGapPair(
                            (float)(2. * _kpDwfPatterns[ idx ][ 1 + 2 * i ]) / lWeight,
                            (float)(2. * _kpDwfPatterns[ idx ][ 2 + 2 * i ]) / lWeight
                        )
                    );
				}
			}
			break;
		case ISO_Dash :
		case ISO_Dash_Space :
		case ISO_Long_Dash_Dot :
		case ISO_Long_Dash_Double_Dot :
		case ISO_Long_Dash_Triple_Dot :
		case ISO_Dot :
		case ISO_Long_Dash_Short_Dash :
		case ISO_Long_Dash_Double_Short_Dash :
		case ISO_Dash_Dot :
		case ISO_Double_Dash_Dot :
		case ISO_Dash_Double_Dot :
		case ISO_Double_Dash_Double_Dot :
		case ISO_Dash_Triple_Dot :
		case ISO_Double_Dash_Triple_Dot :
			{
				int idx = pattern_id() - WT_Line_Pattern::ISO_Dash;

                if(lWeight == 0)
                {
                    lWeight = 1;
                }

                unsigned int i;
				for(i = 0; i < _kpIsoPatterns[ idx ][ 0 ];i++) {

                    //
                    // note : we might need, to comply with ISO, to play with
                    // the page's DPI and other parameter to come up with
                    // accurate pixel values, ect, ect..
                    //
                    rpArray->dashGap().push_back(
                        XamlDrawableAttributes::StrokeDashArray::tDashGapPair(
                            (float)(.5 * _kpIsoPatterns[ idx ][ 1 + 2 * i ]) / lWeight,
                            (float)(.5 * _kpIsoPatterns[ idx ][ 2 + 2 * i ]) / lWeight)
                        );
                }
			}
			break;
		case Decorated_Tracks :
		case Decorated_Wide_Tracks :
		case Decorated_Circle_Fence :
		case Decorated_Square_Fence :
			{
			}
			break;
		default:
			break;
	}

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Line_Pattern::provideStrokeDashOffset(XamlDrawableAttributes::StrokeDashOffset *& rp)    // memento, may be null
{
	if(_pSerializeFile->rendition().line_weight().weight_value() <= 0)
	{
        if(!rp)
        {
            rp = DWFCORE_ALLOC_OBJECT(XamlDrawableAttributes::StrokeDashOffset);
            if(!rp)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
        }

        float dSum = 0.;
        switch(pattern_id())
        {
		    case Solid :
                dSum = 2.;
			    break;
		    case Dashed :
		    case Dotted :
		    case Dash_Dot :
		    case Short_Dash :
		    case Medium_Dash :
		    case Long_Dash :
		    case Short_Dash_X2 :
		    case Medium_Dash_X2 :
		    case Long_Dash_X2 :
		    case Medium_Long_Dash :
		    case Medium_Dash_Short_Dash_Short_Dash :
		    case Long_Dash_Short_Dash :
		    case Long_Dash_Dot_Dot :
		    case Long_Dash_Dot :
		    case Medium_Dash_Dot_Short_Dash_Dot :
		    case Sparse_Dot :
			    {
                    unsigned int i;
				    int idx = pattern_id() - WT_Line_Pattern::Dashed;

				    for(i = 0; i < _kpDwfPatterns[ idx ][ 0 ];i++) {

                        dSum += (float)(2. * _kpDwfPatterns[ idx ][ 1 + 2 * i ]) +
                                (float)(2. * _kpDwfPatterns[ idx ][ 2 + 2 * i ]);
				    }
			    }
			    break;
		    case ISO_Dash :
		    case ISO_Dash_Space :
		    case ISO_Long_Dash_Dot :
		    case ISO_Long_Dash_Double_Dot :
		    case ISO_Long_Dash_Triple_Dot :
		    case ISO_Dot :
		    case ISO_Long_Dash_Short_Dash :
		    case ISO_Long_Dash_Double_Short_Dash :
		    case ISO_Dash_Dot :
		    case ISO_Double_Dash_Dot :
		    case ISO_Dash_Double_Dot :
		    case ISO_Double_Dash_Double_Dot :
		    case ISO_Dash_Triple_Dot :
		    case ISO_Double_Dash_Triple_Dot :
			    {
				    int idx = pattern_id() - WT_Line_Pattern::ISO_Dash;
    				
                    unsigned int i;
				    for(i = 0; i < _kpIsoPatterns[ idx ][ 0 ];i++) {

                        //
                        // note : we might need, to comply with ISO, to play with
                        // the page's DPI and other parameter to come up with
                        // accurate pixel values, ect, ect..
                        //
                        dSum += (float)(.5 * _kpIsoPatterns[ idx ][ 1 + 2 * i ]) +
                                (float)(.5 * _kpIsoPatterns[ idx ][ 2 + 2 * i ]);
                    }
			    }
			    break;
		    case Decorated_Tracks :
		    case Decorated_Wide_Tracks :
		    case Decorated_Circle_Fence :
		    case Decorated_Square_Fence :
			    {
                    WD_Assert(false);
			    }
			    break;
		    default:
			    break;
        }

		dSum *= -1.0;
		rp->value() = dSum;
	}

    return WT_Result::Success;
}

WT_Result WT_XAML_Line_Pattern::serialize(
    WT_File &file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Line_Pattern::serialize( *rFile.w2dContentFile() );
    }

    WD_CHECK( rFile.dump_delayed_drawable() );
	WD_CHECK( rFile.serializeRenditionSyncStartElement() );

	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
    {
		return WT_Result::Internal_Error;
    }

    pW2XSerializer->startElement(XamlXML::kpzLine_Pattern_Element);
	pW2XSerializer->addAttribute(XamlXML::kpzId_Attribute,(int) pattern_id());
	pW2XSerializer->endElement();

    return WT_Result::Success;
}

WT_Result
WT_XAML_Line_Pattern::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

	const char** pPatId = rMap.find(XamlXML::kpzId_Attribute);
	if(pPatId != NULL && *pPatId != NULL)
	{
		set((WT_Line_Pattern::WT_Pattern_ID)atoi(*pPatId));
	}
    else
    {
        return WT_Result::Corrupt_File_Error;
    }
    materialized() = WD_True;
	return WT_Result::Success;
}
