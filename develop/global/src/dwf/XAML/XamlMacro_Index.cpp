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

WT_Result WT_XAML_Macro_Index::serialize(WT_File& file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Macro_Index::serialize( *rFile.w2dContentFile() );
    }

    WD_CHECK( rFile.dump_delayed_drawable() ); //dumps any delayed drawable first

    //Serialize RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncStartElement() );

	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;
	
	pW2XSerializer->startElement(XamlXML::kpzMacro_Index_Element );
    pW2XSerializer->addAttribute(XamlXML::kpzIndex_Attribute, (int)macro() );
	pW2XSerializer->endElement();

	//good to go
    return WT_Result::Success;
}

WT_Result 
WT_XAML_Macro_Index::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

	const char** pMacroIndex = rMap.find(XamlXML::kpzIndex_Attribute);
	if(pMacroIndex != NULL && *pMacroIndex != NULL)
	{
		macro() = (WT_Integer32)atoi(*pMacroIndex);
	}
    else
    {
        return WT_Result::Corrupt_File_Error;
    }

    materialized() = WD_True;
	return WT_Result::Success;

}
