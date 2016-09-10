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

WT_Result
WT_XAML_UserData::serialize(WT_File & file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_UserData::serialize( *rFile.w2dContentFile() );
    }

    DWFXMLSerializer* pSerializer = rFile.w2xSerializer();

    WD_CHECK (rFile.dump_delayed_drawable());
    WD_CHECK (rFile.serializeRenditionSyncEndElement());

    pSerializer->startElement( XamlXML::kpzUserData_Element );

    if( data_description().length() )
    {   
        wchar_t *buf = WT_String::to_wchar( data_description().length(), data_description().unicode() );
        if (buf != NULL)
        {
            pSerializer->addAttribute( XamlXML::kpzDescription_Attribute, buf );
            delete [] buf; //not allocated using DWFCORE_ALLOC_MEMORY;
        }
        else 
            return WT_Result::Out_Of_Memory_Error;
    }

    if ( data_size() > 0 && data() != NULL )
    {

        pSerializer->addAttribute( XamlXML::kpzData_Size_Attribute, (int)data_size() );

        //Base64 encode the data
        size_t nRequired = DWFString::EncodeBase64( data(), data_size(), NULL, 0 );
        WT_Byte* pChars = DWFCORE_ALLOC_MEMORY(WT_Byte, nRequired);
        if (pChars == NULL)
            return WT_Result::Out_Of_Memory_Error;

        size_t nProcessed = DWFString::EncodeBase64( data(), data_size(), pChars, nRequired );
        WD_Assert( nProcessed == nRequired );

        if (nProcessed != nRequired)
        {
            return WT_Result::Internal_Error;
        }
        
        DWFString zCData( (char*)pChars, nProcessed );
        WD_Assert( zCData.chars() > 0 );

        pSerializer->addCData( zCData );
        DWFCORE_FREE_MEMORY( pChars );
    }

    pSerializer->endElement();

	return WT_Result::Success;
}
/// partial materialization of shell from W2X
WT_Result 
WT_XAML_UserData::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Success;

	const char **ppValue = rMap.find( XamlXML::kpzDescription_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
       data_description() = *ppValue; 
    }

    ppValue = rMap.find( XamlXML::kpzData_Size_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
       data_size() = (WT_Integer32)atoi(*ppValue); 
    }
    else
        materialized() = WD_True;

	return WT_Result::Success;

}

WT_Result
WT_XAML_UserData::parseCData(int nLen,const char* pCData)
{
    //Data was EncodedBase64 so lets DecodeBase64
    //Base64 decode the data
    size_t nRequired = DWFString::DecodeBase64( pCData, nLen, NULL, 0 );
    WT_Byte* pChars = DWFCORE_ALLOC_MEMORY(WT_Byte, nRequired+1);
    if (pChars == NULL)
        return WT_Result::Out_Of_Memory_Error;

    size_t nProcessed = DWFString::DecodeBase64( pCData, nLen, pChars, nRequired );
    WD_Assert( nProcessed == nRequired );

    if ( (nProcessed != nRequired) && ((int)nProcessed != data_size()) )
    {
        return WT_Result::Internal_Error;
    }
   
    WT_Result result = set_data((WT_Integer32)nProcessed,pChars);
    materialized() = WD_True;
    DWFCORE_FREE_MEMORY( pChars );

    return result;
}

