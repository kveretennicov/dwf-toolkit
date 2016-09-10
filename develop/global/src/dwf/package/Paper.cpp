//
//  Copyright (c) 2003-2006 by Autodesk, Inc.  All rights reserved.
//
// Permission to use, copy, modify, and distribute this software in
// object code form for any purpose and without fee is hereby granted,
// provided that the above copyright notice appears in all copies and
// that both that copyright notice and the limited warranty and
// restricted rights notice below appear in all supporting
// documentation.
//
// AUTODESK PROVIDES THIS PROGRAM 'AS IS' AND WITH ALL FAULTS.
// AUTODESK SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF
// MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  AUTODESK, INC.
// DOES NOT WARRANT THAT THE OPERATION OF THE PROGRAM WILL BE
// UNINTERRUPTED OR ERROR FREE.
//
// Use, duplication, or disclosure by the U.S. Government is subject to
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//
// $Header: /Components/Internal/DWF Toolkit/v7.1/develop/global/src/dwf/package/Paper.cpp 7     8/11/05 7:56p Kuk $
//

#include "dwfcore/DWFXMLSerializer.h"

#include "dwf/package/Paper.h"
#include "dwf/package/Constants.h"
#include "dwf/package/reader/PackageManifestReader.h"
using namespace DWFToolkit;


_DWFTK_API
DWFPaper::DWFPaper()
throw()
        : _bShow( false )
        , _nWidth( 0.0 )
        , _nHeight( 0.0 )
        , _nColorARGB( 0x00ffffff )
        , _eUnits( eUnknown )
{
    _anClip[0] = _anClip[1] = _anClip[2] = _anClip[3] = 0;
}

_DWFTK_API
DWFPaper::DWFPaper( double        nWidth,
                    double        nHeight,
                    teUnits       eUnits,
                    unsigned int  nColorARGB,
                    const double* anClip,
                    bool          bShow )
throw()
        : _bShow( bShow )
        , _nWidth( nWidth )
        , _nHeight( nHeight )
        , _nColorARGB( nColorARGB )
        , _eUnits( eUnits )
{
    if (anClip)
    {
        _anClip[0] = anClip[0];
        _anClip[1] = anClip[1];
        _anClip[2] = anClip[2];
        _anClip[3] = anClip[3];
    }
    else
    {
        _anClip[0] = _anClip[1] = _anClip[2] = _anClip[3] = 0.0f;
    }
}

_DWFTK_API
DWFPaper::DWFPaper( const DWFPaper& rPaper )
throw()
        : _bShow( rPaper._bShow )
        , _nWidth( rPaper._nWidth )
        , _nHeight( rPaper._nHeight )
        , _nColorARGB( rPaper._nColorARGB )
        , _eUnits( rPaper._eUnits )
{
    _anClip[0] = rPaper._anClip[0];
    _anClip[1] = rPaper._anClip[1];
    _anClip[2] = rPaper._anClip[2];
    _anClip[3] = rPaper._anClip[3];
}

_DWFTK_API
DWFPaper&
DWFPaper::operator=( const DWFPaper& rPaper )
throw()
{
    _bShow = rPaper._bShow;
    _nWidth = rPaper._nWidth;
    _nHeight = rPaper._nHeight;
    _nColorARGB = rPaper._nColorARGB;
    _eUnits = rPaper._eUnits;

    _anClip[0] = rPaper._anClip[0];
    _anClip[1] = rPaper._anClip[1];
    _anClip[2] = rPaper._anClip[2];
    _anClip[3] = rPaper._anClip[3];

    return *this;
}

_DWFTK_API
DWFPaper::~DWFPaper()
throw()
{
    ;
}

_DWFTK_API
void
DWFPaper::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    if (ppAttributeList == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No attributes provided" );
    }

    unsigned char nFound = 0;
    size_t iAttrib = 0;
    const char* pAttrib = NULL;

    for(; ppAttributeList[iAttrib]; iAttrib += 2)
    {
            //
            // skip over any acceptable prefixes in the element name
            //
        if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_DWF, ppAttributeList[iAttrib], 4) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][4];
        } 
        else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_ECommon, ppAttributeList[iAttrib], 8) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][8];
        } 
        else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EPlot, ppAttributeList[iAttrib], 6) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][6];
        }
        else
        {
            pAttrib = &ppAttributeList[iAttrib][0];
        }

            //
            // set the show attribute
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Show) == 0))
        {
            nFound |= 0x01;
            
            if ((ppAttributeList[iAttrib+1][0] == 't') ||      // true
                (ppAttributeList[iAttrib+1][0] == 'T') ||      // True | TRUE
                (ppAttributeList[iAttrib+1][0] == '1') ||      // 1
                (ppAttributeList[iAttrib+1][0] == 'y') ||      // yes
                (ppAttributeList[iAttrib+1][0] == 'Y'))        // Yes | YES
            {
                _bShow = true;
            }
        }
            //
            // set the units
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Units) == 0))
        {
            nFound |= 0x02;

            if ((ppAttributeList[iAttrib+1][0] == 'm') ||
                (ppAttributeList[iAttrib+1][0] == 'M'))
            {
                _eUnits = eMillimeters;
            } 
            else if ((ppAttributeList[iAttrib+1][0] == 'i') ||
                     (ppAttributeList[iAttrib+1][0] == 'I'))
            {
                _eUnits = eInches;
            }
        }
            //
            // set the width
            //
        else if (!(nFound & 0x04) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Width) == 0))
        {
            nFound |= 0x04;
            
            _nWidth = DWFString::StringToDouble( ppAttributeList[iAttrib+1] );
        }
            //
            // set the height
            //
        else if (!(nFound & 0x08) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Height) == 0))
        {
            nFound |= 0x08;

            _nHeight = DWFString::StringToDouble( ppAttributeList[iAttrib+1] );
        }
            //
            // set the color
            //
        else if (!(nFound & 0x10) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Color) == 0))
        {
            nFound |= 0x10;
            
            char* pColor = (char*)ppAttributeList[iAttrib+1];
            char* pSavePtr;
            char* pToken = DWFCORE_ASCII_STRING_TOKENIZE( pColor, /*NOXLATE*/" ", &pSavePtr );
            _nColorARGB = ((unsigned char)::atoi(pToken) << 16);

            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
            _nColorARGB |= ((unsigned char)::atoi(pToken) << 8);
            
            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
            _nColorARGB |= (unsigned char)::atoi(pToken);
        }
            //
            // set the clip region
            //
        else if (!(nFound & 0x20) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Clip) == 0))
        {
            nFound |= 0x20;

            char* pClip = (char*)ppAttributeList[iAttrib+1];
            char* pSavePtr;
            char* pToken = DWFCORE_ASCII_STRING_TOKENIZE( pClip, /*NOXLATE*/" ", &pSavePtr );
            _anClip[0] = DWFString::StringToDouble( pToken );

            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
            _anClip[1] = DWFString::StringToDouble( pToken );
            
            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
            _anClip[2] = DWFString::StringToDouble( pToken );
            
            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
            _anClip[3] = DWFString::StringToDouble( pToken );
        }
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFPaper::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
throw( DWFException )
{
    DWFString zNamespace;

        //
        // namespace dictated by document and section type 
        //
    if (nFlags & DWFPackageWriter::eDescriptor)
    {
        zNamespace.assign( namespaceXML(nFlags) );
    }

#define _DWFTK_PAPER_TEMP_BUFFER_BYTES  1024

    wchar_t zTempBuffer[_DWFTK_PAPER_TEMP_BUFFER_BYTES];

    rSerializer.startElement( DWFXML::kzElement_Paper, zNamespace );

    {
        //DNT_Start
        rSerializer.addAttribute( DWFXML::kzAttribute_Units, (_eUnits == eMillimeters ? /*NOXLATE*/L"mm" : /*NOXLATE*/L"in") );

        rSerializer.addAttribute( DWFXML::kzAttribute_Width, _nWidth );

        rSerializer.addAttribute( DWFXML::kzAttribute_Height, _nHeight );

        _DWFCORE_SWPRINTF( zTempBuffer, _DWFTK_PAPER_TEMP_BUFFER_BYTES, 
                            /*NOXLATE*/L"%d %d %d", 
                            (unsigned char)((_nColorARGB & 0x00ff0000)>>16),
                            (unsigned char)((_nColorARGB & 0x0000ff00)>>8),
                            (unsigned char)(_nColorARGB & 0x000000ff) );

        rSerializer.addAttribute( DWFXML::kzAttribute_Color, zTempBuffer );

        if ((_anClip[0] != 0.0) ||
            (_anClip[1] != 0.0) ||
            (_anClip[2] != 0.0) ||
            (_anClip[3] != 0.0))
        {
            rSerializer.addAttribute( DWFXML::kzAttribute_Clip, _anClip, 4 );
        }
        //DNT_End
    }

    rSerializer.endElement();
}

#endif

