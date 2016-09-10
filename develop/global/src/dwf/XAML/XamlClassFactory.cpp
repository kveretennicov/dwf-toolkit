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


WT_XAML_Class_Factory::WT_XAML_Class_Factory(void)
{
}

WT_XAML_Class_Factory::~WT_XAML_Class_Factory(void)
{
}

WT_DWF_Header* 
WT_XAML_Class_Factory::Create_DWF_Header() throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_DWF_Header );
}

WT_End_Of_DWF* 
WT_XAML_Class_Factory::Create_End_Of_DWF() throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_End_Of_DWF );
}

WT_File* 
WT_XAML_Class_Factory::Create_File()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_File );
}

WT_Polyline* 
WT_XAML_Class_Factory::Create_Polyline()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Polyline );
}

WT_Polyline* 
WT_XAML_Class_Factory::Create_Polyline(int count, const WT_Logical_Point *points, WT_Boolean copy)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Polyline(count,points,copy) );
}

WT_Polyline* 
WT_XAML_Class_Factory::Create_Polyline(const WT_Polyline &pline)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Polyline(pline) );
}

WT_Polymarker* 
WT_XAML_Class_Factory::Create_Polymarker() throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Polymarker );
}

WT_Polymarker* 
WT_XAML_Class_Factory::Create_Polymarker(int count, WT_Logical_Point const * points, WT_Boolean copy ) throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Polymarker( count, points, copy ) );
}

WT_Polymarker* 
WT_XAML_Class_Factory::Create_Polymarker(WT_Polymarker const& marker) throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Polymarker( marker ) );
}

WT_Attribute_URL* 
WT_XAML_Class_Factory::Create_Attribute_URL() 
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Attribute_URL );
}

WT_Attribute_URL* 
WT_XAML_Class_Factory::Create_Attribute_URL(WT_Attribute &attribute)
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Attribute_URL(attribute) );
}

WT_Attribute_URL* 
WT_XAML_Class_Factory::Create_Attribute_URL(WT_Attribute &attribute, WT_URL_Item &item) 
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Attribute_URL(attribute,item) );
}

WT_Attribute_URL* 
WT_XAML_Class_Factory::Create_Attribute_URL(
    WT_Attribute &attribute,
    WT_Integer32 index, 
    WT_Unsigned_Integer16 const * address, 
    WT_Unsigned_Integer16 const * friendly_name )
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Attribute_URL(attribute,index,address,friendly_name) );
}

WT_Attribute_URL* WT_XAML_Class_Factory::Create_Attribute_URL(WT_Attribute_URL const& url )
											throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Attribute_URL( url ) );
}
//Code Page
WT_Code_Page* 
WT_XAML_Class_Factory::Create_Code_Page()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Code_Page );
}
/// Constructs a WT_Code_Page object with the given page number.
WT_Code_Page* 
WT_XAML_Class_Factory::Create_Code_Page( WT_Integer32 page_number )
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Code_Page(page_number) );
}

WT_Code_Page* WT_XAML_Class_Factory::Create_Code_Page( WT_Code_Page const& code_page ) throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Code_Page(code_page) );
}

//Color
//Constructs a WT_Color object. (Defaults to white.).
WT_Color* 
WT_XAML_Class_Factory::Create_Color()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Color );
}
//Constructs a WT_Color object with the given color and optional mapping mode.
WT_Color* 
WT_XAML_Class_Factory::Create_Color(WT_RGBA32 color, WT_Color::WT_Color_Map_Mode m)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Color(color,m) );
}
//Constructs a WT_Color object with the given color and optional mapping mode.
WT_Color* 
WT_XAML_Class_Factory::Create_Color(int red, int green, int blue, WT_Color::WT_Color_Map_Mode m)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Color(red,green,blue,m) );
}
//Constructs a WT_Color object with the given color and optional mapping mode.
WT_Color* 
WT_XAML_Class_Factory::Create_Color(int red, int green, int blue, int alpha, WT_Color::WT_Color_Map_Mode m)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Color(red,green,blue,alpha,m) );
}
//Constructs a WT_Color object from the given index and corresponding color map. 
WT_Color* 
WT_XAML_Class_Factory::Create_Color(WT_Color_Index index, WT_Color_Map const &color_map)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Color(index,color_map) );
}

//Constructs a WT_Color object from the given index and corresponding color map. 
WT_Color* 
WT_XAML_Class_Factory::Create_Color( WT_Color const &color )
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Color( color ) );
}

//Color_Map	
//Constructs a WT_Color_Map object for the optional specific file revision. Can be used a default constructor.
WT_Color_Map* 
WT_XAML_Class_Factory::Create_Color_Map(int file_revision_being_used)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Color_Map(file_revision_being_used) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Color_Map..." );
	}
}
//Constructs a WT_Color_Map object for the given size and map, for the given file.
WT_Color_Map* 
WT_XAML_Class_Factory::Create_Color_Map(int count, WT_RGBA32 const *map, WT_File &file)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Color_Map(count,map,file) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Color_Map..." );
	}
}
//Constructs a WT_Color_Map object for the given size and map, for the given file.
WT_Color_Map* 
WT_XAML_Class_Factory::Create_Color_Map(int count, WT_RGB const *map, WT_File &file)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Color_Map(count,map,file) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Color_Map..." );
	}
}
//Constructs a WT_Color_Map object. Copy constructor. 
WT_Color_Map* 
WT_XAML_Class_Factory::Create_Color_Map(WT_Color_Map const &cmap)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Color_Map(cmap) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Color_Map..." );
	}
}

WT_Contrast_Color* 
WT_XAML_Class_Factory::Create_Contrast_Color() 
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Contrast_Color );
}

WT_Contrast_Color* 
WT_XAML_Class_Factory::Create_Contrast_Color(WT_RGBA32 color)
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Contrast_Color(color) );
}

WT_Contrast_Color* 
WT_XAML_Class_Factory::Create_Contrast_Color(int red,int green,int blue,int alpha) 
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Contrast_Color(red,green,blue,alpha) );
}

WT_Contrast_Color* 
WT_XAML_Class_Factory::Create_Contrast_Color(WT_Contrast_Color const& color) 
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT(WT_XAML_Contrast_Color(color) );
}

//WT_Contour_Set
//Constructs a WT_Contour_Set object.
WT_Contour_Set* 
WT_XAML_Class_Factory::Create_Contour_Set()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Contour_Set );
}
//Constructs a WT_Contour_Set object with the given data.
WT_Contour_Set* 
WT_XAML_Class_Factory::Create_Contour_Set(
    WT_File &file, 
    WT_Integer32 contours, 
    WT_Integer32 const *counts, 
    WT_Integer32 point_count, 
    WT_Logical_Point const *points, 
    WT_Boolean copy)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Contour_Set(file,contours,counts,point_count,points,copy) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Contour_Set..." );
	}
}
//Constructs a WT_Contour_Set object with the given data for one countour (i.e. a polygon). Copies the points locally. 	
WT_Contour_Set* 
WT_XAML_Class_Factory::Create_Contour_Set(WT_File &file, WT_Integer32 point_count, WT_Logical_Point const *points)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Contour_Set(file,point_count,points) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Contour_Set..." );
	}
}
//Constructs a WT_Contour_Set object with the given data. Can be used as a copy constructor. 
WT_Contour_Set* 
WT_XAML_Class_Factory::Create_Contour_Set(WT_Contour_Set const &source, WT_Boolean copy)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Contour_Set(source,copy) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Contour_Set..." );
	}
}

//Dash_Pattern
//Construct a WT_Dash_Pattern object.
WT_Dash_Pattern* 
WT_XAML_Class_Factory::Create_Dash_Pattern()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Dash_Pattern );
}
//Construct a WT_Dash_Pattern object with the given data.
WT_Dash_Pattern* 
WT_XAML_Class_Factory::Create_Dash_Pattern(WT_Integer32 id_number, WT_Integer16 length, WT_Integer16 const *pArray)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Dash_Pattern(id_number,length,pArray) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Dash_Pattern..." );
	}
}
//Construct a WT_Dash_Pattern object. Copy constructor. 
WT_Dash_Pattern* 
WT_XAML_Class_Factory::Create_Dash_Pattern(WT_Dash_Pattern const &r)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Dash_Pattern(static_cast<WT_XAML_Dash_Pattern const &>(r) ));
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Dash_Pattern..." );
	}
}

//Delineate
WT_Delineate* 
WT_XAML_Class_Factory::Create_Delineate()
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Delineate );
}

WT_Delineate* 
WT_XAML_Class_Factory::Create_Delineate(WT_Boolean delineate_state)
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Delineate(delineate_state) );
}

WT_Delineate* 
WT_XAML_Class_Factory::Create_Delineate(WT_Delineate const& delineate) 
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Delineate(static_cast<WT_XAML_Delineate const&>(delineate) ));
}


//Filled Ellipse
WT_Filled_Ellipse* 
WT_XAML_Class_Factory::Create_Filled_Ellipse()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Filled_Ellipse );
}

WT_Filled_Ellipse* 
WT_XAML_Class_Factory::Create_Filled_Ellipse(
    WT_Integer32 x, WT_Integer32 y, 
    WT_Integer32 major, WT_Integer32 minor, 
    WT_Unsigned_Integer16 start, 
    WT_Unsigned_Integer16 end, 
    WT_Unsigned_Integer16 tilt)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Filled_Ellipse(x,y,major,minor,start,end,tilt) );
}

WT_Filled_Ellipse* 
WT_XAML_Class_Factory::Create_Filled_Ellipse(
    WT_Logical_Point const &pos, 
    WT_Integer32 major, WT_Integer32 minor, 
    WT_Unsigned_Integer16 start, 
    WT_Unsigned_Integer16 end, 
    WT_Unsigned_Integer16 tilt)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Filled_Ellipse(pos,major,minor,start,end,tilt) );
}

WT_Filled_Ellipse* 
WT_XAML_Class_Factory::Create_Filled_Ellipse(const WT_Filled_Ellipse& e)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Filled_Ellipse(e)); 
}

//Outline Ellipse
WT_Outline_Ellipse* 
WT_XAML_Class_Factory::Create_Outline_Ellipse()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Outline_Ellipse );
}

WT_Outline_Ellipse* 
WT_XAML_Class_Factory::Create_Outline_Ellipse(
    WT_Integer32 x, WT_Integer32 y, 
    WT_Integer32 major, WT_Integer32 minor, 
    WT_Unsigned_Integer16 start, 
    WT_Unsigned_Integer16 end, 
    WT_Unsigned_Integer16 tilt)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Outline_Ellipse(x,y,major,minor,start,end,tilt) );
}

WT_Outline_Ellipse* 
WT_XAML_Class_Factory::Create_Outline_Ellipse(
    WT_Logical_Point const &pos, 
    WT_Integer32 major, WT_Integer32 minor, 
    WT_Unsigned_Integer16 start, 
    WT_Unsigned_Integer16 end, 
    WT_Unsigned_Integer16 tilt)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Outline_Ellipse(pos,major,minor,start,end,tilt) );
}

WT_Outline_Ellipse* 
WT_XAML_Class_Factory::Create_Outline_Ellipse(const WT_Outline_Ellipse& e)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Outline_Ellipse(e)); 
}

//Embed
WT_Embed* 
WT_XAML_Class_Factory::Create_Embed()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Embed );
}

WT_Embed* 
WT_XAML_Class_Factory::Create_Embed(WT_Embed const &embed)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Embed( embed ) );
}

//Embedded_Font
WT_Embedded_Font* 
WT_XAML_Class_Factory::Create_Embedded_Font() 
throw(DWFException)
{
    return new WT_XAML_Embedded_Font();
}

WT_Embedded_Font* WT_XAML_Class_Factory::Create_Embedded_Font(
    int request_type,
    int privilege_type,
    int character_set_type,
    WT_Integer32 data_size,
    WT_Byte * data,
    WT_Integer32 font_type_face_name_length,
    WT_Byte * font_type_face_name_string,
    WT_Integer32 font_logfont_name_length,
    WT_Byte * font_logfont_name_string,
    WT_Boolean copy) 
throw(DWFException)
{
	try
	{
		return new WT_XAML_Embedded_Font( 
			request_type,
			privilege_type,
			character_set_type,
			data_size,
			data,
			font_type_face_name_length,
			font_type_face_name_string,
			font_logfont_name_length,
			font_logfont_name_string,
			copy);
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Embedded Font..." );
	}
}

//Fill
WT_Fill* 
WT_XAML_Class_Factory::Create_Fill() 
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Fill );
}

WT_Fill* 
WT_XAML_Class_Factory::Create_Fill(WT_Boolean fill) 
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Fill(fill) );
}

WT_Fill* 
WT_XAML_Class_Factory::Create_Fill(WT_Fill const& fill) 
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Fill(fill) );
}

//Fill Pattern
WT_Fill_Pattern* 
WT_XAML_Class_Factory::Create_Fill_Pattern()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Fill_Pattern );
}
//Constructs a WT_Fill_Pattern with the given ID.
WT_Fill_Pattern* 
WT_XAML_Class_Factory::Create_Fill_Pattern(WT_Fill_Pattern::WT_Pattern_ID n)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Fill_Pattern(n) );
}

WT_Fill_Pattern* WT_XAML_Class_Factory::Create_Fill_Pattern(WT_Fill_Pattern const& pattern) throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Fill_Pattern(pattern) );
}

//Font
WT_Font* 
WT_XAML_Class_Factory::Create_Font()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Font );
}

WT_Font* WT_XAML_Class_Factory::Create_Font(WT_Font const& font) 
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Font( font ) );
}

WT_Font* 
WT_XAML_Class_Factory::Create_Font(
    WT_String const &name, WT_Boolean bold, WT_Boolean italic, WT_Boolean underline, 
    WT_Byte charset, WT_Byte pitch, WT_Byte family, WT_Integer32 height, 
    WT_Unsigned_Integer16 rotation, WT_Unsigned_Integer16 width_scale, 
    WT_Unsigned_Integer16 spacing, WT_Unsigned_Integer16 oblique, 
    WT_Integer32 flags)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Font(name,bold,italic,underline,charset,pitch,family,height,rotation,width_scale,spacing,oblique,flags) );
}

//Font Extension											   
WT_Font_Extension*
WT_XAML_Class_Factory::Create_Font_Extension() 
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Font_Extension );
}

WT_Font_Extension*
WT_XAML_Class_Factory::Create_Font_Extension(WT_String const & logfont_name, WT_String const & cannonical_name) 
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Font_Extension( logfont_name, cannonical_name ) );
}

//Font Options
WT_Font_Option_Charset* WT_XAML_Class_Factory::Create_Font_Option_Charset() throw(DWFException)
{
	return new WT_Font_Option_Charset();
}
WT_Font_Option_Charset* WT_XAML_Class_Factory::Create_Font_Option_Charset(WT_Byte charset) throw(DWFException)
{
	return new WT_Font_Option_Charset(charset);
}
WT_Font_Option_Family* WT_XAML_Class_Factory::Create_Font_Option_Family() throw(DWFException)
{
	return new WT_Font_Option_Family();
}
WT_Font_Option_Family*	WT_XAML_Class_Factory::Create_Font_Option_Family(WT_Byte family) throw(DWFException)
{
	return new WT_Font_Option_Family(family);
}
WT_Font_Option_Flags* WT_XAML_Class_Factory::Create_Font_Option_Flags() throw(DWFException)
{
	return new WT_Font_Option_Flags();
}

WT_Font_Option_Flags* WT_XAML_Class_Factory::Create_Font_Option_Flags(WT_Integer32 flags) throw(DWFException)
{
	return new WT_Font_Option_Flags(flags);
}

WT_Font_Option_Font_Name* WT_XAML_Class_Factory::Create_Font_Option_Font_Name() throw(DWFException)
{
	return new WT_Font_Option_Font_Name();
}

WT_Font_Option_Font_Name* WT_XAML_Class_Factory::Create_Font_Option_Font_Name(WT_String const &name) throw(DWFException)
{
	return new WT_Font_Option_Font_Name(name);
}

WT_Font_Option_Height* WT_XAML_Class_Factory::Create_Font_Option_Height() throw(DWFException)
{
	return new WT_Font_Option_Height();
}

WT_Font_Option_Height* WT_XAML_Class_Factory::Create_Font_Option_Height(WT_Integer32 height) throw(DWFException)
{
	return new WT_Font_Option_Height(height);
}

WT_Font_Option_Oblique*	WT_XAML_Class_Factory::Create_Font_Option_Oblique() throw(DWFException)
{
	return new WT_Font_Option_Oblique();
}

WT_Font_Option_Oblique* WT_XAML_Class_Factory::Create_Font_Option_Oblique(WT_Unsigned_Integer16 oblique) throw(DWFException)
{
	return new WT_Font_Option_Oblique(oblique);
}

WT_Font_Option_Pitch* WT_XAML_Class_Factory::Create_Font_Option_Pitch() throw(DWFException)
{
	return new WT_Font_Option_Pitch();
}

WT_Font_Option_Pitch* WT_XAML_Class_Factory::Create_Font_Option_Pitch(WT_Byte pitch) throw(DWFException)
{
	return new WT_Font_Option_Pitch(pitch);
}

WT_Font_Option_Rotation* WT_XAML_Class_Factory::Create_Font_Option_Rotation() throw(DWFException)
{
	return new WT_Font_Option_Rotation();
}

WT_Font_Option_Rotation* WT_XAML_Class_Factory::Create_Font_Option_Rotation(WT_Unsigned_Integer16 rotation) throw(DWFException)
{
	return new WT_Font_Option_Rotation(rotation);
}

WT_Font_Option_Spacing* WT_XAML_Class_Factory::Create_Font_Option_Spacing() throw(DWFException)
{
	return new WT_Font_Option_Spacing();
}

WT_Font_Option_Spacing* WT_XAML_Class_Factory::Create_Font_Option_Spacing(WT_Unsigned_Integer16 spacing) throw(DWFException)
{
	return new WT_Font_Option_Spacing(spacing);
}

WT_Font_Option_Style* WT_XAML_Class_Factory::Create_Font_Option_Style(WT_Boolean bold, WT_Boolean italic,
																  WT_Boolean underlined)
																  throw(DWFException)
{
	return new WT_Font_Option_Style(bold,italic,underlined);
}

WT_Font_Option_Width_Scale* WT_XAML_Class_Factory::Create_Font_Option_Width_Scale() throw(DWFException) 
{
	return new WT_Font_Option_Width_Scale();
}

WT_Font_Option_Width_Scale* WT_XAML_Class_Factory::Create_Font_Option_Width_Scale(WT_Unsigned_Integer16 width_scale) throw(DWFException)
{
	return new WT_Font_Option_Width_Scale(width_scale);
}

//Gouraud Polyline
WT_Gouraud_Polyline*
WT_XAML_Class_Factory::Create_Gouraud_Polyline()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Gouraud_Polyline );
}

WT_Gouraud_Polyline*
WT_XAML_Class_Factory::Create_Gouraud_Polyline(
    int count, WT_Logical_Point const *points, 
    WT_RGBA32 const *colors, WT_Boolean copy)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Gouraud_Polyline(count,points,colors,copy) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Gouraud Polyline..." );
	}
}

WT_Gouraud_Polyline* WT_XAML_Class_Factory::Create_Gouraud_Polyline(WT_Gouraud_Polyline const& pline) throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Gouraud_Polyline(pline) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Gouraud Polyline..." );
	}
}

WT_Gouraud_Polytriangle*
WT_XAML_Class_Factory::Create_Gouraud_Polytriangle()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Gouraud_Polytriangle );
}

WT_Gouraud_Polytriangle*
WT_XAML_Class_Factory::Create_Gouraud_Polytriangle(
    int count, WT_Logical_Point const *points, 
    WT_RGBA32 const *colors, WT_Boolean copy)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Gouraud_Polytriangle(count,points,colors,copy) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Gouraud Polytriangle.." );
	}
}

WT_Gouraud_Polytriangle* WT_XAML_Class_Factory::Create_Gouraud_Polytriangle(WT_Gouraud_Polytriangle const& polytri)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Gouraud_Polytriangle(polytri) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Gouraud Polytriangle.." );
	}
}

//Image
WT_Image* 
WT_XAML_Class_Factory::Create_Image()
throw(DWFException)
{	
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Image );
}

WT_Image* 
WT_XAML_Class_Factory::Create_Image(int image_type)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Image(image_type) );
}
WT_Image* 
WT_XAML_Class_Factory::Create_Image(
    WT_Unsigned_Integer16 rows, 
    WT_Unsigned_Integer16 columns, 
    WT_Image::WT_Image_Format format, WT_Integer32 identifier,
    WT_Color_Map const *color_map, WT_Integer32 data_size, 
    WT_Byte *data, WT_Logical_Point const &min_corner, 
    WT_Logical_Point const &max_corner, WT_Boolean copy,
    WT_Integer32 dpi)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Image(rows,columns,format,identifier,color_map,data_size,data,min_corner,max_corner,copy,dpi) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Image" );
	}
}

//Layer
WT_Layer* 
WT_XAML_Class_Factory::Create_Layer()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Layer );
}

WT_Layer* 
WT_XAML_Class_Factory::Create_Layer(WT_File &file, WT_Integer32 layer_num, char const *layer_name)
throw(DWFException)
{	
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Layer(file,layer_num,layer_name) );
}

WT_Layer* 
WT_XAML_Class_Factory::Create_Layer(WT_File &file, WT_Integer32 layer_num, WT_Unsigned_Integer16 const *layer_name)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Layer(file,layer_num,layer_name) );
}

WT_Layer* 
WT_XAML_Class_Factory::Create_Layer(WT_Layer const &layer)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Layer(static_cast<const WT_XAML_Layer &> (layer)) );
}

//Line Pattern
WT_Line_Pattern* 
WT_XAML_Class_Factory::Create_Line_Pattern()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Line_Pattern );
}

WT_Line_Pattern* 
WT_XAML_Class_Factory::Create_Line_Pattern(WT_Line_Pattern::WT_Pattern_ID id)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Line_Pattern(id) );
}

WT_Line_Pattern* WT_XAML_Class_Factory::Create_Line_Pattern(WT_Line_Pattern const& pat)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Line_Pattern(pat) );
}
//Line Style	
WT_Line_Style* 
WT_XAML_Class_Factory::Create_Line_Style()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Line_Style );
}

WT_Line_Style* 
WT_XAML_Class_Factory::Create_Line_Style(WT_Line_Style const &style)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Line_Style( style ) );
}

//Line Weight
WT_Line_Weight* 
WT_XAML_Class_Factory::Create_Line_Weight()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Line_Weight );
}

WT_Line_Weight* 
WT_XAML_Class_Factory::Create_Line_Weight(WT_Integer32 weight)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Line_Weight(weight) );
}

//Line Weight
WT_Line_Weight* 
WT_XAML_Class_Factory::Create_Line_Weight( WT_Line_Weight const &weight)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Line_Weight( weight ) );
}

//WT_Macro_Definition
WT_Macro_Definition* 
WT_XAML_Class_Factory::Create_Macro_Definition()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Macro_Definition );
}

WT_Macro_Definition* 
WT_XAML_Class_Factory::Create_Macro_Definition( WT_Unsigned_Integer16 index, WT_Integer32 scale_units )
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Macro_Definition( index, scale_units ) );
}

/// Constructs a WT_Macro_Draw object.
WT_Macro_Draw* 
WT_XAML_Class_Factory::Create_Macro_Draw()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Macro_Draw );
}

WT_Macro_Draw* 
WT_XAML_Class_Factory::Create_Macro_Draw(int count, WT_Logical_Point const * points, WT_Boolean copy )
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Macro_Draw( count, points, copy ) );
}

WT_Macro_Draw* WT_XAML_Class_Factory::Create_Macro_Draw(WT_Macro_Draw const& draw)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Macro_Draw(draw) );
}

//Macro_Index
WT_Macro_Index* 
WT_XAML_Class_Factory::Create_Macro_Index()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Macro_Index );
}

WT_Macro_Index* 
WT_XAML_Class_Factory::Create_Macro_Index(WT_Integer32 macro)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Macro_Index( macro ) );
}

WT_Macro_Index* WT_XAML_Class_Factory::Create_Macro_Index(WT_Macro_Index const& index)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Macro_Index(index) );
}

//Macro_Scale
WT_Macro_Scale* 
WT_XAML_Class_Factory::Create_Macro_Scale()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Macro_Scale );
}

WT_Macro_Scale* 
WT_XAML_Class_Factory::Create_Macro_Scale(WT_Integer32 macro_scale)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Macro_Scale( macro_scale ) );
}

WT_Macro_Scale* WT_XAML_Class_Factory::Create_Macro_Scale(WT_Macro_Scale const& scale)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Macro_Scale(scale) );
}

//Merge_Control
WT_Merge_Control* 
WT_XAML_Class_Factory::Create_Merge_Control()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Merge_Control );
}

WT_Merge_Control* 
WT_XAML_Class_Factory::Create_Merge_Control(WT_Merge_Control::WT_Merge_Format merge)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Merge_Control( merge ) );
}

WT_Merge_Control* 
WT_XAML_Class_Factory::Create_Merge_Control(WT_Merge_Control const& merge_control)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Merge_Control(merge_control) );
}

//Named_View_List
WT_Named_View_List* 
WT_XAML_Class_Factory::Create_Named_View_List(void)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Named_View_List );
}

WT_Named_View_List* 
WT_XAML_Class_Factory::Create_Named_View_List(WT_Named_View_List const& named_view_list)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Named_View_List(static_cast<WT_XAML_Named_View_List const &>(named_view_list)) );
}

//Named_View
WT_Named_View* 
WT_XAML_Class_Factory::Create_Named_View(void)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Named_View );
}

WT_Named_View* 
WT_XAML_Class_Factory::Create_Named_View(WT_Named_View const &named_view)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Named_View(named_view) );
}

WT_Named_View* 
WT_XAML_Class_Factory::Create_Named_View(WT_Logical_Box const &view, const char * name)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Named_View(view, name) );
}

WT_Named_View* 
WT_XAML_Class_Factory::Create_Named_View(WT_Logical_Box const &view, WT_Unsigned_Integer16 const * name)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Named_View(view, name) );
}

//Object_Node
WT_Object_Node* 
WT_XAML_Class_Factory::Create_Object_Node()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Object_Node );
}

WT_Object_Node*	
WT_XAML_Class_Factory::Create_Object_Node(
    WT_File &file, 
    WT_Integer32 object_node_num,
    char const * object_node_name)
throw(DWFException)
{	
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Object_Node( file, object_node_num, object_node_name ) );
}

WT_Object_Node*	
WT_XAML_Class_Factory::Create_Object_Node(
    WT_File &file, 
    WT_Integer32 object_node_num,
    WT_Unsigned_Integer16 const * object_node_name)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Object_Node( file, object_node_num, object_node_name ) );
}

WT_Object_Node*	
WT_XAML_Class_Factory::Create_Object_Node(WT_Object_Node const &object_node)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Object_Node( object_node ) );
}

//Overpost
WT_Overpost* 
WT_XAML_Class_Factory::Create_Overpost()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Overpost );
}

WT_Overpost* 
WT_XAML_Class_Factory::Create_Overpost(
    WT_Overpost::WT_AcceptMode acceptMode,
    WT_Boolean renderEntities,
    WT_Boolean addExtents)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Overpost(acceptMode, renderEntities, addExtents) );
}

//Pen_Pattern
WT_Pen_Pattern* 
WT_XAML_Class_Factory::Create_Pen_Pattern()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Pen_Pattern );
}
WT_Pen_Pattern* 
WT_XAML_Class_Factory::Create_Pen_Pattern(WT_Pen_Pattern::WT_Pattern_ID pattern_id)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Pen_Pattern(pattern_id) );
}
WT_Pen_Pattern* 
WT_XAML_Class_Factory::Create_Pen_Pattern(
    WT_Pen_Pattern::WT_Pattern_ID pattern_id, 
    WT_Unsigned_Integer32 screening_percentage,
    WT_Boolean colormap_flag, 
    WT_Color_Map const *color_map, 
    WT_Boolean copy)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Pen_Pattern(pattern_id,screening_percentage,colormap_flag,color_map,copy) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Pen_Pattern..." );
	}
}

WT_Pen_Pattern* 
WT_XAML_Class_Factory::Create_Pen_Pattern(WT_Pen_Pattern const &pattern)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Pen_Pattern(static_cast<const WT_XAML_Pen_Pattern &> (pattern)) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Pen_Pattern..." );
	}
}

//PNG_Group4_Image
WT_PNG_Group4_Image* 
WT_XAML_Class_Factory::Create_PNG_Group4_Image()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_PNG_Group4_Image );
}

WT_PNG_Group4_Image* 
WT_XAML_Class_Factory::Create_PNG_Group4_Image(int image_type)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_PNG_Group4_Image(image_type) );
}

WT_PNG_Group4_Image* WT_XAML_Class_Factory::Create_PNG_Group4_Image(
    WT_Unsigned_Integer16 rows, 
    WT_Unsigned_Integer16 columns, 
    WT_PNG_Group4_Image::WT_PNG_Group4_Image_Format format, 
    WT_Integer32 identifier,
    WT_Color_Map const *color_map, 
    WT_Integer32 data_size, 
    WT_Byte *data, 
    WT_Logical_Point const &min_corner, 
    WT_Logical_Point const &max_corner, 
    WT_Boolean copy,
     WT_Integer32 dpi)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_PNG_Group4_Image(rows,columns,format,identifier,color_map,data_size,data,min_corner,max_corner,copy,dpi) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate PNG_Group4_Image..." );
	}
}

//Polygon
WT_Polygon* 
WT_XAML_Class_Factory::Create_Polygon()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Polygon );
}

WT_Polygon* 
WT_XAML_Class_Factory::Create_Polygon(int count, WT_Logical_Point const *points, WT_Boolean copy)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Polygon(count,points,copy);	 )	
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Polygon..." );
	}
}
WT_Polygon* 
WT_XAML_Class_Factory::Create_Polygon(const WT_Polygon &polygon)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Polygon(polygon) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Polygon..." );
	}
}

//Polytriangle
WT_Polytriangle* 
WT_XAML_Class_Factory::Create_Polytriangle()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Polytriangle );
}

WT_Polytriangle* 
WT_XAML_Class_Factory::Create_Polytriangle(int count, WT_Logical_Point const *points, WT_Boolean copy)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Polytriangle(count,points,copy) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Polytriangle..." );
	}
}
WT_Polytriangle* 
WT_XAML_Class_Factory::Create_Polytriangle(WT_Polytriangle const& polytriangle)
throw(DWFException)
{
	try
	{
		return DWFCORE_ALLOC_OBJECT( WT_XAML_Polytriangle(polytriangle) );
	}
	catch(WT_Result& /*ex*/)
	{
		_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Polytriangle..." );
	}
}

//Projection
WT_Projection* 
WT_XAML_Class_Factory::Create_Projection()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Projection );
}

WT_Projection* 
WT_XAML_Class_Factory::Create_Projection(const WT_Projection::WT_Projection_Type projection_type)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Projection(projection_type) );
}

WT_Projection* 
WT_XAML_Class_Factory::Create_Projection(WT_Projection const& projection)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Projection(projection) );
}

//Text
WT_Text* 
WT_XAML_Class_Factory::Create_Text()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Text );
}

WT_Text* 
WT_XAML_Class_Factory::Create_Text(WT_Text const & text)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Text(static_cast<const WT_XAML_Text &> (text)) );
}

WT_Text* 
WT_XAML_Class_Factory::Create_Text(
    WT_Logical_Point const &position, 
    WT_String const &string, 
    WT_Logical_Point const *bbox, 
    WT_Unsigned_Integer16 overscore_count, 
    WT_Unsigned_Integer16 const *overscore_pos, 
    WT_Unsigned_Integer16 underscore_count, 
    WT_Unsigned_Integer16 const *underscore_pos)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Text(position,string,bbox,overscore_count,overscore_pos,underscore_count,underscore_pos) );
}

WT_Text* 
WT_XAML_Class_Factory::Create_Text(WT_Logical_Point const &position, WT_String const &string)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Text(position,string) );
}

//Text_Background
WT_Text_Background* 
WT_XAML_Class_Factory::Create_Text_Background()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Text_Background );
}

WT_Text_Background* 
WT_XAML_Class_Factory::Create_Text_Background(WT_Text_Background::eBackground background,WT_Integer32 offset)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Text_Background(background,offset) );
}

WT_Text_Background* 
WT_XAML_Class_Factory::Create_Text_Background(WT_Text_Background const& background) 
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Text_Background(background) );
}

//Text_HAlign
WT_Text_HAlign* 
WT_XAML_Class_Factory::Create_Text_HAlign()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Text_HAlign );
}
WT_Text_HAlign* 
WT_XAML_Class_Factory::Create_Text_HAlign(WT_Text_HAlign::WT_HAlign hAlign)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Text_HAlign(hAlign) );
}

WT_Text_HAlign* 
WT_XAML_Class_Factory::Create_Text_HAlign(WT_Text_HAlign const& hAlign) 
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Text_HAlign(hAlign) );
}

//Text_VAlign
WT_Text_VAlign* 
WT_XAML_Class_Factory::Create_Text_VAlign()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Text_VAlign );
}

WT_Text_VAlign* 
WT_XAML_Class_Factory::Create_Text_VAlign(WT_Text_VAlign::WT_VAlign vAlign)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Text_VAlign(vAlign) );
}

WT_Text_VAlign* 
WT_XAML_Class_Factory::Create_Text_VAlign(WT_Text_VAlign const& vAlign)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Text_VAlign(vAlign) );
}

//WT_Units
WT_Units* 
WT_XAML_Class_Factory::Create_Units()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Units );
}

WT_Units* 
WT_XAML_Class_Factory::Create_Units(WT_Units const& u)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Units(u.application_to_dwf_transform(), u.units()) );
}


WT_Units* 
WT_XAML_Class_Factory::Create_Units(WT_Matrix const &xform, WT_String const &units)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Units(xform, units) );
}

//WT_URL
WT_URL* 
WT_XAML_Class_Factory::Create_URL()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_URL );
}
WT_URL* 
WT_XAML_Class_Factory::Create_URL(WT_URL_Item &item)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_URL(item) );
}
WT_URL* 
WT_XAML_Class_Factory::Create_URL(
    WT_Integer32 index, 
    WT_Unsigned_Integer16 const *address, 
    WT_Unsigned_Integer16 const *friendly_name)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_URL(index,address,friendly_name) );
}

WT_URL* 
WT_XAML_Class_Factory::Create_URL(WT_URL const& url)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_URL(url) );
}

//WT_User_Data
WT_UserData* 
WT_XAML_Class_Factory::Create_UserData()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_UserData );
}

WT_UserData* 
WT_XAML_Class_Factory::Create_UserData(WT_String const &data_description)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_UserData(data_description) );
}

WT_UserData* 
WT_XAML_Class_Factory::Create_UserData(
    WT_String const &data_description, 
    WT_Integer32 data_size, 
    WT_Byte *data, 
    WT_Boolean copy)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_UserData(data_description,data_size,data,copy) );
}

//User_Fill_Pattern
WT_User_Fill_Pattern* 
WT_XAML_Class_Factory::Create_User_Fill_Pattern()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_User_Fill_Pattern );
}

WT_User_Fill_Pattern* 
WT_XAML_Class_Factory::Create_User_Fill_Pattern(WT_Integer16 pattern_num)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_User_Fill_Pattern(pattern_num) );
}

WT_User_Fill_Pattern* 
WT_XAML_Class_Factory::Create_User_Fill_Pattern(
    WT_Integer16 pattern_num, 
    WT_Unsigned_Integer16 rows,
    WT_Unsigned_Integer16 columns,
    WT_Unsigned_Integer32 data_size,
    WT_Byte const * data)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_User_Fill_Pattern(pattern_num, rows, columns, data_size, data) );
}

WT_User_Fill_Pattern* 
WT_XAML_Class_Factory::Create_User_Fill_Pattern(WT_Integer16 pattern_num,WT_User_Fill_Pattern::Fill_Pattern * fill_pattern)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_User_Fill_Pattern(pattern_num, fill_pattern) );
}

WT_User_Fill_Pattern* 
WT_XAML_Class_Factory::Create_User_Fill_Pattern(WT_User_Fill_Pattern const &fill_pattern)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_User_Fill_Pattern(fill_pattern) );
}

/// Default Constructor  
WT_User_Hatch_Pattern* 
WT_XAML_Class_Factory::Create_User_Hatch_Pattern()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_User_Hatch_Pattern );
}

WT_User_Hatch_Pattern* 
WT_XAML_Class_Factory::Create_User_Hatch_Pattern(WT_Integer16 hashpatnum)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_User_Hatch_Pattern(hashpatnum) );
}

WT_User_Hatch_Pattern* 
WT_XAML_Class_Factory::Create_User_Hatch_Pattern(WT_Integer16 hashpatnum, WT_Unsigned_Integer16 xsize, WT_Unsigned_Integer16 ysize)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_User_Hatch_Pattern(hashpatnum, xsize, ysize) );
}

WT_User_Hatch_Pattern* 
WT_XAML_Class_Factory::Create_User_Hatch_Pattern(WT_User_Hatch_Pattern const &pattern)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_User_Hatch_Pattern(pattern) );
}

//WT_View
WT_View* 
WT_XAML_Class_Factory::Create_View()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_View() );
}

WT_View* 
WT_XAML_Class_Factory::Create_View(WT_Logical_Box view)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_View(view) );
}

WT_View* 
WT_XAML_Class_Factory::Create_View(char *name)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_View(name) );
}

WT_View* 
WT_XAML_Class_Factory::Create_View(WT_View const &view)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_View(static_cast<const WT_XAML_View &>(view)) );
}

//Viewport 	
WT_Viewport* 
WT_XAML_Class_Factory::Create_Viewport()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Viewport );
}

WT_Viewport* 
WT_XAML_Class_Factory::Create_Viewport(WT_Viewport const &vport)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Viewport(static_cast<const WT_XAML_Viewport &> (vport)) );
}

WT_Viewport* 
WT_XAML_Class_Factory::Create_Viewport(
    WT_File &file, 
    char const *name, 
    int num_points, 
    WT_Logical_Point const *points, 
    WT_Boolean copy)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Viewport(file,name,num_points,points,copy) );
}

WT_Viewport* 
WT_XAML_Class_Factory::Create_Viewport(
    WT_File &file, 
    WT_Unsigned_Integer16 const *name, 
    int num_points, 
    WT_Logical_Point const *points, 
    WT_Boolean copy)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Viewport(file,name,num_points,points,copy) );
}

WT_Viewport* 
WT_XAML_Class_Factory::Create_Viewport(
    WT_File &file, 
    WT_String const &name, 
    WT_Contour_Set const &boundary, 
    WT_Boolean copy)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Viewport(file,name,boundary,copy) );
}

//Visibility
WT_Visibility* 
WT_XAML_Class_Factory::Create_Visibility()
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Visibility );
}

WT_Visibility* 
WT_XAML_Class_Factory::Create_Visibility(WT_Boolean visible)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Visibility(visible) );
}

WT_Visibility* 
WT_XAML_Class_Factory::Create_Visibility(WT_Visibility const& visibility)
throw(DWFException)
{
    return DWFCORE_ALLOC_OBJECT( WT_XAML_Visibility(visibility) );
}

WT_Unknown* 
WT_XAML_Class_Factory::Create_Unknown() 
throw(DWFException)
{
	return DWFCORE_ALLOC_OBJECT( WT_XAML_Unknown );
}


void WT_XAML_Class_Factory::Destroy(WT_Object* pObject)
{
    WT_Class_Factory::Destroy( pObject );
}

void WT_XAML_Class_Factory::Destroy(WT_Attribute*	pAttribute)
{
    WT_Class_Factory::Destroy( pAttribute );
}

void WT_XAML_Class_Factory::Destroy(WT_File*	pFile)
{
    WT_Class_Factory::Destroy( pFile );
}

void WT_XAML_Class_Factory::Destroy(WT_Dash_Pattern*	pDashPat)
{
    WT_Class_Factory::Destroy( pDashPat );
}

void WT_XAML_Class_Factory::Destroy(WT_Layer*	pLayer)
{
    WT_Class_Factory::Destroy( pLayer );
}

void WT_XAML_Class_Factory::Destroy(WT_Named_View*	pNamedView)
{
    WT_Class_Factory::Destroy( pNamedView );
}

void WT_XAML_Class_Factory::Destroy(WT_Object_Node*	pObjectNode)
{
    WT_Class_Factory::Destroy( pObjectNode );
}

