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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/opc/Relationship.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/opc/Relationship.h"
#include "dwf/opc/Constants.h"
#include "dwf/opc/Part.h"
using namespace DWFToolkit;


_DWFTK_API
OPCRelationship::OPCRelationship()
throw()
                 : _pTarget( NULL )
                 , _zRelationshipType()
                 , _eTargetMode( OPCRelationship::eInternal )
                 , _zTargetURI()
                 , _zId()
{;}

_DWFTK_API
OPCRelationship::OPCRelationship( OPCPart* pTarget, 
                                  DWFString zRelationshipType,
                                  OPCRelationship::teTargetMode eTargetMode )
throw()
                 : _pTarget( pTarget )
                 , _zRelationshipType( zRelationshipType )
                 , _eTargetMode( eTargetMode )
                 , _zTargetURI()
                 , _zId()
{;}

_DWFTK_API
OPCRelationship::~OPCRelationship()
throw()
{;}

_DWFTK_API
void
OPCRelationship::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    if (ppAttributeList == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No attributes provided" );
    }

    unsigned char nFound = 0;
    size_t iAttrib = 0;
    const char* pAttrib = NULL;

    for (; ppAttributeList[iAttrib]; iAttrib += 2)
    {
        pAttrib = &ppAttributeList[iAttrib][0];

        //
        //  set the target name
        //
        if (!(nFound & 0x01) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, OPCXML::kzAttribute_Target) == 0))
        {
            nFound |= 0x01;
            _zTargetURI.assign( ppAttributeList[iAttrib+1] );
        }

        else if (!(nFound & 0x02) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, OPCXML::kzAttribute_Type) == 0))
        {
            nFound |= 0x02;
            _zRelationshipType.assign( ppAttributeList[iAttrib+1] );
        }
            
        else if (!(nFound & 0x04) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, OPCXML::kzAttribute_TargetMode) == 0))
        {
            nFound |= 0x04;
            if (DWFCORE_COMPARE_ASCII_STRINGS(ppAttributeList[iAttrib+1], OPCXML::kzAttributeVal_TargetMode_External) == 0)
            {
                _eTargetMode = eExternal;
            }
            else
            {
                _eTargetMode = eInternal;
            }
        }

        else if (!(nFound & 0x08) &&
            (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, OPCXML::kzAttribute_Id) == 0))
        {
            nFound |= 0x08;
            _zId.assign( ppAttributeList[iAttrib+1] );
        }

    }
}

_DWFTK_API
const DWFString&
OPCRelationship::targetURI()
throw()
{
    if (_pTarget != NULL)
    {
        _zTargetURI = _pTarget->uri();
    }

    return _zTargetURI;
}

_DWFTK_API
void
OPCRelationship::setId( const DWFString& zId )
throw()
{
    _zId = zId;
}

_DWFTK_API
const DWFString&
OPCRelationship::id() const
throw()
{
    return _zId;
}


#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
OPCRelationship::serializeXML( DWFXMLSerializer& rSerializer )
throw( DWFException )
{
    rSerializer.startElement( OPCXML::kzElement_Relationship );

    rSerializer.addAttribute( OPCXML::kzAttribute_Target, _pTarget->uri() );

    if (_eTargetMode == eExternal)
    {
        rSerializer.addAttribute( OPCXML::kzAttribute_TargetMode, 
                                  OPCXML::kzAttributeVal_TargetMode_External );
    }

    if (_zId.chars() == 0)
    {
        DWFString zId = /*NOXLATE*/L"_";
        zId.append( rSerializer.nextUUID(false) );
        _zId.assign(zId);
    }

    rSerializer.addAttribute( OPCXML::kzAttribute_Id, _zId );

    rSerializer.addAttribute( OPCXML::kzAttribute_Type, _zRelationshipType );

    rSerializer.endElement();
}

#endif
