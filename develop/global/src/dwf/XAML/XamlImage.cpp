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
#include "dwfcore/MIME.h"

WT_Result WT_XAML_Image::serialize(
    WT_File &file) const
{
    WT_XAML_File& rFile = static_cast<WT_XAML_File &>(file);
    if (rFile.serializingAsW2DContent() )
    {
        if ( !rFile.w2dContentFile() )
            return WT_Result::Toolkit_Usage_Error;
        else
            return WT_Image::serialize( *rFile.w2dContentFile() );
    }

    if (rFile.heuristics().apply_transform())
    {
        const_cast<WT_XAML_Image*>(this)->transform(rFile.heuristics().transform());
    }

    WD_CHECK(rFile.serializeRenditionSyncEndElement()); 

    DWFXMLSerializer* pW2XSerializer = rFile.w2xSerializer();
    if(!pW2XSerializer)
    {
	    return WT_Result::Internal_Error;
    }

    WT_Integer32 nParts =   WT_Rendition::Visibility_Bit      |
                            WT_Rendition::URL_Bit             |
                            WT_Rendition::Viewport_Bit        |
                            WT_Rendition::Layer_Bit           |
                            WT_Rendition::Object_Node_Bit;
   
    DWFString zType;
    switch(format())
    {
        case Indexed :
        {
            nParts |= WT_Rendition::Color_Map_Bit;
        }
        case Bitonal_Mapped :
        case Group3X_Mapped :
        case Mapped :
        {
            //
            // only JPEG is supported as image type on the
            // rendering side - for all other formats, we'll
            // have to request the OPC serializer to convert
            // the input bytes and give us back a TIFF, JPEG
            // or BMP
            //
            // if implemented we need to duplicate the logic from
            // WT_Image.cpp
            //
            return WT_Result::Toolkit_Usage_Error;
        }
        break;
        case RGB :
        case RGBA :
        {
            //
            // only JPEG is supported as image type on the
            // rendering side - for all other formats, we'll
            // have to request the OPC serializer to convert
            // the input bytes and give us back a TIFF, JPEG
            // or BMP
            //
            // also, if a colormap is specified, we'll need to
            // ask the OPC serializer to craft and store an ICC
            // color profile
            //
            WD_Assert(! m_color_map );
            return WT_Result::Toolkit_Usage_Error;
        }
        break;
        case JPEG :
        {
            WD_Assert(! m_color_map );
            zType = DWFMIME::kzMIMEType_JPG;
        }
        break;
        default :
        {
            return WT_Result::Internal_Error;
        }
    }

    DWFString oPath;
    DWFOutputStream *pOut;
    WT_OpcResourceSerializer *&prOpc = rFile.opcResourceSerializer();
    if(prOpc == NULL)
    {
        return WT_Result::Internal_Error;
    }

    pOut = NULL;

    WD_CHECK( prOpc->getPartOutputStream(zType, oPath, &pOut) );
    if(pOut == NULL)
    {
        return WT_Result::Internal_Error;
    }

    pOut->write(data(), data_size());
    pOut->flush();
    DWFCORE_FREE_OBJECT( pOut );
  
   
    WT_Point2D pVtx[ 4 ];
    pVtx[ 0 ].m_x = min_corner().m_x;  pVtx[ 0 ].m_y = min_corner().m_y;
    pVtx[ 1 ].m_x = max_corner().m_x;  pVtx[ 1 ].m_y = min_corner().m_y;
    pVtx[ 2 ].m_x = max_corner().m_x;  pVtx[ 2 ].m_y = max_corner().m_y;
    pVtx[ 3 ].m_x = min_corner().m_x;  pVtx[ 3 ].m_y = max_corner().m_y;

    //
    // layout : y-mirror by hand all the vertices to make sure
    // the final rendering is not reversed
    //
    for(int i = 0; i < 4; i++)
    {
        rFile.flipPoint(pVtx[ i ]);
    }

    WT_XAML_Point_Set_Data oFrame(4, pVtx, false);   

    //
    // we don't need line weights, strokes and other rendition attributes
    // for images - therefore let's use XamlPath.
    //
    // on top of that, there is a bug in the XPS rendering software which
    // hangs if a stroke dash array is specified on a path whose fill
    // is an imagebrush.
    //
    XamlPath *pPath = DWFCORE_ALLOC_OBJECT( XamlImageRenditionPath );
    if(pPath == NULL)
    {
        return WT_Result::Out_Of_Memory_Error;
    }

    XamlPolylineSegment* pPoly = DWFCORE_ALLOC_OBJECT( XamlPolylineSegment(oFrame) );
    if(pPoly == NULL)
    {
        return WT_Result::Out_Of_Memory_Error;
    }

    pPath->addDrawable(pPoly);

    WT_Matrix2D oXfo;

    //
    // note : beware, the y coordinates must be reversed to get a
    // positive scaling factor
    //
    oXfo(0,0) = (double) (pVtx[ 2 ].m_x - pVtx[ 0 ].m_x) / columns();
    oXfo(1,1) = (double) (pVtx[ 0 ].m_y - pVtx[ 2 ].m_y) / rows();
    oXfo(2,0) = pVtx[ 0 ].m_x;
    oXfo(2,1) = pVtx[ 0 ].m_y;

    XamlBrush::Image *pImage = DWFCORE_ALLOC_OBJECT(
        XamlBrush::XamlImageBrush(oPath, oXfo, dpi(),columns(), rows())
        );

    XamlDrawableAttributes::FillProvider oFillProvider(pImage);

    WT_XAML_Rendition& rXamlRendition = static_cast<WT_XAML_Rendition &>(rFile.rendition());

    WD_CHECK( rFile.dump_delayed_drawable() );
    
    WD_CHECK( rFile.desired_rendition().sync(rFile, nParts) );

    WD_CHECK( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath)->consumeFill(&oFillProvider) );

    WD_CHECK( static_cast<WT_XAML_Visibility&>( rXamlRendition.visibility() ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );
    WD_CHECK( static_cast<WT_XAML_URL&>( rXamlRendition.url() ).providePathAttributes( ((XamlDrawableAttributes::PathAttributeConsumer*)pPath), rFile ) );

    WD_CHECK( pPath->serialize(rFile) );

    //Now is the right time to serialize w2x stuff
    rFile.serializeRenditionSyncEndElement();
    pW2XSerializer->startElement(XamlXML::kpzImage_Element);

    pW2XSerializer->addAttribute(
        XamlXML::kpzRefName_Attribute,
        rFile.nameIndexString());

    pW2XSerializer->addAttribute(
        XamlXML::kpzFormat_Attribute,
        format());

    pW2XSerializer->addAttribute(
        XamlXML::kpzRef_Attribute,
        oPath);

    pW2XSerializer->addAttribute(
        XamlXML::kpzWidth_Attribute,
        columns());

    pW2XSerializer->addAttribute(
        XamlXML::kpzHeight_Attribute,
        rows());

    if(dpi() != -1)
        pW2XSerializer->addAttribute(
        XamlXML::kpzDpi_Attribute,
        (int)dpi());

    wchar_t pBuf[ 128 ];

    _DWFCORE_SWPRINTF(pBuf, 128,L"%d,%d,%d,%d", 
        min_corner().m_x,
        min_corner().m_y,
        max_corner().m_x,
        max_corner().m_y);

    pW2XSerializer->addAttribute(
        XamlXML::kpzArea_Attribute,
        pBuf);
   
    
    const WT_Color_Map *pCMap = color_map();

    //
    // note : in order to do this the calling code (exporter for instance)
    // needs to a) instantiate a WT_XAML_Color_Map() from the original
    // WT_Color_Map, b) feed it to the WT_XAML_Image ctor with
    // the copy parameter set to false.
    // if copy is true then WT_Image will create an internal WT_Color_Map
    // and drop the WT_XAML_PNG_Group4_Image one.
    //
    if( (format() == WT_Image::Mapped) ||
        (format() == WT_Image::Group3X_Mapped) || 
        (format() == WT_Image::Bitonal_Mapped) )
    {
        if ( pCMap != NULL )
        {
            const WT_XAML_Color_Map *pCasted = static_cast<const WT_XAML_Color_Map *>(pCMap);
            pCasted->serializeRawColorMap(rFile);
        }
        else
        {
            WD_Assert( WD_False );
            return WT_Result::Toolkit_Usage_Error;
        }
    }

    pW2XSerializer->endElement();

    WD_CHECK( rFile.dump_delayed_drawable() );
    DWFCORE_FREE_OBJECT(pPath);   

    return WT_Result::Success;
}

WT_Result
WT_XAML_Image::parseAttributeList(XamlXML::tAttributeMap& rMap, WT_XAML_File& rFile)
{
	if(!rMap.size())
		return WT_Result::Internal_Error;

	//<Image refName="N37" Format="8" Ref="c:\Poseidon\dwfx\W2dTest\out\23A58E61-B112-4215-B08B-5F372A7AA3A4.jpg"
    //    Width="367" Height="454" Area="1900,96,2267,550" />
    const char** ppValue = rMap.find(XamlXML::kpzFormat_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
       format() = (WT_Byte) atoi(*ppValue);
    }
    else
    {
        return WT_Result::Corrupt_File_Error;
    }
    
    ppValue = rMap.find(XamlXML::kpzWidth_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
       columns() = (WT_Unsigned_Integer16)atoi(*ppValue);        
    }
    else
    {
        return WT_Result::Corrupt_File_Error;
    }

    ppValue = rMap.find(XamlXML::kpzHeight_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
       rows() = (WT_Unsigned_Integer16)atoi(*ppValue);        
    }
    else
    {
        return WT_Result::Corrupt_File_Error;
    }

    ppValue = rMap.find(XamlXML::kpzArea_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
        int minX,minY,maxX,maxY = 0;
        int nProcessed = sscanf( *ppValue, "%d,%d,%d,%d", &minX,&minY,&maxX,&maxY);
        if (nProcessed != 4)
        {
            return WT_Result::Internal_Error;
        }        
        min_corner().m_x = minX;
        min_corner().m_y = minY;
        max_corner().m_x = maxX;
        max_corner().m_y = maxY;    
    }
    else
    {
        return WT_Result::Corrupt_File_Error;
    }

    DWFString oPath;

    ppValue = rMap.find(XamlXML::kpzRef_Attribute);
    if(ppValue != NULL && *ppValue != NULL)
    {
        oPath = *ppValue;
    }

    ppValue = rMap.find(XamlXML::kpzDpi_Attribute);
	if(ppValue != NULL && *ppValue != NULL)
	{
       dpi() = (WT_Integer32)atoi(*ppValue);     
    }

    DWFInputStream *pIn;
    WT_OpcResourceMaterializer *&prOpc = rFile.opcResourceMaterializer();
    if(prOpc == NULL)
    {
        return WT_Result::Internal_Error;
    }

    pIn = NULL;
    WD_CHECK( prOpc->getPartInputStream(oPath, &pIn) );
    if(pIn == NULL)
    {
        return WT_Result::Internal_Error;
    }

	//
	// the return value of the avialable() is  the available size of the compressed data, 
	// not the actual size of the decompressed data. 
	//
    if(pIn->available())
    {
        DWFBufferOutputStream pOut( pIn->available() );
        char buffer[1024];
        size_t nBytes;
		//
		// loop until no bytes in the stream
		//
        do
        {
            nBytes = pIn->read( (void*)buffer, 1024 );
            if(nBytes>0)
                pOut.write( (void*)buffer, nBytes );
        }
        while( pIn->available()>0 && nBytes>0 );

        m_data_size = (WT_Integer32)pOut.bytes();
        m_data = DWFCORE_ALLOC_MEMORY(WT_Byte, m_data_size);
        if(m_data == NULL)
        {
            DWFCORE_FREE_OBJECT( pIn );
            return WT_Result::Internal_Error;
        }

        m_local_data_copy = WD_True;
        DWFCORE_COPY_MEMORY( m_data, pOut.buffer(), m_data_size );
    }
    else
    {
        DWFCORE_FREE_OBJECT( pIn );
        return WT_Result::Internal_Error;
    }
    DWFCORE_FREE_OBJECT( pIn );
    return WT_Result::Success;
}
