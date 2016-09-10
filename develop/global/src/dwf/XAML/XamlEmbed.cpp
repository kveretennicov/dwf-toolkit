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
WT_XAML_Embed::serialize(WT_File & file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Embed::serialize( *rFile.w2dContentFile() );
    }

    DWFXMLSerializer* pSerializer = rFile.w2xSerializer();

    WD_CHECK (rFile.dump_delayed_drawable());
    WD_CHECK (rFile.serializeRenditionSyncEndElement());

    pSerializer->startElement( XamlXML::kpzEmbed_Element );

    DWFString zMIME = L"";
    if( MIME_type().length() )
    {
        wchar_t *buf = WT_String::to_wchar( MIME_type().length(), MIME_type().unicode() );
        if (buf != NULL)
        {
            zMIME.append(buf);
            delete [] buf; //not allocated using DWFCORE_ALLOC_MEMORY;
        }
        else 
            return WT_Result::Out_Of_Memory_Error;
    }

    zMIME.append(L"/");

    if( MIME_subtype().length() )
    {
        wchar_t *buf = WT_String::to_wchar( MIME_subtype().length(), MIME_subtype().unicode() );
        if (buf != NULL)
        {
            zMIME.append(buf);
            delete [] buf; //not allocated using DWFCORE_ALLOC_MEMORY;
        }
        else 
            return WT_Result::Out_Of_Memory_Error;
    }

    zMIME.append(L";");

    if( MIME_options().length() )
    {
        wchar_t *buf = WT_String::to_wchar( MIME_options().length(), MIME_options().unicode() );
        if (buf != NULL)
        {
            zMIME.append(buf);
            delete [] buf; //not allocated using DWFCORE_ALLOC_MEMORY;
        }
        else 
            return WT_Result::Out_Of_Memory_Error;
    }

    pSerializer->addAttribute(XamlXML::kpzMIME_Attribute, zMIME);

    if( description().length() )
    {   
        wchar_t *buf = WT_String::to_wchar( description().length(), description().unicode() );
        if (buf != NULL)
        {
            pSerializer->addAttribute( XamlXML::kpzDescription_Attribute, buf );
            delete [] buf; //not allocated using DWFCORE_ALLOC_MEMORY;
        }
        else 
            return WT_Result::Out_Of_Memory_Error;
    }

    if( filename().length() )
    {   
        wchar_t *buf = WT_String::to_wchar( filename().length(), filename().unicode() );
        if (buf != NULL)
        {
            pSerializer->addAttribute( XamlXML::kpzFilename_Attribute, buf );
            delete [] buf; //not allocated using DWFCORE_ALLOC_MEMORY;
        }
        else 
            return WT_Result::Out_Of_Memory_Error;
    }

    if( url().length() )
    {   
        wchar_t *buf = WT_String::to_wchar( url().length(), url().unicode() );
        if (buf != NULL)
        {
            pSerializer->addAttribute( XamlXML::kpzURL_Attribute, buf );
            delete [] buf; //not allocated using DWFCORE_ALLOC_MEMORY;
        }
        else 
            return WT_Result::Out_Of_Memory_Error;
    }

    pSerializer->endElement();

	return WT_Result::Success;
}

WT_Result WT_XAML_Embed::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& rFile)
{
    if(!rMap.size())
		return WT_Result::Internal_Error;

    //MIME_Type
    //MIME_Subtype
    //MIME_Options are in one string like this "MIME_Type/MIME_Subtype;MIME_Options"
    const char **ppValue = rMap.find( XamlXML::kpzMIME_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        //Look for '/' to get MIME_Type
        char* pPart = NULL;
        int iTypePos = 0;
        const char* pMIME_Part = strchr(*ppValue,'/');
        if(pMIME_Part != NULL)
        {
            iTypePos = (int)(pMIME_Part-*ppValue);
            pPart = DWFCORE_ALLOC_MEMORY(char,iTypePos+1);
            DWFCORE_ZERO_MEMORY(pPart,iTypePos+1);
            strncpy(pPart,*ppValue,iTypePos);
            WD_CHECK( set_MIME_type(pPart,rFile) );
            DWFCORE_FREE_MEMORY(pPart);
        }

        //Looking for ';' to get MIME_Subtype
        int iSubtypePos = 0;
        pMIME_Part = strchr(&(*ppValue)[iTypePos],';');
        if(pMIME_Part != NULL)
        {
            iSubtypePos = (int)(pMIME_Part-&(*ppValue)[iTypePos]);
            pPart = DWFCORE_ALLOC_MEMORY(char,iSubtypePos);
            DWFCORE_ZERO_MEMORY(pPart,(iSubtypePos));
            strncpy(pPart,&(*ppValue)[iTypePos+1],iSubtypePos-1);
            WD_CHECK( set_MIME_subtype(pPart,rFile) );
            DWFCORE_FREE_MEMORY(pPart);

             //Finally copy MIME_Options
            WD_CHECK( set_MIME_options(&pMIME_Part[1],rFile) );
        }
        
    }
    

    //Looking for description
    ppValue = rMap.find( XamlXML::kpzDescription_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        WD_CHECK( set_description(*ppValue,rFile) );
    }

    //Looking for filename
    ppValue = rMap.find( XamlXML::kpzFilename_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        WD_CHECK( set_filename(*ppValue,rFile) );
    }

    //Looking for url
    ppValue = rMap.find( XamlXML::kpzURL_Attribute );
    if ( ppValue!=NULL && *ppValue!=NULL )
    {
        WD_CHECK( set_url(*ppValue,rFile) );
    }

    materialized() = WD_True;
	return WT_Result::Success;
}
