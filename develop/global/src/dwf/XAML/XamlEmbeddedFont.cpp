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
#include "XAML/OpcResourceSerializer.h"
#include "dwfcore/MIME.h"

WT_Result
WT_XAML_Embedded_Font::serialize (WT_File & file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Embedded_Font::serialize( *rFile.w2dContentFile() );
    }

    WT_OpcResourceSerializer *pResourceSerializer = rFile.opcResourceSerializer();
    if (pResourceSerializer == NULL)
    {
        return WT_Result::Internal_Error;
    }

    DWFString zFontPath;
    DWFOutputStream *pOutputStream = NULL;

    //The toolkit will perform obfuscation since it needs to control the obfuscation key,
    //And there is no way to get the key name through the WT_Embedded_Font API.
    //In any case, it's just plain wrong to encrypt the blob outside of here.
    WD_Assert( ! (request_type() & WT_Embedded_Font::Encrypt_Data) );
    if (request_type() & WT_Embedded_Font::Encrypt_Data)
    {
        return WT_Result::Toolkit_Usage_Error;
    }

    if (rFile.obfuscate_embedded_fonts())
    {
        if (request_type() & WT_Embedded_Font::Encrypt_Data)
        {
            return WT_Result::Toolkit_Usage_Error;
        }

        WD_CHECK( pResourceSerializer->getPartOutputStream( DWFCore::DWFMIME::kzMIMEType_OBFUSCATEDOPENTYPE, zFontPath, &pOutputStream ) );

        if (pOutputStream == NULL)
        {
            return WT_Result::File_Open_Error;
        }

        char key[16];
        WD_CHECK( WT_XAML_Font::parseKey( key, zFontPath ) );
        
        DWFBufferInputStream oInputStream( data(), data_size() );
        WD_CHECK( WT_XAML_Font::obfuscate( &oInputStream, pOutputStream, key) );
    }
    else
    {
        DWFString zMime = L"application/vnd.ms-opentype";
        WD_CHECK( pResourceSerializer->getPartOutputStream( zMime, zFontPath, &pOutputStream ) );

        if (pOutputStream == NULL)
        {
            return WT_Result::File_Open_Error;
        }

        if ( (size_t)data_size() != pOutputStream->write( data(), data_size() ) )
        {
            return WT_Result::File_Write_Error;
        }
    }

    pOutputStream->flush();
    DWFCORE_FREE_OBJECT( pOutputStream );

    WT_String zFontName( font_type_face_name_length(), font_type_face_name_string() );
    wchar_t *pFontName = WT_String::to_wchar( zFontName.length(), zFontName );
    if (pFontName == NULL)
        return WT_Result::Out_Of_Memory_Error;

    WD_CHECK( rFile.registerFontUri( pFontName, zFontPath ) );

    WT_String zLogfontName( font_logfont_name_length(), font_logfont_name_string() );
    wchar_t *pLogfontName = WT_String::to_wchar( zLogfontName.length(), zLogfontName );
    if (pLogfontName == NULL)
        return WT_Result::Out_Of_Memory_Error;

	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;

    pW2XSerializer->startElement( XamlXML::kpzEmbedded_Font_Element );
    pW2XSerializer->addAttribute( XamlXML::kpzRequest_Attribute, (int)request_type() );
    pW2XSerializer->addAttribute( XamlXML::kpzPrivilege_Attribute, (int)privilege() );
    pW2XSerializer->addAttribute( XamlXML::kpzCharset_Attribute, (int)character_set_type() );
    pW2XSerializer->addAttribute( XamlXML::kpzFontUri_Attribute, zFontPath );
    pW2XSerializer->addAttribute( XamlXML::kpzCannonical_Attribute, pFontName );
    pW2XSerializer->addAttribute( XamlXML::kpzLogfont_Attribute, pFontName );
    pW2XSerializer->endElement();

    delete [] pFontName; //not allocated using DWFCORE_ALLOC_MEMORY;
    delete [] pLogfontName; //not allocated using DWFCORE_ALLOC_MEMORY;

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Embedded_Font::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& rFile)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

    DWFString zFontPath;
	const char** ppValue = rMap.find(XamlXML::kpzRequest_Attribute);
	if(ppValue!=NULL && *ppValue!=NULL)
	{
        request_type() = (WT_Unsigned_Integer32) atoi( *ppValue );
	}
    ppValue = rMap.find(XamlXML::kpzPrivilege_Attribute);
	if(ppValue!=NULL && *ppValue!=NULL)
	{
        privilege() = (WT_Byte) atoi( *ppValue );
	}
    ppValue = rMap.find(XamlXML::kpzCharset_Attribute);
	if(ppValue!=NULL && *ppValue!=NULL)
	{
        character_set_type() = (WT_Byte) atoi( *ppValue );
	}
    ppValue = rMap.find(XamlXML::kpzFontUri_Attribute);
	if(ppValue!=NULL && *ppValue!=NULL)
	{
        zFontPath = *ppValue;
	}
    ppValue = rMap.find(XamlXML::kpzCannonical_Attribute);
	if(ppValue!=NULL && *ppValue!=NULL)
	{
        WT_String zTemp = *ppValue;
        font_type_face_name_length() = zTemp.length();
        font_type_face_name_string() = DWFCORE_ALLOC_MEMORY( WT_Byte, font_type_face_name_length() );
        if ( font_type_face_name_string() == NULL )
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        DWFCORE_ASCII_STRING_COPY_LENGTH( (char*)font_type_face_name_string(), *ppValue, font_type_face_name_length() );
	}
    ppValue = rMap.find(XamlXML::kpzLogfont_Attribute);
	if(ppValue!=NULL && *ppValue!=NULL)
	{
        WT_String zTemp = *ppValue;
        font_logfont_name_length() = zTemp.length();
        font_logfont_name_string() = DWFCORE_ALLOC_MEMORY( WT_Byte, font_logfont_name_length() );
        if ( font_logfont_name_string() == NULL )
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        DWFCORE_ASCII_STRING_COPY_LENGTH( (char*)font_logfont_name_string(), *ppValue, font_logfont_name_length() );
	}

    //Get the data from the OpcResourceMaterializer via the fontURI
    WT_OpcResourceMaterializer* pMaterializer = rFile.opcResourceMaterializer();
    if (pMaterializer == NULL)
    {
        return WT_Result::Internal_Error;
    }

    DWFInputStream *pInputStream = NULL;
    DWFBufferOutputStream *pOutputStream = DWFCORE_ALLOC_OBJECT( DWFBufferOutputStream( 32768 ) ); //32K, reasonable for a font
    if ( pOutputStream == NULL )
    {
        return WT_Result::Out_Of_Memory_Error;
    }

    WD_CHECK( pMaterializer->getPartInputStream( zFontPath, &pInputStream ) );
    if ( pInputStream == NULL )
    {
        return WT_Result::Internal_Error;
    }

    char buf[1024];
    while ( pInputStream->available() )
    {
        size_t nRead = pInputStream->read( buf, 1024 );
        if (nRead)
        {
            size_t nWritten = pOutputStream->write( buf, nRead );
            if (nWritten != nRead)
            {
                WD_Assert (false);
                DWFCORE_FREE_OBJECT( pOutputStream );
                DWFCORE_FREE_OBJECT( pInputStream );
                return WT_Result::Internal_Error;
            }
        }
    }
    
    void *pBuffer = NULL;
    data_size() = (WT_Integer32) pOutputStream->buffer( &pBuffer );
    data() = (WT_Byte*)pBuffer;

    DWFCORE_FREE_OBJECT( pOutputStream );
    DWFCORE_FREE_OBJECT( pInputStream );

    m_local_data_copy = WD_True;
    materialized() = WD_True;
	return WT_Result::Success;
}
