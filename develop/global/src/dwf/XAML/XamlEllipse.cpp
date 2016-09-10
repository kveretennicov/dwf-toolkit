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


//#if DESIRED_CODE(WHIP_OUTPUT)
WT_Result WT_XAML_Outline_Ellipse::serialize(WT_File & file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Outline_Ellipse::serialize( *rFile.w2dContentFile() );
    }

    WT_Result res = WT_Result::Success;
	if (file.rendition().fill().fill() || file.desired_rendition().fill().fill())
    {
        file.desired_rendition().fill() = WD_False;
    }

    if (file.heuristics().apply_transform())
    {
        const_cast<WT_XAML_Outline_Ellipse*>(this)->transform(file.heuristics().transform());
    }

    XamlArcSegment* pXamlArcSegment = DWFCORE_ALLOC_OBJECT( XamlArcSegment(*this) );
    XamlRenditionPath* pPath = DWFCORE_ALLOC_OBJECT( XamlRenditionPath() );
    
    pPath->addDrawable(pXamlArcSegment); //XamlPathGeometry owns the segment
    res = pPath->serialize(file);
    DWFCORE_FREE_OBJECT( pPath );

	//serialize w2x stuff
	//End RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncEndElement() ); 
    //Serialize a W2X entry indicating that we have x segments...
    rFile.w2xSerializer()->startElement( XamlXML::kpzOutline_Ellipse_Element );
    //Tie the W2X entry to the current Path name
	rFile.w2xSerializer()->addAttribute( XamlXML::kpzRefName_Attribute, rFile.nameIndexString());
	rFile.w2xSerializer()->addAttribute( XamlXML::kpzCenterX_Attribute,  (int)position().m_x);
	rFile.w2xSerializer()->addAttribute( XamlXML::kpzCenterY_Attribute,  (int)position().m_y);
    rFile.w2xSerializer()->addAttribute( XamlXML::kpzStart_Attribute,   (int)start() );
    rFile.w2xSerializer()->addAttribute( XamlXML::kpzEnd_Attribute,     (int)end() );
	rFile.w2xSerializer()->endElement();

    return res;
}
//#else
//WT_Result WT_XAML_Outline_Ellipse::serialize(WT_File & file) const
//{
//    return WT_Result::Success;
//}
//#endif  // DESIRED_CODE()


WT_Result
WT_XAML_Outline_Ellipse::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

	const char** ppValue = rMap.find( XamlXML::kpzCenterX_Attribute );
    if (ppValue==NULL || *ppValue==NULL)
        return WT_Result::Corrupt_File_Error;
    position().m_x = (WT_Integer32) atol( *ppValue );

	ppValue = rMap.find( XamlXML::kpzCenterY_Attribute );
    if (ppValue==NULL || *ppValue==NULL)
        return WT_Result::Corrupt_File_Error;
    position().m_y = (WT_Integer32) atol( *ppValue );

	ppValue = rMap.find( XamlXML::kpzStart_Attribute );
    if (ppValue==NULL || *ppValue==NULL)
        return WT_Result::Corrupt_File_Error;
    start() = (WT_Unsigned_Integer16) atol( *ppValue );

	ppValue = rMap.find( XamlXML::kpzEnd_Attribute );
    if (ppValue==NULL || *ppValue==NULL)
        return WT_Result::Corrupt_File_Error;
    end() = (WT_Unsigned_Integer32) atol( *ppValue );

	return WT_Result::Success;
}

WT_Result WT_XAML_Filled_Ellipse::serialize(WT_File & file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Filled_Ellipse::serialize( *rFile.w2dContentFile() );
    }

    WT_Result res = WT_Result::Success;

	if (!file.rendition().fill().fill() || !file.desired_rendition().fill().fill())
    {
        file.desired_rendition().fill() = WD_True;
    } 

    if (file.heuristics().apply_transform())
    {
        const_cast<WT_XAML_Filled_Ellipse*>(this)->transform(file.heuristics().transform());
    }

    XamlArcSegment* pXamlArcSegment = DWFCORE_ALLOC_OBJECT( XamlArcSegment(*this) );
    XamlRenditionPath* pPath = DWFCORE_ALLOC_OBJECT( XamlRenditionPath );
    
    pPath->addDrawable(pXamlArcSegment); //XamlPathGeometry owns the segment
    res = pPath->serialize(file);
    DWFCORE_FREE_OBJECT( pPath );

	//serialize w2x stuff
	//End RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncEndElement() ); 
	//Serialize a W2X entry indicating that we have x segments...
	rFile.w2xSerializer()->startElement( XamlXML::kpzFilled_Ellipse_Element );
	//Tie the W2X entry to the current Path name
	rFile.w2xSerializer()->addAttribute( XamlXML::kpzRefName_Attribute, rFile.nameIndexString());
	rFile.w2xSerializer()->addAttribute( XamlXML::kpzCenterX_Attribute,  (int)position().m_x);
	rFile.w2xSerializer()->addAttribute( XamlXML::kpzCenterY_Attribute,  (int)position().m_y);
    rFile.w2xSerializer()->addAttribute( XamlXML::kpzStart_Attribute,   (int)start() );
    rFile.w2xSerializer()->addAttribute( XamlXML::kpzEnd_Attribute,     (int)end() );
	rFile.w2xSerializer()->endElement();

	return res;
}

WT_Result
WT_XAML_Filled_Ellipse::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

	const char** ppValue = rMap.find( XamlXML::kpzCenterX_Attribute );
    if (ppValue==NULL || *ppValue==NULL)
        return WT_Result::Corrupt_File_Error;
    position().m_x = (WT_Integer32) atol( *ppValue );

	ppValue = rMap.find( XamlXML::kpzCenterY_Attribute );
    if (ppValue==NULL || *ppValue==NULL)
        return WT_Result::Corrupt_File_Error;
    position().m_y = (WT_Integer32) atol( *ppValue );

	ppValue = rMap.find( XamlXML::kpzStart_Attribute );
    if (ppValue==NULL || *ppValue==NULL)
        return WT_Result::Corrupt_File_Error;
    start() = (WT_Unsigned_Integer16) atol( *ppValue );

	ppValue = rMap.find( XamlXML::kpzEnd_Attribute );
    if (ppValue==NULL || *ppValue==NULL)
        return WT_Result::Corrupt_File_Error;
    end() = (WT_Unsigned_Integer32) atol( *ppValue );

	return WT_Result::Success;
}
