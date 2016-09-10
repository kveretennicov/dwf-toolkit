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
#include "whiptk/w2d_class_factory.h"

WT_Result WT_XAML_Overpost::serialize(WT_File & file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Overpost::serialize( *rFile.w2dContentFile() );
    }

    DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();

    if (!pW2XSerializer)
    {
        return WT_Result::Internal_Error;
    }

    WT_Integer32 parts_to_sync = WT_Rendition::BlockRef_Bit             |
        WT_Rendition::Code_Page_Bit            |
        WT_Rendition::Color_Bit                |
        WT_Rendition::Color_Map_Bit            |
        WT_Rendition::Dash_Pattern_Bit         |
        WT_Rendition::Fill_Bit                 |
        WT_Rendition::Fill_Pattern_Bit         |
        WT_Rendition::Font_Bit                 |
        WT_Rendition::Font_Extension_Bit       |
        WT_Rendition::Layer_Bit                |
        WT_Rendition::Line_Pattern_Bit         |
        WT_Rendition::Line_Style_Bit           |
        WT_Rendition::Line_Weight_Bit          |
        WT_Rendition::Macro_Scale_Bit          |
        WT_Rendition::Macro_Index_Bit          |
        WT_Rendition::Merge_Control_Bit        |
        WT_Rendition::Object_Node_Bit          |
        WT_Rendition::Pen_Pattern_Bit          |
        WT_Rendition::Projection_Bit           |
        WT_Rendition::URL_Bit                  |
        WT_Rendition::Viewport_Bit             |
        WT_Rendition::Visibility_Bit           |
        WT_Rendition::Text_Background_Bit      |
        WT_Rendition::Text_HAlign_Bit          |
        WT_Rendition::Text_VAlign_Bit          |
        WT_Rendition::Delineate_Bit            |
        WT_Rendition::User_Fill_Pattern_Bit    |
        WT_Rendition::User_Hatch_Pattern_Bit   |
        WT_Rendition::Contrast_Color_Bit;

    WD_CHECK (rFile.desired_rendition().sync(file, parts_to_sync));

    WT_Boolean bMerging = rFile.heuristics().allow_drawable_merging();

    if(bMerging)
    {
        rFile.heuristics().set_allow_drawable_merging(false);
        WD_CHECK( rFile.dump_delayed_drawable() );
    }

    WD_CHECK( rFile.serializeRenditionSyncEndElement() ); 

	//Serialize to W2X
    pW2XSerializer->startElement(XamlXML::kpzOverpost_Element);
    pW2XSerializer->addAttribute(XamlXML::kpzMode_Attribute, (int) acceptMode());
    pW2XSerializer->addAttribute(XamlXML::kpzRender_Attribute, renderEntities() == WD_True ? 1 : 0);
    pW2XSerializer->addAttribute(XamlXML::kpzExtent_Attribute, addExtents() == WD_True ? 1 : 0);

	//Create a memory file in which we embed the object stream.
    WT_XAML_File::WT_XAML_Memory_File oMemoryFile;
    oMemoryFile.open();
    rFile.serializingAsW2DContent() = true;
    rFile.w2dContentFile() = &oMemoryFile;

	const char* pchRenderEntities = renderEntities() ? str_True : str_False;
    const char* pchAddExtents     = addExtents() ? str_True : str_False;

    // Extended ASCII only output
    WD_CHECK (oMemoryFile.write("(Overpost "));        // Overpost opcode  + space 
	WD_CHECK (oMemoryFile.write(enum_to_string( acceptMode() ))); // Write out the enum 
    WD_CHECK (oMemoryFile.write((WT_Byte) ' '));       // Space
    WD_CHECK (oMemoryFile.write(pchRenderEntities));    // Write out the Boolean  
    WD_CHECK (oMemoryFile.write((WT_Byte) ' '));       // Space
    WD_CHECK (oMemoryFile.write(pchAddExtents));        // Write out the Boolean
    WD_CHECK (oMemoryFile.write((WT_Byte) ' '));       // Space
	WD_CHECK (WT_Overpost::serialize_stream(file));     // serialize the contents 
    WD_CHECK (oMemoryFile.write((WT_Byte)')'));         // write the closing brackets

    rFile.serializingAsW2DContent() = false;
    rFile.w2dContentFile() = NULL;
    oMemoryFile.close();

    //get the buffer into a DWFString
    size_t nOutBufferBytes = (oMemoryFile.pOutputStream->bytes() * 4) + 1;
    char* pOutBuffer = DWFCORE_ALLOC_MEMORY( char, nOutBufferBytes );
    if (pOutBuffer == NULL)
    {
        return WT_Result::Out_Of_Memory_Error;
    }

    size_t nLength = DWFString::EncodeBase64( oMemoryFile.pOutputStream->buffer(), oMemoryFile.pOutputStream->bytes(), pOutBuffer, nOutBufferBytes );
    DWFString zValue( (const char*) pOutBuffer, nLength ); 
    DWFCORE_FREE_MEMORY( pOutBuffer );
    rFile.w2xSerializer()->addCData( zValue );

    rFile.heuristics().set_allow_drawable_merging(bMerging);

    pW2XSerializer->endElement();

    return WT_Result::Success;
}

WT_Result WT_XAML_Overpost::serialize_stream(WT_File & file) const
{
    WT_XAML_File &rFile = static_cast<WT_XAML_File &>(file);

    for(reset(); get_next(); )
    {
        const WT_Object* pObject = object();
        if (pObject != NULL)
        {
            //If it's a drawable, we serialize it.
            if (pObject->object_type() == WT_Object::Drawable)
            {
                WT_Logical_Box rBox = ((WT_Drawable *)pObject)->bounds(&rFile);
                bool bDone = false;
                switch(acceptMode())
                {
                case AcceptAll :
                    {
                    }
                    break;
                case AcceptAllFit :
                    {
                        if(rFile.isFittingInOverpostClip(rBox) == false)
                        {
                            continue;
                        }
                    }
                    break;
                case AcceptFirstFit :
                    {
                        if(rFile.isFittingInOverpostClip(rBox) == true)
                        {
                            bDone = true;
                        }
                    }
                    break;
                default :
                    {
                        return WT_Result::Internal_Error;
                    }
                }
                //
                // add the drawable extents to the global clip 
                //
                if(addExtents() == WD_True)
                {
                    rFile.addToOverpostClip(rBox);
                }

                WD_CHECK( pObject->serialize( file ) );

                if(bDone)
                {
                    break;
                }
            }

            //
            // not clear according to the spec : should we process attributes ?
            //
            else if (pObject->object_type() == WT_Object::Attribute)
            {
                switch( pObject->object_id() )
                {
                case WT_Object::Color_ID:
                    file.desired_rendition().color() = *(WT_Color*)(pObject);
                    break;
                case WT_Object::Dash_Pattern_ID:
                    file.desired_rendition().dash_pattern() = *(WT_Dash_Pattern*)(pObject);
                    break;
                case WT_Object::Fill_ID:
                    file.desired_rendition().fill() = *(WT_Fill*)(pObject);
                    break;
                case WT_Object::Fill_Pattern_ID:
                    file.desired_rendition().fill_pattern() = *(WT_Fill_Pattern*)(pObject);
                    break;
                case WT_Object::Merge_Control_ID:
                    file.desired_rendition().merge_control() = *(WT_Merge_Control*)(pObject);
                    break;
                case WT_Object::Font_Extension_ID:
                    file.desired_rendition().font_extension() = *(WT_Font_Extension*)(pObject);
                    break;
                case WT_Object::BlockRef_ID:
                    file.desired_rendition().blockref() = *(WT_BlockRef*)(pObject);
                    break;
                case WT_Object::Pen_Pattern_ID:
                    file.desired_rendition().pen_pattern() = *(WT_Pen_Pattern*)(pObject);
                    break;
                case WT_Object::URL_ID:
                    file.desired_rendition().url() = *(WT_URL*)(pObject);
                    break;
                case WT_Object::Visibility_ID:
                    file.desired_rendition().visibility() = *(WT_Visibility*)(pObject);
                    break;
                case WT_Object::Layer_ID:
                    file.desired_rendition().layer() = *(WT_Layer*)(pObject);
                    break;
                case WT_Object::Line_Pattern_ID:
                    file.desired_rendition().line_pattern() = *(WT_Line_Pattern*)(pObject);
                    break;
                case WT_Object::Line_Style_ID:
                    file.desired_rendition().line_style() = *(WT_Line_Style*)(pObject);
                    break;
                case WT_Object::Line_Weight_ID:
                    file.desired_rendition().line_weight() = *(WT_Line_Weight*)(pObject);
                    break;
                case WT_Object::Macro_Scale_ID: 
                    file.desired_rendition().macro_scale() = *(WT_Macro_Scale*)(pObject);
                    break;
                case WT_Object::Macro_Index_ID: 
                    file.desired_rendition().macro_index() = *(WT_Macro_Index*)(pObject);
                    break;
                case WT_Object::Color_Map_ID:
                    file.desired_rendition().color_map() = *(WT_Color_Map*)(pObject);
                    break;
                case WT_Object::Font_ID:
                    file.desired_rendition().font() = *(WT_Font*)(pObject);
                    break;
                case WT_Object::Viewport_ID:
                    file.desired_rendition().viewport() = *(WT_Viewport*)(pObject);
                    break;
                case WT_Object::Code_Page_ID:
                    file.desired_rendition().code_page() = *(WT_Code_Page*)(pObject);
                    break;
                case WT_Object::Object_Node_ID:
                    file.desired_rendition().object_node() = *(WT_Object_Node*)(pObject);
                    break;
                case WT_Object::Text_Background_ID:
                    file.desired_rendition().text_background() = *(WT_Text_Background*)(pObject);
                    break;
                case WT_Object::Text_HAlign_ID:
                    file.desired_rendition().text_halign() = *(WT_Text_HAlign*)(pObject);
                    break;
                case WT_Object::Text_VAlign_ID:
                    file.desired_rendition().text_valign() = *(WT_Text_VAlign*)(pObject);
                    break;
                case WT_Object::Delineate_ID:
                    file.desired_rendition().delineate() = *(WT_Delineate*)(pObject);
                    break;
                case WT_Object::User_Fill_Pattern_ID:
                    file.desired_rendition().user_fill_pattern() = *(WT_User_Fill_Pattern*)(pObject);
                    break;
                case WT_Object::User_Hatch_Pattern_ID:
                    file.desired_rendition().user_hatch_pattern() = *(WT_User_Hatch_Pattern*)(pObject);
                    break;
                case WT_Object::Contrast_Color_ID:
                    file.desired_rendition().contrast_color() = *(WT_Contrast_Color*)(pObject);
                    break;
                default:
                    break;
                } //switch
            }
            else
            {
                WD_Assert( "Unsupported object type in macro stream" );
            }
        }
    }

    WD_CHECK( file.dump_delayed_drawable() );
    return WT_Result::Success;
}

WT_Result WT_XAML_Overpost::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
    if(!rMap.size())
        return WT_Result::Internal_Error;

    const char** ppValue = rMap.find( XamlXML::kpzMode_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
    {
        acceptMode() = (WT_Overpost::WT_AcceptMode)atoi(*ppValue);
    }
    else
    {
        return WT_Result::Corrupt_File_Error;
    }
    ppValue = rMap.find( XamlXML::kpzRender_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
    {
        renderEntities() = (atoi(*ppValue) == 1 ? WD_True : WD_False);
    }
    else
    {
        return WT_Result::Corrupt_File_Error;
    }

    ppValue = rMap.find( XamlXML::kpzExtent_Attribute );
    if (ppValue!=NULL && *ppValue!=NULL)
    {
        addExtents() = (atoi(*ppValue) == 1 ? WD_True : WD_False);
    }
    else
    {
        return WT_Result::Corrupt_File_Error;
    }
   
    return WT_Result::Success;

}

WT_Result WT_XAML_Overpost::parseCData(int nLen,const char* pCData)
{
    //Data was EncodedBase64 so lets DecodeBase64
    //Base64 decode the data
    size_t nRequired = DWFString::DecodeBase64( pCData, nLen, NULL, 0 );
    WT_Byte* pChars = DWFCORE_ALLOC_MEMORY(WT_Byte, nRequired+1);
    if (pChars == NULL)
        return WT_Result::Out_Of_Memory_Error;

    size_t nProcessed = DWFString::DecodeBase64( pCData, nLen, pChars, nRequired );
    WD_Assert( nProcessed == nRequired );

    if ( nProcessed != nRequired )
    {
        return WT_Result::Internal_Error;
    }
   
    DWFInputStream *pInputStream = DWFCORE_ALLOC_OBJECT( DWFBufferInputStream( pChars, nProcessed ) );

	WT_XAML_File::WT_XAML_Memory_File oMemoryFile( pInputStream );
    WT_Result result = oMemoryFile.open();

    bool done = false;
    WT_W2D_Class_Factory oW2DClassFactory;
    while (result == WT_Result::Success && !done)
    {
        result = oMemoryFile.process_next_object();
        if (result == WT_Result::Success)
        {
            const WT_Object* pObject = oMemoryFile.current_object();
            if (pObject->object_id() == WT_Object::Macro_Definition_ID)
            {
                WT_XAML_Macro_Definition* pMacro = const_cast<WT_XAML_Macro_Definition*>( static_cast< const WT_XAML_Macro_Definition*>( pObject ) );
                pMacro->reset();

                while ( pMacro->get_next() && result == WT_Result::Success )
                {
                    const WT_Object* pObject = pMacro->object();
                    if ( pObject != NULL )
                    {
                        result = addObject( pObject );
                    }
                }

                pMacro->set_owner( WD_False ); //the incoming macro will no longer own the object stream
                done = true;;
            }
            oW2DClassFactory.Destroy( const_cast< WT_Object* >( pObject ) );
        }
    }

    WD_CHECK( oMemoryFile.close() );

    materialized() = WD_True;
    DWFCORE_FREE_MEMORY( pChars );

    return result;
}
