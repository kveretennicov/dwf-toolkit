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
WT_Result WT_XAML_User_Fill_Pattern::providePathAttributes(
    XamlDrawableAttributes::PathAttributeConsumer *pConsumer,
    WT_XAML_File& rFile ) const
{
    if(!pConsumer)
    {
        return WT_Result::Internal_Error;
    }

    const_cast<WT_XAML_User_Fill_Pattern*>(this)->_pSerializeFile = &rFile;
    return pConsumer->consumeFill( const_cast<WT_XAML_User_Fill_Pattern*>(this) );
}

//
// from XamlDrawableAttributes::Fill::Provider
//
WT_Result WT_XAML_User_Fill_Pattern::provideFill(
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

    if(fill_pattern())
    {
        rpFill->brush() =
            DWFCORE_ALLOC_OBJECT(XamlBrush::XamlUserPatternBrush(const_cast<WT_User_Fill_Pattern::Fill_Pattern*>( fill_pattern() ), 1. ) );
    }

    return WT_Result::Success;
}

WT_Result WT_XAML_User_Fill_Pattern::serialize(
    WT_File &file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_User_Fill_Pattern::serialize( *rFile.w2dContentFile() );
    }


    WD_CHECK( rFile.dump_delayed_drawable() );

    //Serialize W2XAttributes (top element) if not yet done
    WD_CHECK( rFile.serializeRenditionSyncStartElement());

    //now write whipx hint to w2xserializer
    DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
    if (!pW2XSerializer)
        return WT_Result::Internal_Error;

    pW2XSerializer->startElement(XamlXML::kpzUser_Fill_Pattern_Element);
    //Pattern_number
    pW2XSerializer->addAttribute(XamlXML::kpzId_Attribute,(int)pattern_number());
    //Pattern_Scale
    if(pattern_scale())
        pW2XSerializer->addAttribute(XamlXML::kpzScale_Attribute,(double)pattern_scale());
    if(fill_pattern())
    {
        //rows
        pW2XSerializer->addAttribute(XamlXML::kpzRows_Attribute,(int)fill_pattern()->rows());
        //columns
        pW2XSerializer->addAttribute(XamlXML::kpzColumns_Attribute,(int)fill_pattern()->columns());
        //data-size
        pW2XSerializer->addAttribute(XamlXML::kpzData_Size_Attribute,(int)fill_pattern()->data_size());
        //data
        size_t nOutBufferBytes = (fill_pattern()->data_size() * 4) + 1;
        WT_Byte* pOutBuffer = DWFCORE_ALLOC_MEMORY( WT_Byte, nOutBufferBytes );
        if (pOutBuffer == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
        size_t nLength = DWFString::EncodeBase64( fill_pattern()->data(), fill_pattern()->data_size(), pOutBuffer, nOutBufferBytes );
        DWFString zValue( (const char*) pOutBuffer, nLength ); 
        DWFCORE_FREE_MEMORY( pOutBuffer );

        pW2XSerializer->addAttribute( XamlXML::kpzData_Attribute, zValue );
    }

    pW2XSerializer->endElement();

    return WT_Result::Success;
}

WT_Result
WT_XAML_User_Fill_Pattern::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
    if(!rMap.size())
        return WT_Result::Internal_Error;

    //Pattern_number
    const char** ppValue = rMap.find(XamlXML::kpzId_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
		pattern_number() = (WT_Integer16)atoi(*ppValue);
	}
    else
    {
        return WT_Result::Corrupt_File_Error;
    }
    //pattern_scale if its there - it is possible not to have it
    ppValue = rMap.find(XamlXML::kpzScale_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
        pattern_scale() = atof(*ppValue);
		is_scale_used() = WD_True;
	}
    else
        is_scale_used() = WD_False;

    WT_Unsigned_Integer16   rows = 0;
    WT_Unsigned_Integer16   columns = 0;
    WT_Unsigned_Integer32  data_size = 0;
    WT_Byte * data = WD_Null;
    //Rows
    ppValue = rMap.find(XamlXML::kpzRows_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
        rows = (WT_Unsigned_Integer16)atoi(*ppValue);
    }
    //Columns
    ppValue = rMap.find(XamlXML::kpzColumns_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
        columns = (WT_Unsigned_Integer16)atoi(*ppValue);
    }
    //Data_size
    ppValue = rMap.find(XamlXML::kpzData_Size_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
        data_size = (WT_Unsigned_Integer32)atoi(*ppValue);
    }
    //Data
    ppValue = rMap.find(XamlXML::kpzData_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
        //DecodeBase64 data
        size_t nLen = DWFCORE_ASCII_STRING_LENGTH( *ppValue );
        size_t nRequired = DWFString::DecodeBase64( *ppValue, nLen, NULL, 0 );
        data = DWFCORE_ALLOC_MEMORY( WT_Byte, nRequired+1 ); //cant be anymore than the encoded version
        if (data == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        size_t nProcessed = DWFString::DecodeBase64( *ppValue, nLen, data, nRequired );
        WD_Assert( nProcessed == nRequired );

        if ( (nProcessed != nRequired) && ((int)nProcessed != (int)data_size) )
        {
            return WT_Result::Internal_Error;
        }        

    }
    //if we get everything ok then construct Fill_Pattern here
    if(rows && columns && data_size && data)
    {
        fill_pattern() = WT_User_Fill_Pattern::Fill_Pattern::Construct(rows,columns,data_size,data);
        if(!fill_pattern())
            return WT_Result::Out_Of_Memory_Error;
        fill_pattern()->increment();        
    }

    materialized() = WD_True;
    if(data)
        DWFCORE_FREE_MEMORY( data );    

    return WT_Result::Success;
}
