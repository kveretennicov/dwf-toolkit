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
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/XAML/XamlObjectFactory.cpp#1 $

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include "XAML/pch.h"

#define gfToDegrees ((const double)(180. / M_PI))
#define gfWhipDegrees ((const double)(65536. / 360.))

using namespace XamlDrawableAttributes;


WT_Color *
XamlObjectFactory::_createOptimizedColor( WT_XAML_File& rFile, WT_RGBA32 &rgba )
{
    WT_XAML_Class_Factory oClassFactory;
    WT_Color *pColor = NULL;

    // map color if possible
    WT_Color_Map *pColorMap = &rFile.rendition().color_map();

    // If there is a color map in the object list, we need to sync it first.
    std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iStart;
    std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iEnd;
    if (rFile.object_list().find_by_id(WT_Object::Color_Map_ID, iStart, iEnd))
    {
        if (iStart != iEnd && iStart->second  != NULL && iStart->second->object_id() == WT_Object::Color_Map_ID )
	    {
            pColorMap = static_cast<WT_Color_Map*>(iStart->second);
        }
    }

    if (pColorMap == NULL)
    {
        return NULL;
    }

    int idx = pColorMap->exact_index( rgba );
    if (idx != -1)
    {
        pColor = oClassFactory.Create_Color( idx, *pColorMap );
    }
    else
    {
        pColor = oClassFactory.Create_Color( rgba );
    }

    return pColor;
}

WT_Object *
XamlObjectFactory::findPendingObject( WT_XAML_File& rFile, WT_Object::WT_Type eType, WT_Object::WT_ID eID )
{
	WT_Object* pObject = NULL;
	if (rFile.object_list().find_first_incomplete(eType, eID, pObject))
	{
		return pObject;
	}
	else
	{
	    return NULL;
	}
}

WT_Result 
XamlObjectFactory::processAttributes( WT_XAML_File& rFile, XamlDrawable* pDrawable )
{
    if (pDrawable==NULL)
    {
        return WT_Result::Toolkit_Usage_Error;
    }

    switch( pDrawable->xobject_id() )
    {
    case XamlDrawable::Canvas_ID:
        return _processCanvasAttributes( rFile, static_cast< XamlCanvas * >( pDrawable ));

    case XamlDrawable::Path_ID:
        return _processPathAttributes( rFile, static_cast< XamlPath* >( pDrawable ));

    case XamlDrawable::Glyphs_ID:
        return _processGlyphsAttributes( rFile, static_cast< XamlGlyphs* >( pDrawable ));
    default:
        break;
    }

    return WT_Result::Success;
}

WT_Result 
XamlObjectFactory::processObjects( WT_XAML_File& rFile, XamlDrawable* pDrawable )
{
    if (pDrawable==NULL)
    {
        return WT_Result::Toolkit_Usage_Error;
    }

    switch( pDrawable->xobject_id() )
    {
    case XamlDrawable::Canvas_ID:
        return _processCanvasObjects( rFile, static_cast< XamlCanvas * >( pDrawable ));

    case XamlDrawable::Path_ID:
        return _processPathObjects( rFile, static_cast< XamlPath* >( pDrawable ));

    case XamlDrawable::Glyphs_ID:
        return _processGlyphsObjects( rFile, static_cast< XamlGlyphs* >( pDrawable ));
    default:
        break;
    }

    return WT_Result::Success;
}

WT_Result
XamlObjectFactory::insertObject( WT_XAML_File& rFile, WT_Object *pInsertObject, WT_Class_Factory& rClassFactory )
{
	std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iStart;
	std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iEnd;

    WT_Object *pObject = NULL;
	WT_XAML_File::WT_XAML_ObjectList& rObjList = rFile.object_list();

		//
		// optimize for known work
		//
	if (rObjList.find_by_id(WT_Object::Viewport_ID, iStart, iEnd))
	{
		for (; iStart != iEnd; iStart++)
		{
			pObject = iStart->second;
			if ( !pObject->materialized()  && (pObject->object_id() == pInsertObject->object_id()) )
			{
				// Merge the object data
				WT_XAML_Viewport *pViewport = static_cast< WT_XAML_Viewport* >( pObject );
				WT_XAML_Viewport *pInsertViewport = static_cast< WT_XAML_Viewport* >( pInsertObject );
				pViewport->set( pInsertViewport->contour() );

                rFile.set_materialized( pObject );
				rClassFactory.Destroy( pInsertObject );

				// done
				return WT_Result::Success;
			}
		}
	}

	//
	// no type match, no unmaterialized id match --> insert in front
	//
	rFile.object_list().insert( pInsertObject );

	// done
	return WT_Result::Success;
}


/***
WT_Result
XamlObjectFactory::insertObject( WT_XAML_File& rFile, WT_Object *pInsertObject, WT_Class_Factory& rClassFactory )
{
	std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iStart;
	std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iEnd;

    WT_Object *pObject = NULL;
	WT_Object::WT_ID eID = pInsertObject->object_id();
	WT_XAML_File::WT_XAML_ObjectList& rObjList = rFile.object_list();

		//
		// searching by by each interesting id should be much faster
		// as type queries can return larger lists.
		//
		// the logic is: if the inserted object is not materialized and
		// and matches an existing object in both type and id then
		// it should be merged (currently on viewports supported)
		// otherwise, just add it to the queue
		//
	if (rObjList.find_by_id(eID, iStart, iEnd))
	{
		for (; iStart != iEnd; iStart++)
		{
			pObject = iStart->second;

				//
				// non-materialized & types match
				//
			if ((pObject->materialized() == WD_False) && (pObject->object_type() == pInsertObject->object_type()))
			{
					//
					// object-id specific codes	
					//
				switch (eID)
				{
					case WT_Object::Viewport_ID:
					{
						// Merge the object data
						WT_Viewport *pViewport = static_cast< WT_Viewport* >( pObject );
						WT_Viewport *pInsertViewport = static_cast< WT_Viewport* >( pInsertObject );
						pViewport->set( pInsertViewport->contour() );

						break;
					}
					default:
					{
						WD_Assert( false );
					}
				}

				pObject->materialized() = WD_True;
				rClassFactory.Destroy( pInsertObject );

				// done
				return WT_Result::Success;
			}
		}
	}

	//
	// no type match, no unmaterialized id match --> insert in front
	//
	rFile.object_list().push_back( pInsertObject );

	// done
	return WT_Result::Success;
}
***/

WT_Result
XamlObjectFactory::_processPathAttributes( WT_XAML_File& rFile, XamlPath* pPath )
{
    WT_XAML_Class_Factory oClassFactory;

    //Fill in current rendition attributes

    //Set the default list of attributes that can be taken from the Path
    WT_Integer32 parts_to_sync =    
		WT_Rendition::Color_Bit           |
        WT_Rendition::Visibility_Bit      |
        WT_Rendition::Dash_Pattern_Bit    |
        WT_Rendition::Line_Style_Bit;

    if ( rFile.nested_object_stack().size() > 0 )
    {
        WT_Object *pObject = rFile.nested_object_stack().top();
        if (pObject->object_id() == WT_Object::Gouraud_Polyline_ID ||
            pObject->object_id() == WT_Object::Gouraud_Polytriangle_ID)
        {
            parts_to_sync = WT_Rendition::Visibility_Bit;
        }
        //deal with other special objects here
    }

    ///

    if ( (parts_to_sync & WT_Rendition::Color_Bit) && pPath->fill().brush() == NULL && pPath->stroke().brush() != NULL )
    {
        if (pPath->stroke().brush() != NULL &&
            pPath->stroke().brush()->type() == XamlBrush::kSolidColor &&
            (static_cast< const XamlBrush::SolidColor* > ( 
            static_cast< const XamlBrush::Brush* > ( 
            pPath->stroke().brush() ) )->color() != rFile.rendition().color().rgba() ) )
        {
#ifdef _VERBOSE_DEBUG
            OutputDebugStringA( "Creating object (XamlPath): WT_Color (Stroke)\n" );
#endif
            WT_RGBA32 rgba = static_cast< const XamlBrush::SolidColor* > ( static_cast< const XamlBrush::Brush* > ( pPath->stroke().brush() ) )->color();
            
            WT_Color *pColor = _createOptimizedColor( rFile, rgba );

            if (pColor == NULL)
                return WT_Result::Out_Of_Memory_Error;

            pColor->materialized() = WD_True;
            WD_CHECK( insertObject( rFile, pColor, oClassFactory ) );
        }
    }
    else if ( (parts_to_sync & WT_Rendition::Color_Bit) && pPath->fill().brush() != NULL && pPath->stroke().brush() == NULL )
    {
        if (pPath->fill().brush()->type() == XamlBrush::kSolidColor &&
            (static_cast< const XamlBrush::SolidColor* > ( 
            static_cast< const XamlBrush::Brush* > ( 
            pPath->fill().brush() ) )->color() != rFile.rendition().color().rgba() ) )
        {
#ifdef _VERBOSE_DEBUG
            OutputDebugStringA( "Creating object (XamlPath): WT_Color (Fill)\n" );
#endif
            WT_RGBA32 rgba = static_cast< const XamlBrush::SolidColor* > ( static_cast< const XamlBrush::Brush* > ( pPath->fill().brush() ) )->color();

            WT_Color *pColor = _createOptimizedColor( rFile, rgba );

            if (pColor == NULL)
                return WT_Result::Out_Of_Memory_Error;

            pColor->materialized() = WD_True;
            WD_CHECK( insertObject( rFile, pColor, oClassFactory ) );
        }
    }

    if ( ( parts_to_sync & WT_Rendition::Dash_Pattern_Bit ) && pPath->strokeDashArray().dashGap().size() > 0)
    {
        WT_XAML_Dash_Pattern *pDashPattern = static_cast<WT_XAML_Dash_Pattern *>( findPendingObject( rFile, WT_Object::Attribute, WT_Object::Dash_Pattern_ID ) );
        if ( pDashPattern != NULL )
        {
			if ( pDashPattern->number() != -1)
			{
				WD_CHECK( static_cast< StrokeDashArray::Consumer *>( pDashPattern )->consumeStrokeDashArray( static_cast< StrokeDashArray::Provider* >( pPath ) ) );
			}
            rFile.set_materialized( pDashPattern );
        }
    }

    if ( ( parts_to_sync & WT_Rendition::Visibility_Bit ) &&
        ((pPath->opacity().value() == 0.f) != (rFile.rendition().visibility().visible() == WD_False)) )
    {
#ifdef _VERBOSE_DEBUG
        OutputDebugStringA( "Creating object (XamlPath): WT_Visibility \n" );
#endif
        WT_Boolean bVisible = (pPath->opacity().value() == 0.f) ? WD_False : WD_True;
        WT_Visibility *pVisibility = oClassFactory.Create_Visibility( bVisible );
        if (pVisibility == NULL)
            return WT_Result::Out_Of_Memory_Error;
        pVisibility->materialized() = WD_True;
        WD_CHECK( insertObject( rFile, pVisibility, oClassFactory ) );
    }

    if ( parts_to_sync & WT_Rendition::Line_Style_Bit )
    {
        WT_XAML_Line_Style *pLine_Style = static_cast< WT_XAML_Line_Style* >( findPendingObject( rFile, WT_Object::Attribute, WT_Object::Line_Style_ID ) );
        if (pLine_Style != NULL)
        {
            WD_CHECK( ((StrokeDashCap::Consumer*)(WT_XAML_Line_Style*)pLine_Style)->consumeStrokeDashCap( (StrokeDashCap::Provider*) pPath ) );
            WD_CHECK( ((StrokeEndLineCap::Consumer*)(WT_XAML_Line_Style*)pLine_Style)->consumeStrokeEndLineCap( (StrokeEndLineCap::Provider*) pPath ) );
            WD_CHECK( ((StrokeStartLineCap::Consumer*)(WT_XAML_Line_Style*)pLine_Style)->consumeStrokeStartLineCap( (StrokeStartLineCap::Provider*) pPath ) );
            WD_CHECK( ((StrokeLineJoin::Consumer*)(WT_XAML_Line_Style*)pLine_Style)->consumeStrokeLineJoin( (StrokeLineJoin::Provider*) pPath ) );

            //By now it should have been materialized fully.
            rFile.set_materialized( pLine_Style );
        }
    }

    return WT_Result::Success;
}

WT_Result
XamlObjectFactory::_processPathObjects( WT_XAML_File& rFile, XamlPath* pPath )
{
    WT_XAML_Class_Factory oClassFactory;

    //Algorithm:
    //
    //If Path is not filled...
    //   Create Polylines and Outline Ellipses
    //else
    //   If there are some drawable objects in the object list, materialize them.
    //   Else: Create all polyline and polygon and materialize them.

    // SPECIAL NOTE: If an filled ellipse is serialized as "not closed", we will create an 
    // preceeding line segment from the center point to the start point, and the ellipse will be 
    // serialized as closed.  We need to deal with this in materialization - i.e. ignore the preceeding 
    // line segment.  We can know this has happened because a WT_Filled_Ellipse will be sitting in the 
    // file's object list.  So, if we see a path preceeding the ellipse in our figure vector, we will 
    // know what to do.
     
    if ( rFile.nested_object_stack().size() > 0 )
    {
        WT_Object *pObject = rFile.nested_object_stack().top();
        if (pObject->object_id() == WT_Object::Gouraud_Polyline_ID)
        {
			WT_Result r;

#ifdef _VERBOSE_DEBUG
            OutputDebugStringA( "Merging object (XamlPath): WT_Gouraud_Polyline \n" );
#endif
            WT_XAML_Gouraud_Polyline* pGouraudLine = static_cast< WT_XAML_Gouraud_Polyline* >( pObject );

			const XamlBrush::LinearGradient* pLinearGradient = static_cast< const XamlBrush::LinearGradient* >( static_cast< const XamlBrush::Brush* >( pPath->stroke().brush() ) );
			
            XamlPathGeometry::XamlPathFigure::tGraphicsVector::const_iterator iter = pPath->data().geometry().figure().graphics().begin();
            XamlGraphicsObject *pGraphicsObject = NULL;
			
			//
			// This makes factory be able to process multiple Gouraud_Polylines in a Path
			// even though this is outside the XPS spec 
			//
			for(; iter != pPath->data().geometry().figure().graphics().end() && r == WT_Result::Success; iter++ )
			{
				pGraphicsObject = *iter;
				if(pGraphicsObject)
				{
					if ( pGraphicsObject->type() != XamlGraphicsObject::PolylineSegment )
					{
						WD_Assert( false );
						return WT_Result::Internal_Error;
					}

					if ( pPath->stroke().brush() == NULL )
					{
						WD_Assert( false );
						return WT_Result::Internal_Error;
					}
					if ( pPath->stroke().brush()->type() != XamlBrush::kLinearGradient )
					{
						WD_Assert( false );
						return WT_Result::Internal_Error;
					}

					// Pull the WT_RGBA32 color out of the Linear Gradient Brush
					XamlPolylineSegment *pSegment = static_cast< XamlPolylineSegment* >( pGraphicsObject );
					r = pGouraudLine->materializeSegment( *pSegment, pLinearGradient->startColor(), pLinearGradient->endColor(), rFile );
				}
				else
				{
					return WT_Result::Internal_Error;
				}
			}
			return r;
        }

        else if (pObject->object_id() == WT_Object::Gouraud_Polytriangle_ID)
        {
#ifdef _VERBOSE_DEBUG
            OutputDebugStringA( "Merging object (XamlPath): WT_Gouraud_Polytriangle \n" );
#endif
            WT_XAML_Gouraud_Polytriangle* pGouraudTriangle = static_cast< WT_XAML_Gouraud_Polytriangle* >( pObject );
            if ( pPath->data().geometry().figure().graphics().size() != 1 )
            {
                WD_Assert( false );
                return WT_Result::Internal_Error;
            }

            if ( pPath->fill().brush() == NULL )
            {
                WD_Assert( false );
                return WT_Result::Internal_Error;
            }
            if ( pPath->fill().brush()->type() != XamlBrush::kLinearGradient )
            {
                WD_Assert( false );
                return WT_Result::Internal_Error;
            }

            // Pull the WT_RGBA32 color out of the Linear Gradient Brush
            const XamlBrush::LinearGradient* pLinearGradient = static_cast< const XamlBrush::LinearGradient* >( static_cast< const XamlBrush::Brush* >( pPath->fill().brush() ) );
            return pGouraudTriangle->materializePoint( pLinearGradient->startPoint(), pLinearGradient->startColor(), rFile);
        }

        else if (pObject->object_id() == WT_Object::Polymarker_ID)
        {
#ifdef _VERBOSE_DEBUG
            OutputDebugStringA( "Merging object (XamlPath): WT_Polymarker \n" );
#endif

            size_t nPoints = pPath->data().geometry().figure().graphics().size();
            WT_Logical_Point *pPoints = DWFCORE_ALLOC_MEMORY( WT_Logical_Point, nPoints );
            if (pPoints == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }

            int nPointIndex = 0;
            XamlPathGeometry::XamlPathFigure::tGraphicsVector::const_iterator iter = pPath->data().geometry().figure().graphics().begin();
            for( ; iter != pPath->data().geometry().figure().graphics().end(); iter++ )
            {
                XamlGraphicsObject *pGraphicsObject = *iter;
                if ( pGraphicsObject->type() != XamlGraphicsObject::PolylineSegment )
                {
                    WD_Assert( false );
                    return WT_Result::Internal_Error;
                }

                const XamlPolylineSegment *pSegment = static_cast<const XamlPolylineSegment *>( pGraphicsObject );
                WD_Assert( pSegment->pointSet().count() == 2 ); //else it's not a polymarker

                float fWidth = rFile.convertToPaperSpace( (float)rFile.rendition().line_weight().weight_value() );

                pPoints[ nPointIndex ].m_x = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[0].m_x + (fWidth/2) ); // nudge it back
                pPoints[ nPointIndex++ ].m_y = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[0].m_y );
            }

            WT_Polymarker* pPolymarker = static_cast< WT_Polymarker* >( pObject );
            pPolymarker->set( (int)nPoints, pPoints, WD_True );
            rFile.set_materialized( pPolymarker );
            DWFCORE_FREE_MEMORY( pPoints );

            return WT_Result::Success;
        }
        else if (pObject->object_id() == WT_Object::Polygon_ID)
        {

#ifdef _VERBOSE_DEBUG
    OutputDebugStringA( "Merging object (XamlPath): WT_Polygon \n" );
#endif
            int nIndex = 0;
            int nCount = 0;
            WT_Logical_Point *pPoints = NULL;
            XamlPathGeometry::XamlPathFigure::tGraphicsVector::const_iterator iter = pPath->data().geometry().figure().graphics().begin();
            for( ; iter != pPath->data().geometry().figure().graphics().end(); iter++ )
            {
                XamlGraphicsObject *pGraphicsObject = *iter;
                if ( pGraphicsObject->type() != XamlGraphicsObject::PolylineSegment )
                {
                    WD_Assert( false );
                    return WT_Result::Internal_Error;
                }

                const XamlPolylineSegment *pSegment = static_cast<const XamlPolylineSegment *>( pGraphicsObject );
                nCount = pSegment->pointSet().count() + (pSegment->closed() ? 1 : 0);
                pPoints = DWFCORE_ALLOC_MEMORY( WT_Logical_Point, nCount );
                if (pPoints == NULL)
                {
                    return WT_Result::Out_Of_Memory_Error;
                }
                for ( int i=0; i< pSegment->pointSet().count(); i++ )
                {
                    pPoints[nIndex].m_x = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[i].m_x ); //round for fuzz or noise
                    pPoints[nIndex++].m_y = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[i].m_y ); //round for fuzz or noise
                }
                if (pSegment->closed())
                {
                    // add first point if figure is closed
                    pPoints[nIndex].m_x = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[0].m_x ); //round for fuzz or noise
                    pPoints[nIndex++].m_y = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[0].m_y ); //round for fuzz or noise
                }
            }          

            WT_Polygon* pPolygon = static_cast< WT_Polygon* >( pObject );
            pPolygon->set( nCount, pPoints, WD_True );
            rFile.set_materialized( pPolygon );
            DWFCORE_FREE_MEMORY( pPoints );

            return WT_Result::Success;
        }
        //deal with other nested objects here

        else
        {
            //do not let special objects create standard things
            return WT_Result::Success;
        }
    }

    // For degenerate polygons, there is no need to make a special logic branch,
    // The way to handle degenerate polygons is not adding a closing point to the end.
    // Currently xps viewer can't draw degenerate polygons. So if ADR support this,
    // there maybe some inconsistency between ADR and XPS Viewer. 

    //If not filled...
    if ( pPath->fill().brush() == NULL && pPath->stroke().brush() != NULL )
    {
        //Check for fill
        if(rFile.rendition().fill().fill())
        {
            WT_Fill* pFill = oClassFactory.Create_Fill(WD_False);
            if (pFill == NULL)
                return WT_Result::Out_Of_Memory_Error;
            pFill->materialized() = WD_True;
            WD_CHECK( insertObject( rFile, pFill, oClassFactory ) );
        }

        //Create Polylines and Outline Ellipses
        bool bIgnoreNextArc = false; //for specially closed arcs
        XamlPathGeometry::XamlPathFigure::tGraphicsVector::const_iterator iter = pPath->data().geometry().figure().graphics().begin();
        for(; iter != pPath->data().geometry().figure().graphics().end(); iter++)
        {
            XamlGraphicsObject *pXamlObject = (*iter);
            switch( pXamlObject->type() )
            {
            case XamlGraphicsObject::ArcSegment:
                {
                    if (bIgnoreNextArc)
                    {
                        bIgnoreNextArc = false;
                        break;
                    }
#ifdef _VERBOSE_DEBUG
                    OutputDebugStringA( "Merging object (XamlPath): WT_Outline_Ellipse \n" );
#endif
                    bool bFilledIn = false;
					std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iStart;
					std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iEnd;
					
					if (rFile.object_list().find_by_id(WT_Object::Outline_Ellipse_ID, iStart, iEnd))
					{
						for(; iStart != iEnd; iStart++)
						{
							// Merge the object data
							if (!iStart->second->materialized())
							{
								WT_Outline_Ellipse* pEllipse = static_cast< WT_Outline_Ellipse* >( iStart->second );

								//Fill in ellipse
								const XamlArcSegment *pSegment = static_cast<const XamlArcSegment *>( pXamlObject );
								double fIntDegrees = pSegment->tilt_radian() * gfToDegrees * gfWhipDegrees;
								pEllipse->tilt() = (WT_Integer16) floor( 0.5 + fIntDegrees );
								pEllipse->major() = pSegment->major();
								pEllipse->minor() = pSegment->minor();
                                rFile.set_materialized( pEllipse );
								bFilledIn = true;
                                WT_Unsigned_Integer32 nEnd = pEllipse->end();
                                while (nEnd > 65535) nEnd -= 65536;
								if ( pEllipse->start() == nEnd )
								{
									bIgnoreNextArc = true;
								}
								break;
							}
						}
                    }

                    if (!bFilledIn)
                    {
                        WD_Assert( false );
                        return WT_Result::Internal_Error;
                    }

                    break;
                }
            case XamlGraphicsObject::PolylineSegment:
                {
#ifdef _VERBOSE_DEBUG
                    OutputDebugStringA( "Creating object (XamlPath): WT_Polyline \n" );
#endif
                    //Create Polyline
                    const XamlPolylineSegment *pSegment = static_cast<const XamlPolylineSegment *>( pXamlObject );

                    int nCount = pSegment->pointSet().count() + (pSegment->closed() ? 1 : 0);
                    WT_Logical_Point *pPoints = DWFCORE_ALLOC_MEMORY( WT_Logical_Point, nCount );
                    if (pPoints == NULL)
                    {
                        return WT_Result::Out_Of_Memory_Error;
                    }

                    int nIndex = 0;
                    for ( int i=0; i< pSegment->pointSet().count(); i++ )
                    {
                        pPoints[nIndex].m_x = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[i].m_x ); //round for fuzz or noise
                        pPoints[nIndex++].m_y = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[i].m_y ); //round for fuzz or noise
                    }
                    if (pSegment->closed())
                    {
                        // add first point if figure is closed
                        pPoints[nIndex].m_x = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[0].m_x ); //round for fuzz or noise
                        pPoints[nIndex++].m_y = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[0].m_y ); //round for fuzz or noise
                    }

                    WT_Polyline *pPolyline = oClassFactory.Create_Polyline( nCount, pPoints, WD_True );
                    if (pPolyline == NULL)
                    {
                        return WT_Result::Out_Of_Memory_Error;
                    }
                    DWFCORE_FREE_MEMORY( pPoints );

                    pPolyline->materialized() = WD_True;
                    WD_CHECK( insertObject( rFile, pPolyline, oClassFactory ) );
                    break;
                }
            }
        }
    }
    else if ( pPath->fill().brush() != NULL && pPath->stroke().brush() == NULL )
    {
        //Check for fill
        if(!rFile.rendition().fill().fill())
        {
            WT_Fill* pFill = oClassFactory.Create_Fill(WD_True);
            if (pFill == NULL)
                return WT_Result::Out_Of_Memory_Error;
            pFill->materialized() = WD_True;
            WD_CHECK( insertObject( rFile, pFill, oClassFactory ) );
        }

        // materialize the object parsed from W2X
        //Get the next Polygon or Contour_Set shell if present
        std::multimap<WT_Object::WT_Type, WT_Object*>::iterator iStart;
        std::multimap<WT_Object::WT_Type, WT_Object*>::iterator iEnd;
        WT_Object* pObject = NULL;
        XamlPathGeometry::XamlPathFigure::tGraphicsVector::const_iterator iter = pPath->data().geometry().figure().graphics().begin();
        if (rFile.object_list().find_by_type(WT_Object::Drawable, iStart, iEnd))
        {
            // loop to get all the unmaterialized drawable in object list
            for(; iStart != iEnd; iStart++)
            {
                if ( !iStart->second->materialized() )
                {
                    pObject = iStart->second;
                    switch (pObject->object_id())
                    {
                    case WT_Object::Contour_Set_ID:
                    {
                        WT_Contour_Set* pContourSet = static_cast< WT_Contour_Set* >( pObject );
                        int nContours = pContourSet->contours();
                        WT_Integer32 *pCounts = DWFCORE_ALLOC_MEMORY( WT_Integer32, nContours );
                        if (pCounts == NULL)
                        {
                            return WT_Result::Out_Of_Memory_Error;
                        }
                        WT_Integer32 nPointCount = pContourSet->total_points();
                        WT_Logical_Point *pPoints = DWFCORE_ALLOC_MEMORY( WT_Logical_Point, nPointCount );
                        if (pPoints == NULL)
                        {
                            return WT_Result::Out_Of_Memory_Error;
                        }
                        int nCount = 0,nIndex = 0;
                        // loop to get enough PolylineSegment to materialize the contour set
                        for(int iContour=0;iContour<nContours;iContour++,iter++)
                        {
                            XamlGraphicsObject *pXamlObject = (*iter);
                            // why here can be casted as XamlPolylineSegment?
                            const XamlPolylineSegment *pSegment = static_cast<const XamlPolylineSegment *>( pXamlObject );

                            nCount = pSegment->pointSet().count() + (pSegment->closed() ? 1 : 0);
                            pCounts[iContour] = nCount;
                            for ( int i=0; i< pSegment->pointSet().count(); i++ )
                            {
                                pPoints[nIndex].m_x = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[i].m_x ); //round for fuzz or noise
                                pPoints[nIndex++].m_y = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[i].m_y ); //round for fuzz or noise
                            }
                            if (pSegment->closed())
                            {
                                // add first point if figure is closed
                                pPoints[nIndex].m_x = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[0].m_x ); //round for fuzz or noise
                                pPoints[nIndex++].m_y = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[0].m_y ); //round for fuzz or noise
                            }
                        }
                        
#ifdef _VERBOSE_DEBUG
                        OutputDebugStringA( "Merging object (XamlPath): WT_Contour_Set \n" );
#endif
                        WD_Assert( nPointCount == nIndex  );
                        WD_CHECK( pContourSet->set(rFile.next_incarnation(),nContours, pCounts, nPointCount, pPoints, WD_True) );
                        pContourSet->materialized() = WD_True;                             
                        DWFCORE_FREE_MEMORY( pPoints );
                        if (pCounts != NULL)
                        {
                            DWFCORE_FREE_MEMORY( pCounts );
                        }
                        break;                       
                    }
                    case WT_Object::Polygon_ID:
                    {                            
                        XamlGraphicsObject *pXamlObject = (*iter);
                        iter++;
                        const XamlPolylineSegment *pSegment = static_cast<const XamlPolylineSegment *>( pXamlObject );

                        //
                        // if the polygon is degenerate, do not add a closing point
                        // otherwise it won't render in the viewer
                        //
                        int nCount = pSegment->pointSet().count();
                        if(nCount > 2)
                        {
                            nCount += (pSegment->closed() ? 1 : 0);
                        }
                        WT_Logical_Point *pPoints = DWFCORE_ALLOC_MEMORY( WT_Logical_Point, nCount );
                        if (pPoints == NULL)
                        {
                            return WT_Result::Out_Of_Memory_Error;
                        }

                        int nIndex = 0;
                        for ( int i=0; i< pSegment->pointSet().count(); i++ )
                        {
                            pPoints[nIndex].m_x = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[i].m_x ); //round for fuzz or noise
                            pPoints[nIndex++].m_y = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[i].m_y ); //round for fuzz or noise
                        }
                        if (nCount > 2 && pSegment->closed())
                        {
                            // add first point if figure is closed
                            pPoints[nIndex].m_x = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[0].m_x ); //round for fuzz or noise
                            pPoints[nIndex++].m_y = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[0].m_y ); //round for fuzz or noise
                        }

#ifdef _VERBOSE_DEBUG
                        OutputDebugStringA( "Merging object (XamlPath): WT_Polygon \n" );                
#endif
                        WT_Polygon* pPolygon = static_cast< WT_Polygon* >( pObject );
                        pPolygon->set(nIndex, pPoints, WD_True);
                        pPolygon->materialized() = WD_True;                                                 
                        DWFCORE_FREE_MEMORY( pPoints ); 

                        break;
                    } 
                    case WT_Object::Filled_Ellipse_ID:
                    {
                        XamlGraphicsObject *pXamlObject = (*iter);
                        // find a ArcSegment in the graphic vector
                        while (pXamlObject->type() != XamlGraphicsObject::ArcSegment) 
                        {
                            if ( iter != pPath->data().geometry().figure().graphics().end() )
                            {
                                iter++;
                            }
                            else
                                return WT_Result::Waiting_For_Data;
                            pXamlObject = (*iter);
                        }
                        if ( iter != pPath->data().geometry().figure().graphics().end() )
                        {
                            iter++;
                        }
                        WT_Filled_Ellipse* pEllipse = static_cast< WT_Filled_Ellipse* >( iStart->second );

                        //Fill in ellipse
                        const XamlArcSegment *pSegment = static_cast<const XamlArcSegment *>( pXamlObject );

                        double fIntDegrees = pSegment->tilt_radian() * gfToDegrees * gfWhipDegrees;
                        pEllipse->tilt() = (WT_Integer16) floor( 0.5 + fIntDegrees );
                        pEllipse->major() = pSegment->major();
                        pEllipse->minor() = pSegment->minor();
                        rFile.set_materialized( pEllipse );
                        // for closed ellipse, we should ignore the second arc
                        // check the serialize() of ArcSegment
                        WT_Unsigned_Integer32 nEnd = pEllipse->end();
                        while (nEnd > 65535) nEnd -= 65536;
						if ( pEllipse->start() == nEnd )
						{
                            while ( iter != pPath->data().geometry().figure().graphics().end() )
                            {
                                // the may be an line before the arc, skip it
                                if ( pXamlObject->type() != XamlGraphicsObject::ArcSegment )
                                    iter++;
                                else
                                {
                                    iter++;
                                    break;
                                }
                            }
						}
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        }
        else
        {
            for(; iter != pPath->data().geometry().figure().graphics().end(); iter++)
            {
                XamlGraphicsObject *pXamlObject = (*iter);                    
                const XamlPolylineSegment *pSegment = static_cast<const XamlPolylineSegment *>( pXamlObject );

                int nCount = pSegment->pointSet().count() + (pSegment->closed() ? 1 : 0);
                WT_Logical_Point *pPoints = DWFCORE_ALLOC_MEMORY( WT_Logical_Point, nCount );
                if (pPoints == NULL)
                {
                    return WT_Result::Out_Of_Memory_Error;
                }

                int nIndex = 0;
                for ( int i=0; i< pSegment->pointSet().count(); i++ )
                {
                    pPoints[nIndex].m_x = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[i].m_x ); //round for fuzz or noise
                    pPoints[nIndex++].m_y = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[i].m_y ); //round for fuzz or noise
                }
                if (pSegment->closed())
                {
                    // add first point if figure is closed
                    pPoints[nIndex].m_x = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[0].m_x ); //round for fuzz or noise
                    pPoints[nIndex++].m_y = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[0].m_y ); //round for fuzz or noise
                }
                if(nIndex > 2)
                {
                    #ifdef _VERBOSE_DEBUG
                        OutputDebugStringA( "Creating object (XamlPath): WT_Polytriangle \n" );
                    #endif
                    WT_Logical_Point* pDestPoints = NULL;
                    //If this wasn't just a simple triangle
                    if (nIndex>4 || nIndex==4 && pPoints[0] != pPoints[3])
                    {
                        //Points were reordered for Path to draw polygon
                        //so reordering them back to what they were
                        int i = nIndex-1,j=0,k=0;
                        pDestPoints = DWFCORE_ALLOC_MEMORY(WT_Logical_Point,nIndex);
                        if(pDestPoints == NULL)
                        {
                            return WT_Result::Out_Of_Memory_Error;
                        }
                        for(k=0;j<nIndex/2;i--,j++,k+=2)
                        {
                            pDestPoints[k] = pPoints[i];
                            pDestPoints[k+1]= pPoints[j];
                        }
                        if (nIndex%2==1) //odd count, means we have one additional point to deal with
                        {
                            pDestPoints[k++] = pPoints[j];
                        } 
                    }

                    //Create a Polytriangle
                    WT_Polytriangle *pPolytriangle = oClassFactory.Create_Polytriangle( nIndex, pDestPoints!=NULL ? pDestPoints : pPoints, WD_True );
                    if (pPolytriangle == NULL)
                    {
                        return WT_Result::Out_Of_Memory_Error;
                    }
                    pPolytriangle->materialized() = WD_True;
                    WD_CHECK( insertObject( rFile, pPolytriangle, oClassFactory ) );
                    if (pDestPoints != NULL)
                    {
                        DWFCORE_FREE_MEMORY(pDestPoints);
                    }
                    DWFCORE_FREE_MEMORY( pPoints ); 
                }
                else //This is 2 point filled path...should be Polyline then 
                {
                    WT_Polyline *pPolyline = oClassFactory.Create_Polyline( nIndex, pPoints, WD_True );
                    if (pPolyline == NULL)
                    {
                        return WT_Result::Out_Of_Memory_Error;
                    }                     

                    pPolyline->materialized() = WD_True;
                    WD_CHECK( insertObject( rFile, pPolyline, oClassFactory ) );
                }
                DWFCORE_FREE_MEMORY( pPoints ); 
            }
        }
    } //filled

    return WT_Result::Success;
}

WT_Result
XamlObjectFactory::_processCanvasAttributes( WT_XAML_File& rFile, XamlCanvas* pCanvas )
{
    WT_XAML_Class_Factory oClassFactory;

    //Fill in current rendition attributes

    if ( rFile.nested_object_stack().size() > 0 )
    {
        //for any special object which may have
		//a canvas attribute we want to respect,
		//deal with that here.
    }

    //Does this canvas belong to a viewport?
    bool bNonMaterializedViewport = false;

	std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iStart;
	std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iEnd;

	if (rFile.object_list().find_by_id(WT_Object::Viewport_ID, iStart, iEnd))
	{
		for(; iStart != iEnd; iStart++)
		{
			if (!iStart->second->materialized())
			{
				bNonMaterializedViewport = true;
				break;
			}
			else if (iStart->second->object_type() != WT_Object::Attribute)
			{
				break;
			}
		}
	}

    //Si!  Esta Viewport!  Vamanos!
    if (bNonMaterializedViewport)
    {
        //Does the viewport have a bound?
        if (pCanvas->clip().geometry().figure().graphics().size() != 0)
        {
#ifdef _VERBOSE_DEBUG
            OutputDebugStringA( "Merging object (XamlCanvas): WT_Viewport \n" );
#endif
            WT_Viewport *pViewport = oClassFactory.Create_Viewport();
            if (pViewport == NULL)
                return WT_Result::Out_Of_Memory_Error;
    
            static_cast<Clip::Consumer*>( static_cast<WT_XAML_Viewport*>( pViewport ) )->consumeClip( pCanvas );
            pViewport->materialized() = WD_True;
            WD_CHECK( insertObject( rFile, pViewport, oClassFactory ) );
        }
    }

    return WT_Result::Success;
}

WT_Result
XamlObjectFactory::_processCanvasObjects( WT_XAML_File& rFile, XamlCanvas* pCanvas )
{
    if ( pCanvas != rFile.xamlParser()->viewportCanvas() )
    {
        if ( rFile.nested_object_stack().size() > 0 )
        {
            //for any special objects from which we can generate
			//objects, deal with those here
        }
    }

    return WT_Result::Success;
}

WT_Result
XamlObjectFactory::_processGlyphsAttributes( WT_XAML_File& rFile, XamlGlyphs* pGlyphs )
{
    WT_XAML_Class_Factory oClassFactory;

    //Fill in current rendition attributes

    //Set the default list of attributes that can be taken from the Glyphs
    WT_Integer32 parts_to_sync =    WT_Rendition::Color_Bit           |
        WT_Rendition::Visibility_Bit;

    if ( rFile.nested_object_stack().size() > 0 )
    {
        WT_Object *pObject = rFile.nested_object_stack().top();
        if (pObject->object_id() == WT_Object::Text_ID)
        {
            WT_XAML_Text* pText = static_cast< WT_XAML_Text* >( pObject );
            if ( (rFile.rendition().text_background().background() == WT_Text_Background::Ghosted)
                && (rFile.rendition().text_background().offset() > 0) )
            {
                if (pText->ghostPosition() == WT_XAML_Text::Undefined)
                {
                    pText->ghostPosition() = WT_XAML_Text::Left;
                    return WT_Result::Success; //skip this glyph
                }
                else if (pText->ghostPosition() == WT_XAML_Text::Left)
                {
                    pText->ghostPosition() = WT_XAML_Text::Top;
                    return WT_Result::Success; //skip this glyph
                }
                else if (pText->ghostPosition() == WT_XAML_Text::Top)
                {
                    pText->ghostPosition() = WT_XAML_Text::Right;
                    return WT_Result::Success; //skip this glyph
                }
                else if (pText->ghostPosition() == WT_XAML_Text::Right)
                {
                    pText->ghostPosition() = WT_XAML_Text::Bottom;
                    return WT_Result::Success; //skip this glyph
                }
                else if (pText->ghostPosition() == WT_XAML_Text::Bottom)
                {
                    pText->ghostPosition() = WT_XAML_Text::None;
                    //Process this one
                }
            }
        }

        //deal with other special objects here
    }

    if ( (parts_to_sync & WT_Rendition::Color_Bit) && (pGlyphs->fill().brush() != NULL) )
    {
        if (pGlyphs->fill().brush()->type() == XamlBrush::kSolidColor &&
            (static_cast< const XamlBrush::SolidColor* > ( 
            static_cast< const XamlBrush::Brush* > ( 
            pGlyphs->fill().brush() ) )->color() != rFile.rendition().color().rgba() ) )
        {
#ifdef _VERBOSE_DEBUG
            OutputDebugStringA( "Creating object (XamlGlyphs): WT_Color (Fill)\n" );
#endif
            WT_RGBA32 rgba = static_cast< const XamlBrush::SolidColor* > ( static_cast< const XamlBrush::Brush* > ( pGlyphs->fill().brush() ) )->color();

            WT_Color *pColor = _createOptimizedColor( rFile, rgba );

            if (pColor == NULL)
                return WT_Result::Out_Of_Memory_Error;

            pColor->materialized() = WD_True;
            WD_CHECK( insertObject( rFile, pColor, oClassFactory ) );
        }
    }

    if ( (parts_to_sync & WT_Rendition::Visibility_Bit) && ((pGlyphs->opacity().value() == 0.f) != (rFile.rendition().visibility().visible() == WD_False)) )
    {
#ifdef _VERBOSE_DEBUG
            OutputDebugStringA( "Creating object (XamlGlyphs): WT_Visibility\n" );
#endif
        WT_Boolean bVisible = (pGlyphs->opacity().value() == 0.f) ? WD_False : WD_True;
        WT_Visibility *pVisibility = oClassFactory.Create_Visibility( bVisible );
        if (pVisibility == NULL)
            return WT_Result::Out_Of_Memory_Error;

        pVisibility->materialized() = WD_True;
        WD_CHECK( insertObject( rFile, pVisibility, oClassFactory ) );
    }

    return WT_Result::Success;
}

WT_Result
XamlObjectFactory::_processGlyphsObjects( WT_XAML_File& rFile, XamlGlyphs* pGlyphs )
{
    WT_XAML_Text* pText = NULL;

    if ( rFile.nested_object_stack().size() > 0 )
    {
        WT_Object *pObject = rFile.nested_object_stack().top();
        if ( pObject->object_id() == WT_Object::Text_ID)
        {
            if ( !pObject->materialized() )
            {
#ifdef _VERBOSE_DEBUG
            OutputDebugStringA( "Merging object (XamlPath): WT_Text \n" );
#endif

                pText = static_cast< WT_XAML_Text* >( pObject );
            }
            else
            {
                //Ignore subsequent glyphs when we already have a materialized
                //nested text object (the others are the ghosties).
                return WT_Result::Success;
            }
        }
    }

    if (pText == NULL )
    {
		std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iStart;
		std::multimap<WT_Object::WT_ID, WT_Object*>::iterator iEnd;

		if (rFile.object_list().find_by_id(WT_Object::Text_ID, iStart, iEnd))
		{
			for(; iStart != iEnd; iStart++)
			{
				// Merge the object data
				if (!iStart->second->materialized())
				{
					pText = static_cast< WT_XAML_Text* >( iStart->second );
					break;
				}
			}
		}
    }

    if (pText == NULL)
    {
        WD_Assert( false );
        return WT_Result::Internal_Error;
    }

    //Fill in the text object
    DWFString zString = pGlyphs->unicodeString().string();
    size_t nLen = zString.chars();
    WD_CHECK( pText->string().set( (int)nLen, zString ) );
    rFile.set_materialized( pText );

    return WT_Result::Success;
}
