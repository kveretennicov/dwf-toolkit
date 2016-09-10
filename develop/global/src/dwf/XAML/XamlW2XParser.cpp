//
//  Copyright (c) 1996-2006 by Autodesk, Inc.
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
#include "XAML/XamlW2XParser.h"

#ifdef _VERBOSE_DEBUG
namespace XAML_W2X_Parser {
    int gNesting = 0;
};
using namespace XAML_W2X_Parser;
#endif


WT_XAML_W2X_Parser::WT_XAML_W2X_Parser(WT_XAML_File& file) throw(WT_Result)
: _pListKindOfObject(NULL)
, _pW2XParser(NULL)
, _rXamlFile(file)
, _oMap()
, _pClassFactory(NULL)
, _nLastIndex(0)
, _zCurrentElementName()
, _bPendingElementEnded(false)
, _bSuspended(false)
, _bAttributeURLStarted(false)
, _nInsideMacroDraw(0)
{
    //Create Class Factory
    _pClassFactory = DWFCORE_ALLOC_OBJECT( WT_XAML_Class_Factory );
    if(_pClassFactory == NULL)
        throw WT_Result::Out_Of_Memory_Error;

    _pW2XParser = DWFCORE_ALLOC_OBJECT( DWFXMLParser(this) );
    if(_pW2XParser == NULL)
        throw WT_Result::Out_Of_Memory_Error;

}

WT_XAML_W2X_Parser::~WT_XAML_W2X_Parser()
throw()
{
    if(w2xParser())
        DWFCORE_FREE_OBJECT(_pW2XParser);

    //delete class factory pointer here
    if(xamlClassFactory())
        DWFCORE_FREE_OBJECT(_pClassFactory);
}

#define ECHO(a) #a

#define CHECK_CREATE_OBJECT( signature )\
    if ( DWFCORE_COMPARE_ASCII_STRINGS(pcName, XamlXML::kpz##signature##_Element) == 0) { \
    WT_##signature* p = xamlClassFactory()->Create_##signature(); \
    if(p == NULL) { return WT_Result::Out_Of_Memory_Error; } \
    WD_CHECK( ( static_cast<WT_XAML_##signature *>(p) )->parseAttributeList(_oMap,_rXamlFile) ); \
    _rXamlFile.object_list().insert(p); \
    return WT_Result::Success;}

WT_Result WT_XAML_W2X_Parser::Create_Object_Shell(const char* pcName)
{    
    //The second argument indicates whether the data is completely specified 
    //within the W2X, or whether there may be additional data in the XAML
    CHECK_CREATE_OBJECT( Color );
    CHECK_CREATE_OBJECT( Contour_Set );
    CHECK_CREATE_OBJECT( Line_Style );
    CHECK_CREATE_OBJECT( Layer );
    CHECK_CREATE_OBJECT( Line_Pattern );
    CHECK_CREATE_OBJECT( Line_Weight );
    CHECK_CREATE_OBJECT( Dash_Pattern );
    CHECK_CREATE_OBJECT( Delineate );
    CHECK_CREATE_OBJECT( Embedded_Font );
    CHECK_CREATE_OBJECT( Fill_Pattern );
    CHECK_CREATE_OBJECT( Font );
    CHECK_CREATE_OBJECT( Font_Extension );
    CHECK_CREATE_OBJECT( Macro_Index );
    CHECK_CREATE_OBJECT( Macro_Scale );
    CHECK_CREATE_OBJECT( Merge_Control );
    CHECK_CREATE_OBJECT( Object_Node );
    CHECK_CREATE_OBJECT( Overpost );
    CHECK_CREATE_OBJECT( Pen_Pattern );
    CHECK_CREATE_OBJECT( Projection );
    CHECK_CREATE_OBJECT( Text );
    CHECK_CREATE_OBJECT( Text_Background );
    CHECK_CREATE_OBJECT( Text_HAlign );
    CHECK_CREATE_OBJECT( Text_VAlign );
    CHECK_CREATE_OBJECT( Units );
    CHECK_CREATE_OBJECT( User_Fill_Pattern );
    CHECK_CREATE_OBJECT( View );
    CHECK_CREATE_OBJECT( Viewport );
    CHECK_CREATE_OBJECT( Outline_Ellipse );
    CHECK_CREATE_OBJECT( Filled_Ellipse );
    CHECK_CREATE_OBJECT( Gouraud_Polyline );
    CHECK_CREATE_OBJECT( Gouraud_Polytriangle );
    CHECK_CREATE_OBJECT( Macro_Definition );
    CHECK_CREATE_OBJECT( Macro_Draw );
    CHECK_CREATE_OBJECT( Embed );
    CHECK_CREATE_OBJECT( UserData );
    
    
    if(DWFCORE_COMPARE_ASCII_STRINGS(pcName,XamlXML::kpzURL_Item_Element) == 0)
    {
        if(_bAttributeURLStarted)
            return Create_Attribute_URL_Item_In_List();
        else
            return Create_URL_Item_In_List();
    }
    else if(DWFCORE_COMPARE_ASCII_STRINGS(pcName,XamlXML::kpzColor_Map_Element) == 0)
    {
        //Is this is a Image's or PNG_Group4_Image's color_map?
        if(_pListKindOfObject == NULL)
        {
            CHECK_CREATE_OBJECT( Color_Map );
        }
        else
            return Create_Image_Color_Map();       
    }
    else if(DWFCORE_COMPARE_ASCII_STRINGS(pcName,XamlXML::kpzNamed_View_Element) == 0)
    {
        if(_pListKindOfObject == NULL)
        {
            CHECK_CREATE_OBJECT( Named_View );
        }
        else
            return Create_Named_View_In_List();
    }
     else if(DWFCORE_COMPARE_ASCII_STRINGS(pcName,XamlXML::kpzHatch_Pattern_Element) == 0)
    {
        if(_pListKindOfObject != NULL)
        {
            return Create_Hatch_Pattern_In_User_Hatch_Pattern();
        }
    }    

	//unknown elements, just blithely accept them (we're not creating objects from them, so life is good.)
	return WT_Result::Success;  
}

WT_Result WT_XAML_W2X_Parser::Create_DWF_Header()
{
    WT_DWF_Header *pHeader = xamlClassFactory()->Create_DWF_Header();
    if(pHeader == NULL)
        return WT_Result::Out_Of_Memory_Error;

    WT_Result res = static_cast<WT_XAML_DWF_Header *>(pHeader)->parseAttributeList(_oMap,_rXamlFile);
    if (res == WT_Result::Success || res == WT_Result::Minor_Version_Warning)
    {
        _rXamlFile.object_list().insert(pHeader);
    }

    return res;
}

WT_Result WT_XAML_W2X_Parser::Create_Named_View_In_List()
{
    WT_Named_View* pNamedView = xamlClassFactory()->Create_Named_View();
    if(pNamedView == NULL)
        return WT_Result::Out_Of_Memory_Error;

    WD_CHECK( static_cast<WT_XAML_Named_View *>(pNamedView)->parseAttributeList(_oMap,_rXamlFile) );

    (static_cast<WT_XAML_Named_View_List *>(_pListKindOfObject))->add_named_view(*pNamedView);
    xamlClassFactory()->Destroy(pNamedView);

    return WT_Result::Success;
}

//This is a special kind of object where it has subelements.
//So need to create it and delay adding it to XamlFile's object_list.
WT_Result WT_XAML_W2X_Parser::Create_Named_View_List_Shell()
{
    WT_Named_View_List* pList = xamlClassFactory()->Create_Named_View_List();
    if(pList == NULL)
        return WT_Result::Out_Of_Memory_Error;

    //store the pointer for later insertion to file's object list
    _pListKindOfObject = pList;

    return WT_Result::Success;

}

//This is a special kind of object where it has subelements.
//So need to create it and delay adding it to XamlFile's object_list.
WT_Result WT_XAML_W2X_Parser::Create_URL_List_Shell()
{
    if(!_oMap.size())
        _rXamlFile.desired_rendition().url().clear();
    else
    {
   
        WT_URL* pURL = xamlClassFactory()->Create_URL();
        if(pURL == NULL)
            return WT_Result::Out_Of_Memory_Error;

         //store the pointer for later insertion to file's object list
        _pListKindOfObject = pURL;
    }

     return WT_Result::Success;

}

//This is a special kind of object where it has subelements.
//So need to create it and delay adding it to XamlFile's object_list.
WT_Result WT_XAML_W2X_Parser::Create_Attribute_URL_List_Shell()
{
   
    WT_Attribute_URL* pAttributeURL = xamlClassFactory()->Create_Attribute_URL();
    if(pAttributeURL == NULL)
        return WT_Result::Out_Of_Memory_Error;

    //Need to get Count and attribute_id from w2x
    WD_CHECK( ( static_cast<WT_XAML_Attribute_URL *>(pAttributeURL) )->parseAttributeList(_oMap,_rXamlFile) );

    //store the pointer for later insertion to file's object list
    _pListKindOfObject = pAttributeURL;
    _bAttributeURLStarted = true;

    return WT_Result::Success;

}

WT_Result WT_XAML_W2X_Parser::Create_URL_Item_In_List()
{
    //This is a child element (Item) of parent URL
    //Parent URL is already created as a shell from parent element
    //So now just need to parse the item and add it to URL's url_list.
    WD_CHECK( static_cast<WT_XAML_URL *>(_pListKindOfObject)->parseAttributeList(_oMap,_rXamlFile) ); 

    return WT_Result::Success;
}

WT_Result WT_XAML_W2X_Parser::Create_Attribute_URL_Item_In_List()
{
    //This is a child element (Item) of parent URL
    //Parent URL is already created as a shell from parent element
    //So now just need to parse the item and add it to URL's url_list.
    WD_CHECK( static_cast<WT_XAML_Attribute_URL *>(_pListKindOfObject)->parseAttributeList(_oMap,_rXamlFile) ); 

    return WT_Result::Success;
}

WT_Result WT_XAML_W2X_Parser::Create_Image_Shell()
{
    WT_Image* pImage = xamlClassFactory()->Create_Image();
    if(pImage == NULL)
        return WT_Result::Out_Of_Memory_Error;

    WD_CHECK( static_cast<WT_XAML_Image *>(pImage)->parseAttributeList(_oMap,_rXamlFile) );

    if( (pImage->format() == WT_Image::Mapped) ||
        (pImage->format() == WT_Image::Group3X_Mapped) || 
         (pImage->format() == WT_Image::Bitonal_Mapped) )
    {
        //store the pointer for later insertion to file's object list
        _pListKindOfObject = pImage;
    }
    else
    {
        pImage->materialized() = WD_True;
        _rXamlFile.object_list().insert(pImage);
    }

    return WT_Result::Success;
}

WT_Result WT_XAML_W2X_Parser::Create_PNG_Group4_Image_Shell()
{
    WT_PNG_Group4_Image* pPNG4Image = xamlClassFactory()->Create_PNG_Group4_Image();
    if(pPNG4Image == NULL)
        return WT_Result::Out_Of_Memory_Error;

    WD_CHECK( static_cast<WT_XAML_PNG_Group4_Image *>(pPNG4Image)->parseAttributeList(_oMap,_rXamlFile) );

    if(pPNG4Image->format() == WT_PNG_Group4_Image::Group4X_Mapped)
    {
        //store the pointer for later insertion to file's object list
        _pListKindOfObject = pPNG4Image;
    }
    else
    {
        pPNG4Image->materialized() = WD_True;
        _rXamlFile.object_list().insert(pPNG4Image);
    }

    return WT_Result::Success;
}

WT_Result WT_XAML_W2X_Parser::Create_Image_Color_Map()
{
    WT_Color_Map* pColorMap = xamlClassFactory()->Create_Color_Map();
    if(pColorMap == NULL)
        return WT_Result::Out_Of_Memory_Error;

    WD_CHECK( static_cast<WT_XAML_Color_Map *>(pColorMap)->parseAttributeList(_oMap,_rXamlFile) );

    if(_pListKindOfObject->object_id() == WT_Object::PNG_Group4_Image_ID)
        WD_CHECK( (static_cast<WT_XAML_PNG_Group4_Image *>(_pListKindOfObject))->set(pColorMap) );
    else
        WD_CHECK( (static_cast<WT_XAML_Image *>(_pListKindOfObject))->set(pColorMap) );

    _pListKindOfObject->materialized() = WD_True;
    xamlClassFactory()->Destroy(pColorMap);

    return WT_Result::Success;
}

WT_Result WT_XAML_W2X_Parser::Create_Polymarker_Shell()
{
    WT_Polymarker* pPolymaker = xamlClassFactory()->Create_Polymarker();
    if(pPolymaker == NULL)
        return WT_Result::Out_Of_Memory_Error;

    _rXamlFile.object_list().insert( pPolymaker );
    return WT_Result::Success;
}

WT_Result WT_XAML_W2X_Parser::Create_Polygon_Shell()
{
    WT_Polygon* pPolygon = xamlClassFactory()->Create_Polygon();
    if(pPolygon == NULL)
        return WT_Result::Out_Of_Memory_Error;

    _rXamlFile.object_list().insert( pPolygon );
    return WT_Result::Success;
}

WT_Result WT_XAML_W2X_Parser::Create_User_Hatch_Pattern_Shell()
{
    WT_User_Hatch_Pattern* pUserHatchPattern = xamlClassFactory()->Create_User_Hatch_Pattern();
    if(pUserHatchPattern == NULL)
        return WT_Result::Out_Of_Memory_Error;

    //Need to get height,width and pattern_numer from w2x
    WD_CHECK( ( static_cast<WT_XAML_User_Hatch_Pattern *>(pUserHatchPattern) )->parseAttributeList(_oMap,_rXamlFile) );

    //store the pointer for later insertion to file's object list
    _pListKindOfObject = pUserHatchPattern;

    return WT_Result::Success;
}
    
WT_Result WT_XAML_W2X_Parser::Create_Hatch_Pattern_In_User_Hatch_Pattern()
{
    //Need to get Hatch_Pattern related info(x,y,spacing,skew,angle,data_size,data) from w2x
    if(_pListKindOfObject)
        WD_CHECK( ( static_cast<WT_XAML_User_Hatch_Pattern *>(_pListKindOfObject) )->parseAttributeList(_oMap,_rXamlFile) );

    return WT_Result::Success;
}

void WT_XAML_W2X_Parser::notifyStartElement( const char*   zName,
                                            const char**  ppAttributeList )
                                            throw()
{
    XamlXML::populateAttributeMap(ppAttributeList,_oMap);
    
    const char** pName = _oMap.find(XamlXML::kpzRefName_Attribute);

#ifdef _VERBOSE_DEBUG
#ifdef _WIN32
    char buf[1024];
    char *pzSpaces = "                                                     ";
    if (pName==NULL || *pName==NULL) 
        sprintf_s(buf, 1024, " W2X: %.*s<%s>\n", gNesting*2, pzSpaces, zName );
    else
        sprintf_s(buf, 1024, " W2X: %.*s<%s refName=\"%s\">\n", gNesting*2, pzSpaces, zName, *pName );
    OutputDebugStringA( buf );
    gNesting++;
#endif
#endif

    _zCurrentElementName = zName;
    _nLastIndex = _rXamlFile.nameIndex();

    if (pName!=NULL && *pName!=NULL)
    {
        _nLastIndex = _rXamlFile.parseNameIndex( *pName );
    }

    if ( _nLastIndex <= _rXamlFile.nameIndex() )
    {
        _processStartElement();
    }
    else 
    {
        w2xParser()->stopParser();
        _bSuspended = true;


#ifdef _VERBOSE_DEBUG
#ifdef _WIN32
        OutputDebugStringA( "Higher name index found, stopping W2X parser\n" );
#endif
#endif
    }
}

void WT_XAML_W2X_Parser::notifyEndElement( const char* zName )
throw()
{
    WD_Assert( !_bPendingElementEnded );

    if (_bSuspended )
    {
        _bPendingElementEnded = true;
        return;
    }

    _zCurrentElementName = zName;
    _processEndElement();
}

void 
WT_XAML_W2X_Parser::_processEndElement(void) 
throw()
{
    const char* zName = _zCurrentElementName.ascii();

#ifdef _VERBOSE_DEBUG
#ifdef _WIN32
    gNesting--;
    char buf[1024];
    char *pzSpaces = "                                                     ";
    sprintf_s(buf, 1024, " W2X: %.*s</%s>\n", gNesting*2, pzSpaces, zName );
    OutputDebugStringA( buf );
#endif
#endif

    _bPendingElementEnded = false;

    if (DWFCORE_COMPARE_ASCII_STRINGS(zName,XamlXML::kpzMacro_Draw_Element ) == 0)
    {
        _nInsideMacroDraw--;
    }
    else if (_nInsideMacroDraw == 0)
    {
        if( (_pListKindOfObject != NULL) && 
            ((DWFCORE_COMPARE_ASCII_STRINGS(zName,XamlXML::kpzNamed_View_List_Element) == 0) ||
            (DWFCORE_COMPARE_ASCII_STRINGS(zName,XamlXML::kpzAttribute_URL_Element) == 0) ||
            (DWFCORE_COMPARE_ASCII_STRINGS(zName,XamlXML::kpzURL_Element) == 0) ||
            (DWFCORE_COMPARE_ASCII_STRINGS(zName,XamlXML::kpzPNG_Group4_Image_Element) == 0) ||
            (DWFCORE_COMPARE_ASCII_STRINGS(zName,XamlXML::kpzImage_Element) == 0) ||
            (DWFCORE_COMPARE_ASCII_STRINGS(zName,XamlXML::kpzUser_Hatch_Pattern_Element) == 0))
            )
        {
            _rXamlFile.object_list().insert(_pListKindOfObject);
            _pListKindOfObject = NULL;
            _bAttributeURLStarted = false;
        }

        else if (_pListKindOfObject == NULL && DWFCORE_COMPARE_ASCII_STRINGS(zName,XamlXML::kpzURL_Element) == 0)
        {
            WT_URL* pURL = xamlClassFactory()->Create_URL();
            if(pURL != NULL)
            {
                //Special case - an URL end notification.
                static_cast<WT_XAML_URL *>(pURL)->parseAttributeList(_oMap,_rXamlFile); 
                _rXamlFile.object_list().insert(pURL);
                _bAttributeURLStarted = false;
            }
        }


    }

}

///
///\copydoc DWFXMLCallback::notifyCharacterData
///
void WT_XAML_W2X_Parser::notifyCharacterData( const char* zCData, 
                                             int  nLength ) 
                                             throw()
{

    const char* zName = _zCurrentElementName.ascii();

    if(DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzUserData_Element ) == 0)
    {
		std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iStart;
		std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iEnd;

		if (_rXamlFile.object_list().find_by_id(WT_Object::UserData_ID, iStart, iEnd))
		{
			iEnd--;

			WT_Result result = static_cast<WT_XAML_UserData *>(iEnd->second)->parseCData(nLength,zCData);

            if(result != WT_Result::Success)
				//throw result;
                 return;
		}
	}
    else if(DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzMacro_Definition_Element ) == 0)
    {
		std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iStart;
		std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iEnd;

		if (_rXamlFile.object_list().find_by_id(WT_Object::Macro_Definition_ID, iStart, iEnd))
		{
			iEnd--;

			WT_Result result = static_cast<WT_XAML_Macro_Definition *>(iEnd->second)->parseCData(nLength,zCData);

            if(result != WT_Result::Success)
				//throw result;
                 return;
		}
	}
    else if(DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzOverpost_Element ) == 0)
    {
		std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iStart;
		std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iEnd;

		if (_rXamlFile.object_list().find_by_id(WT_Object::Overpost_ID, iStart, iEnd))
		{
			iEnd--;

			WT_Result result = static_cast<WT_XAML_Overpost*>(iEnd->second)->parseCData(nLength,zCData);

            if(result != WT_Result::Success)
				//throw result;
                 return;
		}
	}
}

///
///\copydoc DWFXMLCallback::notifyStartNamespace()
///
void WT_XAML_W2X_Parser::notifyStartNamespace( const char* /*zPrefix*/,
                                              const char* /*zURI*/ )
                                              throw()
{

}

///
///\copydoc DWFXMLCallback::notifyEndNamespace()
///
void WT_XAML_W2X_Parser::notifyEndNamespace( const char*   /*zPrefix*/ )
throw()
{
}

void WT_XAML_W2X_Parser::_processStartElement()
throw()
{
    WT_Result result = WT_Result::Success;
    const char* zName = _zCurrentElementName.ascii();

    if(DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzMacro_Draw_Element ) == 0)
    {
        if (_nInsideMacroDraw == 0) //if top level
        {
            result = Create_Object_Shell( zName );
        }
        _nInsideMacroDraw++;
#ifdef _VERBOSE_DEBUG
        char buf[1024];
        sprintf( buf, " W2X: Inside Macro Draw, level %d\n", _nInsideMacroDraw );
        OutputDebugStringA( buf );
#endif

    }
    else if (_nInsideMacroDraw == 0) //if top level
    {
        if(DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzNamed_View_List_Element ) == 0)
        {
            result = Create_Named_View_List_Shell();
        }
        else if(DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzAttribute_URL_Element ) == 0)
        {
            result = Create_Attribute_URL_List_Shell();
        }
        else if(DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzURL_Element ) == 0)
        {
            result = Create_URL_List_Shell();
        }
        else if(DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzPolymarker_Element ) == 0)
        {
            result = Create_Polymarker_Shell();

        }        
        else if(DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzPolygon_Element ) == 0)
        {
            result = Create_Polygon_Shell();

        }
        else if(DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzUser_Hatch_Pattern_Element ) == 0)
        {
            result = Create_User_Hatch_Pattern_Shell();
        }
        else if(DWFCORE_COMPARE_ASCII_STRINGS(zName,XamlXML::kpzPNG_Group4_Image_Element) == 0)
        {
            //Create PNG_Group4_Image and if it is of type
            //Group4X_Mapped then we need to wait for Color_Map element to be materialized
            //In such case, Image is assigned to _pListKindOfObject and has not been added to 
            //file's object_list.
            result = Create_PNG_Group4_Image_Shell();
        }
        else if(DWFCORE_COMPARE_ASCII_STRINGS(zName,XamlXML::kpzImage_Element) == 0)
        {
            //Create Image and if it is of type
            //Mapped,Group3X_Mapped or Bitonal_Mapped then we need to wait for Color_Map element to be materialized
            //In such case, Image is assigned to _pListKindOfObject and has not been added to 
            //file's object_list.
            result = Create_Image_Shell();
        }
        else if(DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzW2X_Element ) == 0)
        {
            const char** ppPrefix = _oMap.find(XamlXML::kpzNamePrefix_Attribute);
            if ( ppPrefix!=NULL && *ppPrefix!=NULL )
            {
                _rXamlFile.nameIndexPrefix() = *ppPrefix;
            }
            
            result = Create_DWF_Header();
        }
        else if(DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzW2X_Element ) != 0)
        {
            result = Create_Object_Shell( zName );
        }
    }
    else
    {
#ifdef _VERBOSE_DEBUG
        OutputDebugStringA( " W2X: Inside Macro Draw, ignoring ");
        OutputDebugStringA( zName );
        OutputDebugStringA( "\n" );
#endif
    }
   
    WD_Assert( result == WT_Result::Success);
}

WT_Result WT_XAML_W2X_Parser::parseW2X()
{
    if (_nLastIndex <= _rXamlFile.nameIndex() )
    {
        try
        {
            if ( _oMap.size() > 0 )
            {
#ifdef _VERBOSE_DEBUG
                wchar_t buf[1024];
                wsprintf( buf, L"Pending W2X element <%s Name=\"%s%d\">, processing now...\n", _zCurrentElementName.unicode(), (const wchar_t*)_rXamlFile.nameIndexPrefix(), _nLastIndex );
                OutputDebugString( buf );
#endif
                _processStartElement();
                if (_bPendingElementEnded)
                {
                    _processEndElement();
                }
            }

#ifdef _VERBOSE_DEBUG
            OutputDebugStringA("Starting W2X parser, looking for name == ");
            OutputDebugString( _rXamlFile.nameIndexString() );
            OutputDebugStringA(" \n");
#endif

           _bSuspended = false;
           w2xParser()->parseDocument(*_rXamlFile.w2xStreamIn());
        }
        catch(...)
        {
            return WT_Result::Internal_Error;
        }
    }
    else
    {
#ifdef _VERBOSE_DEBUG
        OutputDebugStringA("Last name index greater than current, W2X not parsing\n");
#endif
    }

    return WT_Result::Success;
}
