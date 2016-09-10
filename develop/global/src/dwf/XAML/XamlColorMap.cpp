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

WT_Result WT_XAML_Color_Map::serialize(WT_File &file) const
{
	WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Color_Map::serialize( *rFile.w2dContentFile() );
    }

    WD_CHECK( rFile.dump_delayed_drawable() ); //dumps any delayed drawable first

    //Serialize RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncStartElement() );

    return serializeRawColorMap( rFile );
}

WT_Result WT_XAML_Color_Map::serializeRawColorMap(WT_XAML_File &rFile) const
{
	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;
	
	pW2XSerializer->startElement(XamlXML::kpzColor_Map_Element);
	if(size())
	{
		pW2XSerializer->addAttribute(XamlXML::kpzCount_Attribute,size());
		DWFString strMap;
		for(int i=0;i<size();i++)
		{
			wchar_t buf[16];
			WT_RGBA32 rgba = map((WT_Byte)i);
			if (rgba.m_rgb.a != 0xFF)
			{
                _DWFCORE_SWPRINTF(buf, 16, L"#%02X%02X%02X%02X", rgba.m_rgb.a, rgba.m_rgb.r, rgba.m_rgb.g, rgba.m_rgb.b );
			}
			else
			{
                _DWFCORE_SWPRINTF(buf, 16, L"#%02X%02X%02X", rgba.m_rgb.r, rgba.m_rgb.g, rgba.m_rgb.b );
			}
			strMap.append(buf);
			strMap.append(L" ");
		}
		pW2XSerializer->addAttribute(XamlXML::kpzColor_Attribute, strMap);
	}
	pW2XSerializer->endElement();

	//good to go
    return WT_Result::Success;
}


WT_Result 
WT_XAML_Color_Map::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& rFile)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

    //<Color_Map Count="3" Color="FF0000 00FF00 0000FF "/>
    int iCount = 0;
    const char** pCount = rMap.find(XamlXML::kpzCount_Attribute);
	if(pCount != NULL && *pCount != NULL)
	{
       iCount = atoi(*pCount);
    }
    if(iCount)
    {
       WT_RGBA32* pMap = DWFCORE_ALLOC_MEMORY( WT_RGBA32, iCount );
        //Colors are separated  by spaces so read tokens for each color
       const char** pColor = rMap.find(XamlXML::kpzColor_Attribute);
       if(pColor != NULL && *pColor != NULL)
	   {
           iCount = 0;
           char* pSavePtr;
           char* token = DWFCORE_ASCII_STRING_TOKENIZE( (char *)(*pColor), /*NOXLATE*/" ", &pSavePtr );
           while( token != NULL )
           {
               WD_CHECK( XamlBrush::Brush::ReadColor(pMap[iCount++],token) );
               /* Get next token: */
               token = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
           }

           WT_Result result = set(iCount,pMap,rFile);
           DWFCORE_FREE_MEMORY( pMap );
           materialized() = WD_True;
           return result;
       }       
    }

	return WT_Result::Success;
}
