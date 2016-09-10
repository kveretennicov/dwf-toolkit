//
//  Copyright (c) 1996-2006 by Autodesk, Inc.
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
// restrictions set forth in FAR 52.227-19 (Commercial Computer
// Software - Restricted Rights) and DFAR 252.227-7013(c)(1)(ii)
// (Rights in Technical Data and Computer Software), as applicable.
//
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/Resource.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//


#include "dwfcore/DWFXMLSerializer.h"
#include "dwfcore/MIME.h"
#include "dwfcore/zip/zip.h"
#include "dwfcore/zip/password.h"
#include "dwfcore/DWFDecryptingInputStream.h"
#include "dwfcore/DecompressingInputStream.h"
using namespace DWFCore;

#include "dwf/package/Resource.h"
#include "dwf/package/Constants.h"
#include "dwf/package/reader/PackageManifestReader.h"
using namespace DWFToolkit;


_DWFTK_API
DWFResource::DWFResource( DWFPackageReader* pPackageReader )
throw()
           : _nSize( 0 )
           , _zTargetHRef()
           , _oContentID()
           , _oRelationships()
           , _pPackageReader( pPackageReader )
           , _zTitle()
           , _zRole()
           , _zMIME()
           , _zHRef()
           , _zObjectID()
           , _zParentObjectID()
           , _zRequestedIdentity()
           , _zInternalID()
           , _eCustomZipMode( DWFZipFileDescriptor::eZip )
           , _bUseCustomZipMode( false )
           , _bDWFXProtected( false )
           , _pStream( NULL )
           , _oNotifySinks()
{
    ;
}

_DWFTK_API
DWFResource::DWFResource( const DWFString& zTitle,
                          const DWFString& zRole,
                          const DWFString& zMIME,
                          const DWFString& zHREF )
throw()
           : _nSize( 0 )
           , _zTargetHRef()
           , _oContentID()
           , _oRelationships()
           , _pPackageReader( NULL )
           , _zTitle( zTitle )
           , _zRole( zRole )
           , _zMIME( zMIME )
           , _zHRef( zHREF )
           , _zObjectID()
           , _zParentObjectID()
           , _zRequestedIdentity()
           , _zInternalID()
           , _eCustomZipMode( DWFZipFileDescriptor::eZip )
           , _bUseCustomZipMode( false )
           , _bDWFXProtected( false )
           , _pStream( NULL )
           , _oNotifySinks()
{
    ;
}

_DWFTK_API
DWFResource::~DWFResource()
throw()
{
    //
    // Clean up notify sinks
    //
    _oNotifySinks.clear();

    //
    // Clean up relationship list
    //
    if(!_oRelationships.empty())
    {
        DWFResourceRelationship::tList::Iterator *pIter = _oRelationships.iterator();
        if (pIter)
        {
            for (; pIter->valid(); pIter->next())
            {
                DWFResourceRelationship *pRelationship = pIter->get();
                if(pRelationship != NULL)
                {
                    DWFCORE_FREE_OBJECT( pRelationship );
                }
            }

            DWFCORE_FREE_OBJECT( pIter );
        }
    }

    if ( _pStream )
    {
        DWFCORE_FREE_OBJECT( _pStream );
    }

    //
    // requirement of the DWFOwnable contract
    //
    DWFOwnable::_notifyDelete();
}

_DWFTK_API
void
DWFResource::setMIME( const DWFString& zMIME )
throw()
{
    _zMIME = zMIME;
}

_DWFTK_API
bool
DWFResource::addContentID( const DWFString& zContentID )
throw( DWFException )
{
    size_t iPos;
    if (!_oContentID.findFirst( zContentID, iPos ))
    {
        _oContentID.push_back( zContentID );
        return true;
    }
    else
    {
        return false;
    }
}

_DWFTK_API
DWFInputStream*
DWFResource::getInputStream( bool bCache )
throw( DWFException )
{
        //
        // return the "set" stream
        // TODO: fix this so it's not inconsistent
        //
    if (_pStream)
    {
        //
        // since the caller must delete this pointer,
        // we must ensure that it is returned only once
        //
        DWFInputStream* pStream = _pStream;
        _pStream = NULL;

        return pStream;
    }
        //
        // always a new stream
        //
    else if (_pPackageReader)
    {
		if(_bDWFXProtected)
		{
			//
			// wrap in password stream if special dwfx protected resource and can get a password
			// we need to allow NULL passwords to pass through, so the stream can later be extracted 
            // and the password error can be detected at that point.
            //
			DWFString zPassword;
            _pPackageReader->_requestResourcePassword(*this, zPassword);
            DWFInputStream* pOriginal = _pPackageReader->extract(_zHRef, bCache);
			if (pOriginal)
			{
				pOriginal = DWFCORE_ALLOC_OBJECT( DWFDecryptingInputStream(pOriginal, zPassword) ) ;
				return DWFCORE_ALLOC_OBJECT(DWFDecompressingInputStream(pOriginal, true));
			}
		}
        return _pPackageReader->extract( _zHRef, bCache );
    }
    else
    {
        return NULL;
    }
}

#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFResource::setInputStream( DWFInputStream* pStream, size_t nBytes )
throw( DWFException )
{
    _nSize = nBytes;
    _pStream = pStream;
}

#endif

_DWFTK_API
void
DWFResource::setPublishedIdentity( const DWFString& zSectionName,
                                   const DWFString& zObjectID )
throw()
{
        //
        // keep the original object ID or create a new one
        // either way, only the incoming ID is used for making the HREF!
        //
    if (_zObjectID.bytes() == 0)
    {
        _zObjectID.assign( zObjectID );
    }

    //
    //  Set this only if an identity has not been requested already
    //
    if (_zRequestedIdentity.chars() == 0)
    {
        _zTargetHRef.assign( zSectionName );
        _zTargetHRef.append( /*NOXLATE*/L"\\" );
        _zTargetHRef.append( zObjectID );

        DWFString zExtension = DWFMIME::GetExtension(_zMIME);
        if (zExtension.bytes())
        {
            _zTargetHRef.append( /*NOXLATE*/L"." );
            _zTargetHRef.append( zExtension );
        }
    }
}

_DWFTK_API
void
DWFResource::setParentResource( const DWFResource* pResource )
throw()
{
    if (pResource)
    {
        _zParentObjectID.assign( pResource->objectID() );
    }
    else
    {
        _zParentObjectID.destroy();
    }
}

_DWFTK_API
DWFResource::tStringIterator*
DWFResource::_tokenizeBySpace( char* szInput )
throw()
{
    //
    // TODO: REPLACE wcstok with a thread-safe DWFString tokenizer class
    // for the VC7.1 build
    //

    _DWFTK_STD_VECTOR(DWFString)  oTokens;

#if defined(_DWFCORE_WIN32_SYSTEM) && (_MSC_VER<1400)

    char* szToken = ::strtok( szInput, /*NOXLATE*/" " );
    while (szToken)
    {
        oTokens.push_back( DWFString(szToken) );
        szToken = ::strtok( NULL, /*NOXLATE*/" " );
    }

#else

    typedef char* (*tpfTokenizer)(char*, const char*, char**);

    #if defined(_MSC_VER) && (_MSC_VER>=1400)
        tpfTokenizer pfTokenizer = &(::strtok_s);
    #else
        tpfTokenizer pfTokenizer = &(::strtok_r);
    #endif

    //
    //  These are safe tokenizers since the intermediate state information is stored in zwcState
    //
    char* szState;
    char* szToken = (*pfTokenizer)( szInput, /*NOXLATE*/" ", &szState);
    while (szToken)
    {
        oTokens.push_back( DWFString(szToken) );
        szToken = (*pfTokenizer)( NULL, /*NOXLATE*/" ", &szState);
    }

#endif

    typedef DWFVectorIterator<DWFString, _DWFTK_STL_ALLOCATOR<DWFString> > _tStringVectorIterator;
    tStringIterator* piToken = DWFCORE_ALLOC_OBJECT( _tStringVectorIterator(oTokens) );

    return piToken;
}

_DWFTK_API
void
DWFResource::parseAttributeList( const char** ppAttributeList )
throw( DWFException )
{
    if (ppAttributeList == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No attributes provided" );
    }

    unsigned short nFound = 0;
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
            // set the role
            //
        if (!(nFound & 0x01) &&
			(DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_MIME) == 0))
        {
            nFound |= 0x01;

            _zMIME.assign( ppAttributeList[iAttrib+1] );

            //
            //
            //
            size_t nLen = _zMIME.chars();
            wchar_t* pMIME = DWFCORE_ALLOC_MEMORY( wchar_t, nLen+1 );
            DWFCORE_WIDE_STRING_COPY( pMIME, (const wchar_t*)_zMIME );
            if ((pMIME[nLen-1] == L'p') &&
                (pMIME[nLen-2] == L'-'))
            {
                _bDWFXProtected = true;

                pMIME[nLen-2] = 0;
                _zMIME.assign( pMIME );
            }
            DWFCORE_FREE_MEMORY( pMIME );
        }
            //
            // set the href
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_HRef) == 0))
        {
            nFound |= 0x02;

            notifyBeforeHRefChanged();
            _zHRef.assign( ppAttributeList[iAttrib+1] );
            notifyAfterHRefChanged();
        }
            //
            // set the MIME type
            //
        else if (!(nFound & 0x04) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Role) == 0))
        {
            nFound |= 0x04;

            _zRole.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the title
            //
        else if (!(nFound & 0x08) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Title) == 0))
        {
            nFound |= 0x08;

            _zTitle.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the size
            //
        else if (!(nFound & 0x10) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Size) == 0))
        {
            nFound |= 0x10;

            _nSize = ::atoi( ppAttributeList[iAttrib+1] );
        }
            //
            // set the object ID
            //
        else if (!(nFound & 0x20) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ObjectID) == 0))
        {
            nFound |= 0x20;

            _zObjectID.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the parent object ID
            //
        else if (!(nFound & 0x40) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ParentObjectID) == 0))
        {
            nFound |= 0x40;

            _zParentObjectID.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the content ID
            //
        else if (!(nFound & 0x80) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ContentIDs) == 0 ||
                   DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ContentID) == 0))
        {
            nFound |= 0x80;

            tStringIterator* piToken = _tokenizeBySpace( (char*)(ppAttributeList[iAttrib+1]) );
            if (piToken)
            {
                for (; piToken->valid(); piToken->next())
                {
                    _oContentID.push_back( piToken->get() );
                }
                DWFCORE_FREE_OBJECT( piToken );
            }
        }
            //
            // set the internal ID
            //
        else if (!(nFound & 0x0100) &&
                (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_InternalID) == 0))
        {
            nFound |= 0x0100;

            _zInternalID.assign( ppAttributeList[iAttrib+1] );
        }
    }
}


#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFResource::serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
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
    else
    {
        zNamespace.assign( DWFXML::kzNamespace_DWF );
    }

    DWFString zHRef( (_zTargetHRef.bytes() > 0) ? _zTargetHRef : _zHRef );

        //
        // serialize for manifest
        // (this is an abbreviated attribute list)
        //
    if (nFlags & DWFPackageWriter::eManifest)
    {
        //
        // root element
        //
        rSerializer.startElement( DWFXML::kzElement_Resource, zNamespace );
        {
                //
                // attributes
                //
            {
                rSerializer.addAttribute( DWFXML::kzAttribute_Role, _zRole );
                rSerializer.addAttribute( DWFXML::kzAttribute_MIME, _zMIME );
                rSerializer.addAttribute( DWFXML::kzAttribute_HRef,  zHRef );
            }
        }
        rSerializer.endElement();
    }
        //
        // serialize in detail for descriptor
        // unless this resource is the descriptor itself...
        //
    else if ((nFlags & DWFPackageWriter::eDescriptor) &&
             (_zRole != DWFXML::kzRole_Descriptor))
    {

            //
            // if the derived class has already open the element
            // we can only serialize the base attributes here
            //
        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            rSerializer.startElement( DWFXML::kzElement_Resource, zNamespace );
        }

        {
                //
                // common attributes
                //
            {
                rSerializer.addAttribute( DWFXML::kzAttribute_Role, _zRole );
                rSerializer.addAttribute( DWFXML::kzAttribute_MIME, _zMIME );
                rSerializer.addAttribute( DWFXML::kzAttribute_HRef,  zHRef );
                rSerializer.addAttribute( DWFXML::kzAttribute_Title, _zTitle );

                //
                // query the size from the stream
                //
                if ((_nSize == 0) && _pStream)
                {
                    _nSize = _pStream->available();
                }

				//
				//query the size from input stream
				//
				else if((_nSize == 0) && !_pStream)
                {
                    DWFInputStream* pInputStream = NULL;
                    pInputStream = this->getInputStream();
                    if(pInputStream)
                    {
                        // The _nSize should be in uncompressed bytes
                        if(!_bDWFXProtected)
                        {
                            _nSize = pInputStream->available();
                        }
                        else
                        {
                            // pInputStream must be a compressed stream,
                            // the return value of available() is not the exact size of the uncompressed stream.
                            char buffer[1024];
                            size_t nRead;
                            _nSize = 0;
                            while(pInputStream->available())
                            {
                                nRead = pInputStream->read((void*)buffer,sizeof(buffer));
                                _nSize += nRead;
                            }
                        }

                        DWFCORE_FREE_OBJECT(pInputStream);
                    }
                }

                    //
                    // optional anyway, don't bother if zero
                    //
                if (_nSize > 0)
                {
                    rSerializer.addAttribute( DWFXML::kzAttribute_Size, (int)_nSize );
                }

                if (_zInternalID.chars())
                {
                    rSerializer.addAttribute( DWFXML::kzAttribute_InternalID, _zInternalID );
                }

                rSerializer.addAttribute( DWFXML::kzAttribute_ObjectID, _zObjectID );
                rSerializer.addAttribute( DWFXML::kzAttribute_ParentObjectID, _zParentObjectID );

                    //
                    // serialize contentIDs into a single attribute
                    //
                if (_oContentID.size() > 0)
                {
                    DWFString zContentIDs;
                    size_t nIndex = 0;
                    for (; nIndex < _oContentID.size(); ++nIndex)
                    {
                        zContentIDs.append( _oContentID[nIndex] );
                        zContentIDs.append( /*NOXLATE*/L" " );
                    }

                    if (zContentIDs.chars() > 0)
                    {
                        rSerializer.addAttribute( DWFXML::kzAttribute_ContentIDs, zContentIDs );
                    }
                }
            }

                //
                // if the derived class has already open the element
                // we can only serialize the base attributes here,
                // also, object definition resources, shouldn't seriailize their
                // properties here as they will be detailed in those documents.
                //
            if (((nFlags & DWFXMLSerializer::eElementOpen) == 0) &&
                (_zRole != DWFXML::kzRole_ObjectDefinition)      &&
                (_zRole != DWFXML::kzRole_MarkupObjectDefinition))
            {
                //
                // properties
                //
                DWFXMLSerializable& rSerializable = DWFPropertyContainer::getSerializable();
                rSerializable.serializeXML( rSerializer, nFlags );
            }

                //
                // Serialize relationships.
                //
            if(((nFlags & DWFXMLSerializer::eElementOpen) == 0) && (!_oRelationships.empty()))
            {
                DWFResourceRelationship::tList::Iterator *pIter = _oRelationships.iterator();
                if (pIter)
                {
                    rSerializer.startElement( DWFXML::kzElement_Relationships, zNamespace );

                    for (; pIter->valid(); pIter->next())
                    {
                        DWFResourceRelationship *pRelationship = pIter->get();
                        if(pRelationship != NULL)
                        {
                            rSerializer.startElement( DWFXML::kzElement_Relationship, zNamespace );
                            rSerializer.addAttribute( DWFXML::kzAttribute_ObjectID, pRelationship->resourceID() );
                            rSerializer.addAttribute( DWFXML::kzAttribute_Type, pRelationship->type());
                            rSerializer.endElement();
                        }
                    }
                    DWFCORE_FREE_OBJECT( pIter );

                    rSerializer.endElement();
                }
            }
        }

        if ((nFlags & DWFXMLSerializer::eElementOpen) == 0)
        {
            rSerializer.endElement();
        }
    }
}

#endif

_DWFTK_API
void
DWFResource::enableCustomZipMode( bool bUseCustomZipMode,
                                  DWFZipFileDescriptor::teFileMode eZipMode )
throw()
{
    _bUseCustomZipMode = bUseCustomZipMode;
    _eCustomZipMode = eZipMode;
}



_DWFTK_API
void
DWFResource::addRelationship( DWFResourceRelationship* pResourceRel)
    throw(DWFException)
{
    if(pResourceRel==NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No resource relationship object provided" );
    }

    _oRelationships.insert(pResourceRel, DWFResourceRelationship::tList::eBack);
}

_DWFTK_API
void
DWFResource::addRelationship( const DWFResource* pResource, const DWFString& zType)
    throw(DWFException)
{
    DWFResourceRelationship *pRel = DWFCORE_ALLOC_OBJECT(DWFResourceRelationship(pResource, zType));
    if(pRel==NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate Relationship" );
    }

    addRelationship(pRel);
}

_DWFTK_API
void 
DWFResource::removeRelationship( DWFResourceRelationship* pResourceRel, bool bDelete )
        throw( DWFException )
{
    if ( pResourceRel == NULL )
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, 
            /*NOXLATE*/L"No resource relationship object provided" );
    }
    _oRelationships.erase( pResourceRel );
    if ( pResourceRel && bDelete )
    {
        DWFCORE_FREE_OBJECT( pResourceRel );
    }
}


_DWFTK_API
const DWFString&
DWFResourceRelationship::resourceID() const
    throw(DWFException)
{
    if(_zResourceID.chars() > 0)
    {
        return _zResourceID;
    }

    if(_pResource!=NULL)
    {
        return _pResource->objectID();
    }

    _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No ObjectID or resource provided" );
}

_DWFTK_API
void
DWFResource::setNotificationSink( NotificationSink* pSink )
throw()
{
    if (pSink)
    {
        _tNotifySinks::iterator iFind = std::find( _oNotifySinks.begin(), _oNotifySinks.end(), pSink );

        //
        //  Don't add the same sink twice.
        //
        if (iFind == _oNotifySinks.end())
        {
            _oNotifySinks.push_back( pSink );
        }
    }
}

_DWFTK_API
void
DWFResource::removeNotificationSink( NotificationSink* pSink )
throw()
{
    if (pSink)
    {
        _tNotifySinks::iterator iEnd = std::remove( _oNotifySinks.begin(), _oNotifySinks.end(), pSink );
        _oNotifySinks.erase( iEnd, _oNotifySinks.end() );
    }
}

_DWFTK_API
void DWFResource::notifyBeforeHRefChanged()
    throw()
{
    if (!_oNotifySinks.empty())
    {
        _tNotifySinks::iterator iSink = _oNotifySinks.begin();
        for (; iSink != _oNotifySinks.end(); ++iSink)
        {
            (*iSink)->onBeforeResourceHRefChanged( this );
        }
    }
}


_DWFTK_API
void DWFResource::notifyAfterHRefChanged()
    throw()
{
    if (!_oNotifySinks.empty())
    {
        _tNotifySinks::iterator iSink = _oNotifySinks.begin();
        for (; iSink != _oNotifySinks.end(); ++iSink)
        {
            (*iSink)->onAfterResourceHRefChanged( this );
        }
    }
}


_DWFTK_API
void
DWFResourceRelationship::parseAttributeList( const char** ppAttributeList )
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
            // set the ObjectID
            //
        if (!(nFound & 0x01) &&
             (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_ObjectID) == 0))
        {
            nFound |= 0x01;

            _zResourceID.assign( ppAttributeList[iAttrib+1] );
        }
            //
            // set the type
            //
        else if (!(nFound & 0x02) &&
                  (DWFCORE_COMPARE_ASCII_STRINGS(pAttrib, DWFXML::kzAttribute_Type) == 0))
        {
            nFound |= 0x02;

            _zType.assign( ppAttributeList[iAttrib+1] );
        }
    }
}


