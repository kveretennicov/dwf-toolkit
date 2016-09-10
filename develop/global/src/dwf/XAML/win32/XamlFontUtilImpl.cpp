//
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
#include <windows.h>
#include "XAML/win32/XamlFontUtilImpl.h"

//file static functions are bad - namespce it so we don't have conflicts elsewhere
namespace XamlFontUtilImplCpp
{
    #define SWAPWORD(x) MAKEWORD(HIBYTE(x), LOBYTE(x))
    #define SWAPLONG(x) MAKELONG(SWAPWORD(HIWORD(x)),SWAPWORD(LOWORD(x)))

    int
    GetTTFData( WT_XAML_Font::CachedTTFFontData& rFontData, DWFBufferOutputStream& rStream )
    {
        unsigned char *p = (unsigned char*)rStream.buffer();

        //define and read file header
        WT_XAML_Font::TT_OFFSET_TABLE ttOffsetTable = {NULL};
        DWFCORE_COPY_MEMORY( &ttOffsetTable, rStream.buffer(), sizeof(WT_XAML_Font::TT_OFFSET_TABLE) );
        p += sizeof(WT_XAML_Font::TT_OFFSET_TABLE);

        //remember to rearrange bytes in the field you're going to use
        ttOffsetTable.uNumOfTables = SWAPWORD(ttOffsetTable.uNumOfTables);
        ttOffsetTable.uMajorVersion = SWAPWORD(ttOffsetTable.uMajorVersion);
        ttOffsetTable.uMinorVersion = SWAPWORD(ttOffsetTable.uMinorVersion);

        //check if this is a TrueType font and the version is 1.0
        if( ttOffsetTable.uMajorVersion != 1 ||
            ttOffsetTable.uMinorVersion != 0)
            return -1;

        WT_XAML_Font::TT_TABLE_DIRECTORY *pTblDir = NULL;
        char name[5];

        size_t nHEAD_offset = 0;
        size_t nHEAD_length = 0;
        size_t nHHEA_offset = 0;
        size_t nHHEA_length = 0;
        size_t nHMTX_offset = 0;
        size_t nHMTX_length = 0;
        size_t nCMAP_offset = 0;
        size_t nCMAP_length = 0;

        //Find the tables we want 
        for(int i=0; i< ttOffsetTable.uNumOfTables; i++)
        {
            p += sizeof(WT_XAML_Font::TT_TABLE_DIRECTORY);
            pTblDir = (WT_XAML_Font::TT_TABLE_DIRECTORY*)p;

            //the table's tag cannot exceed 4 characters
            DWFCORE_COPY_MEMORY(name, pTblDir->szTag, 4);
            name[4] = '\0';
            if( DWFCORE_COMPARE_ASCII_STRINGS_NO_CASE(name, "cmap") == 0)
            {
                nCMAP_length = SWAPLONG(pTblDir->uLength);
                nCMAP_offset = SWAPLONG(pTblDir->uOffset);
            }
            else if( DWFCORE_COMPARE_ASCII_STRINGS_NO_CASE(name, "hhea") == 0)
            {
                nHHEA_length = SWAPLONG(pTblDir->uLength);
                nHHEA_offset = SWAPLONG(pTblDir->uOffset);
            }
            else if( DWFCORE_COMPARE_ASCII_STRINGS_NO_CASE(name, "hmtx") == 0)
            {
                nHMTX_length = SWAPLONG(pTblDir->uLength);
                nHMTX_offset = SWAPLONG(pTblDir->uOffset);
            }
            else if( DWFCORE_COMPARE_ASCII_STRINGS_NO_CASE(name, "head") == 0)
            {
                nHEAD_length = SWAPLONG(pTblDir->uLength);
                nHEAD_offset = SWAPLONG(pTblDir->uOffset);
            }

            if (nCMAP_length > 0 && nHHEA_length > 0 && nHMTX_length > 0 && nHEAD_length)
            {
                //we got what we need
                break;
            }
        }

        if (nCMAP_length == 0 || nHHEA_length == 0 || nHMTX_length == 0 || nHEAD_length == 0)
        {
            return -2;
        }

        //Get the units from the HEAD table
        p = (unsigned char*)rStream.buffer() + nHEAD_offset;
        WT_XAML_Font::TT_HEAD_TABLE *pHead = (WT_XAML_Font::TT_HEAD_TABLE*) p;
        rFontData.nUnitsPerEM = SWAPWORD( pHead->nUnitsPerEm );

        //copy off the HHEA table
        p = (unsigned char*)rStream.buffer() + nHHEA_offset;
        DWFCORE_COPY_MEMORY( &rFontData.ttTblHHEA, p, sizeof(WT_XAML_Font::TT_HHEA_TABLE_HEADER) );
        rFontData.ttTblHHEA.nNumberOfHMetrics = SWAPWORD(rFontData.ttTblHHEA.nNumberOfHMetrics);
        rFontData.ttTblHHEA.nAscender = SWAPWORD(rFontData.ttTblHHEA.nAscender);
        rFontData.ttTblHHEA.nDescender = SWAPWORD(rFontData.ttTblHHEA.nDescender);
        rFontData.ttTblHHEA.nMetricDataFormat = SWAPWORD(rFontData.ttTblHHEA.nMetricDataFormat);
        rFontData.ttTblHHEA.nAdvanceWidthMax = SWAPWORD(rFontData.ttTblHHEA.nAdvanceWidthMax);
        rFontData.ttTblHHEA.nMinLeftSideBearing = SWAPWORD(rFontData.ttTblHHEA.nMinLeftSideBearing);
        rFontData.ttTblHHEA.nMinRightSideBearing = SWAPWORD(rFontData.ttTblHHEA.nMinRightSideBearing);
        rFontData.ttTblHHEA.nXMaxExtent = SWAPWORD(rFontData.ttTblHHEA.nXMaxExtent);
        
        //Allocate and process HMTX advanced widths table
        rFontData.ppHorzMetrics = DWFCORE_ALLOC_MEMORY( WT_XAML_Font::TT_HORZ_METRIC*, 1 );
        if (rFontData.ppHorzMetrics==NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }
        *rFontData.ppHorzMetrics = DWFCORE_ALLOC_MEMORY( WT_XAML_Font::TT_HORZ_METRIC, rFontData.ttTblHHEA.nNumberOfHMetrics);
        if (*rFontData.ppHorzMetrics==NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        p = (unsigned char*)rStream.buffer() + nHMTX_offset;
        for (size_t i=0; i < rFontData.ttTblHHEA.nNumberOfHMetrics; i++)
        {
            WT_XAML_Font::TT_HORZ_METRIC *pMetric = (WT_XAML_Font::TT_HORZ_METRIC*)p;
            p += sizeof(WT_XAML_Font::TT_HORZ_METRIC);
            (*rFontData.ppHorzMetrics)[i].advanceWidth = SWAPWORD( pMetric->advanceWidth );
            (*rFontData.ppHorzMetrics)[i].lsb = SWAPWORD( pMetric->lsb );
        }

        //Get CMAP table header
        WT_XAML_Font::TT_CMAP_TABLE_HEADER ttTblCMAP;
        p = (unsigned char*)rStream.buffer() + nCMAP_offset;
        DWFCORE_COPY_MEMORY( &ttTblCMAP, p, sizeof(WT_XAML_Font::TT_CMAP_TABLE_HEADER) );
        ttTblCMAP.nVersion = SWAPWORD( ttTblCMAP.nVersion );
        ttTblCMAP.nNumTables = SWAPWORD( ttTblCMAP.nNumTables );

        p = (unsigned char*)rStream.buffer() + nCMAP_offset + sizeof(WT_XAML_Font::TT_CMAP_TABLE_HEADER) ;
        WT_XAML_Font::TT_CMAP_TABLE_RECORD ttRecordCMAP = {0};
        bool bFound = false;
        for (size_t i=0; i < ttTblCMAP.nNumTables; i++ )
        {
            DWFCORE_COPY_MEMORY( &ttRecordCMAP, p, sizeof(WT_XAML_Font::TT_CMAP_TABLE_RECORD) );
            p+= sizeof(WT_XAML_Font::TT_CMAP_TABLE_RECORD);

            ttRecordCMAP.nPlatformID = SWAPWORD( ttRecordCMAP.nPlatformID );
            ttRecordCMAP.nEncodingID = SWAPWORD( ttRecordCMAP.nEncodingID );
            ttRecordCMAP.nOffset     = SWAPLONG( ttRecordCMAP.nOffset );

            //Set pointer to proper CMAP record
            if (ttRecordCMAP.nPlatformID == 3 /*Microsoft*/ && (ttRecordCMAP.nEncodingID == 1 /*Unicode*/ || ttRecordCMAP.nEncodingID == 0 /*Symbol*/))
            {
                rFontData.nEncodingID = ttRecordCMAP.nEncodingID;//save the encoding type.
                bFound = true;
                break;
            }
        }

        if (bFound)
        {
            //we have the address of the proper cmap, so we just need to figure out how
            //it is encoded, and then we'll have our character-to-glyph-index mapping.

            p = (unsigned char*)rStream.buffer() + nCMAP_offset + ttRecordCMAP.nOffset;
            WT_XAML_Font::TT_CMAP_ENCODING_HEADER *pPreview = (WT_XAML_Font::TT_CMAP_ENCODING_HEADER  *) p;
            unsigned short nDataLength = SWAPWORD( pPreview->nLength );

            rFontData.ppCMapTable = (WT_XAML_Font::TT_CMAP_ENCODING_HEADER**) DWFCORE_ALLOC_MEMORY( WT_XAML_Font::TT_CMAP_ENCODING_HEADER*, 1 );
            if (rFontData.ppCMapTable == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }
            *rFontData.ppCMapTable = (WT_XAML_Font::TT_CMAP_ENCODING_HEADER*) DWFCORE_ALLOC_MEMORY( unsigned char, nDataLength );
            if (*rFontData.ppCMapTable == NULL)
            {
                return WT_Result::Out_Of_Memory_Error;
            }

            DWFCORE_COPY_MEMORY( *rFontData.ppCMapTable, p, nDataLength );
            (*rFontData.ppCMapTable)->nLength = SWAPWORD( (*rFontData.ppCMapTable)->nLength );
            (*rFontData.ppCMapTable)->nFormat = SWAPWORD( (*rFontData.ppCMapTable)->nFormat );

            switch ((*rFontData.ppCMapTable)->nFormat)
            {
            default:
            case 0: //byte encoding table
                {
                    //WT_XAML_Font::TT_CMAP_ENCODING_0 *pTable = (WT_XAML_Font::TT_CMAP_ENCODING_0 *) (*rFontData.ppCMapTable);
                    WD_Assert( WD_False );
                    break;
                }
            case 2: //high-byte mapping through table
                {
                    //WT_XAML_Font::TT_CMAP_ENCODING_2 *pTable = (WT_XAML_Font::TT_CMAP_ENCODING_2 *) (*rFontData.ppCMapTable);
                    WD_Assert( WD_False );
                    break;
                }
            case 4: //segment mapping to delta values
                {
                    WT_XAML_Font::TT_CMAP_ENCODING_4 *pTable = (WT_XAML_Font::TT_CMAP_ENCODING_4 *) (*rFontData.ppCMapTable);
                    pTable->nSegCountX2 = SWAPWORD( pTable->nSegCountX2 );
                    pTable->nSearchRange = SWAPWORD( pTable->nSearchRange );
                    pTable->nEntrySelector = SWAPWORD( pTable->nEntrySelector );
                    pTable->nRangeShift = SWAPWORD( pTable->nRangeShift );
                    unsigned short segCount = pTable->nSegCountX2 / 2;
                    p = (unsigned char*)&(pTable->nRangeShift) + 2;
                    unsigned short *endCount = (unsigned short *)p;

                    size_t offset = 2 * ( segCount + 1/*reservePad*/ );
                    p = (unsigned char*)endCount + offset;
                    unsigned short *startCount =        (unsigned short*) p;
                    offset -= 2;
                    p = (unsigned char*)startCount + offset;
                    short *idDelta =                    (short*) p;
                    p = (unsigned char*)idDelta + offset;
                    unsigned short *idRangeOffset =     (unsigned short*) p;
                    p = (unsigned char*)idRangeOffset + offset;
                    unsigned short *glyphIdArray =      (unsigned short*) p;

                    for ( unsigned short j = 0; j < segCount; j++ ) 
                    {
                        unsigned short nStart = SWAPWORD( startCount[j] );
                        unsigned short nEnd = SWAPWORD( endCount[j] );
                        for ( unsigned short i = nStart; i <= nEnd ; i++ ) 
                        {
                            if (i==0xFFFF)
                                break;
                            unsigned short nGlyphIndex = 0;;
                            unsigned short nIdRangeOffset = SWAPWORD( idRangeOffset[j] );

                            if ( nIdRangeOffset != 0 ) 
                            {
                                
                                nGlyphIndex = glyphIdArray[ nIdRangeOffset / 2 + ( i - nStart ) - ( segCount - j ) ];
                                nGlyphIndex = SWAPWORD( nGlyphIndex );
                            }
                            else 
                            {
                                short nIdDelta = SWAPWORD( idDelta[j] );
                                nGlyphIndex = nIdDelta + i;
                            }

                            rFontData.oGlyphIndexMap.insert( WT_XAML_Font::CachedTTFFontData::tGlyphIndexMap::value_type( i, nGlyphIndex ) );
                        }
                    }
                    break;
                }
            case 6: //trimmed table mapping
                {
                    //WT_XAML_Font::TT_CMAP_ENCODING_6 *pTable = (WT_XAML_Font::TT_CMAP_ENCODING_6 *) (*rFontData.ppCMapTable);
                    WD_Assert( WD_False );
                    break;
                }
            case 8: // mixed 16-bit and 32-bit coverage
                {
                    //WT_XAML_Font::TT_CMAP_ENCODING_8 *pTable = (WT_XAML_Font::TT_CMAP_ENCODING_8 *) (*rFontData.ppCMapTable);
                    WD_Assert( WD_False );
                    break;
                }
            case 10: //trimmed array
                {
                    //WT_XAML_Font::TT_CMAP_ENCODING_10 *pTable = (WT_XAML_Font::TT_CMAP_ENCODING_10 *) (*rFontData.ppCMapTable);
                    WD_Assert( WD_False );
                    break;
                }
            case 12: //segmented coverage
                {
                    //WT_XAML_Font::TT_CMAP_ENCODING_12 *pTable = (WT_XAML_Font::TT_CMAP_ENCODING_12 *) (*rFontData.ppCMapTable);
                    WD_Assert( WD_False );
                    break;
                }
            }

        }

        return 0;
    }

    int
    GetTTFStringData( WT_XAML_Font::CachedTTFFontData& rFontData, const WT_String &rString, double &rfWidth, double &rfHeight, double &rfBase, double &rfAscent )
    {
        rfAscent = rFontData.ttTblHHEA.nAscender;
        rfBase = - rFontData.ttTblHHEA.nDescender;
        rfHeight = rFontData.ttTblHHEA.nAscender + rfBase; //should be an EM
        rfWidth = 0.;

        const unsigned short *pString = NULL;
        int nLength = -1;
        
        wchar_t* szWideCharString = NULL;//Check the glyph index type.

		//
        //nEncodingID: 1  Unicode; 0  Symbol.
		//
        if(rFontData.nEncodingID==0 && rString.is_ascii())
        {
            int iLen = MultiByteToWideChar(CP_SYMBOL,0,rString.ascii(),rString.length(),0,0);
            if(iLen)
            {
                szWideCharString = new wchar_t[iLen];
                MultiByteToWideChar(CP_SYMBOL,0,rString.ascii(),rString.length(),szWideCharString,iLen);
                pString = (unsigned short*)szWideCharString;
                nLength = iLen;
            }            
        }
        if(!pString)
        {
            pString = rString.unicode();
            nLength = rString.length();
        }
                
        for (int i=0; i<nLength; i++ )
        {
            unsigned short nIndex = 0; //default to glyph index 0, unknown
            WT_XAML_Font::CachedTTFFontData::tGlyphIndexMap::const_iterator iter = rFontData.oGlyphIndexMap.find( pString[i] );
            if (iter != rFontData.oGlyphIndexMap.end())
            {
                nIndex = iter->second;
            }
            if (nIndex > rFontData.ttTblHHEA.nNumberOfHMetrics && rFontData.ttTblHHEA.nNumberOfHMetrics > 0)
            {
                nIndex = 0;
            }
            else if (rFontData.ttTblHHEA.nNumberOfHMetrics == 0)
            {
                WD_Assert( WD_False ); //unlikely
                rfWidth += rFontData.ttTblHHEA.nAdvanceWidthMax + rFontData.ttTblHHEA.nMinLeftSideBearing;
				break;
            }
            
            unsigned short aw = min( (*rFontData.ppHorzMetrics)[nIndex].advanceWidth, rFontData.ttTblHHEA.nAdvanceWidthMax );
            short lsb = max( (*rFontData.ppHorzMetrics)[nIndex].lsb, rFontData.ttTblHHEA.nMinLeftSideBearing );
            rfWidth +=  min(lsb + aw, rFontData.ttTblHHEA.nXMaxExtent);
        }

		if(szWideCharString)
            delete [] szWideCharString;
        
		return 0;
    }
}

using namespace XamlFontUtilImplCpp;




WT_Result
XamlFontUtilImpl::getFontStringWidth( const WT_XAML_File &crFile, const WT_String &rString, double &rfWidth, double &rfHeight, double &rfBase, double &rfAscent ) const
{
    //Make sure we can materialize a font part
    WT_XAML_File &rFile = const_cast<WT_XAML_File&>(crFile);
    if (rFile.opcResourceMaterializer() == NULL)
        return WT_Result::Toolkit_Usage_Error;

    WT_Font & oFont = rFile.desired_rendition().font();
    WT_Font_Extension & oFontExtension = rFile.desired_rendition().font_extension();

    //Get the font name
    WT_String zwsFontName = oFontExtension.cannonical_name().length() ? oFontExtension.cannonical_name() 
                                                                      : ( oFontExtension.logfont_name().length() ? oFontExtension.logfont_name()
                                                                                                                 : oFont.font_name().name() );

    wchar_t* zFontName = WT_String::to_wchar( zwsFontName.length(), zwsFontName );
    if (zFontName == NULL)
        return WT_Result::Out_Of_Memory_Error;

    const wchar_t* zResolvedFontPath = rFile.resolveFontUri( zFontName );
    if (zResolvedFontPath == NULL)
    {
        if (rFile.desired_rendition().font_extension().logfont_name().length()!=0)
        {
            zwsFontName = rFile.desired_rendition().font_extension().logfont_name();
            delete [] zFontName; //not allocated using DWFCORE_ALLOC_MEMORY;
            zFontName = WT_String::to_wchar( zwsFontName.length(), zwsFontName );
            zResolvedFontPath = rFile.resolveFontUri( zFontName );
            if (zResolvedFontPath == NULL)
            {
                if (rFile.desired_rendition().font().font_name().name().length()!=0)
                {
                    zwsFontName = rFile.desired_rendition().font().font_name().name();
                    delete [] zFontName; //not allocated using DWFCORE_ALLOC_MEMORY;
                    zFontName = WT_String::to_wchar( zwsFontName.length(), zwsFontName );
                    zResolvedFontPath = rFile.resolveFontUri( zFontName );
                    if (zResolvedFontPath == NULL)
                    {
                        return WT_Result::Internal_Error;
                    }
                }
            }
        }
    }

    WT_XAML_Font::CachedTTFFontData *pFontData = NULL;

    //ask the File if we already have the font buffer cached...
    WT_XAML_File::tCachedTTFFontMap::iterator font_iter = rFile.cached_ttf_fonts().find( zResolvedFontPath );

    if ( font_iter != rFile.cached_ttf_fonts().end() )
    {
        pFontData = font_iter->second;
    }

    if (pFontData == NULL)
    {
        pFontData = DWFCORE_ALLOC_OBJECT( WT_XAML_Font::CachedTTFFontData );
        if (pFontData == NULL)
        {
            return WT_Result::Out_Of_Memory_Error;
        }

        rFile.cached_ttf_fonts().insert( WT_XAML_File::tCachedTTFFontMap::value_type( zResolvedFontPath, pFontData ) );

        //Get the font from the packager
        DWFInputStream *pFontStream = NULL;
        WT_Result result = rFile.opcResourceMaterializer()->getPartInputStream( zResolvedFontPath, &pFontStream );

        if (result != WT_Result::Success || pFontStream == NULL)
        {
            return WT_Result::Internal_Error;
        }

        //Read the font into a buffer stream, and deobfuscate it
        char key[16];
        DWFBufferOutputStream oOutputStream( pFontStream->available() );
        WD_CHECK( WT_XAML_Font::parseKey( key, zResolvedFontPath ) );
        WD_CHECK( WT_XAML_Font::obfuscate( pFontStream, &oOutputStream, key ) );

        int nTTF = GetTTFData( *pFontData, oOutputStream ); 
        if (-1 == nTTF )
        {
            //Re-acquire the font stream, necessary if the InputStram implementation doesn't support seeking
            DWFCORE_FREE_OBJECT( pFontStream );
            result = rFile.opcResourceMaterializer()->getPartInputStream( zResolvedFontPath, &pFontStream );
            if (result != WT_Result::Success || pFontStream == NULL)
            {
                return WT_Result::Internal_Error;
            }

            size_t nAvailable = pFontStream->available();
            if ( nAvailable )
            {
                DWFBufferOutputStream oOutputStream2( nAvailable );
                char pBuf[1024];
                do
                {
                    size_t nBytes = pFontStream->read( pBuf, sizeof(pBuf) );
                    if (nBytes)
                    {
                        oOutputStream2.write( pBuf, nBytes );
                    }
                    nAvailable = pFontStream->available();
                }
                while (nAvailable);

                nTTF = GetTTFData( *pFontData, oOutputStream2 ); 
            }
        }

        DWFCORE_FREE_OBJECT( pFontStream );

        WD_Assert( nTTF == 0 );
        if (nTTF != 0)
            return WT_Result::Internal_Error;
    }

    GetTTFStringData( *pFontData, rString, rfWidth, rfHeight, rfBase, rfAscent );
    double fScaleFactor = oFont.height().height() / (double)pFontData->nUnitsPerEM;
    rfWidth *= fScaleFactor;
    rfHeight *= fScaleFactor;
    rfBase *= fScaleFactor;
    rfAscent *= fScaleFactor;

    WD_Assert( rfWidth != 0.);
    WD_Assert( rfHeight != 0.);

    delete[] zFontName;
    return WT_Result::Success;
}
