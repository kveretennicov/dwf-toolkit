//
//  Copyright (c) 2004-2006 by Autodesk, Inc.
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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/reader/PackageReader.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//



#include "dwfcore/Pointer.h"
#include "dwfcore/MonitoredInputStream.h"
#include "dwfcore/UnzippingInputStream.h"
using namespace DWFCore;

#include "dwf/Version.h"
#include "dwf/package/XML.h"
#include "dwf/package/Manifest.h"
#include "dwf/package/EPlotSection.h"
#include "dwf/package/EModelSection.h"
#include "dwf/package/DataSection.h"
#include "dwf/package/SignatureSection.h"
#include "dwf/package/GlobalSection.h"
#include "dwf/package/SectionBuilder.h"
#include "dwf/package/reader/PackageReader.h"
#include "dwf/package/reader/PackageManifestReader.h"
#include "dwf/opc/CoreProperties.h"
#include "dwf/opc/RelationshipContainer.h"
#include "dwf/dwfx/Constants.h"
#include "dwf/dwfx/DWFDocumentSequence.h"
#include "dwf/dwfx/DWFProperties.h"
#include "dwf/dwfx/CustomProperties.h"
#include "dwf/opc/ZipFileReader.h"
#include "dwf/dwfx/reader/FixedPageResourceExtractor.h"
using namespace DWFToolkit;


_DWFTK_API
DWFPackageReader::DWFPackageReader()
throw()
                : _pDWFStream( NULL )
                , _pDWFXPackage( NULL )
                , _pPackageManifest( NULL )
                , _pSectionBuilder( NULL )
                , _oZipFileIndex()
                , _oFixedPageResourceExtractorMap()
                , _zCachedDWFXManifestName( )
                , _pDWFXDWFProperties( NULL )
                , _pDWFXCustomProperties( NULL )
                , _pOPCCoreProperties( NULL )
{
	_tPackageInfo.nVersion = 0L;
    _tPackageInfo.eType = eUnknown;
    _tPackageInfo.bDigitallySigned = false;
    _oExtractMutex.init();
}

_DWFTK_API
DWFPackageReader::DWFPackageReader( const DWFFile&      zDWFPackageFile,
                                    const DWFString&    zDWFPackagePassword )
throw()
                : _zDWFPackage( zDWFPackageFile )
                , _zDWFPassword( zDWFPackagePassword )
                , _pDWFStream( NULL )
                , _pDWFXPackage( NULL )
                , _pPackageManifest( NULL )
                , _pSectionBuilder( NULL )
                , _oZipFileIndex()
                , _oFixedPageResourceExtractorMap()
                , _zCachedDWFXManifestName( )
                , _pDWFXDWFProperties( NULL )
                , _pDWFXCustomProperties( NULL )
                , _pOPCCoreProperties( NULL )
{
    _tPackageInfo.nVersion = 0L;
    _tPackageInfo.eType = eUnknown;
    _tPackageInfo.bDigitallySigned = false;
    _oExtractMutex.init();
}

_DWFTK_API
DWFPackageReader::DWFPackageReader( DWFInputStream&     rDWFPackageStream,
                                    const DWFString&    zDWFPackagePassword )
throw()
                : _zDWFPassword( zDWFPackagePassword )
                , _pDWFStream( &rDWFPackageStream )
                , _pDWFXPackage( NULL )
                , _pPackageManifest( NULL )
                , _pSectionBuilder( NULL )
                , _zCachedDWFXManifestName( )
                , _pDWFXDWFProperties( NULL )
                , _pDWFXCustomProperties( NULL )
                , _pOPCCoreProperties( NULL )
{
    _tPackageInfo.nVersion = 0L;
    _tPackageInfo.eType = eUnknown;
    _tPackageInfo.bDigitallySigned = false;
    _oExtractMutex.init();
}

_DWFTK_API
DWFPackageReader::~DWFPackageReader()
throw()
{
    if (_pPackageManifest)
    {
        DWFCORE_FREE_OBJECT( _pPackageManifest );
    }

    DWFStringKeySkipList<DWFTempFile*>::Iterator* piCache = _oLocalFileCache.iterator();

    if (piCache)
    {
        for (; piCache->valid(); piCache->next())
        {
            DWFCORE_FREE_OBJECT( piCache->value() );
        }

        DWFCORE_FREE_OBJECT( piCache );
    }

    if (_pSectionBuilder)
    {
        DWFCORE_FREE_OBJECT( _pSectionBuilder );
    }

    for( _tFixedPageResourceExtractorMap::iterator iExtractors = _oFixedPageResourceExtractorMap.begin();
        iExtractors != _oFixedPageResourceExtractorMap.end();
        iExtractors++)
    {
        DWFCORE_FREE_OBJECT( iExtractors->second );
    }

    if (_pDWFXDWFProperties)
    {
        DWFCORE_FREE_OBJECT( _pDWFXDWFProperties );
    }
    if (_pDWFXCustomProperties)
    {
        DWFCORE_FREE_OBJECT(_pDWFXCustomProperties );
    }
    if (_pOPCCoreProperties)
    {
        DWFCORE_FREE_OBJECT(_pOPCCoreProperties );
    }


}

DWFZipFileDescriptor*
DWFPackageReader::_open()
throw( DWFException )
{
    DWFZipFileDescriptor* pPackageDescriptor = NULL;

    if (_pDWFStream)
    {
        pPackageDescriptor = DWFCORE_ALLOC_OBJECT( DWFZipFileDescriptor(*_pDWFStream) );
    }
    else
    {
        pPackageDescriptor = DWFCORE_ALLOC_OBJECT( DWFZipFileDescriptor(_zDWFPackage, DWFZipFileDescriptor::eUnzip) );
    }

    if (pPackageDescriptor == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate file descriptor" );
    }

    _makeSectionBuilder();

    try
    {
        pPackageDescriptor->openIndexed(&_oZipFileIndex);
    }
    catch (DWFInvalidTypeException& e)
    {
        DWFCORE_FREE_OBJECT( pPackageDescriptor );
        throw e;
    }

    return pPackageDescriptor;
}

_DWFTK_API
void
DWFPackageReader::setPackagePassword( const DWFString& zDWFPackagePassword )
throw()
{
	_zDWFPassword = zDWFPackagePassword;
}

int
DWFPackageReader::_requestResourcePassword( DWFResource& /*rResource*/, DWFString& zPassword )
{
	zPassword = _zDWFPassword;
	return ((int)zPassword.bytes());
}

// find the signature resource in the container
// check if it is XPS-based
// if so, return false (not OPC signed)
// else, return true.
bool
DWFPackageReader::_findSignatureResource(OPCRelationshipContainer &oSignatureRels)
throw( DWFException )
{
    // Assume it is OPC signed as long as there are relationships to check
    bool bIsOPCSigned = (oSignatureRels.relationshipCount() > 0 ? true : false);

    // Find the signature resource
    DWFPointer<OPCRelationship::tIterator> iter( oSignatureRels.relationships(), false );
    for( ; !iter.isNull() && iter->valid(); iter->next() )
    {
        // Examine each relationship
        OPCRelationship* pRel = iter->get();
        DWFString zSourcePath = pRel->targetURI();

        DWFString zRetVal;
        // Look for the signature extension ".psdsxs"
        DWFString zMSXPS(OPCPackage::kzURI_PackageDigitalSignatureExt);
        off_t nPosLastSlash( zSourcePath.findLast(/* NOXLATE */'/') );
        off_t nPosLastDot( zSourcePath.findLast(/* NOXLATE */'.') );

        if ((nPosLastDot >= 0 ) &&
            (nPosLastDot > nPosLastSlash) )
        {
            zRetVal = zSourcePath.substring(nPosLastDot);
        }
        // If it is the signature resource, check it out
        if (zRetVal == zMSXPS)
        {
            // Get a stream to the resource
            DWFPointer<DWFInputStream> spInstream ( this->extract( zSourcePath, false ), false );
            if (!spInstream.isNull())
            {
                // Get the required buffer size 
                size_t bufferSize = spInstream->available();
                // Allocate the buffer
                BYTE* buffer = DWFCORE_ALLOC_MEMORY( BYTE, bufferSize );
                if (buffer != NULL)
                {
                    // Read the resource into buffer
                    size_t nRead = spInstream->read( buffer, bufferSize );
                    if (nRead == bufferSize)
                    {
                        DWFString zBuffer((const char*)buffer);
                        // look for microsoft/xps string
                        DWFString zMSXPS(OPCPackage::kzURI_PackageDigitalSignatureXPS);
                        if (zBuffer.find(zMSXPS) != -1)
                        {
                            // XPS found! Signature is not OPC, return false
                            bIsOPCSigned = false;
                        }
                    }
                    DWFCORE_FREE_MEMORY(buffer);
                }
            }
            break;
        }
    }
    return bIsOPCSigned;
}

//
// Check if a digitally signed file is in the XPS format.
// If so, we do not support it. We only recognize files signed
// by ADR using the OPC protocol.
//
// Input: none, but we only call this method if the file is signed.
//
// Return: bool true  - OPC format (signed by ADR)
//              false - XPS format (signed in IE, Word, etc.)
//
bool DWFPackageReader::_isOPCSigned()
throw( DWFException )
{
    // Setup the relationship container to receive relationships
    OPCRelationshipContainer oSignatureRels;
    
    // Set the target path to the digital signature relationships file
    // Look for this file "/package/services/digital-signature/_rels/origin.psdsor.rels"
    DWFString zSourcePath(OPCPackage::kzURI_PackageDigitalSignatureRels);
    // Get the XML ".rels" resource stream
    DWFInputStream* pInstream = extract( zSourcePath, false );
    if (pInstream == NULL)
    {
        // error, return false
        return false;
    }

    // Load the relationships (TK code does the parsing for us - magically delicious!)
    oSignatureRels.loadRelationships( pInstream );

    // Release the resource stream
    DWFCORE_FREE_OBJECT( pInstream );

    // Find and examine the signature resource,
    // return true if OPC, false if XPS.
    return _findSignatureResource(oSignatureRels);
}


_DWFTK_API
void
DWFPackageReader::getPackageInfo( tPackageInfo& rInfo )
throw( DWFException )
{
    if ( _tPackageInfo.nVersion != 0L)
    {
        rInfo = _tPackageInfo;
        return;
    }

    _tPackageInfo.eType = eUnknown;
    _tPackageInfo.bDigitallySigned = false;
    _tPackageInfo.zTypeGUID.destroy();

        //
        // snarf the first 12 bytes
        //
    unsigned char pBuffer[12];
        //
        // reset the stream
        //
    if (_pDWFStream)
    {
        _pDWFStream->seek( SEEK_SET, 0 );

            //
            // snarf the first 12 bytes
            //
        if (12 != _pDWFStream->read(pBuffer, 12))
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Invalid DWF stream" );
        }
    }

    else
    {
        //
        // open as stream
        //
        DWFStreamFileDescriptor* pStreamDescriptor = DWFCORE_ALLOC_OBJECT( DWFStreamFileDescriptor(_zDWFPackage, /*NOXLATE*/L"rb") );
        if (pStreamDescriptor == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate stream descriptor" );
        }

        //
        // open the file
        //
        pStreamDescriptor->open();

            //
            // snarf the first 12 bytes
            //
        size_t nBytesRead = pStreamDescriptor->read(pBuffer, 12);
        DWFCORE_FREE_OBJECT( pStreamDescriptor );

        if (12 != nBytesRead)
        {
            _DWFCORE_THROW( DWFIOException, /*NOXLATE*/L"Invalid DWF file" );
        }
    }

    bool bDWF = (DWFCORE_COMPARE_MEMORY(pBuffer, "(DWF V", 6) == 0);
    bool bW2D = (bDWF ? false : (DWFCORE_COMPARE_MEMORY(pBuffer, "(W2D V", 6) == 0));

    if (!bDWF && !bW2D)
    {
        const unsigned char pZipHeader[4] = {'P', 'K', 0x03, 0x04};
        bool bZip = (DWFCORE_COMPARE_MEMORY(pBuffer, pZipHeader, 4) == 0);
        if (bZip)
        {
            //
            // test for DWFX
            //
            DWFZipFileDescriptor* pPackageDescriptor = _open();
            if (pPackageDescriptor->locate( OPCPackage::kzURI_PackageRels ))
            {
                _tPackageInfo.eType = eDWFXPackage;
                _tPackageInfo.nVersion = 700;

                ///////////////////////////////////////////////////////////////////////////////////////
                //
                // Check for a digital signature component
                //
                if (pPackageDescriptor->locate( OPCPackage::kzURI_PackageDigitalSignature ))
                {

                    // mark as signed if it passes the OPC (not XPS) test
                    _tPackageInfo.bDigitallySigned = _isOPCSigned();
                }
                ///////////////////////////////////////////////////////////////////////////////////////

            }
            else
            {
                _tPackageInfo.eType = eZIPFile;
            }

            DWFCORE_FREE_OBJECT( pPackageDescriptor );
        }
        else
        {
            _tPackageInfo.eType = eUnknown;
        }
			   
		//
        // test for password encryption and dwf format version
        //
		if (_tPackageInfo.eType == eDWFXPackage)
		{			
			DWFXDWFProperties* pProperties = getDWFProperties();
			if(pProperties != NULL)
			{
                //check password encryption
				char* flag(NULL);
				pProperties->passwordProtected().getUTF8(&flag);
				if( DWFCORE_COMPARE_ASCII_STRINGS_NO_CASE( /*NOXLATE*/"true",flag ) == 0 )
				{
					_tPackageInfo.eType = eDWFXPackageEncrypted;
				}
                DWFCORE_FREE_MEMORY(flag);

                //check dwf format version
                DWFString szFormat = pProperties->dwfFormatVersion();
                if ( szFormat.chars() != 0)
                {
                    double fVersion = DWFString::StringToDouble( szFormat ) * 100;
                    _tPackageInfo.nVersion = (unsigned long) fVersion;

                    if (_tPackageInfo.nVersion < 700)
                    {
                        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/ L"Unexpected DWFx Package version < 7.00" );
                    }
                }
			}
		}

        rInfo = _tPackageInfo;
        return;
    }

    unsigned char* pChar = (unsigned char*)&pBuffer[6];

    if (*pChar < '0' || *pChar++ > '9' ||
        *pChar < '0' || *pChar++ > '9' ||
        *pChar++ != '.'                ||
        *pChar < '0' || *pChar++ > '9' ||
        *pChar < '0' || *pChar++ > '9' ||
        *pChar != ')')
    {
        _tPackageInfo.eType = eUnknown;
        _tPackageInfo.nVersion = 0L;
        _tPackageInfo.zTypeGUID.destroy();
        return;
    }

    _tPackageInfo.nVersion = ((pBuffer[6] - '0')*10 + (pBuffer[7] - '0'))*100 + ((pBuffer[9] - '0')*10 + (pBuffer[10] - '0'));

    _tPackageInfo.eType = (_tPackageInfo.nVersion >= _DWF_FORMAT_VERSION_PACKAGE && bW2D) ? eW2DStream :
                    (_tPackageInfo.nVersion >= _DWF_FORMAT_VERSION_PACKAGE && bDWF) ? eDWFPackage :
                        (_tPackageInfo.nVersion < _DWF_FORMAT_VERSION_PACKAGE && bDWF) ? eDWFStream : eUnknown;
        //
        // DWF 6+ package - we can test for password encryption
        //
    if (_tPackageInfo.eType == eDWFPackage)
    {
            //
            // open up the package
            //
        DWFPointer<DWFZipFileDescriptor> apPackageDescriptor( _open(), false );

        if (!apPackageDescriptor.isNull())
        {
            DWFZipFileDescriptor::teEncryption enc = apPackageDescriptor->encryption();

                //
                // check for password encryption using the manifest
                // if it wasn't discovered already with the typeinfo file
                //
            if (enc == DWFZipFileDescriptor::eEncryptionUnknown)
            {
                apPackageDescriptor->locate( /*NOXLATE*/L"manifest.xml" );
                enc = apPackageDescriptor->encryption();
            }

            if (enc == DWFZipFileDescriptor::eEncrypted)
            {
                _tPackageInfo.eType = eDWFPackageEncrypted;
            }
        }
    }
    
    rInfo = _tPackageInfo;
}

_DWFTK_API
void
DWFPackageReader::GetPackageInfo( const unsigned char* pBuffer,
                                  tPackageInfo&        rInfo )
throw( DWFException )
{
    rInfo.nVersion = 0L;
    rInfo.eType = eUnknown;
    rInfo.zTypeGUID.destroy();

    bool bDWF = (DWFCORE_COMPARE_MEMORY(pBuffer, "(DWF V", 6) == 0);
    bool bW2D = (bDWF ? false : (DWFCORE_COMPARE_MEMORY(pBuffer, "(W2D V", 6) == 0));

    if (!bDWF && !bW2D)
    {
        const unsigned char pZipHeader[4] = {'P', 'K', 0x03, 0x04};
        rInfo.eType = (DWFCORE_COMPARE_MEMORY(pBuffer, pZipHeader, 4) == 0) ? eZIPFile : eUnknown;
        return;
    }

    unsigned char* pChar = (unsigned char*)&pBuffer[6];

    if (*pChar < '0' || *pChar++ > '9' ||
        *pChar < '0' || *pChar++ > '9' ||
        *pChar++ != '.'                ||
        *pChar < '0' || *pChar++ > '9' ||
        *pChar < '0' || *pChar++ > '9' ||
        *pChar != ')')
    {
        rInfo.eType = eUnknown;
        rInfo.nVersion = 0L;
        rInfo.zTypeGUID.destroy();
        return;
    }

    rInfo.nVersion = ((pBuffer[6] - '0')*10 + (pBuffer[7] - '0'))*100 + ((pBuffer[9] - '0')*10 + (pBuffer[10] - '0'));

    rInfo.eType = (rInfo.nVersion >= _DWF_FORMAT_VERSION_PACKAGE && bW2D) ? eW2DStream :
                    (rInfo.nVersion >= _DWF_FORMAT_VERSION_PACKAGE && bDWF) ? eDWFPackage :
                        (rInfo.nVersion < _DWF_FORMAT_VERSION_PACKAGE && bDWF) ? eDWFStream : eUnknown;
}

_DWFTK_API
DWFToolkit::DWFManifest&
DWFPackageReader::getManifest( DWFManifestReader* pFilter )
throw( DWFException )
{
    if (_pPackageManifest == NULL)
    {
        _pPackageManifest = DWFCORE_ALLOC_OBJECT( DWFManifest(this) );

        if (_pPackageManifest == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate manifest" );
        }

        if (pFilter)
        {
            _pPackageManifest->setFilter( pFilter );
        }

        try
        {
            getManifest( *_pPackageManifest );
        }
        catch(...)
        {
            //
            // if any error, free _pPackageManifest
            //
            DWFCORE_FREE_OBJECT( _pPackageManifest );
            throw;
        }
    }

    return *_pPackageManifest;
}

_DWFTK_API
void
DWFPackageReader::getManifest( DWFManifestReader& rManifestReader )
throw( DWFException )
{
    //
    // acquire a stream to the file and don't bother caching it
    // since this will probably (a) be a one-time event and
    // (b) it probably won't be very large
    //
    DWFString zManifestFilename;
    _getManifestFilename( zManifestFilename );
    DWFPointer<DWFInputStream> apManifestStream( extract(zManifestFilename, false), false );
	DWFXMLParser manifestParser(&rManifestReader);
    manifestParser.parseDocument(apManifestStream);
	//
	// If the package is encrypted. Get the first section and read the resource
	// if an invalid password is encountered then an exception is thrown
	//
	if(_tPackageInfo.eType == DWFPackageReader::eDWFXPackageEncrypted)
	{
		DWFManifest::SectionIterator* pSecIter = _pPackageManifest->getSections();
        DWFResourceContainer::ResourceKVIterator* iter = NULL;
		if (pSecIter)
        {
            DWFSection* pRefSection = pSecIter->get();
            if (pRefSection)
			{
				iter = pRefSection->getResourcesByMIME();
				if(iter)
                {
                    DWFResource * pEncryptedResource = iter->value();
					DWFString oTemp3DRole(L"3d streaming graphics");
					DWFString oTemp2DRole(L"2d streaming graphics");
					DWFString oRightsDoc(L"Rights Document");

					DWFResourceContainer::ResourceIterator* piRightsDocs = pRefSection->findResourcesByRole( oRightsDoc);
					if (piRightsDocs)
					{
						//
						// Skip 2D or 3D file
						//	
						for(; iter->valid(); pEncryptedResource = iter->value() )
						{
							if( pEncryptedResource->role() == L"object definition" ||
								pEncryptedResource->role() == L"markup content presentation" ||
								pEncryptedResource->role() == L"content definition"||
								pEncryptedResource->role() == L"2d graphics extension" ||
								pEncryptedResource->role() == L"2d graphics dictionary")
							{
								break;
							}
							else
							{
								iter->next();
							}
						}	
					}
					else
					{
						for(; iter->valid(); pEncryptedResource = iter->value() )
						{
							if( pEncryptedResource->role() == oTemp3DRole ||
								pEncryptedResource->role() == oTemp2DRole    )
							{
								break;
							}
							else
							{
								iter->next();
							}
						}	
					}

					DWFInputStream * pStream = pEncryptedResource->getInputStream();
					DWFString oTempRole = pEncryptedResource->role();
					if(pStream)
					{
						char pBuffer[1024];
						try
						{
							size_t size = pStream->read(pBuffer, 1024);
							DWFString oTempString((const char*) pBuffer,size );	
							if(oTempRole == oTemp3DRole)
							{
								if(oTempString.find(L"HSF",0,false) < 0)
								{
									_DWFCORE_THROW( DWFZlibDataException, /*NOXLATE*/L"ZLIB data error occured" );

								}
							}else if( oTempString.find(L"<",0,false) < 0 || oTempString.find(L"/>",0,false) < 0)
							{								
								_DWFCORE_THROW( DWFZlibDataException, /*NOXLATE*/L"ZLIB data error occured" );									
							}

							DWFCORE_FREE_OBJECT( pStream );
                        }
                        catch (DWFZlibDataException&)
	                    {
		                    DWFCORE_FREE_OBJECT( pStream );
                            DWFCORE_FREE_OBJECT( iter );
                            DWFCORE_FREE_OBJECT( pSecIter ); 
                            _DWFCORE_THROW( DWFInvalidPasswordException, L"Bad password provided" );
	                    }
					}
                }

            }
        }
        DWFCORE_FREE_OBJECT( iter );
        DWFCORE_FREE_OBJECT( pSecIter );
	}
}

class _MutexLocker
{
    DWFThreadMutex& _rMutex;
    _MutexLocker(const _MutexLocker&);
    _MutexLocker& operator=(const _MutexLocker&);
public:
    _MutexLocker(DWFThreadMutex& rMutex) : _rMutex(rMutex)
    {
        _rMutex.lock();
    }
    ~_MutexLocker()
    {
        _rMutex.unlock();
    }
};

_DWFTK_API
DWFInputStream*
DWFPackageReader::extract( const DWFString& zFilename,
                           bool             bCache )
throw( DWFException )
{
    //
    // only one thread at a time please
    //
    _MutexLocker oLockMutex(_oExtractMutex);
        //
        // check the cache first
        //
    DWFTempFile** ppFile = _oLocalFileCache.find( zFilename );
    if (ppFile)
    {
        return (*ppFile)->getInputStream();
    }

	//
	// Only when _tPackageInfo is eUnknown, call getPackageInfo. This is to avoid infinite  
	// call in getPackageInfo -> getDWFProperties when it is a password protected DWFX file.
	//
	if( _tPackageInfo.eType == eUnknown )
	{
		getPackageInfo( _tPackageInfo );
	}

    DWFInputStream* pArchiveStream = NULL;

        //
        // open up the package if necessary
        //
    DWFZipFileDescriptor* pPackageDescriptor = _open();

    if ((_tPackageInfo.eType == eDWFPackage) || 
        (_tPackageInfo.eType == eDWFPackageEncrypted))
    {
            //
            // make sure the file exists...
            //
        if (pPackageDescriptor->locate(zFilename) == false)
        {
             DWFCORE_FREE_OBJECT( pPackageDescriptor );
            _DWFCORE_THROW( DWFDoesNotExistException, /*NOXLATE*/L"Requested archive not found in DWF package" );
        }

        //
        // acquire stream for reading archive
        //
        pArchiveStream = pPackageDescriptor->unzip( zFilename, _zDWFPassword );
            //
        {
            DWFUnzippingInputStream* pUnzipStream = (DWFUnzippingInputStream*)pArchiveStream;
            pUnzipStream->attach( pPackageDescriptor, true );
        }
    }
    else if ( (_tPackageInfo.eType == eDWFXPackage) ||
		      (_tPackageInfo.eType == eDWFXPackageEncrypted))
    {


        OPCZipFileReader* pOPCZipFileReader = DWFCORE_ALLOC_OBJECT( OPCZipFileReader( pPackageDescriptor ) );

        if (pOPCZipFileReader == NULL)
        {
            DWFCORE_FREE_OBJECT( pPackageDescriptor );
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate OPC zip file reader" );
        }
            //
            // make sure the file exists...
            //
        //if (pPackageDescriptor->locate(zFilename) == false)
        //{
        //    _DWFCORE_THROW( DWFDoesNotExistException, /*NOXLATE*/L"Requested archive not found in DWF package" );
        //}

        //
        // acquire stream for reading archive
        //
        int nOffset = zFilename.find( L'?' );
        if (nOffset != -1)
        {
            DWFString zPartname = zFilename.substring( 0, nOffset );
            DWFString zLocator = zFilename.substring( nOffset + 1 );
            DWFXFixedPageResourceExtractor* pFixedPageResourceExtractor(NULL);
            _tFixedPageResourceExtractorMap::iterator iExtractor = _oFixedPageResourceExtractorMap.find( zPartname );
            if (iExtractor == _oFixedPageResourceExtractorMap.end())
            {
            
                DWFInputStream* pPartStream = pOPCZipFileReader->read( zPartname, true );
                pFixedPageResourceExtractor = DWFCORE_ALLOC_OBJECT(DWFXFixedPageResourceExtractor( pPartStream, true ) );
                if (pFixedPageResourceExtractor == NULL)
                {
                    _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Unable to allocate fixed page resource extractor" );
                }
                _oFixedPageResourceExtractorMap.insert( _tFixedPageResourceExtractorMap::value_type( zPartname, pFixedPageResourceExtractor ) );
                pFixedPageResourceExtractor->parse();
            }
            else
            {
                DWFCORE_FREE_OBJECT( pOPCZipFileReader );
                pFixedPageResourceExtractor = iExtractor->second;
            }

            pArchiveStream = pFixedPageResourceExtractor->getResourceStream( zLocator );
        }
        else
        {
            pArchiveStream = pOPCZipFileReader->read( zFilename, true );
        }

        //
        // If for some reason, a non-existant object was requested...
        //
        if ( !pArchiveStream )
        {
            DWFCORE_FREE_OBJECT( pOPCZipFileReader );
        }
    }
    else
    {
        _DWFCORE_THROW( DWFIllegalStateException, /*NOXLATE*/L"Not a package format" );
    }
        // disk cache requested
        //
    if (bCache)
    {
        //
        // self-managed temp file
        //
        DWFString zTemplate( /*NOXLATE*/L"_dwfpkgreader_" );
        DWFTempFile* pCache = DWFTempFile::Create( zTemplate, true );

        //
        // create a monitor around the archive stream
        // this wrapper provides two useful bits of functionality:
        //  1. as the caller reads from his stream to extract the archive,
        //      this monitor will stream the same bytes into the temp file.
        //  2. when the caller deletes the stream pointer,
        //      the underlying archive stream will go along for the ride
        //
        DWFMonitoredInputStream* pMonitor = DWFCORE_ALLOC_OBJECT( DWFMonitoredInputStream(pArchiveStream, true) );

        if (pMonitor == NULL)
        {
            DWFCORE_FREE_OBJECT( pCache );

            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate stream monitor" );
        }

        //
        // map the requested archive to the temp file for future lookups
        //
        _oLocalFileCache.insert( zFilename, pCache );

        //
        // attach the stream to populate the temp file
        // and don't delete the temp file's output stream...
        //
        pMonitor->attach( &(pCache->getOutputStream()), false );

        //
        // assign the outbound pointer
        //
        pArchiveStream = pMonitor;
    }

    return pArchiveStream;
}

_DWFTK_API
DWFSectionBuilder&
DWFPackageReader::getSectionBuilder()
throw( DWFException )
{
    _makeSectionBuilder();

    return *_pSectionBuilder;
}

void
DWFPackageReader::_makeSectionBuilder()
throw( DWFException )
{
    if (_pSectionBuilder == NULL)
    {
        _pSectionBuilder = DWFCORE_ALLOC_OBJECT( DWFSectionBuilder );

        if (_pSectionBuilder == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate section builder" );
        }

        //
        // add known section factories
        //
        _pSectionBuilder->addFactory( DWFCORE_ALLOC_OBJECT(DWFEPlotSection::Factory) );
        _pSectionBuilder->addFactory( DWFCORE_ALLOC_OBJECT(DWFEModelSection::Factory) );
        _pSectionBuilder->addFactory( DWFCORE_ALLOC_OBJECT(DWFDataSection::Factory) );
        _pSectionBuilder->addFactory( DWFCORE_ALLOC_OBJECT(DWFSignatureSection::Factory) );
        _pSectionBuilder->addFactory( DWFCORE_ALLOC_OBJECT(DWFEPlotGlobalSection::Factory) );
        _pSectionBuilder->addFactory( DWFCORE_ALLOC_OBJECT(DWFEModelGlobalSection::Factory) );
    }
}

void
DWFPackageReader::_getManifestFilename( DWFString& zManifestFilename )
throw( DWFException )
{
    getPackageInfo( _tPackageInfo );

    if ( (_tPackageInfo.eType == eDWFPackage) ||
        (_tPackageInfo.eType == eDWFPackageEncrypted) )
    {
        zManifestFilename = /*NOXLATE*/L"manifest.xml";
    }
    else if (_tPackageInfo.eType == eDWFXPackage
		|| _tPackageInfo.eType == eDWFXPackageEncrypted)
    {
        if(_zCachedDWFXManifestName.chars() == 0)
        {
            //
            // get the package level relationships
            //
            OPCPackage* pOPCPackage = DWFCORE_ALLOC_OBJECT( OPCPackage );
            if (pOPCPackage == NULL)
            {
                _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate package" );
            }

            DWFZipFileDescriptor* pPackageDescriptor = _open();

            OPCZipFileReader* pOPCZipFileReader = DWFCORE_ALLOC_OBJECT( OPCZipFileReader( pPackageDescriptor ) );

            pOPCPackage->readRelationships( pOPCZipFileReader );

            //
            // iterate through the package level rels, looking for the documentsequence relationship
            //
            OPCRelationship::tIterator* piRels = pOPCPackage->relationshipsByType(DWFXXML::kzRelationship_DocumentSequence);
            if (piRels == NULL)
            {
                DWFCORE_FREE_OBJECT( pOPCZipFileReader );
                DWFCORE_FREE_OBJECT( pOPCPackage );
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Unable to locate document sequence part" );
            }

            DWFString zDWFDocumentSeqURI;
            OPCRelationship* pRel = piRels->get();
            zDWFDocumentSeqURI = pRel->targetURI();
            DWFCORE_FREE_OBJECT( piRels );
            DWFCORE_FREE_OBJECT( pOPCPackage );

            //
            // get the document sequence part stream
            //
            DWFPointer<DWFInputStream> apDWFDocumentSeqStream( extract(zDWFDocumentSeqURI, false), false );
            DWFXDWFDocumentSequence* pDWFDocumentSequence = DWFCORE_ALLOC_OBJECT( DWFXDWFDocumentSequence );
            if (pDWFDocumentSequence == NULL)
            {
                DWFCORE_FREE_OBJECT( pOPCZipFileReader );
                _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate document sequence" );
            }

            //
            // read the document sequence part and get the manifest URI
            //
            DWFXMLParser dwfDocumentSeqParser( pDWFDocumentSequence );
            dwfDocumentSeqParser.parseDocument( apDWFDocumentSeqStream );
            DWFOrderedVector<DWFString>::ConstIterator* piDWFDocuments = pDWFDocumentSequence->dwfDocumentURIs();
            if (piDWFDocuments == NULL)
            {
                DWFCORE_FREE_OBJECT( pDWFDocumentSequence );
                DWFCORE_FREE_OBJECT( pOPCZipFileReader );
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Unable to locate manifest" );
            }

            _zCachedDWFXManifestName = piDWFDocuments->get();
            DWFCORE_FREE_OBJECT( piDWFDocuments );
            DWFCORE_FREE_OBJECT( pDWFDocumentSequence );
            DWFCORE_FREE_OBJECT( pOPCZipFileReader );
        }

        zManifestFilename = _zCachedDWFXManifestName;
    }
    else
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Not a package file format" );
    }
}


OPCPart*
DWFPackageReader::_getDWFXManifestPart(bool bInitializeStream, bool bLoadRelationships)
    throw( DWFException )
{
    if (_tPackageInfo.eType != eDWFXPackage)
    {
        return NULL;
    }

    DWFString zManifestFilename;
    _getManifestFilename( zManifestFilename );

    off_t pos = zManifestFilename.findLast(L'/');
    if(pos == -1)
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Unable to parse manifest path" );
    }

    //
    // Create a part for the manifest
    //
    OPCPart* pOPCManifestPart = DWFCORE_ALLOC_OBJECT( OPCPart );
    if (pOPCManifestPart == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate part" );
    }
    pOPCManifestPart->setPath(zManifestFilename.substring(0, pos));
    pOPCManifestPart->setName(zManifestFilename.substring(pos+1));

    if(bInitializeStream)
    {
        // Tell the part to own this stream.
#ifndef DWFTK_READ_ONLY
        pOPCManifestPart->setInputStream(extract(zManifestFilename, false), 0, true);
#endif
    }

    if(bLoadRelationships)
    {
        DWFPointer<DWFInputStream> apRelsStream(extract(pOPCManifestPart->relationshipUri(), false), false);
        pOPCManifestPart->loadRelationships(apRelsStream);
    }

    return pOPCManifestPart;
}

DWFXDWFProperties*
DWFPackageReader::getDWFProperties()
    throw( DWFException )
{
    //
    // Only DWFX can extract DWFX properties.
    //
    if (_tPackageInfo.eType != eDWFXPackage)
    {
        return NULL;
    }

    if(_pDWFXDWFProperties == NULL)
    {
        //
        // Get the manifest part. Ask for relationships to be loaded.
        //
        DWFPointer<OPCPart> apManifestPart(_getDWFXManifestPart(false, true), false);

        //
        // iterate through the manifest rels, looking for the DWFProperties relationship
        //
        DWFPointer<OPCRelationship::tIterator> apiDWFPropRels(apManifestPart->relationshipsByType(DWFXXML::kzRelationship_DWFProperties), false);
        if (!apiDWFPropRels.isNull())
        {
            OPCRelationship* pRel = apiDWFPropRels->get();
            DWFString zDWFPropertiesURI(pRel->targetURI());
            if(zDWFPropertiesURI.chars() > 0)
            {
                _pDWFXDWFProperties = DWFCORE_ALLOC_OBJECT(DWFXDWFProperties);
                if (_pDWFXDWFProperties == NULL)
                {
                    _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate DWFXDWFProperties" );
                }

                DWFPointer<DWFInputStream> apDWFXPropsStream( extract(zDWFPropertiesURI, false), false );
                DWFXMLParser DWFXPropsParser( _pDWFXDWFProperties );
                DWFXPropsParser.parseDocument( apDWFXPropsStream );
            }
        }
    }
    return _pDWFXDWFProperties;
}

DWFXCustomProperties*
DWFPackageReader::getCustomProperties()
    throw( DWFException )
{
    //
    // Only DWFX can extract custom properties.
    //
    if (_tPackageInfo.eType != eDWFXPackage)
    {
        return NULL;
    }

    if(_pDWFXCustomProperties == NULL)
    {
        //
        // Get the manifest part. Ask for relationships to be loaded.
        //
        DWFPointer<OPCPart> apManifestPart(_getDWFXManifestPart(false, true), false);

        //
        // iterate through the manifest rels, looking for the CustomProperties relationship
        //
        DWFPointer<OPCRelationship::tIterator> apiCustomPropRels(apManifestPart->relationshipsByType(DWFXXML::kzRelationship_CustomProperties), false);
        if (!apiCustomPropRels.isNull())
        {
            OPCRelationship* pRel = apiCustomPropRels->get();
            DWFString zCustomPropertiesURI(pRel->targetURI());
            if(zCustomPropertiesURI.chars() > 0)
            {
                _pDWFXCustomProperties = DWFCORE_ALLOC_OBJECT(DWFXCustomProperties);
                if (_pDWFXCustomProperties == NULL)
                {
                    _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate DWFXCustomProperties" );
                }

                DWFPointer<DWFInputStream> apCustomPropsStream( extract(zCustomPropertiesURI, false), false );
                DWFXMLParser customPropsParser( _pDWFXCustomProperties );
                customPropsParser.parseDocument( apCustomPropsStream );
            }
        }
    }
    return _pDWFXCustomProperties;
}

OPCCoreProperties*
DWFPackageReader::getCoreProperties()
    throw( DWFException )
{
    //
    // Only DWFX can extract core properties.
    //
    if (_tPackageInfo.eType != eDWFXPackage)
    {
        return NULL;
    }

    if(_pOPCCoreProperties == NULL)
    {
        //
        // get the package level relationships
        //
        DWFPointer<OPCPackage> apOPCPackage(DWFCORE_ALLOC_OBJECT( OPCPackage ), false);
        if (apOPCPackage.isNull())
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate package" );
        }

        DWFZipFileDescriptor* pPackageDescriptor = _open();

        OPCZipFileReader* pOPCZipFileReader = DWFCORE_ALLOC_OBJECT( OPCZipFileReader( pPackageDescriptor ) );

        apOPCPackage->readRelationships( pOPCZipFileReader );

        //
        // iterate through the package level rels, looking for the CoreProperties relationship
        //
        DWFPointer<OPCRelationship::tIterator> apiRels(apOPCPackage->relationshipsByType(OPCXML::kzRelationship_CoreProperties), false);
        if (apiRels.isNull())
        {
            DWFCORE_FREE_OBJECT( pOPCZipFileReader );
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Unable to locate CoreProperties part" );
        }

        OPCRelationship* pRel = apiRels->get();
        if (pRel == NULL)
        {
            DWFCORE_FREE_OBJECT( pOPCZipFileReader );
            _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Unable to locate CoreProperties part" );
        }

        //
        // Get the CoreProperties part URI.
        //
        DWFString zOPCCorePropertiesURI(pRel->targetURI());
        if(zOPCCorePropertiesURI.chars() > 0)
        {
            //
            // get the CoreProperties part stream
            //
            DWFPointer<DWFInputStream> apOPCCorePropertiesStream( extract(zOPCCorePropertiesURI, false), false );
            if(!apOPCCorePropertiesStream.isNull())
            {
                _pOPCCoreProperties = DWFCORE_ALLOC_OBJECT(OPCCoreProperties);
                if (_pOPCCoreProperties == NULL)
                {
                    _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate OPCCoreProperties" );
                }

                //
                // read the CoreProperties
                //
                DWFXMLParser OPCCorePropertiesParser( _pOPCCoreProperties );
                OPCCorePropertiesParser.parseDocument( apOPCCorePropertiesStream );
            }
            DWFCORE_FREE_OBJECT( pOPCZipFileReader );
        }

    }
    return _pOPCCoreProperties;
}
