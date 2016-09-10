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

WT_Result WT_XAML_Attribute_URL::serialize(WT_File& file) const
{
	WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
	if (rFile.serializingAsW2DContent() )
	{
		if ( !rFile.w2dContentFile() )
			return WT_Result::Toolkit_Usage_Error;
		else
			return WT_Attribute_URL::serialize( *rFile.w2dContentFile() );
	}

	WD_CHECK( rFile.dump_delayed_drawable() ); //dumps any delayed drawable first

	//Serialize RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncStartElement() );

	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;
	
	pW2XSerializer->startElement(XamlXML::kpzAttribute_URL_Element);
	WT_XAML_Attribute_URL& rAttributeUrl = const_cast<WT_XAML_Attribute_URL &>(*this);
	if(attribute_id() >= 0)
		pW2XSerializer->addAttribute(XamlXML::kpzId_Attribute,(int)attribute_id());

	int iCount = rAttributeUrl.url().count();
	if(iCount)
	{
		WD_CHECK( rAttributeUrl.serializeUrlList(rFile));
	}
    
	pW2XSerializer->endElement();

	return WT_Result::Success;
}

WT_Result WT_XAML_Attribute_URL::serializeUrlList(WT_XAML_File& rFile)
{
	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;

	WT_URL_Item *current     = (WT_URL_Item *) url().get_head();

    WT_URL_List & master_list = rFile.desired_rendition().url_lookup_list();

    while (current)
    {
        WT_Integer32 index = master_list.index_from_url_item( *current );

        if ( current->index() != index )
        {
            if (current->index() >= 0 && index < 0 )
            {
                //couldn't find it, so re-key the index and send it out.
                current->index() = master_list.count();
                master_list.add( *current );
            }
            else
            {
                current->index() = index;
            }
        }

		pW2XSerializer->startElement(XamlXML::kpzURL_Item_Element);
        if (master_list.check_whether_emitted(current->index()))
        {
			pW2XSerializer->addAttribute(XamlXML::kpzIndex_Attribute,(int)current->index());
        }
        else
        {
			pW2XSerializer->addAttribute(XamlXML::kpzIndex_Attribute,(int)current->index());
			wchar_t* pStr = WT_String::to_wchar(current->address().length(),current->address().unicode());
			if (pStr == NULL)
				return WT_Result::Out_Of_Memory_Error;
			pW2XSerializer->addAttribute(XamlXML::kpzAddress_Attribute,pStr);
			delete [] pStr; //not allocated using DWFCORE_ALLOC_MEMORY;
			pStr = WT_String::to_wchar(current->friendly_name().length(),current->friendly_name().unicode());
			if (pStr == NULL)
				return WT_Result::Out_Of_Memory_Error;
			pW2XSerializer->addAttribute(XamlXML::kpzFriendly_Name_Attribute,pStr);  
			delete [] pStr; //not allocated using DWFCORE_ALLOC_MEMORY;
            rFile.desired_rendition().url_lookup_list().set_as_being_emitted(current->index());
        }

        current = (WT_URL_Item *) current->next();
		pW2XSerializer->endElement();
	}
	return WT_Result::Success;
}

WT_Result WT_XAML_Attribute_URL::providePathAttributes( XamlDrawableAttributes::PathAttributeConsumer* pConsumer, WT_XAML_File& ) const
{
    return pConsumer->consumeNavigateUri( const_cast<WT_XAML_Attribute_URL*>(this) );
}

WT_Result WT_XAML_Attribute_URL::provideNavigateUri(XamlDrawableAttributes::NavigateUri*& rpNavigateUri)
{
	int iCount = url().count();
	if(!iCount)
		return WT_Result::Success;

	if (rpNavigateUri == NULL)
    {
        rpNavigateUri = DWFCORE_ALLOC_OBJECT( XamlDrawableAttributes::NavigateUri() );
        if (rpNavigateUri == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

	//XAML doesnt support multiple Uris so we want to set to atleast one
    //Check if XamlUri exists then set that otherwise
	//set the first one from the list as it is(i.e. whip format)
    if(xamlURL().bytes())
        rpNavigateUri->uri() = (const wchar_t*)xamlURL();
    else
    {
	    WT_URL_Item* firstUrl = (WT_URL_Item *)url().get_head(); 
	    if(firstUrl)
	    {
		    wchar_t* pUri = WT_String::to_wchar(firstUrl->address().length(),firstUrl->address());
		    rpNavigateUri->uri() = pUri;
		    delete [] pUri; //not allocated using DWFCORE_ALLOC_MEMORY;
	    }
    }

    return WT_Result::Success;
}


WT_Result WT_XAML_Attribute_URL::provideGlyphsAttributes( XamlDrawableAttributes::GlyphsAttributeConsumer* pConsumer, WT_XAML_File& ) const
{
    return pConsumer->consumeNavigateUri( const_cast<WT_XAML_Attribute_URL*>(this) );
}

WT_Result WT_XAML_Attribute_URL::provideCanvasAttributes( XamlDrawableAttributes::CanvasAttributeConsumer* pConsumer, WT_XAML_File& ) const
{
    return pConsumer->consumeNavigateUri( const_cast<WT_XAML_Attribute_URL*>(this) );
}

/// partial materialization of shell from W2X
WT_Result 
WT_XAML_Attribute_URL::parseAttributeList(XamlXML::tAttributeMap& rMap,WT_XAML_File& file)
{
    //Empty Attribute_URL elements are fine, indicates we're closing the hyperlink "container"
	if(!rMap.size())
    {
        materialized() = WD_True;
		return WT_Result::Success;
    }

    //attribute_Id
    const char** pAttributeId = rMap.find(XamlXML::kpzId_Attribute);
	if(pAttributeId != NULL && *pAttributeId != NULL)
	{
		attribute_id() = (WT_Integer32)atoi(*pAttributeId);
	}
    else
    {
        //Index
        WT_Integer32 iIndex = 0;
        const char** pIndex = rMap.find(XamlXML::kpzIndex_Attribute);
	    if(pIndex != NULL && *pIndex != NULL)
	    {
            iIndex = (WT_Integer32) atoi(*pIndex);
        }
        const char** pAddress = rMap.find(XamlXML::kpzAddress_Attribute);
        const char** pFriendlyName = rMap.find(XamlXML::kpzFriendly_Name_Attribute);
	    if((pAddress != NULL && *pAddress != NULL) &&
           (pFriendlyName != NULL && *pFriendlyName != NULL) )
	    {
			DWFString convertor;
			size_t Bytes = DWFCORE_ASCII_STRING_LENGTH(*pFriendlyName);
			wchar_t* pString = DWFCORE_ALLOC_MEMORY( wchar_t, Bytes + 1 );
			convertor.DecodeUTF8(*pFriendlyName, Bytes, pString, (Bytes + 1) * sizeof(wchar_t));

			WT_URL_Item tmp_item(iIndex, WT_String(*pAddress), WT_String(pString));

			delete [] pString;

			WT_Integer32 index = file.desired_rendition().url_lookup_list().index_from_url_item(tmp_item);
			if (index >= 0)
			{
				tmp_item.index() = index;
			}
			else
			{
				file.desired_rendition().url_lookup_list().add(tmp_item);
			}

			m_url_list.add(tmp_item);
        }
        else
        {
            WT_URL_Item *pItem = file.desired_rendition().url_lookup_list().url_item_from_index(iIndex);
            if(pItem)
                add_url_optimized(*pItem,file);
        }
		
        materialized() = WD_True;
    }
	return WT_Result::Success;

}

