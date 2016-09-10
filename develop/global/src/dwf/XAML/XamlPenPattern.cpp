//
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

WT_Result WT_XAML_Pen_Pattern::serialize(WT_File& file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Pen_Pattern::serialize( *rFile.w2dContentFile() );
    }

    WD_CHECK( rFile.dump_delayed_drawable() );

	//Serialize W2XAttributes (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncStartElement());

	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;
	
    pW2XSerializer->startElement(XamlXML::kpzPen_Pattern_Element);

	pW2XSerializer->addAttribute(XamlXML::kpzId_Attribute,pen_pattern());
	pW2XSerializer->addAttribute(XamlXML::kpzScreening_Percentage_Attribute,(int)screening_percentage());
	if(color_map() == NULL)
		pW2XSerializer->addAttribute(XamlXML::kpzColorMap_Flag_Attribute,L"0");
	else
	{
		pW2XSerializer->addAttribute(XamlXML::kpzColorMap_Flag_Attribute,L"1");
		WT_XAML_Class_Factory xamlCF;
		WT_Color_Map* pClrMap = xamlCF.Create_Color_Map(*color_map());
		pClrMap->serialize(rFile);
		xamlCF.Destroy(pClrMap);
	}

	pW2XSerializer->endElement();

    return WT_Result::Success;
}

WT_Result WT_XAML_Pen_Pattern::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

	const char** pPatId = rMap.find(XamlXML::kpzId_Attribute);
	const char** pScreeningPercentage = rMap.find(XamlXML::kpzScreening_Percentage_Attribute);
	const char** pColormapFlag = rMap.find(XamlXML::kpzColorMap_Flag_Attribute);

	if(pPatId != NULL && *pPatId != NULL)
	{
		WT_Pen_Pattern::WT_Pattern_ID nPatId = (WT_Pen_Pattern::WT_Pattern_ID)atoi(*pPatId);
		if((nPatId <= 0) || (nPatId >= Count))
			return WT_Result::Internal_Error;

		set_pen_pattern(nPatId);
	}
	if(pScreeningPercentage != NULL && *pScreeningPercentage != NULL)
	{
		set_screening_percentage((WT_Unsigned_Integer32)atoi(*pScreeningPercentage));
	}

	if(pColormapFlag != NULL && *pColormapFlag != NULL)
	{
		int iColorMapFlag = atoi(*pColormapFlag);
		if(iColorMapFlag)
		{
			//TODO: materialize colormap here
		}
		
	}

    materialized() = WD_True;
	return WT_Result::Success;

}
