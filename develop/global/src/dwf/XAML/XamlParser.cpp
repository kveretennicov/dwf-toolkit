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
#include "XAML/XamlParser.h"
#include "XAML/XamlBrushes.h"

#ifdef _VERBOSE_DEBUG
#ifdef _WIN32
namespace XAML_Xaml_Parser {
    int gNesting = 0;
};
using namespace XAML_Xaml_Parser;
#endif
#endif

WT_XAML_Xaml_Parser::WT_XAML_Xaml_Parser(WT_XAML_File& file) throw(WT_Result)
: _pXamlParser(NULL)
, _rXamlFile(file)
, _oMap()
, _pViewportCanvas(NULL)
, _oDrawables()
, _nLastIndex(0)
, _zCurrentElementName()
, _bPendingElementEnded(false)
, _bSuspended(false)
{
    if (_rXamlFile.xamlStreamIn()->available())
    {
        _pXamlParser = DWFCORE_ALLOC_OBJECT( DWFXMLParser(this) );

        if(_pXamlParser == NULL)
            throw WT_Result::Out_Of_Memory_Error;
    }
}

WT_XAML_Xaml_Parser::~WT_XAML_Xaml_Parser()
throw()
{
    if(xamlParser())
        DWFCORE_FREE_OBJECT(_pXamlParser);
}


void WT_XAML_Xaml_Parser::notifyStartElement( const char*   zName,
                                             const char**  ppAttributeList )
                                             throw()
{
    XamlXML::populateAttributeMap(ppAttributeList,_oMap);

    const char** pName = _oMap.find(XamlXML::kpzName_Attribute);

#ifdef _VERBOSE_DEBUG
#ifdef _WIN32
    char buf[1024];
    char *pzSpaces = "                                                     ";
    if (pName==NULL || *pName==NULL) 
        sprintf_s(buf, 1024, "XAML(%d): %.*s<%s>\n", _oDrawables.size(), gNesting*2, pzSpaces, zName );
    else
        sprintf_s(buf, 1024, "XAML(%d): %.*s<%s Name=\"%s\">\n", _oDrawables.size(), gNesting*2, pzSpaces, zName, *pName );
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
        _pXamlParser->stopParser();
        _bSuspended = true;

#ifdef _VERBOSE_DEBUG
#ifdef _WIN32
        OutputDebugStringA( "Higher name index found, stopping XAML parser\n" );
#endif
#endif
    }
}

void WT_XAML_Xaml_Parser::notifyEndElement( const char* zName )
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
WT_XAML_Xaml_Parser::_processEndElement(void) 
throw()
{
    const char* zName = _zCurrentElementName.ascii();

#ifdef _VERBOSE_DEBUG
#ifdef _WIN32
    {
        gNesting--;
        char buf[1024];
        char *pzSpaces = "                                                     ";
        sprintf_s(buf, 1024, "XAML(%d): %.*s</%s>\n", _oDrawables.size(), gNesting*2, pzSpaces, zName );
        OutputDebugStringA( buf );
    }
#endif
#endif

    _bPendingElementEnded = false;

    if (_nLastIndex < _rXamlFile.nameIndex() )
    {
        return;
    }

    if ( DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzPath_Element ) == 0  ||
        DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzGlyphs_Element ) == 0  ||
        DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzCanvas_Element ) == 0 )
    {
        XamlDrawable *pDrawable = NULL;
        if (! _oDrawables.empty() )
        {
            pDrawable = _oDrawables.top();
        }

        WT_Result res = XamlObjectFactory::processAttributes( _rXamlFile, pDrawable );
        WD_Assert(res == WT_Result::Success);

        res = XamlObjectFactory::processObjects( _rXamlFile, pDrawable );
        WD_Assert(res == WT_Result::Success);

        if ( pDrawable != NULL )
        {
            if (pDrawable == _pViewportCanvas)
            {
                _pViewportCanvas = NULL; //we've found the close of the viewport
            }
            DWFCORE_FREE_OBJECT( pDrawable );
            _oDrawables.pop();
        }
    }
}

void WT_XAML_Xaml_Parser::notifyCharacterData( const char* /*zCData*/, 
                                              int /* nLength*/ ) 
                                              throw()
{

}

void WT_XAML_Xaml_Parser::notifyStartNamespace( const char* /*zPrefix*/,
                                               const char* /*zURI*/ )
                                               throw()
{

}

void WT_XAML_Xaml_Parser::notifyEndNamespace( const char*   /*zPrefix*/ )
throw()
{
}

void WT_XAML_Xaml_Parser::_processStartElement()
throw(DWFException)
{
    if (_nLastIndex < _rXamlFile.nameIndex() )
    {
#ifdef _VERBOSE_DEBUG
        OutputDebugStringA( "XAML: Last index < desired.  Skipping element\n" );
#endif
        return;
    }

    static const char* kpzCanvasResources = "Canvas.Resources";
    static const char* kpzLinearGradientBrushGradientStops = "LinearGradientBrush.GradientStops";
    static const char* kpzPathFill = "Path.Fill";
    static const char* kpzPathStroke = "Path.Stroke";

    const char *zName = _zCurrentElementName.ascii();

    if ( DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzPath_Element ) == 0 )
    {
        XamlPath *pPath = DWFCORE_ALLOC_OBJECT( XamlPath );
        if (pPath == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate memory for Path object." );
        }
        XamlPath::AttributeParser oAttributeParser( _oMap );
        WT_Result res = oAttributeParser.providePathAttributes( pPath, _rXamlFile );
        if (res != WT_Result::Success)
        {
           _DWFCORE_THROW( DWFUnexpectedException, L"Error parsing Path attributes." );
        }

        _oDrawables.push( pPath );
    }
    else if ( DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzGlyphs_Element ) == 0 )
    {
        XamlGlyphs *pGlyphs = DWFCORE_ALLOC_OBJECT( XamlGlyphs );
        if (pGlyphs == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate memory for Glyphs object." );
        }

        XamlGlyphs::AttributeParser oAttributeParser( _oMap );
        WT_Result res = oAttributeParser.provideGlyphsAttributes( pGlyphs, _rXamlFile );
        if (res != WT_Result::Success)
        {
           _DWFCORE_THROW( DWFUnexpectedException, L"Error parsing Glyph attributes." );
        }

        _oDrawables.push( pGlyphs );
    }
    else if ( DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzCanvas_Element ) == 0 )
    {
        XamlCanvas *pCanvas = DWFCORE_ALLOC_OBJECT( XamlCanvas );
        if (pCanvas == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate memory for Canvas object." );
        }

        XamlCanvas::AttributeParser oAttributeParser( _oMap );
        WT_Result res = oAttributeParser.provideCanvasAttributes( pCanvas, _rXamlFile );
        if (res != WT_Result::Success)
        {
           _DWFCORE_THROW( DWFUnexpectedException, L"Error parsing Canvas attributes." );
        }

        _oDrawables.push( pCanvas );
        const wchar_t* zCanvasName = _getName( pCanvas );

        //Determine if we're entering a viewport
        //Note, nested viewports are not allowed.
        bool bIsAViewport = false;
        
        if (zCanvasName != NULL)
        {
			std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iStart;
			std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iEnd;

			if (_rXamlFile.object_list().find_by_id(WT_Object::Viewport_ID, iStart, iEnd))
			{
                WD_Assert( iStart != iEnd );

                _pViewportCanvas = pCanvas;
				bIsAViewport = true;
                
				WT_XAML_Viewport *pVp = (WT_XAML_Viewport *)iStart->second;
                
				pVp->consumeClip(pCanvas);
                _rXamlFile.set_materialized( pVp );
                iStart++;

                WD_Assert( iStart == iEnd ); //there better only be one viewport
			}
		}

        if (!bIsAViewport && zCanvasName != NULL)
        {
            //We have something special here
            //Find the first non-materialized drawable, that's our target
            bool bFound = false;
			std::multimap<WT_Object::WT_Type, WT_Object*>::iterator iStart;
			std::multimap<WT_Object::WT_Type, WT_Object*>::iterator iEnd;

			if (_rXamlFile.object_list().find_by_type(WT_Object::Drawable, iStart, iEnd))
			{
				for(; iStart != iEnd; iStart++)
				{
					if ( !iStart->second->materialized() )
					{
						_rXamlFile.nested_object_stack().push( iStart->second );
						bFound = true;
						break;
					}
				}
            }

			//Note, fills are materialized via W2X
        }
    }
    else if ( DWFCORE_COMPARE_ASCII_STRINGS( zName, kpzCanvasResources ) == 0 )
    {
        //Do nothing, the real processing is when we see the dictionary reference
    }
    else if ( DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzLinearGradientBrush_Element ) == 0 )
    {
        // We should expect a "special object" to be waiting for us
        if (_rXamlFile.nested_object_stack().size() == 0 )
        {
            WD_Assert(false);
            return;
        }

        //Create a LinearGradient in the Path
        WT_Object *pObject = _rXamlFile.nested_object_stack().top();

        if (pObject->object_id() != WT_Object::Gouraud_Polytriangle_ID &&
            pObject->object_id() != WT_Object::Gouraud_Polyline_ID )
        {
            WD_Assert(false);
            return;
        }

        if (_oDrawables.size() == 0 )
        {
            WD_Assert(false);
            return;
        }

        XamlDrawable* pDrawable = _oDrawables.top();
        if ( pDrawable->xobject_id() != XamlDrawable::Path_ID )
        {
            WD_Assert(false);
            return;
        }

        XamlPath *pPath = static_cast< XamlPath* >( pDrawable );
        //This is really just a placeholder.
        XamlBrush::LinearGradient *pLinearGradient = DWFCORE_ALLOC_OBJECT( XamlBrush::LinearGradient );

        const char **ppValue = _oMap.find( XamlXML::kpzStartPoint_Attribute );
        if (ppValue!=NULL && *ppValue!=NULL)
        {
            WT_Point2D oStartPt;
            char *pPtr = const_cast< char*> (*ppValue);
            WT_Result res = XamlPathGeometry::XamlPathFigure::getPoint( NULL, pPtr, oStartPt, false );
            if (res != WT_Result::Success)
            {
                WD_Assert(false);
                return;
            }
             _rXamlFile.unflipPoint( oStartPt );
            pLinearGradient->startPoint() = oStartPt;
        }
        ppValue = _oMap.find( XamlXML::kpzEndPoint_Attribute );
        if (ppValue!=NULL && *ppValue!=NULL)
        {
            WT_Point2D oEndPt;
            char *pPtr = const_cast< char*> (*ppValue);
            WT_Result res = XamlPathGeometry::XamlPathFigure::getPoint( NULL, pPtr, oEndPt, false );
            if (res != WT_Result::Success)
            {
                WD_Assert(false);
                return;
            }
            _rXamlFile.unflipPoint( oEndPt );
            pLinearGradient->endPoint() = oEndPt;
        }

        if ( pObject->object_id() == WT_Object::Gouraud_Polytriangle_ID )
        {
            //if the WT_ object is a polytri, we set the brush in Fill
            XamlDrawableAttributes::FillProvider oProvider( pLinearGradient );
            ((XamlDrawableAttributes::Fill::Consumer*)pPath)->consumeFill( &oProvider );
        }
        else
        {
            //the WT_ object is a polyline, we set the brush in Stroke
            XamlDrawableAttributes::StrokeProvider oProvider( pLinearGradient );
            ((XamlDrawableAttributes::Stroke::Consumer*)pPath)->consumeStroke( &oProvider );
        }
    }
    else if ( DWFCORE_COMPARE_ASCII_STRINGS( zName, kpzLinearGradientBrushGradientStops ) == 0 )
    {
        //Do nothing, we're only interested in the stops themselves
    }
    else if ( DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzGradientStop_Element ) == 0 )
    {
        if (_oDrawables.size() == 0 )
        {
            WD_Assert(false);
            return;
        }

        XamlDrawable* pDrawable = _oDrawables.top();
        if ( pDrawable->xobject_id() != XamlDrawable::Path_ID )
        {
            WD_Assert(false);
            return;
        }

        XamlPath *pPath = static_cast< XamlPath* >( pDrawable );
        const XamlBrush::Brush *pBrush = NULL;

        if ( pPath->fill().brush() != NULL )
        {
            pBrush = pPath->fill().brush();
        }
        else
        {
            pBrush = pPath->stroke().brush();
        }

        if ( pBrush->type() != XamlBrush::kLinearGradient)
        {
            WD_Assert(false);
            return;
        }

        XamlBrush::LinearGradient *pLinearGradient = static_cast< XamlBrush::LinearGradient* >( const_cast< XamlBrush::Brush* > ( pBrush ) );

        //Get the color of the brush
        WT_RGBA32 oColor;
        const char **ppValue = _oMap.find( XamlXML::kpzColor_Attribute );
        if (ppValue!=NULL && *ppValue!=NULL)
        {
            XamlBrush::Brush::ReadColor( oColor, *ppValue );
        }

        //Set the appropriate color of the brush
        ppValue = _oMap.find( XamlXML::kpzOffset_Attribute );
        if (ppValue!=NULL && *ppValue!=NULL)
        {
            int nOffset = atoi( *ppValue );
            if (nOffset == 0)
            {
                pLinearGradient->startColor() = oColor;
            }
            else if (nOffset == 1)
            {
                pLinearGradient->endColor() = oColor;
            }
            else
            {
                WD_Assert(false);
                return;
            }
        }
    }
    else if ( DWFCORE_COMPARE_ASCII_STRINGS( zName, kpzPathFill ) == 0 )
    {
        //We don't materialize these, we use the W2X instead.
    }
    else if ( DWFCORE_COMPARE_ASCII_STRINGS( zName, kpzPathStroke ) == 0 )
    {
        //We don't materialize these, we use the W2X instead.
    }
    else if ( DWFCORE_COMPARE_ASCII_STRINGS( zName, XamlXML::kpzResourceDictionary_Element ) == 0 )
    {
        XamlDrawable *pDrawable = _oDrawables.top();
        if ( pDrawable != NULL && pDrawable->xobject_id() == XamlDrawable::Canvas_ID )
        {
            XamlCanvas* pCanvas = static_cast< XamlCanvas* >( pDrawable );

            //read the source ref
            const char** pValue = _oMap.find( XamlXML::kpzSource_Attribute );
            WD_Assert( pValue != NULL && *pValue != NULL );
            if ( pValue == NULL || *pValue == NULL )
            {
               _DWFCORE_THROW( DWFUnexpectedException, L"Corrupted File." );
            }

            //set the source ref on the canvas
            XamlDrawableAttributes::ResourcesProvider oProvider( *pValue );
            WT_Result res = static_cast< XamlDrawableAttributes::CanvasAttributeConsumer* >( pCanvas )->consumeResources( &oProvider );
            WD_Assert( res == WT_Result::Success );
            if (res != WT_Result::Success)
            {
                _DWFCORE_THROW( DWFUnexpectedException, L"Error consuming Resources." );
            }

            //connect resource dictionary to file
            DWFInputStream *pInputStream = NULL;
            res = _rXamlFile.opcResourceMaterializer()->getPartInputStream( *pValue, &pInputStream );
            WD_Assert( res == WT_Result::Success );
            if (res != WT_Result::Success)
            {
                _DWFCORE_THROW( DWFUnexpectedException, L"Error getting Resource Stream." );

            }
            _rXamlFile.xamlDictionaryStreamIn() = pInputStream;
            //Note, macros are materialized entirely via W2X
        }
        else
        {
            WD_Assert(false);
            _DWFCORE_THROW( DWFUnexpectedException, L"Corrupted File." );
        }

    }
    else
    {
        //Code for other missing elements here.
        WD_Assert(false);
        _DWFCORE_THROW( DWFUnexpectedException, L"Corrupted File." );
    }
}

const wchar_t* 
WT_XAML_Xaml_Parser::_getName( XamlDrawable* pDrawable )
{
    const wchar_t* pName = NULL;
    switch( pDrawable->xobject_id() )
    {
    case XamlDrawable::Canvas_ID:
        pName = static_cast< XamlCanvas* >( pDrawable )->name().name();
        break;

    case XamlDrawable::Path_ID:
        pName = static_cast< XamlPath* >( pDrawable )->name().name();
        break;

    case XamlDrawable::Glyphs_ID:
        pName = static_cast< XamlGlyphs* >( pDrawable )->name().name();
    default:
        break;
    }

    return pName;
}

WT_Result WT_XAML_Xaml_Parser::parseXaml()
{
    if (_nLastIndex <= _rXamlFile.nameIndex() )
    {
        try
        {
            if ( _oMap.size() > 0 )
            {
#ifdef _VERBOSE_DEBUG
                wchar_t buf[1024];
                wsprintf( buf, L"Pending XAML element <%s Name=\"%s%d\">, processing now...\n", _zCurrentElementName.unicode(), (const wchar_t*)_rXamlFile.nameIndexPrefix(), _nLastIndex );
                OutputDebugString( buf );
#endif
                _processStartElement();
                if (_bPendingElementEnded)
                {
                    _processEndElement();
                }
            }

#ifdef _VERBOSE_DEBUG
            OutputDebugStringA("Starting XAML parser, looking for name == ");
            OutputDebugString( _rXamlFile.nameIndexString() );
            OutputDebugStringA(" \n");
#endif

            _bSuspended = false;
            //
            // account for 0-byte xaml stream
            //
            if (xamlParser())
            {
                xamlParser()->parseDocument(*_rXamlFile.xamlStreamIn());
            }
        }
        catch( DWFUserCancelledException& )
        {
            return WT_Result::User_Requested_Abort;
        }
        catch(...)
        {
            return WT_Result::Internal_Error;
        }
    }
    else
    {
#ifdef _VERBOSE_DEBUG
        OutputDebugStringA("Last name index greater than current, XAML not parsing\n");
#endif
    }

    return WT_Result::Success;

}

