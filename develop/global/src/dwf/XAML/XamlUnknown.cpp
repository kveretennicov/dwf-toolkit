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

WT_Result WT_XAML_Unknown::serialize(WT_File& file) const
{

    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_XAML_Unknown::serialize( *rFile.w2dContentFile() );
    }

    DWFXMLSerializer* pSerializer = rFile.w2xSerializer();

    WD_CHECK (rFile.dump_delayed_drawable());
    WD_CHECK (rFile.serializeRenditionSyncEndElement());

    pSerializer->startElement( XamlXML::kpzUnknown_Element );

    int nLen = (const_cast<WT_XAML_Unknown *>(this))->get_pass_thru_byte_length();
    if( nLen )
    {   
        pSerializer->addAttribute( XamlXML::kpzData_Size_Attribute, nLen );

        //Base64 encode the data
        WT_Byte* pData = (const_cast<WT_XAML_Unknown *>(this))->get_pass_thru_bytes();
        size_t nRequired = DWFString::EncodeBase64( pData, nLen, NULL, 0 );
        WT_Byte* pChars = DWFCORE_ALLOC_MEMORY(WT_Byte, nRequired);
        if (pChars == NULL)
            return WT_Result::Out_Of_Memory_Error;

        size_t nProcessed = DWFString::EncodeBase64(pData , nLen, pChars, nRequired );
        WD_Assert( nProcessed == nRequired );

        if (nProcessed != nRequired)
        {
            return WT_Result::Internal_Error;
        }
        
        DWFString zData( (char*)pChars, nProcessed );
        WD_Assert( zData.chars() > 0 );

        pSerializer->addAttribute(XamlXML::kpzData_Attribute,zData);
        DWFCORE_FREE_MEMORY( pChars );
    }

    pSerializer->endElement();

	return WT_Result::Success;
}

WT_Result
WT_XAML_Unknown::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
    WT_Result result = WT_Result::Success;

    //Nothing to parse so return
    if(!rMap.size())
        return WT_Result::Success;

    //pattern_number
    const char** ppValue = rMap.find( XamlXML::kpzData_Size_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
	{
        get_pass_thru_byte_length() = (WT_Unsigned_Integer16)atoi(*ppValue);
        
    }

    if(get_pass_thru_byte_length())
    {
        ppValue = rMap.find( XamlXML::kpzData_Attribute );
        if (ppValue!=NULL && *ppValue!=NULL)
        {
            size_t nLen = DWFCORE_ASCII_STRING_LENGTH(*ppValue);
            size_t nRequired = DWFString::DecodeBase64( *ppValue, nLen, NULL, 0 );
            //parse tokens to get double values for data array
            WT_Byte* pData = DWFCORE_ALLOC_MEMORY(WT_Byte,nRequired+1);
            if(pData == NULL)
                return WT_Result::Out_Of_Memory_Error;

            size_t nProcessed = DWFString::DecodeBase64( *ppValue, nLen, pData, nRequired );
            WD_Assert( nProcessed == nRequired );

            if ( (nProcessed != nRequired) && ((int)nProcessed != get_pass_thru_byte_length()) )
            {
                return WT_Result::Internal_Error;
            }
           
            result = set_pass_thru_bytes((WT_Integer16)nProcessed,pData);
            DWFCORE_FREE_MEMORY( pData );

        }
        else
        {
            return WT_Result::Corrupt_File_Error;
        }
    }

    materialized() = WD_True;
    return result;
}
