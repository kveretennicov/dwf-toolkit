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

WT_Result 
WT_XAML_Line_Weight::serialize(WT_File& file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Line_Weight::serialize( *rFile.w2dContentFile() );
    }

    WD_CHECK( rFile.dump_delayed_drawable() ); //dumps any delayed drawable first

    //Serialize RenditionSync (top element) if not yet done
    WD_CHECK( rFile.serializeRenditionSyncStartElement() );

    //now write whipx hint to w2xserializer
    DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
    if (!pW2XSerializer)
	    return WT_Result::Internal_Error;
	
	pW2XSerializer->startElement(XamlXML::kpzLine_Weight_Element);
	wchar_t buf[10];
	_DWFCORE_SWPRINTF(buf, 10, L"%d", weight_value() );
	pW2XSerializer->addAttribute( XamlXML::kpzWeight_Attribute, buf );
	pW2XSerializer->endElement();

    return WT_Result::Success;
}

WT_Result
WT_XAML_Line_Weight::provideStrokeThickness(XamlDrawableAttributes::StrokeThickness* & rpStrokeThickness)
{
	if ( _pSerializeFile == NULL )
    {
        return WT_Result::Internal_Error;
    }

//
// hairline rendering does not seem to work on the DWFx side. Just bypass the (weight() <= 0)
// case and use a scaled weight
//
#if 0
	//If line pattern is not solid and hairline weight then dont want to serialize StrokeThickness.
	//For such case StrokeDashArray and StrokeDashOffset are serialized by the LinePattern object.
	if((weight_value()<=0) && (_pSerializeFile->rendition().line_pattern().pattern_id() != WT_Line_Pattern::Solid))		
		return WT_Result::Success;
#endif

	if (rpStrokeThickness == NULL)
    {
        rpStrokeThickness = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::StrokeThickness() );
        if (rpStrokeThickness == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

	int nWeight = weight_value();
	if(!nWeight)
	{
		nWeight = 1;
	}
	else if(nWeight < 0)
	{
		nWeight = -nWeight;
	}
	float lProjected = _pSerializeFile->convertToPaperSpace((float) nWeight);
	if(lProjected > 0. && lProjected < 1.)
	{
		nWeight = (int) (nWeight / lProjected);
	}

    rpStrokeThickness->weight() = nWeight;

	return WT_Result::Success;
}

WT_Result
WT_XAML_Line_Weight::providePathAttributes( XamlDrawableAttributes::PathAttributeConsumer* pConsumer, WT_XAML_File& rFile) const
{
	const_cast<WT_XAML_Line_Weight*>(this)->_pSerializeFile = &rFile;
    return pConsumer->consumeStrokeThickness( const_cast<WT_XAML_Line_Weight*>(this) );
}

WT_Result
WT_XAML_Line_Weight::parseAttributeList(XamlXML::tAttributeMap& rMap,WT_File& /*file*/)
{
	if(!rMap.size())
		return WT_Result::Success;

	const char** pWeight = rMap.find(XamlXML::kpzWeight_Attribute);
	if(pWeight != NULL && *pWeight != NULL)
	{
		weight_value() = (WT_Integer32)atoi(*pWeight);
	}

    materialized() = WD_True;
	return WT_Result::Success;
}
