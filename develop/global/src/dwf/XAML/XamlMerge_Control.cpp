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

WT_Result WT_XAML_Merge_Control::serialize(WT_File & file) const
{
	//Serialize in w2x
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Merge_Control::serialize( *rFile.w2dContentFile() );
    }

    WD_CHECK( rFile.dump_delayed_drawable() );

	//Serialize RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncStartElement() );

	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;
	
    pW2XSerializer->startElement(XamlXML::kpzMerge_Control_Element);

    switch ( get_merge_control() )
    {
        case WT_Merge_Control::Opaque:
            pW2XSerializer->addAttribute(XamlXML::kpzStyle_Attribute, XamlXML::kpzOpaque_Value );
            break;
        case WT_Merge_Control::Merge:
    	    pW2XSerializer->addAttribute(XamlXML::kpzStyle_Attribute, XamlXML::kpzMerge_Value );
            break;
        case WT_Merge_Control::Transparent:
	        pW2XSerializer->addAttribute(XamlXML::kpzStyle_Attribute, XamlXML::kpzTransparent_Value );
            break;
    }

	pW2XSerializer->endElement();

	return WT_Result::Success;
}

WT_Result 
WT_XAML_Merge_Control::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

	const char** pMergeControl = rMap.find(XamlXML::kpzStyle_Attribute);
	if(pMergeControl != NULL && *pMergeControl != NULL)
	{
		if ( DWFCORE_COMPARE_ASCII_STRINGS(*pMergeControl, XamlXML::kpzOpaque_Value) == 0)
			set_merge_control( WT_Merge_Control::Opaque );
		else if( DWFCORE_COMPARE_ASCII_STRINGS(*pMergeControl, XamlXML::kpzMerge_Value) == 0)
			set_merge_control( WT_Merge_Control::Merge );
		else if( DWFCORE_COMPARE_ASCII_STRINGS(*pMergeControl, XamlXML::kpzTransparent_Value) == 0)
			set_merge_control( WT_Merge_Control::Transparent );        
	}
    else
    {
        return WT_Result::Corrupt_File_Error;
    }

    materialized() = WD_True;
	return WT_Result::Success;

}
