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
// from XamlDrawableAttributes::PathAttributeProvider
//
WT_Result 
WT_XAML_Dash_Pattern::providePathAttributes(
    XamlDrawableAttributes::PathAttributeConsumer *pConsumer,
    WT_XAML_File& rFile ) const
{
    if(!pConsumer)
    {
        return WT_Result::Internal_Error;
    }

    const_cast<WT_XAML_Dash_Pattern*>(this)->_pSerializeFile = &rFile;
    return pConsumer->consumeStrokeDashArray( const_cast<WT_XAML_Dash_Pattern*>(this) );
}

//
// from XamlDrawableAttributes::Fill::Provider
//
WT_Result 
WT_XAML_Dash_Pattern::provideStrokeDashArray(
    XamlDrawableAttributes::StrokeDashArray *& rpArray)
{
    if (_pSerializeFile == NULL)
    {
        return WT_Result::Internal_Error;
    }

    //
    // - if the fill is null (what case would it be ?), alloc it
    // - alloc our XAML translator and set it in the fill
    //
    if(!rpArray)
    {
        rpArray = DWFCORE_ALLOC_OBJECT(XamlDrawableAttributes::StrokeDashArray);
        if(!rpArray)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
    }

    //
    // - kNull's id (-1, from defaulting ctor) turns off dashing
    // - the # of gap/dash pairs must be even
    // - the id must be greater or equal than WT_Line_Pattern::Count
    //
    if(number() != -1)
    {
        WT_Integer16 i;
        WT_Integer16 const *pData = pattern();
        
        WD_Assert(!(length() % 2));
        WD_Assert(number() >= WT_Line_Pattern::Count);

        for(i = 0; i < length() / 2; i++)
        {
            rpArray->dashGap().push_back(
                XamlDrawableAttributes::StrokeDashArray::tDashGapPair(
                    (float)pData[ 2 * i ], (float)pData[ 1 + 2 * i])
            );
        }
    }
    return WT_Result::Success;
}

WT_Result 
WT_XAML_Dash_Pattern::consumeStrokeDashArray( XamlDrawableAttributes::StrokeDashArray::Provider *pProvider )
{
    if (pProvider == NULL)
    {
        return WT_Result::Toolkit_Usage_Error;
    }

    XamlDrawableAttributes::StrokeDashArray oStrokeDashArray, *p = &oStrokeDashArray;
    WD_CHECK( pProvider->provideStrokeDashArray( p ) );

    size_t nCount = oStrokeDashArray.dashGap().size();
    WT_Integer16 *pArray = DWFCORE_ALLOC_MEMORY( WT_Integer16, nCount*2 );
    if (pArray == NULL)
    {
        return WT_Result::Out_Of_Memory_Error;
    }

    for (size_t i=0; i< nCount; i++ )
    {
        pArray[i*2]     = (WT_Integer16)floor( 0.5 + oStrokeDashArray.dashGap()[i].first );
        pArray[(i*2)+1] = (WT_Integer16)floor( 0.5 + oStrokeDashArray.dashGap()[i].second );
    }

    set_pattern( (WT_Integer16)nCount*2, pArray );
    DWFCORE_FREE_MEMORY( pArray );
    
    return WT_Result::Success;
}

WT_Result 
WT_XAML_Dash_Pattern::serialize(
    WT_File &file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Dash_Pattern::serialize( *rFile.w2dContentFile() );
    }

    WD_CHECK( rFile.dump_delayed_drawable() );

	//Serialize RenditionSync (top element) if not yet done
	WD_CHECK( rFile.serializeRenditionSyncStartElement() );

	//now write whipx hint to w2xserializer
	DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
	if (!pW2XSerializer)
		return WT_Result::Internal_Error;

    pW2XSerializer->startElement(XamlXML::kpzDash_Pattern_Element);
    pW2XSerializer->addAttribute(XamlXML::kpzValue_Attribute,(int) number() );
	pW2XSerializer->endElement();

    return WT_Result::Success;
}

WT_Result 
WT_XAML_Dash_Pattern::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& /*rFile*/)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

	const char** pDashPat = rMap.find(XamlXML::kpzValue_Attribute);
	if(pDashPat != NULL && *pDashPat != NULL)
	{
		number() = (WT_Integer32)atoi(*pDashPat);
	}
    else
    {
        return WT_Result::Corrupt_File_Error;
    }

	return WT_Result::Success;

}
