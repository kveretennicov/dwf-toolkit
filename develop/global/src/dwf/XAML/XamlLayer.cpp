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

WT_Result WT_XAML_Layer::serialize(WT_File & file) const
{
    //Serialize in w2x
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Layer::serialize( *rFile.w2dContentFile() );
    }

    WD_CHECK( rFile.dump_delayed_drawable() );

    //Serialize RenditionSync (top element) if not yet done
    WD_CHECK( rFile.serializeRenditionSyncStartElement() );

    //now write whipx hint to w2xserializer
    DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
    if (!pW2XSerializer)
        return WT_Result::Internal_Error;

    pW2XSerializer->startElement(XamlXML::kpzLayer_Element);
    WT_XAML_Layer& rLayer = const_cast<WT_XAML_Layer &>(*this);
    // Have we seen this layer before?
    bool found = !!rFile.layer_list().find_layer_from_index (rLayer.layer_num());
    if (!found)
    {
        if(rLayer.layer_name().is_ascii())
            pW2XSerializer->addAttribute(XamlXML::kpzName_Attribute,rLayer.layer_name().ascii());
        else
        {
            wchar_t *pString = WT_String::to_wchar( rLayer.layer_name().length(), rLayer.layer_name() );
            if (pString == NULL)
                return WT_Result::Out_Of_Memory_Error;
            pW2XSerializer->addAttribute( XamlXML::kpzName_Attribute, pString );
            delete [] pString; //This was not allocated with DWFCORE_ALLOC_MEMORY
        }
        rFile.layer_list().add_layer(*this);
    }

    pW2XSerializer->addAttribute(XamlXML::kpzNumber_Attribute,(int)rLayer.layer_num());

    pW2XSerializer->endElement();

	return WT_Result::Success;
}

WT_Result WT_XAML_Layer::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& rFile)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

	const char** pLayerName = rMap.find(XamlXML::kpzName_Attribute);
	const char** pLayerNumber = rMap.find(XamlXML::kpzNumber_Attribute);
	if((pLayerNumber != NULL && *pLayerNumber != NULL))
	{
		layer_num() = (WT_Integer32)atoi(*pLayerNumber);
	}

    if((pLayerName != NULL && *pLayerName != NULL))
    {	
		// use DWFString to decode UTF8 code to wchar
		DWFString convertor;
		size_t Bytes = DWFCORE_ASCII_STRING_LENGTH(*pLayerName);
		wchar_t* pString = DWFCORE_ALLOC_MEMORY( wchar_t, Bytes + 1 );
		convertor.DecodeUTF8(*pLayerName, Bytes, pString, (Bytes + 1) * sizeof(wchar_t));
        set((WT_Unsigned_Integer16*)pString, rFile);
		delete [] pString;
    }

    materialized() = WD_True;
	return WT_Result::Success;

}

