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
WT_Result WT_XAML_Fill_Pattern::providePathAttributes(
    XamlDrawableAttributes::PathAttributeConsumer *pConsumer,
    WT_XAML_File& rFile ) const
{
    if(!pConsumer)
    {
        return WT_Result::Internal_Error;
    }

    const_cast<WT_XAML_Fill_Pattern*>(this)->_pSerializeFile = &rFile;
    return pConsumer->consumeFill( const_cast<WT_XAML_Fill_Pattern*>(this) );
}

//
// from XamlDrawableAttributes::Fill::Provider
//
WT_Result WT_XAML_Fill_Pattern::provideFill(
    XamlDrawableAttributes::Fill *& rpFill)
{
    if (_pSerializeFile == NULL)
    {
        return WT_Result::Internal_Error;
    }

    if ( _pSerializeFile->rendition().fill().fill() == WD_False )
    {
        // Fill mode is off
        return WT_Result::Success;
    }

    //
    // - if the fill is null (what case would it be ?), alloc it
    // - alloc our XAML translator and set it in the fill
    //
    if(!rpFill)
    {
        rpFill = DWFCORE_ALLOC_OBJECT(XamlDrawableAttributes::Fill);
        if(!rpFill)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    XamlBrush::Brush *pBrush = 0;

    if(pattern_id() == WT_Fill_Pattern::Solid) {

        //
        // special-case : the Solid pattern is just a plain fill
        // no need for a Visual brush in this case, instantiate
        // a SolidColor brush instead
        //
        pBrush = DWFCORE_ALLOC_OBJECT(
            XamlBrush::SolidColor(
                _pSerializeFile->rendition().color().rgba()
                )
            );
    } else {

        //
        // our visual is the XamlFixedPatternBrush
        //
        pBrush = DWFCORE_ALLOC_OBJECT(
            XamlBrush::XamlFixedPatternBrush(pattern_id(), pattern_scale())
            );
    }
    rpFill->brush() = pBrush;

    return WT_Result::Success;
}

WT_Result WT_XAML_Fill_Pattern::serialize(
    WT_File &file) const
{
    WT_XAML_File &rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Fill_Pattern::serialize( *rFile.w2dContentFile() );
    }

    //
    // warning : WT_Fill_Pattern has the good idea to hide the
    // rendition update within the serialize() method..
    //
    file.rendition().fill_pattern() = *this;

    WD_CHECK( rFile.dump_delayed_drawable() );

	//Serialize RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncStartElement() );

	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;
	
    pW2XSerializer->startElement(XamlXML::kpzFill_Pattern_Element);
    pW2XSerializer->addAttribute(XamlXML::kpzValue_Attribute, (int) pattern_id() );
    pW2XSerializer->addAttribute(XamlXML::kpzScale_Attribute, (double) pattern_scale() );
	pW2XSerializer->endElement();

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Fill_Pattern::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

	const char** ppValue = rMap.find(XamlXML::kpzValue_Attribute);
	if(ppValue!=NULL && *ppValue!=NULL)
	{
		set((WT_Pattern_ID)atoi(*ppValue));
	}

	ppValue = rMap.find(XamlXML::kpzScale_Attribute);
	if(ppValue!=NULL && *ppValue!=NULL)
	{
        pattern_scale() = atof(*ppValue);
	}
    else
    {
        return WT_Result::Corrupt_File_Error;
    }

    materialized() = WD_True;
	return WT_Result::Success;

}
