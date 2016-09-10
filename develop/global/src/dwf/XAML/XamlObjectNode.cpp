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

WT_Result WT_XAML_Object_Node::serialize(WT_File & file) const
{
	//Serialize in w2x
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Object_Node::serialize( *rFile.w2dContentFile() );
    }

    WD_CHECK( rFile.dump_delayed_drawable() );

	//Serialize RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncStartElement() );

	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;
	
    pW2XSerializer->startElement(XamlXML::kpzObject_Node_Element);
	WT_XAML_Object_Node& rObject_Node = const_cast<WT_XAML_Object_Node &>(*this);
	// Have we seen this object_node before?
    bool found = !!rFile.object_node_list().find_object_node_from_index (rObject_Node.object_node_num());
    if (!found)
	{
		if(rObject_Node.object_node_name().is_ascii())
			pW2XSerializer->addAttribute(XamlXML::kpzName_Attribute,rObject_Node.object_node_name().ascii());
		else
		{
			wchar_t *pString = WT_String::to_wchar( rObject_Node.object_node_name().length(), rObject_Node.object_node_name() );
			if (pString == NULL)
				return WT_Result::Out_Of_Memory_Error;
			pW2XSerializer->addAttribute( XamlXML::kpzName_Attribute, pString );
			delete [] pString; //This was not allocated with DWFCORE_ALLOC_MEMORY
		}
		rFile.object_node_list().add_object_node(*this);
	}

	pW2XSerializer->addAttribute(XamlXML::kpzNumber_Attribute,(int)rObject_Node.object_node_num());

	pW2XSerializer->endElement();

	return WT_Result::Success;
}

WT_Result WT_XAML_Object_Node::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& rFile)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

	const char** pNodeName = rMap.find(XamlXML::kpzName_Attribute);
	const char** pNodeNumber = rMap.find(XamlXML::kpzNumber_Attribute);
	if( (pNodeName != NULL && *pNodeName != NULL) &&
		(pNodeNumber != NULL && *pNodeNumber != NULL) )
	{
		set(rFile,(WT_Integer32)atoi(*pNodeNumber),*pNodeName);
	}
	else if(pNodeNumber != NULL && *pNodeNumber != NULL)
		set(rFile,(WT_Integer32)atoi(*pNodeNumber));

    materialized() = WD_True;
	return WT_Result::Success;

}
