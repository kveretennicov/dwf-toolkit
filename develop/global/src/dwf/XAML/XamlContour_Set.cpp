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

WT_Result WT_XAML_Contour_Set::serialize(
    WT_File &file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Contour_Set::serialize( *rFile.w2dContentFile() );
    }

    WT_Integer32 i, count, total;

    //Contour sets should not be delayed/merged
    WT_Boolean bMerge = rFile.heuristics().allow_drawable_merging();
    rFile.heuristics().set_allow_drawable_merging( WD_False );

    WD_CHECK( rFile.dump_delayed_drawable() );

    //XAML serialization here
    XamlRenditionPath* pPath = DWFCORE_ALLOC_OBJECT( XamlRenditionPath );

    if (pPath == NULL)
    {
        return WT_Result::Out_Of_Memory_Error;
    }

    if (file.heuristics().apply_transform())
    {
        const_cast<WT_XAML_Contour_Set*>(this)->transform(file.heuristics().transform());
    }

    if (!file.desired_rendition().fill().fill() || !file.rendition().fill().fill() )
    {
        file.desired_rendition().fill() = WD_True;
    }

    for(i = count = total = 0; i < contours(); i++, total += count)
    {
        count = counts()[ i ];

        WT_XAML_Point_Set_Data local( count, points() + total );

        //
        // layout : y-mirror by hand all the vertices to make sure
        // the final rendering is not reversed
        //
        // use a temporary point_set to avoid modifying the original
        // coordinates (this must be a local to the serialization)
        //
        WT_Point2D *pVtx = local.points();
        for(int k = 0; k < local.count(); k++)
        {
            WD_CHECK( rFile.flipPoint(pVtx[ k ]) );
        }

        XamlPolylineSegment *pSegment = DWFCORE_ALLOC_OBJECT( XamlPolylineSegment(local) );

        if(pSegment != NULL)
        {
            pPath->addDrawable(pSegment);
        }
    }

    WD_CHECK(pPath->serialize(rFile)); //calls rFile.serializeRenditionSyncEndElement 
    DWFCORE_FREE_OBJECT( pPath );

    //W2X serialization here
    DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if(!pW2XSerializer)
    {
		return WT_Result::Internal_Error;
    }

    rFile.nameIndex()--;  //Path serialization bumps the name index, so we restore it to what it was ...
    DWFString zNameString = rFile.nameIndexString();
    rFile.nameIndex()++;  //... and then re-bump the index

    pW2XSerializer->startElement(XamlXML::kpzContour_Set_Element);
    pW2XSerializer->addAttribute(
        XamlXML::kpzRefName_Attribute,
        zNameString);
    
    pW2XSerializer->addAttribute(
        XamlXML::kpzContours_Attribute,
        (int)contours());
    
    pW2XSerializer->addAttribute(
        XamlXML::kpzCount_Attribute,
        (int)total_points());

    pW2XSerializer->endElement();


    rFile.heuristics().set_allow_drawable_merging( bMerge );

	return WT_Result::Success;
}
WT_Result
WT_XAML_Contour_Set::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

	const char** ppValue = rMap.find( XamlXML::kpzContours_Attribute );
    if (ppValue==NULL || *ppValue==NULL)
        return WT_Result::Corrupt_File_Error;

    contours() = (WT_Integer32) atoi( *ppValue );

    ppValue = rMap.find( XamlXML::kpzCount_Attribute );
    if (ppValue==NULL || *ppValue==NULL)
        return WT_Result::Corrupt_File_Error;

    total_points() = (WT_Integer32) atoi( *ppValue );

    return WT_Result::Success;
}
