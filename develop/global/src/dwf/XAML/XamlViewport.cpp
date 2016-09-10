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
// from XamlCanvasAttributes::AttributeProvider
//
WT_Result WT_XAML_Viewport::provideCanvasAttributes(
    XamlDrawableAttributes::CanvasAttributeConsumer *pConsumer,
    WT_XAML_File &rFile) const
{
    if(!pConsumer)
    {
        return WT_Result::Internal_Error;
    }

    const_cast<WT_XAML_Viewport *>(this)->_pSerializeFile = &rFile;

    WD_CHECK(pConsumer->consumeClip(const_cast<WT_XAML_Viewport *>(this)));

    return WT_Result::Success;
}

//
// from XamlDrawableAttributes::Clip::Provider
//
WT_Result WT_XAML_Viewport::provideClip(
    XamlDrawableAttributes::Clip *& rpClip)
{
    if (_pSerializeFile == NULL)
    {
        return WT_Result::Internal_Error;
    }

    //
    // - if the clip is null (what case would it be ?), alloc it
    //
    if(!rpClip)
    {
        rpClip = DWFCORE_ALLOC_OBJECT(XamlDrawableAttributes::Clip);
        if(!rpClip)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }
    
    rpClip->geometry().figure().reset();

    const WT_Contour_Set *pSet = contour();
    if(!pSet)
    {
        return WT_Result::Success;
    }

    WT_Integer32 i, j, count, total;

    for(i = count = total = 0; i < pSet->contours(); i++, total += count)
    {
        count = pSet->counts()[ i ];

        WT_XAML_Point_Set_Data data(count, pSet->points() + total);

        //
        // layout : y-mirror by hand all the vertices to make sure
        // the final rendering is not reversed
        //
        // note : it looks like the clipping loop must be oriented -
        // since the y-mirror will flip its normal, inverse the loop
        // by swapping its vertices
        //
        const WT_Logical_Point *pVtx = pSet->points() + total;
        for(j = 0; j < count; j++)
        {
            WT_Point2D p( pVtx[j].m_x, pVtx[j].m_y );

            _pSerializeFile->flipPoint(p);

            data.points()[count - j - 1] = p;
        }

        XamlPolylineSegment *pSegment = DWFCORE_ALLOC_OBJECT( XamlPolylineSegment(data) );

        if(pSegment == NULL)
            return WT_Result::Out_Of_Memory_Error;

        rpClip->geometry().figure().addDrawable(pSegment);
    }

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Viewport::consumeClip( 
    XamlDrawableAttributes::Clip::Provider* pProvider)
{
    WT_XAML_Class_Factory oClassFactory;

    XamlDrawableAttributes::Clip oClip, *pClip = &oClip;
    WD_CHECK( pProvider->provideClip( pClip ) );

    WT_Integer32 nContours = (WT_Integer32)oClip.geometry().figure().graphics().size();
    WT_Integer32 *pCounts = DWFCORE_ALLOC_MEMORY( WT_Integer32, nContours );
    if (pCounts == NULL)
    {
        return WT_Result::Out_Of_Memory_Error;
    }

    //FUTURE: there is VERY similar code (to that below) in the XamlObjectFactory which creates contour sets / polylines and polygons
    //Pull that out in a utility method and use it instead of the below code.  Reduce the code, reduce the bugs.

    //preprocess to get the point count
    WT_Integer32 nPointCount = 0, nIndex = 0;
    XamlPathGeometry::XamlPathFigure::tGraphicsVector::const_iterator iter = oClip.geometry().figure().graphics().begin();
    for(; iter != oClip.geometry().figure().graphics().end(); iter++, nIndex++)
    {
        XamlGraphicsObject* pXamlGraphicsObject = *iter;
        if (pXamlGraphicsObject->type() == XamlGraphicsObject::ArcSegment)
        {
            WD_Assert( false ); //we cannot abide arcs in our viewport contours!
            return WT_Result::Corrupt_File_Error;
        }
        else
        {
            //it's a polyline segment, count the points
            const XamlPolylineSegment *pSegment = static_cast<const XamlPolylineSegment *>( pXamlGraphicsObject );
            pCounts[nIndex] = pSegment->pointSet().count() + (pSegment->closed() ? 1 : 0);
            nPointCount += pCounts[nIndex];
        }
    }

    //
    // note : points are already un-flipped (when materializing the geometry figure)
    //
    WT_Point_Set_Data oPointSet;
    nIndex = 0;
    WT_Logical_Point *pPoints = DWFCORE_ALLOC_MEMORY( WT_Logical_Point, nPointCount );
    if (pPoints == NULL)
    {
        return WT_Result::Out_Of_Memory_Error;
    }

    iter = oClip.geometry().figure().graphics().begin();
    for(; iter != oClip.geometry().figure().graphics().end(); iter++)
    {
        XamlGraphicsObject *pObject = (*iter);
        const XamlPolylineSegment *pSegment = static_cast<const XamlPolylineSegment *>( pObject );

        for ( int i = pSegment->pointSet().count(); i; i-- )
        {
            pPoints[nIndex].m_x = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[ i - 1 ].m_x ); //round for fuzz or noise
            pPoints[nIndex++].m_y = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[ i - 1].m_y ); //round for fuzz or noise
        }
        if (pSegment->closed())
        {
            // add first point if figure is closed
            pPoints[nIndex].m_x = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[0].m_x ); //round for fuzz or noise
            pPoints[nIndex++].m_y = (WT_Integer32)floor( 0.5 + pSegment->pointSet().points()[0].m_y ); //round for fuzz or noise
        }
    }

    //Create a Contour Set
    WT_Contour_Set *pContourSet = oClassFactory.Create_Contour_Set();
    if (pContourSet == NULL)
    {
        return WT_Result::Out_Of_Memory_Error;
    }

    //
    // don't forget to correctly set the incarnation #
    //
    pContourSet->set( _nContourIncarnation, nContours, pCounts, nPointCount, pPoints, WD_True );
    WD_CHECK( set( pContourSet ) );
    oClassFactory.Destroy( pContourSet );

    DWFCORE_FREE_MEMORY( pPoints );
    DWFCORE_FREE_MEMORY( pCounts );

    return WT_Result::Success;
}

WT_Result WT_XAML_Viewport::serialize(
    WT_File & file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Viewport::serialize( *rFile.w2dContentFile() );
    }

	WD_CHECK( rFile.dump_delayed_drawable() ); //dumps any delayed drawable first

    WD_CHECK(rFile.serializeRenditionSyncStartElement()); 
    //now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;

    rFile.w2xSerializer()->startElement(XamlXML::kpzViewport_Element);
    wchar_t *zBuf = WT_String::to_wchar( name().length(), name() );
    if (zBuf == NULL)
    {
        return WT_Result::Out_Of_Memory_Error;
    }
	rFile.w2xSerializer()->addAttribute( XamlXML::kpzName_Attribute, zBuf );
    if (contour()==NULL)
    {
        rFile.w2xSerializer()->addAttribute( XamlXML::kpzEnd_Attribute, true );
    }

    const WT_Viewport_Option_Viewport_Units &rUnits = viewport_units(); 

    if(m_fields_defined & VIEWPORT_UNITS_BIT) 
    { 
        pW2XSerializer->addAttribute( 
            XamlXML::kpzLabel_Attribute, 
            rUnits.units().ascii()); 

        wchar_t pBuf[ 1024 ]; 
        WT_Matrix rMatrix = rUnits.application_to_dwf_transform(); 

        if (file.heuristics().apply_transform()) 
        { 
            rMatrix *= file.heuristics().transform(); 
        } 

        if (file.heuristics().transform().rotation() && 
            file.heuristics().apply_transform_to_units_matrix()) 
        { 
            WT_Matrix temp; 

            rMatrix.rotate(temp, file.heuristics().transform().rotation()); 

            rMatrix = temp; 
        } 

        _DWFCORE_SWPRINTF(pBuf, 1024, L"%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls,%ls",
            (const wchar_t*)DWFString::DoubleToString(rMatrix(0,0),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(0,1),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(0,2),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(0,3),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(1,0),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(1,1),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(1,2),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(1,3),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(2,0),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(2,1),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(2,2),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(2,3),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(3,0),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(3,1),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(3,2),10), 
            (const wchar_t*)DWFString::DoubleToString(rMatrix(3,3),10) 
            ); 

        pW2XSerializer->addAttribute( 
            XamlXML::kpzTransform_Attribute, 
            pBuf); 
    } 

    rFile.w2xSerializer()->endElement();
    delete [] zBuf;

	//Only if contours are set/changed we need to start new Canvas element
	const WT_Contour_Set *pSet = contour();
    if(pSet)
	{
        if (rFile.heuristics().apply_transform())
        {
            const_cast<WT_Contour_Set*>(pSet)->transform(rFile.heuristics().transform());
        }

        //
        // Spago/DWFx driver fix : we do need to update the current rendition's viewport
        // with the possibly modified coordinates (if heuristic transform)
        // serialize() is called anyway during sync(), so we're not breaking anything.
        //
        rFile.rendition().viewport() = *this;

		//canvas serialization to xaml
		XamlCanvas* pCanvas = DWFCORE_ALLOC_OBJECT( XamlCanvas(false) );
		if(!pCanvas)
			return WT_Result::Out_Of_Memory_Error;
		WD_CHECK( pCanvas->serialize(rFile) );
		DWFCORE_FREE_OBJECT(pCanvas);
	}
    else
    {
        rFile.serializeCanvasEndElement( false );
    }

    return WT_Result::Success;
}

WT_Result
WT_XAML_Viewport::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& rFile)
{
    //
    // needed to sync the rendition upon materialization
    // the wtk opcode implementation checks that thing and will fail
    // systematically if not incremented
    // same for the contour set, except it gets created later on where
    // we don't have access to the xaml file - we therefore have to save
    // the new incarnation #
    //
    m_incarnation = rFile.next_incarnation();
    _nContourIncarnation = rFile.next_incarnation();

    if(!rMap.size())
        return WT_Result::Success;

    WT_Viewport_Option_Viewport_Units &rUnits = const_cast<WT_Viewport_Option_Viewport_Units &>(viewport_units()); 

    const char** ppXfo = rMap.find(XamlXML::kpzTransform_Attribute); 
    if(ppXfo != NULL && *ppXfo != NULL) 
    { 
        WT_Matrix rMatrix; 

        int nProcessed = sscanf( *ppXfo, 
            "%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG,%lG", 
            &rMatrix(0,0), 
            &rMatrix(0,1), 
            &rMatrix(0,2), 
            &rMatrix(0,3), 
            &rMatrix(1,0), 
            &rMatrix(1,1), 
            &rMatrix(1,2), 
            &rMatrix(1,3), 
            &rMatrix(2,0), 
            &rMatrix(2,1), 
            &rMatrix(2,2), 
            &rMatrix(2,3), 
            &rMatrix(3,0), 
            &rMatrix(3,1), 
            &rMatrix(3,2), 
            &rMatrix(3,3)); 

            if (nProcessed == 16) 
            { 
                rUnits.set_application_to_dwf_transform(rMatrix); 
                 
                const char** ppLabel = rMap.find(XamlXML::kpzLabel_Attribute); 
                if(ppLabel != NULL && *ppLabel != NULL) 
                { 
                    rUnits.set_units(*ppLabel);  
                }        

                m_fields_defined |= VIEWPORT_UNITS_BIT; 
            } 
    } 

    const char** ppValue = rMap.find(XamlXML::kpzName_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
        set(WT_String(*ppValue));
    }

    ppValue = rMap.find(XamlXML::kpzEnd_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
        //This is indicates a viewport that has no boundary information,
        //so there is no Canvas wrapper containing a clip.
        materialized() = WD_True;
    }
    
	return WT_Result::Success;
}
