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

using namespace XamlDrawableAttributes;

XamlRenditionPath::XamlRenditionPath( bool bNamed )
: _bNamed( bNamed )
{
}


XamlRenditionPath::~XamlRenditionPath(void)
{	
}

WT_Result XamlRenditionPath::delay(WT_File & file) const
{
    WT_XAML_File& xFile = static_cast<WT_XAML_File&>(file);

    XamlRenditionPath * delayed = DWFCORE_ALLOC_OBJECT( XamlRenditionPath(*this) );
    if (!delayed)
        return WT_Result::Out_Of_Memory_Error;

    xFile.set_delayed_path(delayed);

    return WT_Result::Success;
}

WT_Result XamlRenditionPath::serialize(WT_File& file) const
{
    WT_XAML_File& rXamlFile = static_cast<WT_XAML_File&>(file);
    DWFXMLSerializer* pXmlSerializer = rXamlFile.xamlSerializer();
    if (!pXmlSerializer)
        return WT_Result::Internal_Error;

    WT_Integer32    parts_to_sync =     WT_Rendition::Color_Bit           |
                                        WT_Rendition::Color_Map_Bit       |
                                        WT_Rendition::Delineate_Bit       |
                                        WT_Rendition::Contrast_Color_Bit  |
                                        WT_Rendition::Fill_Bit            |
                                        WT_Rendition::Fill_Pattern_Bit    |
                                        WT_Rendition::Merge_Control_Bit   |
                                        WT_Rendition::BlockRef_Bit        |
                                        WT_Rendition::Visibility_Bit      |
                                        WT_Rendition::Line_Weight_Bit     |
                                        WT_Rendition::Pen_Pattern_Bit     |
                                        WT_Rendition::Line_Pattern_Bit    |
                                        WT_Rendition::Dash_Pattern_Bit    |
                                        WT_Rendition::Line_Style_Bit      |
                                    //  WT_Rendition::Marker_Size_Bit     |
                                    //  WT_Rendition::Marker_Symbol_Bit   |
                                        WT_Rendition::URL_Bit             |
										WT_Rendition::Viewport_Bit		  |
                                        WT_Rendition::Visibility_Bit      |
                                        WT_Rendition::Layer_Bit           |
                                        WT_Rendition::Object_Node_Bit     |
                                        WT_Rendition::User_Fill_Pattern_Bit | 
                                        WT_Rendition::User_Hatch_Pattern_Bit;

	WD_CHECK( rXamlFile.desired_rendition().sync(file, parts_to_sync) );

    // Spin through the XAML Path needs, and with special attention to precedence,
    // examine the rendition attributes to gather the necessary information:
    WT_XAML_Rendition& rXamlRendition = static_cast<WT_XAML_Rendition&>(rXamlFile.rendition());
    XamlPath* thisPath = const_cast<XamlRenditionPath*>(this);

    #define PROVIDE(type,var) static_cast<PathAttributeProvider*>(static_cast<type*>(&var))->providePathAttributes( thisPath, rXamlFile )
    WD_CHECK( PROVIDE( WT_XAML_Color,        rXamlRendition.color() ));
    WD_CHECK( PROVIDE( WT_XAML_Fill,         rXamlRendition.fill() ));
    WD_CHECK( PROVIDE( WT_XAML_Fill_Pattern, rXamlRendition.fill_pattern() ));
    WD_CHECK( PROVIDE( WT_XAML_Line_Weight,  rXamlRendition.line_weight() ));
    WD_CHECK( PROVIDE( WT_XAML_Line_Pattern, rXamlRendition.line_pattern() ));
    WD_CHECK( PROVIDE( WT_XAML_Dash_Pattern, rXamlRendition.dash_pattern() ));
    WD_CHECK( PROVIDE( WT_XAML_Line_Style,   rXamlRendition.line_style() ));
    WD_CHECK( PROVIDE( WT_XAML_URL,          rXamlRendition.url() ));
	WD_CHECK( PROVIDE( WT_XAML_Attribute_URL, rXamlRendition.attribute_url() ));
    //WD_CHECK( PROVIDE( WT_XAML_Viewport,     rXamlRendition.viewport() ));
    WD_CHECK( PROVIDE( WT_XAML_User_Fill_Pattern,  rXamlRendition.user_fill_pattern() ));
    WD_CHECK( PROVIDE( WT_XAML_User_Hatch_Pattern, rXamlRendition.user_hatch_pattern() ));
    WD_CHECK( PROVIDE( WT_XAML_Visibility,    rXamlRendition.visibility() ));
    #undef PROVIDE   

    return rXamlFile.merge_or_delay_path(*this);
}

WT_Result XamlRenditionPath::dump(WT_File & file) const
{
    if (_bNamed)
    {
        // Get the current element name from the file
        WT_XAML_File& rXamlFile = static_cast<WT_XAML_File&>(file);
        WD_CHECK( const_cast<XamlRenditionPath*>(this)->consumeName( &rXamlFile ));
	    WD_CHECK( XamlPath::dump( file ) );
	    rXamlFile.nameIndex()++;
    	return rXamlFile.serializeRenditionSyncEndElement(); //we only end the rendition group for named paths
    }
    else
    {
        return XamlPath::dump( file );
    }
}

XamlImageRenditionPath::XamlImageRenditionPath()
{
}


XamlImageRenditionPath::~XamlImageRenditionPath(void)
{	
}

WT_Result XamlImageRenditionPath::delay(WT_File & file) const
{
    WT_XAML_File& xFile = static_cast<WT_XAML_File&>(file);

    XamlImageRenditionPath * delayed = DWFCORE_ALLOC_OBJECT( XamlImageRenditionPath(*this) );
    if (!delayed)
        return WT_Result::Out_Of_Memory_Error;

    xFile.set_delayed_path(delayed);

    return WT_Result::Success;
}

WT_Result XamlImageRenditionPath::serialize(WT_File& file) const
{
    WT_XAML_File& rXamlFile = static_cast<WT_XAML_File&>(file);
    DWFXMLSerializer* pXmlSerializer = rXamlFile.xamlSerializer();
    if (!pXmlSerializer)
        return WT_Result::Internal_Error;

    WT_Integer32    parts_to_sync =     //WT_Rendition::Color_Bit               |
                                        //WT_Rendition::Color_Map_Bit           |
                                        WT_Rendition::Delineate_Bit             |
                                        //WT_Rendition::Contrast_Color_Bit      |
                                        //WT_Rendition::Fill_Bit                |
                                        //WT_Rendition::Fill_Pattern_Bit        |
                                        WT_Rendition::Merge_Control_Bit         |
                                        WT_Rendition::BlockRef_Bit              |
                                        WT_Rendition::Visibility_Bit            |
                                        //WT_Rendition::Line_Weight_Bit         |
                                        //WT_Rendition::Pen_Pattern_Bit         |
                                        //WT_Rendition::Line_Pattern_Bit        |
                                        //WT_Rendition::Dash_Pattern_Bit        |
                                        //WT_Rendition::Line_Style_Bit          |
                                        // WT_Rendition::Marker_Size_Bit        |
                                        // WT_Rendition::Marker_Symbol_Bit      |
                                        WT_Rendition::URL_Bit                   |
										WT_Rendition::Viewport_Bit		        |
                                        WT_Rendition::Visibility_Bit            |
                                        WT_Rendition::Layer_Bit                 |
                                        //WT_Rendition::User_Fill_Pattern_Bit   | 
                                        //WT_Rendition::User_Hatch_Pattern_Bit  |
                                        WT_Rendition::Object_Node_Bit;

	WD_CHECK( rXamlFile.desired_rendition().sync(file, parts_to_sync) );

    // Spin through the XAML Path needs, and with special attention to precedence,
    // examine the rendition attributes to gather the necessary information:
    WT_XAML_Rendition& rXamlRendition = static_cast<WT_XAML_Rendition&>(rXamlFile.rendition());
    XamlPath* thisPath = const_cast<XamlImageRenditionPath*>(this);

    #define PROVIDE(type,var) static_cast<PathAttributeProvider*>(static_cast<type*>(&var))->providePathAttributes( thisPath, rXamlFile )
    //WD_CHECK( PROVIDE( WT_XAML_Color,        rXamlRendition.color() ));
    //WD_CHECK( PROVIDE( WT_XAML_Fill,         rXamlRendition.fill() ));
    //WD_CHECK( PROVIDE( WT_XAML_Fill_Pattern, rXamlRendition.fill_pattern() ));
    //WD_CHECK( PROVIDE( WT_XAML_Line_Weight,  rXamlRendition.line_weight() ));
    //WD_CHECK( PROVIDE( WT_XAML_Line_Pattern, rXamlRendition.line_pattern() ));
    //WD_CHECK( PROVIDE( WT_XAML_Dash_Pattern, rXamlRendition.dash_pattern() ));
    //WD_CHECK( PROVIDE( WT_XAML_Line_Style,   rXamlRendition.line_style() ));
    WD_CHECK( PROVIDE( WT_XAML_URL,          rXamlRendition.url() ));
	WD_CHECK( PROVIDE( WT_XAML_Attribute_URL, rXamlRendition.attribute_url() ));
    //WD_CHECK( PROVIDE( WT_XAML_Viewport,     rXamlRendition.viewport() ));
    //WD_CHECK( PROVIDE( WT_XAML_User_Fill_Pattern,  rXamlRendition.user_fill_pattern() ));
    //WD_CHECK( PROVIDE( WT_XAML_User_Hatch_Pattern, rXamlRendition.user_hatch_pattern() ));
    WD_CHECK( PROVIDE( WT_XAML_Visibility,    rXamlRendition.visibility() ));
    #undef PROVIDE   

    return rXamlFile.merge_or_delay_path(*this);
}

WT_Result XamlImageRenditionPath::dump(WT_File & file) const
{
    // Get the current element name from the file
    WT_XAML_File& rXamlFile = static_cast<WT_XAML_File&>(file);

    WD_CHECK( const_cast<XamlImageRenditionPath*>(this)->consumeName( &rXamlFile ));
	WD_CHECK( XamlPath::dump( file ) );
	rXamlFile.nameIndex()++;
	return rXamlFile.serializeRenditionSyncEndElement();
}
