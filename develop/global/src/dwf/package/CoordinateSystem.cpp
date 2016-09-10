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

#include "dwf/package/CoordinateSystem.h"
#include "dwf/package/Constants.h"
using namespace DWFToolkit;

_DWFTK_API
DWFCoordinateSystem::DWFCoordinateSystem( )
throw()
{
    _eType = ePublished;

    //
    // Initialize the origin to 0,0,0
    //
    for (int i = 0; i < 3; i++)
    {
        _anOrigin[i] = 0.0;
    }

    //
    //Initialize the rotation to the unit matrix
    //
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (i == j)
            {
                _anRotation[i][j] = 1.0;
            }
            else
            {
                _anRotation[i][j] = 0.0;
            }
        }
    }
}

_DWFTK_API
DWFCoordinateSystem::DWFCoordinateSystem( teType eType,
                                          const double* anOrigin,
                                          const double* anRotation,
                                          const DWFString& zName,
                                          const DWFString& zID )
throw()
{
    set( eType, anOrigin, anRotation, zName, zID );
}


_DWFTK_API
DWFCoordinateSystem::DWFCoordinateSystem( const DWFCoordinateSystem& rCoordinateSystem )
throw()
{
    *this = rCoordinateSystem;
}

_DWFTK_API
DWFCoordinateSystem& DWFCoordinateSystem::operator=( const DWFCoordinateSystem& rCoordinateSystem )
throw()
{
    _eType = rCoordinateSystem._eType;
    memcpy( _anOrigin, rCoordinateSystem._anOrigin, sizeof(double) * 3 );
    memcpy( _anRotation, rCoordinateSystem._anRotation, sizeof(double) * 9 );
    _zName = rCoordinateSystem._zName;
    _zID = rCoordinateSystem._zID;
    return *this;
}

_DWFTK_API
DWFCoordinateSystem::~DWFCoordinateSystem()
throw()
{
}

_DWFTK_API
void DWFCoordinateSystem::set( teType eType,
                               const double* anOrigin,
                               const double* anRotation,
                               const DWFString& zName,
                               const DWFString& zID )
throw()
{
    _eType = eType;
    memcpy( _anOrigin, anOrigin, sizeof(double) * 3 );
    memcpy( _anRotation, anRotation, sizeof(double) * 9 );
    _zName = zName;
    _zID = zID;
}

_DWFTK_API
void
DWFCoordinateSystem::parseAttributeList( const char** ppAttributeList )
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
        else if (DWFCORE_COMPARE_MEMORY(DWFXML::kzNamespace_EModel, ppAttributeList[iAttrib], 7) == 0)
        {
            pAttrib = &ppAttributeList[iAttrib][7];
        }
        else
        {
            pAttrib = &ppAttributeList[iAttrib][0];
        }

            //
            // set the type
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Type) == 0))
        {
            nFound |= 0x01;
            if (DWFCORE_COMPARE_ASCII_STRINGS( ppAttributeList[iAttrib+1], "published" ) == 0)
            {
                _eType = ePublished;
            }
            else
            {
                _eType = eMarkedup;
            }
        }
            //
            // set OriginX
            //
        if (!(nFound & 0x02) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_OriginX) == 0))
        {
            nFound |= 0x02;
            _anOrigin[0] = DWFString::StringToDouble( ppAttributeList[iAttrib+1] );
        }
            //
            // set OriginY
            //
        else if (!(nFound & 0x04) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_OriginY) == 0))
        {
            nFound |= 0x04;
            _anOrigin[1] = DWFString::StringToDouble( ppAttributeList[iAttrib+1] );
        }
            //
            // set OriginZ
            //
        else if (!(nFound & 0x08) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_OriginZ) == 0))
        {
            nFound |= 0x08;
            _anOrigin[2] = DWFString::StringToDouble( ppAttributeList[iAttrib+1] );
        }
            //
            // set the rotation transformation
            //
        else if (!(nFound & 0x10) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Rotation) == 0))
        {
            nFound |= 0x10;

            char* pRotation = (char*)ppAttributeList[iAttrib+1];
            char* pSavePtr;
            char* pToken = DWFCORE_ASCII_STRING_TOKENIZE( pRotation, /*NOXLATE*/" ", &pSavePtr );
            _anRotation[0][0] = DWFString::StringToDouble( pToken );

            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
            _anRotation[0][1] = DWFString::StringToDouble( pToken );
            
            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
            _anRotation[0][2] = DWFString::StringToDouble( pToken );
            
            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
            _anRotation[1][0] = DWFString::StringToDouble( pToken );
            
            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
            _anRotation[1][1] = DWFString::StringToDouble( pToken );
            
            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
            _anRotation[1][2] = DWFString::StringToDouble( pToken );
            
            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
            _anRotation[2][0] = DWFString::StringToDouble( pToken );
            
            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
            _anRotation[2][1] = DWFString::StringToDouble( pToken );
            
            pToken = DWFCORE_ASCII_STRING_TOKENIZE( NULL, /*NOXLATE*/" ", &pSavePtr );
            _anRotation[2][2] = DWFString::StringToDouble( pToken );            
        }
            //
            // set the name
            //
        else if (!(nFound & 0x20) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Name) == 0))
        {
            nFound |= 0x20;
            _zName = ppAttributeList[iAttrib+1];
        }
            //
            // set the name
            //
        else if (!(nFound & 0x40) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ID) == 0))
        {
            nFound |= 0x40;
            _zID = ppAttributeList[iAttrib+1];
        }
    }    
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFCoordinateSystem::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
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

    rSerializer.startElement( DWFXML::kzElement_CoordinateSystem, zNamespace );
        //
        // attributes
        //
    {
        rSerializer.addAttribute( DWFXML::kzAttribute_Type, _eType == ePublished ? L"published" : L"markedup" );

        rSerializer.addAttribute( DWFXML::kzAttribute_OriginX, _anOrigin[0] );

        rSerializer.addAttribute( DWFXML::kzAttribute_OriginY, _anOrigin[1] );

        rSerializer.addAttribute( DWFXML::kzAttribute_OriginZ, _anOrigin[2] );

        rSerializer.addAttribute( DWFXML::kzAttribute_Rotation, (double*)_anRotation, 9 );

        rSerializer.addAttribute( DWFXML::kzAttribute_Name, _zName );

        //
        // if no ID has been specified, create one
        //
        if (_zID.chars() == 0)
        {
            _zID = rSerializer.nextUUID( true );
        }

        rSerializer.addAttribute( DWFXML::kzAttribute_ID, _zID );
    }

        //
        // properties
        //
        DWFXMLSerializable& rSerializable = DWFPropertyContainer::getSerializable();
        rSerializable.serializeXML( rSerializer, nFlags );

    rSerializer.endElement();
}

#endif

