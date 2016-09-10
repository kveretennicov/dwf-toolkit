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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/opc/Part.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//
//

#include "dwf/opc/Part.h"
#include "dwf/opc/RelationshipContainer.h"
using namespace DWFToolkit;


_DWFTK_API
OPCPart::OPCPart()
throw()
       : _zPath( /*NOXLATE*/L"" )
       , _zName( /*NOXLATE*/L"" )
       , _pInputStream( NULL )
       , _nStreamBytes( 0 )
       , _bOwnStream( false )
       , _pRelsContainer( DWFCORE_ALLOC_OBJECT( OPCRelationshipContainer ) )
{;}

_DWFTK_API
OPCPart::~OPCPart()
throw()
{
    if (_pRelsContainer)
    {
        DWFCORE_FREE_OBJECT( _pRelsContainer );
    }

    if (_bOwnStream && 
        _pInputStream != NULL)
    {
        DWFCORE_FREE_OBJECT( _pInputStream );
    }
}

_DWFTK_API
void
OPCPart::setName( const DWFString& zName )
throw( DWFException )
{
    if (zName.find(/*NOXLATE*/L'/',0)==0 ||
        zName.find(/*NOXLATE*/L"..",0)==0)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"A part name cannot start with '/' or '..'" );
    }

    _zName = zName;
}

_DWFTK_API
DWFString
OPCPart::uri() const
throw( DWFException )
{
    size_t nNameLen = _zName.chars();
    if (nNameLen == 0)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"No name was provided for the part. Cannot compute the uri." );
    }

    size_t nPathLen = _zPath.chars();

    if (nPathLen == 0)
    {
        return _zName;
    }
    else
    {
        DWFString zURI( _zPath );

        if (_zPath.find(/*NOXLATE*/L'/', off_t(nPathLen-1)) == -1)
        {
            zURI.append( /*NOXLATE*/L"/" );
        }
        zURI.append( _zName );

        return zURI;
    }
}

_DWFTK_API
size_t
OPCPart::relationshipCount() const
throw()
{
    return _pRelsContainer->relationshipCount();
}

_DWFTK_API
DWFString
OPCPart::relationshipUri() const
throw( DWFException )
{
    DWFString zURI( uri() );
    DWFString zRelURI( /*NOXLATE*/L"" );

    if (zURI.chars())
    {
        off_t offset = zURI.findLast( /*NOXLATE*/L'/' );

        DWFString zPath;
        DWFString zPartOnly;
        if (offset != -1)
        {
            //
            //  Get the path, including the '/' character
            //
            zPath = zURI.substring( 0, offset+1 );

            //
            //  Get the part name without the path
            //
            zPartOnly = zURI.substring( offset+1 );

        }
        else
        {
            //
            // No '/' in our URI
            //
            zPath = /*NOXLATE*/L"/";
            zPartOnly = zURI;
        }

        zRelURI.append( zPath );
        zRelURI.append( /*NOXLATE*/L"_rels/" );
        zRelURI.append( zPartOnly );
        zRelURI.append( /*NOXLATE*/L".rels" );
    }
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"The URI of the part could not be determined." );
    }

    return zRelURI;
}

_DWFTK_API
OPCRelationship*
OPCPart::addRelationship( OPCPart* pTargetPart, 
                          const DWFString& zRelationshipType, 
                          OPCRelationship::teTargetMode eTargetMode )
throw( DWFException )
{
    return  _pRelsContainer->addRelationship( pTargetPart, zRelationshipType, eTargetMode );
}

_DWFTK_API
bool
OPCPart::deleteRelationship( OPCRelationship* pRelationship )
throw()
{
    return _pRelsContainer->deleteRelationship( pRelationship );
}


_DWFTK_API
OPCRelationship::tIterator*
OPCPart::relationships()
throw()
{
    return  _pRelsContainer->relationships();
}

_DWFTK_API
OPCRelationship::tIterator*
OPCPart::relationshipsByTarget( const DWFString& zTargetURI ) const
throw()
{
    return  _pRelsContainer->relationshipsByTarget( zTargetURI );
}

_DWFTK_API
OPCRelationship::tIterator*
OPCPart::relationshipsByTarget( OPCPart* pPart ) const
throw()
{
    return  _pRelsContainer->relationshipsByTarget( pPart );
}

_DWFTK_API
OPCRelationship::tIterator*
OPCPart::relationshipsByType( const DWFString& zType ) const
throw()
{
    return  _pRelsContainer->relationshipsByType( zType );
}

_DWFTK_API
DWFInputStream* 
OPCPart::getRelationshipsInputStream()
throw( DWFException )
{
    return _pRelsContainer->getInputStream();
}

_DWFTK_API
void
OPCPart::loadRelationships(DWFInputStream* pRelsInputStream)
throw( DWFException )
{
    _pRelsContainer->loadRelationships(pRelsInputStream);
}

_DWFTK_API
void
OPCPart::deleteRelationshipsByTarget( OPCPart* pPart )
throw( DWFException )
{
    _pRelsContainer->deleteRelationshipsByTarget( pPart );
}

_DWFTK_API
DWFInputStream*
OPCPart::getInputStream()
throw( DWFException )
{
    return _pInputStream;
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
OPCPart::setInputStream( DWFInputStream* pInputStream, 
                         size_t nBytes,
                         bool bOwnStream )
throw( DWFException )
{
    _nStreamBytes = nBytes;
    _pInputStream = pInputStream;

    if ((_nStreamBytes == 0) && _pInputStream)
    {
        _nStreamBytes = _pInputStream->available();
    }

    _bOwnStream = bOwnStream;
}

#endif

