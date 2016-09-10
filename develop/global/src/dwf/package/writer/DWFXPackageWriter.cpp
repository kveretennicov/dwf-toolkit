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


#ifndef DWFTK_READ_ONLY

#ifdef HAVE_CONFIG_H
#include "dwf/config.h"
#endif

#include "dwfcore/MIME.h"
#include "dwfcore/Owner.h"
#include "dwfcore/zip/zip.h"
#include "dwfcore/DWFEncryptingInputStream.h"
#include "dwfcore/DWFCompressingInputStream.h"
using namespace DWFCore;

#include "dwf/Version.h"
#include "dwf/package/XML.h"
#include "dwf/package/Manifest.h"
#include "dwf/package/Constants.h"
#include "dwf/package/EPlotSection.h"
#include "dwf/package/EModelSection.h"
#include "dwf/package/GlobalSection.h"
#include "dwf/package/SignatureSection.h"
#include "dwf/package/ContentManager.h"
#include "dwf/package/utility/PropertyContainer.h"
#include "dwf/package/writer/DWFXPackageWriter.h"
#include "dwf/presentation/PackageContentPresentations.h"

#include "dwf/opc/ZipWriter.h"
#include "dwf/opc/CoreProperties.h"
#include "dwf/opc/Constants.h"
#include "dwf/dwfx/Package.h"
#include "dwf/dwfx/DWFDocumentSequence.h"
#include "dwf/dwfx/FixedDocumentSequence.h"
#include "dwf/dwfx/FixedDocument.h"
#include "dwf/dwfx/FixedPage.h"
#include "dwf/dwfx/DWFProperties.h"
#include "dwf/dwfx/CustomProperties.h"
#include "dwf/dwfx/Constants.h"

#include "dwf/opc/Package.h"
#include "dwf/opc/ContentTypes.h"
#include "dwf/opc/ZipFileReader.h"
#include "dwf/xps/XPSFontResourceExtractor.h"
#include "dwf/xps/XPSDocRefResourceExtractor.h"
#include "dwf/xps/XPSFixedDocumentResourceExtractor.h"
#include "dwf/xps/XPSFixedPageResourceExtractor.h"
#include "dwf/package/writer/extensions/7.10x/PackageVersionExtension.h"

using namespace DWFToolkit;


class DWFXProtectedSection : public DWFSection
{
public:
    DWFXProtectedSection( DWFSection* pSection ) : DWFSection(*pSection), _pSection(pSection) {;}
    virtual ~DWFXProtectedSection() throw()
    { 
       if (_pSection)
       {
           DWFCORE_FREE_OBJECT( _pSection );
       }
    }
    DWFSection* section() const { return _pSection; }
public:
    virtual const DWFString& name() const
        throw()
    {
        return _pSection->name();
    }
    virtual void rename( const DWFString& zName )
        throw()
    {
        _pSection->rename( zName );
    }
    const DWFString& type() const
        throw()
    {
        return _pSection->type();
    }
    virtual const DWFString& title() const
        throw()
    {
        return _pSection->title();
    }
    virtual void retitle( const DWFString& zTitle )
        throw()
    {
        _pSection->retitle( zTitle );
    }
    virtual void setLabel( const DWFString& zLabel )
        throw()
    {
        _pSection->setLabel( zLabel );
    }
    virtual const DWFString& label()
        throw()
    {
        return _pSection->label();
    }
    virtual void setLabelIconResource( DWFResource& rResource )
        throw()
    {
        _pSection->setLabelIconResource( rResource );
    }
    virtual void setLabelIconResourceID( const DWFString& zResourceID )
        throw()
    {
        _pSection->setLabelIconResourceID( zResourceID );
    }
    virtual const DWFString& labelIconResourceID()
        throw()
    {
        return _pSection->labelIconResourceID();
    }
    virtual const DWFString& initialURI()
        throw()
    {
        return _pSection->initialURI();
    }
    virtual void setInitialURI( const DWFString& zInitialURI )
        throw()
    {
        _pSection->setInitialURI( zInitialURI );
    }
    virtual const DWFSource& source() const
        throw()
    {
        return _pSection->source();
    }
    virtual void addSource( const DWFSource& rSource )
        throw()
    {
        _pSection->addSource( rSource );
    }

	virtual DWFResourceContainer::ResourceIterator*
	removeResourcesByRole( const DWFString& zRole, bool bDeleteIfOwned )
		throw( DWFException )
	{
		return _pSection->removeResourcesByRole(zRole, bDeleteIfOwned);
	}

    virtual DWFInterface* buildInterface()
        throw( DWFException )
    {
        return _pSection->buildInterface();
    }
    virtual const DWFString& objectID() const
        throw()
    {
        return _pSection->objectID();
    }
    virtual double version() const
        throw()
    {
        return _pSection->version();
    }
    virtual double order() const
        throw()
    {
        return _pSection->order();
    }
    virtual void reorder( double nPlotOrder )
        throw()
    {
        _pSection->reorder( nPlotOrder );
    }
    virtual void setContentManager( DWFContentManager* pContentManager )
        throw()
    {
        _pSection->setContentManager( pContentManager );
    }
    virtual DWFContentManager* getContentManager() const
        throw()
    {
        return _pSection->getContentManager();
    }
    virtual DWFResource* addResource( DWFResource*       pResource,
                                      bool               bOwnResource,
                                      bool               bReplace = true,
                                      bool               bDeleteReplacedIfOwned = true,
                                      const DWFResource* pParentResource = NULL )
        throw( DWFException )
    {
        return _pSection->addResource( pResource, bOwnResource, bReplace, bDeleteReplacedIfOwned, pParentResource );
    }
    virtual
    const DWFResource& readDescriptor( DWFSectionDescriptorReader* pSectionDescriptorReader = NULL ) const
        throw( DWFException )
    {
        return _pSection->readDescriptor( pSectionDescriptorReader );
    }
    virtual 
    void readDescriptor( DWFSectionDescriptorReader& rSectionDescriptorReader,
                         DWFResource&                rResource ) const
        throw( DWFException )
    {
        _pSection->readDescriptor( rSectionDescriptorReader, rResource );
    }
    virtual 
    void readDescriptor( DWFSectionDescriptorReader& rSectionDescriptorReader,
                         DWFInputStream&             rSectionDescriptorStream ) const
        throw( DWFException )
    {
        _pSection->readDescriptor( rSectionDescriptorReader, rSectionDescriptorStream );
    }
    virtual DWFContent* getContentDefinition( DWFResource* pResource, 
                                              DWFContentResourceReader* pReaderFilter = NULL,
                                              bool bLoadContent = true,
											  unsigned int nProviderFlags = DWFContentReader::eProvideAll)
        throw( DWFException )
    {
        return _pSection->getContentDefinition( pResource, pReaderFilter, bLoadContent, nProviderFlags );
    }
    virtual DWFContent::tIterator* getContentDefinition( DWFContentResourceReader* pReaderFilter = NULL,
                                                         bool bLoadContent = true,
														 unsigned int nProviderFlags = DWFContentReader::eProvideAll)
        throw( DWFException )
    {
        return _pSection->getContentDefinition( pReaderFilter, bLoadContent, nProviderFlags );
    }
    virtual void getContentDefinition( DWFContent::tMap& oResourceObjectIDContent,
                                       DWFContentResourceReader* pReaderFilter = NULL,
                                       bool bLoadContent = true,
									   unsigned int nProviderFlags = DWFContentReader::eProvideAll)
        throw( DWFException )
    {
        _pSection->getContentDefinition( oResourceObjectIDContent, pReaderFilter, bLoadContent, nProviderFlags );
    }
    virtual void getContentDefinition( DWFContentResourceReader& rReader,
                                       DWFResource* pResource = NULL )
        throw( DWFException )
    {
        _pSection->getContentDefinition( rReader, pResource );
    }
    virtual void getContentDefinition( DWFContentResourceReader& rReader,
                                       DWFInputStream& rContentResourceStream )
        throw( DWFException )
    {
        _pSection->getContentDefinition( rReader, rContentResourceStream );
    }
    virtual void getAssociatedContentIDs( tStringMultiMap& oResourceIDContentIDMap ) const
        throw()
    {
        _pSection->getAssociatedContentIDs( oResourceIDContentIDMap );
    }
    virtual void updateResourceContentMapping( const DWFString& zResourceObjectID,
                                               const DWFString& zOldContentID,
                                               const DWFString& zNewContentID )
        throw( DWFException )
    {
        _pSection->updateResourceContentMapping( zResourceObjectID, zOldContentID, zNewContentID );
    }
    virtual void updateResourceContentMapping( DWFResource* pResource,
                                               const DWFString& zOldContentID,
                                               const DWFString& zNewContentID )
        throw( DWFException )
    {
        _pSection->updateResourceContentMapping( pResource, zOldContentID, zNewContentID );
    }
    virtual DWFObjectDefinition* getObjectDefinition(unsigned char nProviderFlags = DWFObjectDefinitionReader::eProvideAll) const
        throw( DWFException )
    {
        return _pSection->getObjectDefinition(nProviderFlags);
    }
    virtual DWFObjectDefinition* getObjectDefinition( DWFObjectDefinitionReader& rObjectDefinitionFilter,
                                                      DWFResource&               rResource ) const
        throw( DWFException )
    {
        return _pSection->getObjectDefinition( rObjectDefinitionFilter, rResource );
    }
    virtual 
    void getObjectDefinition( DWFObjectDefinitionReader& rObjectDefinitionReader,
                              DWFResource*               pResource = NULL ) const
        throw( DWFException )
    {
        _pSection->getObjectDefinition( rObjectDefinitionReader, pResource );
    }
    virtual 
    void getObjectDefinition( DWFObjectDefinitionReader& rObjectDefinitionReader,
                              DWFInputStream&            rObjectDefinitionStream ) const
        throw( DWFException )
    {
        _pSection->getObjectDefinition( rObjectDefinitionReader, rObjectDefinitionStream );
    }
    virtual
    void readContentPresentations()
        throw( DWFException )
    {
        _pSection->readContentPresentations();
    }
    virtual
    void readContentPresentations( DWFContentPresentationReader& rContentPresentationFilter,
                                   DWFResource&                  rResource ) const
        throw( DWFException )
    {
        _pSection->readContentPresentations( rContentPresentationFilter, rResource );
    }
    virtual 
    void readContentPresentations( DWFContentPresentationReader& rContentPresentationReader,
                                   DWFResource*                  pResource) const
        throw( DWFException )
    {
        _pSection->readContentPresentations( rContentPresentationReader, pResource );
    }
    virtual 
    void readContentPresentations( DWFContentPresentationReader& rContentPresentationReader,
                                   DWFInputStream&               rContentPresentationStream ) const
        throw( DWFException )
    {
        _pSection->readContentPresentations( rContentPresentationReader, rContentPresentationStream );
    }
    virtual void parseAttributeList( const char** ppAttributeList )
        throw( DWFException )
    {
        _pSection->parseAttributeList( ppAttributeList );
    }
    virtual DWFSection::tBehavior behavior() const
        throw()
    {
        return _pSection->behavior();
    }
    virtual void applyBehavior( const DWFSection::tBehavior& rBehavior )
        throw()
    {
        _pSection->applyBehavior( rBehavior );
    }
    virtual DWFOwner* owner()
        throw( DWFException )
    {
        return _pSection->owner();
    }

#ifndef DWFTK_READ_ONLY
    virtual void serializeXML( DWFXMLSerializer& rSerializer, unsigned int nFlags )
        throw( DWFException )
    {
        _pSection->serializeXML( rSerializer, nFlags );
    }
#endif

private:
    DWFSection* _pSection;
};


    //
    // used for temporary buffer to transfer bytes
    // from a resource to the dwf archive
    //
#define _DWFTK_PACKAGE_WRITER_RESOURCE_STREAM_BUFFER_BYTES  16384

//DNT_Start

_DWFTK_API
DWFXPackageWriter::DWFXPackageWriter( const DWFFile&    rDWFPackageFile,
                                      const DWFString&  zDWFPackagePassword,
                                      DWFXPackageVersionExtension*  pVersionExtension,
                                      bool              bNoPasswordSalting)
throw()
                : DWFPackageWriter( rDWFPackageFile )
                , _zDWFPassword( zDWFPackagePassword )
                , _pPackage( NULL )
                , _oResourcePartMap()
                , _oResourceIDMap()
                , _pCoreProperties( NULL )
                , _pDWFProperties( NULL )
                , _pCustomProperties( NULL )
                , _pProxyGraphicsSection( NULL )
                , _eProxyGraphicsRules( eAlways )
                , _bProxyGraphicsGoOnFirstPage( true )
                , _pVersionExtension( pVersionExtension )

{
    _bNoPasswordSalting = bNoPasswordSalting;
}

_DWFTK_API
DWFXPackageWriter::~DWFXPackageWriter()
throw()
{
    ;
}

_DWFTK_API
void DWFXPackageWriter::addSection( DWFSection*    pSection,
                                    DWFInterface*  pInterface )
throw( DWFException )
{
    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"No section provided" );
    }

    if (_zDWFPassword.chars() > 0)
    {
        DWFPackageWriter::addSection( DWFCORE_ALLOC_OBJECT(DWFXProtectedSection(pSection)), pInterface );
    }
    else
    {
       DWFPackageWriter::addSection( pSection, pInterface );
    }
}

_DWFTK_API
void DWFXPackageWriter::addGlobalSection( DWFGlobalSection* pSection )
throw( DWFException )
{
    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"No section provided" );
    }

    DWFPackageWriter::addGlobalSection( pSection );
}

_DWFTK_API
void DWFXPackageWriter::addProxyGraphicsSection( DWFSection* pSection, teProxyGraphicsRulesType eRules, bool bAsFirstPage )
    throw( DWFException )
{
    if (pSection == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, L"No section provided" );
    }

    _pProxyGraphicsSection          = pSection;
    _eProxyGraphicsRules            = eRules;
    _bProxyGraphicsGoOnFirstPage    = bAsFirstPage;
}


_DWFTK_API
void DWFXPackageWriter::addProxyPage( const DWFString& zProxyPageXPSPath )
    throw( DWFException )
{
	//
    // create a new EPlotSection
    //    
	
	double  fPaperWidth(8.5);
    double  fPaperHeight(11.0);
    double anPaperClip[4] = { 0, 0, 11.0, 8.5 };
	DWFToolkit::DWFPaper proxyPaper( fPaperWidth, fPaperHeight, DWFToolkit::DWFPaper::eInches, 0x00ffffff, (const double*)anPaperClip );

    DWFSource oPlotSource( /*NOXLATE*/L"DWFXPackageWriter",
                           /*NOXLATE*/L"DWFToolkit",
                           /*NOXLATE*/L"" );

    DWFEPlotSection* pPlotSection = 
        DWFCORE_ALLOC_OBJECT( DWFEPlotSection( /*NOXLATE*/L"Proxy Page", 
                                               /*NOXLATE*/L"",
                                               1,
                                               oPlotSource,
                                               0, // not used
                                               &proxyPaper) );

	if (pPlotSection == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to add the new proxy plot" );
    }

	//
	// forward definition of graphic resource.
	//
	DWFGraphicResource* pGraphicResource = NULL;

	//
	// Extract fixed page and font resources from the soure XPS file.
	//
    DWFFile oDWF( zProxyPageXPSPath );

	DWFZipFileDescriptor* pPackageDescriptor = NULL;

	pPackageDescriptor = DWFCORE_ALLOC_OBJECT( DWFZipFileDescriptor(oDWF, DWFZipFileDescriptor::eUnzip) );
    
    if (pPackageDescriptor == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate file descriptor" );
    }

	DWFZipFileIndex         _oZipFileIndex;
    try
    {
        pPackageDescriptor->openIndexed(&_oZipFileIndex);
    }
    catch (DWFInvalidTypeException& e)
    {
		//
		// the source proxy page xps file doesn't exist or failed to open.
		//
        DWFCORE_FREE_OBJECT( pPackageDescriptor );
        throw e;
    }

	OPCZipFileReader* pOPCZipFileReader = DWFCORE_ALLOC_OBJECT( OPCZipFileReader( pPackageDescriptor ) );

	OPCPackage* pOPCPackage = DWFCORE_ALLOC_OBJECT( OPCPackage );
    if (pOPCPackage == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate package" );
    }

    pOPCPackage->readRelationships( pOPCZipFileReader );

    //
    // documentsequence targetURI.
    //
	DWFString zFDocSeq(/*NOXLATE*/L"\\FixedDocumentSequence.fdseq");
	DWFInputStream* pArchiveStream = NULL;
	pArchiveStream = pPackageDescriptor->unzip( zFDocSeq );    

	XPSDocRefResourceExtractor* pResourceExtractor = 
					DWFCORE_ALLOC_OBJECT(XPSDocRefResourceExtractor( pArchiveStream, true ) );
    if (pResourceExtractor == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Unable to allocate document reference resource extractor" );
    }

    pResourceExtractor->parse();

	DWFOrderedVector<DWFString>::ConstIterator* iter = pResourceExtractor->documentURIs();
	for(; iter->valid(); iter->next())
	{
		DWFString zFixedDoc = iter->get();
		pArchiveStream = pPackageDescriptor->unzip(zFixedDoc);
	
		XPSFixedDocumentResourceExtractor* pFDResourceExtractor = 
				DWFCORE_ALLOC_OBJECT(XPSFixedDocumentResourceExtractor( pArchiveStream, true ) );
		if (pFDResourceExtractor == NULL)
		{
			_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Unable to allocate fixed document resource extractor" );
		}
		
		pFDResourceExtractor->parse();

		DWFOrderedVector<DWFString>::ConstIterator* iter1 = pFDResourceExtractor->documentURIs();

		for(; iter1->valid(); iter1->next())
		{
			DWFString zFPageURI = iter1->get();

			pArchiveStream = pPackageDescriptor->unzip(zFPageURI);
		    
			XPSFixedPageResourceExtractor* pFixedPageResourceExtractor = 
					DWFCORE_ALLOC_OBJECT(XPSFixedPageResourceExtractor( pArchiveStream, true ) );
			if (pFixedPageResourceExtractor == NULL)
			{
				_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Unable to allocate fixed page resource extractor" );
			}
			
			pFixedPageResourceExtractor->parse();

			//
			// Produce the FixedPage content.
			// 

			//
			// create the graphic resource.
			//
			pGraphicResource = 
			DWFCORE_ALLOC_OBJECT( DWFGraphicResource( /*NOXLATE*/L"ProxyFixedPage",
													  DWFXML::kzRole_Graphics2d,
													  DWFMIME::kzMIMEType_FIXEDPAGEXML) );

			DWFUUID oObjectID;
			pGraphicResource->setObjectID(oObjectID.uuid(true));

			double anTransform[4][4] = { {1/96.0, 0, 0, 0}, 
						     {0, 1/96.0, 0, 0},
						     {0, 0, 1/96.0, 0}, 
						     {0, 0, 0, 1/96.0} };

			pGraphicResource->configureGraphic( (const double*)anTransform, NULL, (const double*)anPaperClip );

			//
			// bind the streams
			//
			pGraphicResource->setInputStream( pFixedPageResourceExtractor->getResourceStream() );

		
			pPlotSection->addResource( pGraphicResource, true );
	
			//DWFXFixedPageResourceExtractor
			off_t iOffset = zFPageURI.findLast( /*NOXLATE*/L'/' );

			DWFString fileName;
			DWFString fRelsURI;
			if (iOffset != -1 &&
				size_t(iOffset+1) != zFPageURI.chars())
			{
				size_t iExtLen = zFPageURI.chars()-(iOffset+1);
				fileName = zFPageURI.substring( iOffset+1, iExtLen);
			
				fRelsURI = zFPageURI.substring(0, iOffset + 1);
				fRelsURI.append(/*NOXLATE*/L"_rels/");
				fRelsURI.append(fileName);
				fRelsURI.append(/*NOXLATE*/L".rels");
			}

			pArchiveStream = pPackageDescriptor->unzip(fRelsURI);

			XPSFontResourceExtractor* pXPSFontResourceExtractor = 
					DWFCORE_ALLOC_OBJECT(XPSFontResourceExtractor( pArchiveStream, true ) );
			if (pXPSFontResourceExtractor == NULL)
			{
				_DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Unable to allocate fixed page resource extractor" );
			}
			
			pXPSFontResourceExtractor->parse();

			DWFOrderedVector<DWFString>::ConstIterator* iter2 = pXPSFontResourceExtractor->documentURIs();

			for(; iter2->valid(); iter2->next())
			{
				DWFString zFontURI = iter2->get();

			    pArchiveStream = pPackageDescriptor->unzip(zFontURI);

				off_t iOffset = zFontURI.findLast( '/' );

				DWFString fileName;
				if (iOffset != -1 &&
					size_t(iOffset+1) != zFontURI.chars())
				{
					size_t iExtLen = zFontURI.chars()-(iOffset+1);
					fileName = zFontURI.substring( iOffset+1, iExtLen);
				}

		        //
				// self-managed temp file, the odttf file.
				//
				DWFTempFile* pFontFile = DWFTempFile::Create( fileName, true );
				DWFOutputStream& rFileStream = pFontFile->getOutputStream();

#define _DWFCRAWL_BUFFER_BYTES 16385

				char zBuffer[_DWFCRAWL_BUFFER_BYTES] = {0};
				size_t nRead = 0;
				while (pArchiveStream->available() > 0)
				{
					DWFCORE_ZERO_MEMORY( zBuffer, _DWFCRAWL_BUFFER_BYTES );
					nRead = pArchiveStream->read( zBuffer, _DWFCRAWL_BUFFER_BYTES-1 );
					if(nRead==0)
						break;
					else
						rFileStream.write(zBuffer, nRead);
				}
				rFileStream.flush();

				DWFToolkit::DWFResource *pFontResource = 
				DWFCORE_ALLOC_OBJECT(
					DWFToolkit::DWFResource(
					/*NOXLATE*/L"Font File",
					DWFXML::kzRole_Font, 
					DWFCore::DWFMIME::kzMIMEType_OBFUSCATEDOPENTYPE
					)
				);
			
				pFontResource->setInputStream( pFontFile->getInputStream() );
				pFontResource->setRequestedName( fileName );

                pPlotSection->addResource( pFontResource, true );

                DWFCORE_FREE_OBJECT(pFontFile);
                DWFCORE_FREE_OBJECT(pArchiveStream);

            }

            DWFCORE_FREE_OBJECT(iter2);
            DWFCORE_FREE_OBJECT(pXPSFontResourceExtractor);
            DWFCORE_FREE_OBJECT(pFixedPageResourceExtractor);
        }

        DWFCORE_FREE_OBJECT(iter1);
        DWFCORE_FREE_OBJECT(pFDResourceExtractor);

    }

    DWFCORE_FREE_OBJECT(iter);
    DWFCORE_FREE_OBJECT(pResourceExtractor);

    DWFCORE_FREE_OBJECT(pOPCPackage);
    DWFCORE_FREE_OBJECT(pOPCZipFileReader);

    //
    // finally, drop the page into the writer
    //

    //
    //  Set the predefined name
    //
	DWFUUID oObjectID;
    pPlotSection->rename( oObjectID.uuid(false) );

    //
    //  Prevent any renaming
    //
    DWFSection::tBehavior behavior = pPlotSection->behavior();
    behavior.bRenameOnPublish = false;
    pPlotSection->applyBehavior( behavior );

	DWFResource* pDWFResource = NULL;
    DWFKVIterator<const wchar_t*, DWFResource*>* piAllResources = pPlotSection->getResourcesByRole();
	
    if (piAllResources != NULL && pGraphicResource != NULL)
    {
        for (; piAllResources->valid(); piAllResources->next())
        {
            //
			// add relationships to the resources in the DWF Plot
			//			
			pDWFResource = piAllResources->value();
			if( pDWFResource->role() == DWFXML::kzRole_Graphics2dExtension )
			{
				pGraphicResource->addRelationship( pDWFResource, DWFXXML::kzRelationship_Graphics2dExtensionResource  );
			}
			else if( pDWFResource->role() == DWFXML::kzRole_Graphics2dDictionary )
			{
				pGraphicResource->addRelationship( pDWFResource, DWFXXML::kzRelationship_Graphics2dDictionaryResource );
			}
			else if( pDWFResource->role() == DWFXML::kzRole_ObservationMesh )
			{
				pGraphicResource->addRelationship( pDWFResource, DWFXXML::kzRelationship_ObservationMeshResource);
			}
			else if( pDWFResource->role() == DWFXML::kzRole_GeographicCoordinateSystem )
			{
				pGraphicResource->addRelationship( pDWFResource, DWFXXML::kzRelationship_GeographicCoordinateSystemResource );
			}
			else if( pDWFResource->role() == DWFXML::kzRole_RasterReference )
			{
				pGraphicResource->addRelationship( pDWFResource, DWFXXML::kzRelationship_RasterReferenceResource );
			}
			else if( pDWFResource->role() == DWFXML::kzRole_Font )
			{
				pGraphicResource->addRelationship( pDWFResource, DWFXXML::kzRelationship_FontResource );
			}
		}
	}
	
    DWFCORE_FREE_OBJECT(piAllResources);

	DWFXPackageWriter::teProxyGraphicsRulesType rulesType = DWFXPackageWriter::eIfOnlyPartialPageOutput;

    addProxyGraphicsSection( pPlotSection, rulesType );
}


_DWFTK_API
void DWFXPackageWriter::setCoreProperties( DWFPropertySet* pSet )
throw( DWFException )
{
    if (pSet->getSchemaID() != OPCXML::kzSchemaID_CoreProperties)
    {
        _DWFCORE_THROW( DWFUnexpectedException, L"The PropertySet does not have the CoreProperties SchemaID" );
    }

    if (_pCoreProperties == NULL)
    {
       _pCoreProperties = DWFCORE_ALLOC_OBJECT( OPCCoreProperties() );
       if (_pCoreProperties == NULL)
       {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate CoreProperties" );
       }
    }

    unsigned int nFound = 0;
    DWFProperty::tMap::Iterator* piProperties = pSet->getProperties();
    if (piProperties)
    {
        for (; piProperties->valid(); piProperties->next())
        {
            DWFString& zName = piProperties->key();
            DWFProperty* pProperty = piProperties->value();

            if (!(nFound & 0x00001) && (zName == OPCXML::kzCoreProperty_Keywords))
            {
                _pCoreProperties->setPropertyKeywords( pProperty->value() );
                nFound |= 0x0001;
            }

            else if (!(nFound & 0x0002) && (zName == OPCXML::kzCoreProperty_ContentType))
            {
                _pCoreProperties->setPropertyContentType( pProperty->value() );
                nFound |= 0x0002;
            }

            else if (!(nFound & 0x0004) && (zName == OPCXML::kzCoreProperty_Category))
            {
                _pCoreProperties->setPropertyCategory( pProperty->value() );
                nFound |= 0x0004;
            }

            else if (!(nFound & 0x0008) && (zName == OPCXML::kzCoreProperty_Version))
            {
                _pCoreProperties->setPropertyVersion( pProperty->value() );
                nFound |= 0x0008;
            }

            else if (!(nFound & 0x0010) && (zName == OPCXML::kzCoreProperty_Revision))
            {
                _pCoreProperties->setPropertyRevision( pProperty->value() );
                nFound |= 0x0010;
            }

            else if (!(nFound & 0x0020) && (zName == OPCXML::kzCoreProperty_LastModifiedBy))
            {
                _pCoreProperties->setPropertyLastModifiedBy( pProperty->value() );
                nFound |= 0x0020;
            }

            else if (!(nFound & 0x0040) && (zName == OPCXML::kzCoreProperty_LastPrinted))
            {
                _pCoreProperties->setPropertyLastPrinted( pProperty->value() );
                nFound |= 0x0040;
            }

            else if (!(nFound & 0x0080) && (zName == OPCXML::kzCoreProperty_ContentStatus))
            {
                _pCoreProperties->setPropertyContentStatus( pProperty->value() );
                nFound |= 0x0080;
            }

            else if (!(nFound & 0x0100) && (zName == OPCXML::kzCoreProperty_Creator))
            {
                _pCoreProperties->setPropertyCreator( pProperty->value() );
                nFound |= 0x0100;
            }

            else if (!(nFound & 0x0200) && (zName == OPCXML::kzCoreProperty_Identifier))
            {
                _pCoreProperties->setPropertyIdentifier( pProperty->value() );
                nFound |= 0x0200;
            }

            else if (!(nFound & 0x0400) && (zName == OPCXML::kzCoreProperty_Title))
            {
                _pCoreProperties->setPropertyTitle( pProperty->value() );
                nFound |= 0x0400;
            }

            else if (!(nFound & 0x0800) && (zName == OPCXML::kzCoreProperty_Subject))
            {
                _pCoreProperties->setPropertySubject( pProperty->value() );
                nFound |= 0x0800;
            }

            else if (!(nFound & 0x1000) && (zName == OPCXML::kzCoreProperty_Description))
            {
                _pCoreProperties->setPropertyDescription( pProperty->value() );
                nFound |= 0x1000;
            }

            else if (!(nFound & 0x2000) && (zName == OPCXML::kzCoreProperty_Language))
            {
                _pCoreProperties->setPropertyLanguage( pProperty->value() );
                nFound |= 0x2000;
            }

            else if (!(nFound & 0x4000) && (zName == OPCXML::kzCoreProperty_Created))
            {
                _pCoreProperties->setPropertyCreated( pProperty->value() );
                nFound |= 0x4000;
            }

            else if (!(nFound & 0x8000) && (zName == OPCXML::kzCoreProperty_Modified))
            {
                _pCoreProperties->setPropertyModified( pProperty->value() );
                nFound |= 0x8000;
            }
        }

        DWFCORE_FREE_OBJECT( piProperties );
    }
}

_DWFTK_API
void DWFXPackageWriter::setDWFProperties( DWFPropertySet* pSet )
throw( DWFException )
{
    if (pSet->getSchemaID() != DWFXXML::kzSchemaID_DWFProperties)
    {
        _DWFCORE_THROW( DWFUnexpectedException, L"The PropertySet does not have the CoreProperties SchemaID" );
    }

    if (_pDWFProperties == NULL)
    {
       _pDWFProperties = DWFCORE_ALLOC_OBJECT( DWFXDWFProperties() );
       if (_pDWFProperties == NULL)
       {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate DWFProperties" );
       }
    }

    unsigned int nFound = 0;
    DWFProperty::tMap::Iterator* piProperties = pSet->getProperties();
    if (piProperties)
    {
        for (; piProperties->valid(); piProperties->next())
        {
            DWFString& zName = piProperties->key();
            DWFProperty* pProperty = piProperties->value();

            if (!(nFound & 0x00001) && (zName == DWFXML::kzDWFProperty_SourceProductVendor))
            {
                _pDWFProperties->setSourceProductVendor( pProperty->value() );
                nFound |= 0x0001;
            }

            else if (!(nFound & 0x0002) && (zName == DWFXML::kzDWFProperty_SourceProductName))
            {
                _pDWFProperties->setSourceProductName( pProperty->value() );
                nFound |= 0x0002;
            }

            else if (!(nFound & 0x0004) && (zName == DWFXML::kzDWFProperty_SourceProductVersion))
            {
                _pDWFProperties->setSourceProductVersion( pProperty->value() );
                nFound |= 0x0004;
            }

            else if (!(nFound & 0x0008) && (zName == DWFXML::kzDWFProperty_DWFProductVendor))
            {
                _pDWFProperties->setDWFProductVendor( pProperty->value() );
                nFound |= 0x0008;
            }

            else if (!(nFound & 0x0010) && (zName == DWFXML::kzDWFProperty_DWFProductVersion))
            {
                _pDWFProperties->setDWFProductVersion( pProperty->value() );
                nFound |= 0x0010;
            }

            else if (!(nFound & 0x0020) && (zName == DWFXML::kzDWFProperty_DWFToolkitVersion))
            {
                _pDWFProperties->setDWFToolkitVersion( pProperty->value() );
                nFound |= 0x0020;
            }

			else if (!(nFound & 0x0040) && (zName == DWFXML::kzDWFProperty_PasswordEncryptedDocument))
            {
                _pDWFProperties->setPasswordProtected( pProperty->value() );
                nFound |= 0x0040;
            }

            else if (!(nFound & 0x0080) && (zName == DWFXML::kzDWFProperty_DWFFormatVersion))
            {
                _pDWFProperties->setDWFFormatVersion( pProperty->value() );
                nFound |= 0x0080;
            }

        }
        DWFCORE_FREE_OBJECT( piProperties );
    }
}

_DWFTK_API
void DWFXPackageWriter::setCustomProperties( DWFPropertySet* pSet )
throw( DWFException )
{
    //
    //  The documentation of this method in PackageWriter mentions that CustomProperties does
    //  not support nested property sets. This is for backwards compatability reasons.
    //
    if (_pCustomProperties == NULL)
    {
       _pCustomProperties = DWFCORE_ALLOC_OBJECT( DWFXCustomProperties() );
       if (_pCustomProperties == NULL)
       {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate Custom Properties" );
       }
    }

    DWFProperty::tMap::Iterator* piProperties = pSet->getProperties();
    if (piProperties)
    {
        for (; piProperties->valid(); piProperties->next())
        {
            DWFProperty* pProperty = piProperties->value();
            _pCustomProperties->addProperty( pProperty, false );
        }
        DWFCORE_FREE_OBJECT( piProperties );
    }
}
    
_DWFTK_API
void
DWFXPackageWriter::write( const DWFString&                   zSourceProductVendor,
                          const DWFString&                   zSourceProductName,
                          const DWFString&                   zSourceProductVersion,
                          const DWFString&                   zDWFProductVendor,
                          const DWFString&                   zDWFProductVersion,
                          DWFZipFileDescriptor::teFileMode   eCompressionMode )
throw( DWFException )
{

        //
        // if we are aggregating, do this before using or serializing the content
        // information.
        //
    if (_bAggregating)
    {
        _aggregateContent();
    }
    
    if (_pPackageManifest == NULL)
    {
        _DWFCORE_THROW( DWFNullPointerException, /*NOXLATE*/L"Cannot write package - no manifest exists" );
    }
        //
        // open the package
        //
    if (_pPackage == NULL)
    {
        _pPackage = DWFCORE_ALLOC_OBJECT( DWFXPackage() );

        if (_pPackage == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate DWFX package" );     
        }

        if (_pCoreProperties != NULL)
        {
            //
            //  The package takes ownership of the properties part
            //
            _pPackage->setCoreProperties( _pCoreProperties, true );
        }
    }

    //
    // create the DocumentSequence
    //
    DWFXDWFDocumentSequence* pDocumentSequence = DWFCORE_ALLOC_OBJECT( DWFXDWFDocumentSequence() );
    _pPackage->setDWFDocumentSequence( pDocumentSequence );

    //
    // create the only (for now) document
    //
    DWFXDWFDocument* pDocument = DWFCORE_ALLOC_OBJECT( DWFXDWFDocument( _pPackageManifest ) );
    pDocumentSequence->addDWFDocument( pDocument );

    //
    //  This sets the toolkit version in DWFProperties.
    //  It also supports the old API where DWF properties were entered using arguments
    //  to write(). These values will go into DWF properties. 
    //
    _updateProperties( zSourceProductVendor, zSourceProductName, zSourceProductVersion, 
                          zDWFProductVendor, zDWFProductVersion );

    if (_pDWFProperties != NULL)
    {
        //
        //  The document takes ownership of the properties part
        //
        pDocument->setDWFProperties( _pDWFProperties, true );
    }

    if (_pCustomProperties != NULL)
    {
        //
        //  The document takes ownership of the properties part
        //
        pDocument->setCustomProperties( _pCustomProperties, true );
    }

    _pPackageManifest->provideContentPresentations( _pPackageContentPresentations );

    //
    // add all the contents in the content manager to the document.
    //
    DWFContentManager* pContentManager = _pPackageManifest->getContentManager();
    if (pContentManager)
    {
        DWFContent::tMap::Iterator* piContents = pContentManager->getContents();

        if (piContents)
        {
            DWFContent* pContent = NULL;
            for (; piContents->valid(); piContents->next())
            {
                pContent = piContents->value();

                //
                // If Toolkit consume application has not loaded the "content", the dwfx file will lose the content information after saving. 
                // So here we will force loading the content before saving
                //
                if( pContent->isLoaded() == false )
                {
                    pContent->load();
                }

                //
                // TODO - ensure DWFXContentPart will do the right thing depending on whether
                // content is loaded or not
                //
                pContent->setHRef( L"" );
                DWFXContentPart* pContentPart = DWFCORE_ALLOC_OBJECT( DWFXContentPart( pContent ) );
                pDocument->addContentPart( pContentPart );
            }

            DWFCORE_FREE_OBJECT( piContents );
        }
    }

    //
    // add the document level presentations to the document
    //
    if (_pPackageContentPresentations->presentationCount() > 0)
    {
        DWFXDocumentPresentationsPart* pDocumentPresentationsPart = DWFCORE_ALLOC_OBJECT( DWFXDocumentPresentationsPart( _pPackageContentPresentations ) );
        pDocument->addDocumentPresentationsPart( pDocumentPresentationsPart );
    }

    DWFString zSection;
    DWFSection* pSection = NULL;

    DWFSection::tBehavior tSectionBehavior;

    //
    // Add any global sections first
    //
    DWFGlobalSection::tList::const_iterator iGlobalSection;

    for (iGlobalSection = _oGlobalSections.begin();
         iGlobalSection!= _oGlobalSections.end();
         iGlobalSection++)
    {
        pSection = *iGlobalSection;

        //
        // Grab the behavior from the section. In 7.1, this was on CustomSection.
        // In 7.2, we've promoted it to Section in general.
        //
        tSectionBehavior = pSection->behavior();

        DWFXDWFSection* pDescriptorPart(NULL);
            //
            // manage section descriptor
            //
        if (tSectionBehavior.bPublishDescriptor)
        {
            //
            // create the descriptor part
            //
            pDescriptorPart = DWFCORE_ALLOC_OBJECT( DWFXDWFSection( pSection ) );
            pDocument->addDWFSection( pDescriptorPart );
        }

        //
        // resources
        //
        DWFResource* pResource = NULL;
        DWFKVIterator<const wchar_t*, DWFResource*>* piAllResources = pSection->getResourcesByRole();

        if (piAllResources)
        {
            for (; piAllResources->valid(); piAllResources->next())
            {
                pResource = piAllResources->value();

                _oResourceIDMap.insert( std::make_pair(pResource->objectID(), pResource) );
                if (pResource->role() != DWFXML::kzRole_Descriptor)
                {
                    _addResourcePart( *pResource, pDescriptorPart );
                }
            }

            DWFCORE_FREE_OBJECT( piAllResources );
        }
    }

    //
    // Check if proxy graphics are required.
    //
    bool bNeedProxyGraphics(false);
    if(_pProxyGraphicsSection != NULL)
    {
        if(_eProxyGraphicsRules == eAlways)
        {
            bNeedProxyGraphics = true;
        }
        else if(_eProxyGraphicsRules == eIfNoPageOutput)
        {
            bNeedProxyGraphics = true;
            DWFSection::tList::const_iterator iSection;
            for (iSection = _oSections.begin();
                 iSection!= _oSections.end();
                 iSection++)
            {
                pSection = *iSection;
                if(_fixedPageDesired( pSection ))
                {
                    bNeedProxyGraphics = false;
                    break;
                }
            }
        }
        else if(_eProxyGraphicsRules == eIfOnlyPartialPageOutput)
        {
            bNeedProxyGraphics = false;
            DWFSection::tList::const_iterator iSection;
            for (iSection = _oSections.begin();
                 iSection!= _oSections.end();
                 iSection++)
            {
                pSection = *iSection;
                if(!_fixedPageDesired( pSection ))
                {
                    bNeedProxyGraphics = true;
                    break;
                }
            }
        }
    }

    if(bNeedProxyGraphics && _bProxyGraphicsGoOnFirstPage)
    {
        _writeSection(_pProxyGraphicsSection, pDocument);
    }

        //
        // Add the non-global sections
        //
    DWFSection::tList::const_iterator iSection;
    for (iSection = _oSections.begin();
         iSection!= _oSections.end();
         iSection++)
    {
        pSection = *iSection;

        DWFXProtectedSection* pProtected = dynamic_cast<DWFXProtectedSection*>(pSection);
        if (pProtected)
        {
            pSection = pProtected->section();

            //
            // resources
            //
            DWFResource* pResource = NULL;
            DWFKVIterator<const wchar_t*, DWFResource*>* piAllResources = pSection->getResourcesByRole();

            if (piAllResources)
            {
                for (; piAllResources->valid(); piAllResources->next())
                {
                    pResource = piAllResources->value();

                    DWFString zMime = pResource->mime();
					if (pResource->role() != DWFXML::kzRole_Descriptor)
                    {
                        
						//
						// Only encrypt the stream if there is a password
						//
						if (_zDWFPassword.chars() > 0)
						{
							if(zMime.substring(zMime.chars() - 2) != L"-p")   //append only once
                            {                            
                                zMime.append( L"-p" );
                            }
							pResource->setMIME( zMime );
							
							DWFInputStream* pOriginal = pResource->getInputStream();
						    if (pOriginal)
						    {
							    pOriginal = DWFCORE_ALLOC_OBJECT(DWFCompressingInputStream(pOriginal, 
								    eCompressionMode, true));
							    pResource->setInputStream( DWFCORE_ALLOC_OBJECT(DWFEncryptingInputStream(pOriginal, _zDWFPassword)) );
						    }
						}
                    }
                }
                DWFCORE_FREE_OBJECT( piAllResources );
    		}
        }
        
        _writeSection(pSection, pDocument);
    }

    if(bNeedProxyGraphics)
    {
        if(!_bProxyGraphicsGoOnFirstPage)
        {
            _writeSection(_pProxyGraphicsSection, pDocument);
        }
        _addRelationships(*_pProxyGraphicsSection);
    }

    //
    //  This must be called after all resources have been added.
    //
    _addResourceRelationships();

    //
    //  Add undeclared package relationships 
    //
    //  IMPORTANT: This object needs to stay in the same runtime scope as _pPackage,
    //  as it temporarily owns OPCPart objects used by _pPackage until serialization
    //
    DWFXUndeclaredPackageRelationshipsReceiver oRelationshipsReceiver( _pPackage ); 
    if ( _pUndeclaredPackageRelationshipsProvider != NULL )
    {
        _pUndeclaredPackageRelationshipsProvider->provideRelationships( &oRelationshipsReceiver );
    }

    //
    //  Add undeclared resources' content types (those resources added to 
    //  the OPC package that are not declared in the DWF manifest / descriptor 
    //  hierarchy. )
    //
    if ( _pUndeclaredContentTypeProvider != NULL )
    {
        DWFXUndeclaredContentTypeReceiver oContentTypeReceiver( _pPackage );
        _pUndeclaredContentTypeProvider->provideContentTypes( &oContentTypeReceiver );
    }

    DWFZipFileDescriptor* pZipFD = DWFCORE_ALLOC_OBJECT( DWFZipFileDescriptor( _rDWFPackage, eCompressionMode ) );
    pZipFD->open();
    OPCZipWriter oZipper( pZipFD);
    _pPackage->serialize( oZipper );
    
    //
    //  if the package is encrypted, after storing the package to the disk, the resource streams in the memory 
    //  still have suffix '-p' in their MIME type. So we must remove the '-p' from the resource streams 
    //  in order to read the stream correctly again later.
    //
    for (iSection = _oSections.begin();
        iSection!= _oSections.end();
        iSection++)
    {
        pSection = *iSection;

        DWFXProtectedSection* pProtected = dynamic_cast<DWFXProtectedSection*>(pSection);
        if (pProtected)
        {
            pSection = pProtected->section();

            //
            // resources
            //
            DWFResource* pResource = NULL;
            DWFKVIterator<const wchar_t*, DWFResource*>* piAllResources = pSection->getResourcesByRole();

            if (piAllResources)
            {
                for (; piAllResources->valid(); piAllResources->next())
                {
                    pResource = piAllResources->value();

                    DWFString zMime = pResource->mime();
                    if (pResource->role() != DWFXML::kzRole_Descriptor)
                    {

                        //
                        // Only decrypt the stream mime type if there is a password
                        //
                        if (_zDWFPassword.chars() > 0)
                        {
                            if(zMime.substring(zMime.chars() - 2) == L"-p")  
                            {      
                                size_t nLen = zMime.chars();
                                wchar_t* pMIME = DWFCORE_ALLOC_MEMORY( wchar_t, nLen+1 );
                                DWFCORE_WIDE_STRING_COPY( pMIME, (const wchar_t*)zMime );
                                pMIME[nLen-2] = 0;
                                zMime.assign( pMIME );
                                DWFCORE_FREE_MEMORY( pMIME );  

                            }

                            pResource->setMIME( zMime );
                        }
                    }
                }

                DWFCORE_FREE_OBJECT( piAllResources );
            }
        }
    }

    //
    //  Add undeclared resources (those resources added to the package that
    //  are not declared in the manifest / descriptor hierarchy. )
    //
    if ( _pUndeclaredResourceProvider != NULL )
    {
        DWFXUndeclaredResourceReceiver oResourceReceiver( &oZipper );
        _pUndeclaredResourceProvider->provideResources( &oResourceReceiver );
    }

    // 
    //  Cleanup
    //

    if (_pPackage)
    {
        DWFCORE_FREE_OBJECT( _pPackage );
    }

    if(_pProxyGraphicsSection)
    {
        DWFCORE_FREE_OBJECT( _pProxyGraphicsSection );
    }
}

void
DWFXPackageWriter::_writeSection( DWFSection* pSection, DWFXDWFDocument* pDocument )
    throw( DWFException )
{
    //
    // Grab the behavior from the section. In 7.1, this was on CustomSection.
    // In 7.2, we've promoted it to Section in general.
    //
    DWFSection::tBehavior tSectionBehavior(pSection->behavior());

    DWFXDWFSection* pDescriptorPart( NULL );
    DWFXProtectedSection* pProtected = dynamic_cast<DWFXProtectedSection*>(pSection);

        //
        // manage section descriptor
        //
    if (tSectionBehavior.bPublishDescriptor)
    {
        //
        // create the descriptor part
        //
        pDescriptorPart = DWFCORE_ALLOC_OBJECT( DWFXDWFSection( pProtected ? pProtected->section() : pSection ) );
        pDocument->addDWFSection( pDescriptorPart );
    }


    //
    //  This will iterate over the section resources and create a fixed page for it
    //  if the underlying resources are fixed-page compatible, or if the section
    //  requests an XPS fixed page for alternate content. 
    //
    //  Note: In some cases this will return NULL in which we will have a section
    //  that will not be visible in an XPS viewer.
    //
    DWFXFixedPage* pFixedPage = pProtected ? NULL : _createFixedPage( pSection );


    //
    // Add certain resources in-order.
    //
        //
        // thumbnails
        //
    _addResourcePartsByRole( DWFXML::kzRole_Thumbnail, pDescriptorPart, pFixedPage );
        //
        // preview
        //
    _addResourcePartsByRole( DWFXML::kzRole_Preview, pDescriptorPart, pFixedPage );
        //
        // fonts
        //
    _addResourcePartsByRole( DWFXML::kzRole_Font, pDescriptorPart, pFixedPage );
        //
        // dictionary
        //
    _addResourcePartsByRole( DWFXML::kzRole_Graphics2dDictionary, pDescriptorPart, pFixedPage );
        //
        // raster reference
        //
    _addResourcePartsByRole( DWFXML::kzRole_RasterReference, pDescriptorPart, pFixedPage );
        //
        // w2d or FPXML
        //
    _addResourcePartsByRole( DWFXML::kzRole_Graphics2d, pDescriptorPart, pFixedPage );
        //
        // textures
        //
    _addResourcePartsByRole( DWFXML::kzRole_Texture, pDescriptorPart, pFixedPage );
        //
        // w3d
        //
    _addResourcePartsByRole( DWFXML::kzRole_Graphics3d, pDescriptorPart, pFixedPage );

    //
    // remaining resources
    //
    DWFResource* pResource = NULL;
    DWFPointer<DWFKVIterator<const wchar_t*, DWFResource*> > piAllResources( pSection->getResourcesByRole(), false );
    if (!piAllResources.isNull())
    {
        for (; piAllResources->valid(); piAllResources->next())
        {
            pResource = piAllResources->value();
            _oResourceIDMap.insert( std::make_pair(pResource->objectID(), pResource) );

                //
                // if someone can think of a faster, cleaner way of doing this, please fix it
                //
            if ((pResource->role() != DWFXML::kzRole_Descriptor)                &&
                (pResource->role() != DWFXML::kzRole_Thumbnail)                 &&
                (pResource->role() != DWFXML::kzRole_Preview)                   &&
                (pResource->role() != DWFXML::kzRole_Font)                      &&
                (pResource->role() != DWFXML::kzRole_Graphics2dDictionary)      &&
                (pResource->role() != DWFXML::kzRole_RasterReference)           &&
                (pResource->role() != DWFXML::kzRole_Graphics2d)                &&
                (pResource->role() != DWFXML::kzRole_Texture)                   &&
                (pResource->role() != DWFXML::kzRole_Graphics3d))
            {
                _addResourcePart( *pResource, pDescriptorPart, pFixedPage );
            }
        }
    }
}

void
DWFXPackageWriter::_addResourcePartsByRole( const DWFString& zRole,
                                            DWFXDWFSection* pDescriptorPart,
                                            DWFXFixedPage* pFixedPage )
throw( DWFException )
{
    DWFSection* pSection = pDescriptorPart->section();

    //
    // Put the iterator in a smart pointer to ensure exceptions don't cause trouble
    //
    DWFPointer<DWFResource::tIterator> piResources( pSection->findResourcesByRole( zRole), false );

    if (!piResources.isNull())
    {
        for (; piResources->valid(); piResources->next())
        {
            _addResourcePart( *(piResources->get()), pDescriptorPart, pFixedPage );
        }
    }
}

void
DWFXPackageWriter::_addResourcePart( DWFResource& rResource,
                                     DWFXDWFSection* pDescriptorPart,
                                     DWFXFixedPage* pFixedPage )
throw( DWFException )
{
    if (pFixedPage != NULL)
    {
        DWFString zRole( rResource.role() );
        DWFString zMIME( rResource.mime() );
        if (
            (zRole == DWFXML::kzRole_Graphics2d ||
             zRole == DWFXML::kzRole_Graphics2dOverlay ||
             zRole == DWFXML::kzRole_Graphics2dMarkup) &&
             zMIME == DWFMIME::kzMIMEType_FIXEDPAGEXML
           )
        {
            DWFGraphicResource* pGraphicResource = dynamic_cast<DWFGraphicResource*>(&rResource);
            if (pGraphicResource)
            {
                pFixedPage->addGraphicResource( pGraphicResource );
                _oResourcePartMap.insert( std::make_pair( &rResource, pFixedPage ) );
            }
            else
            {
                _DWFCORE_THROW( DWFUnexpectedException, L"A resource with a graphics2D or related role should be a GraphicResource" );
            }
        }
        else if (
                 (zRole == DWFXML::kzRole_RasterOverlay ||
                  zRole == DWFXML::kzRole_RasterMarkup) &&
                 (zMIME == DWFMIME::kzMIMEType_JPG ||
                  zMIME == DWFMIME::kzMIMEType_PNG ||
                  zMIME == DWFMIME::kzMIMEType_TIFF)
                )
        {
            DWFXResourcePart* pResourcePart = DWFCORE_ALLOC_OBJECT( DWFXResourcePart( &rResource ) );
            pDescriptorPart->addResourcePart( pResourcePart );
            _oResourcePartMap.insert( std::make_pair( &rResource, pResourcePart ) );
            pFixedPage->addRasterPart( pResourcePart );
        }
        else
        {
            DWFXResourcePart* pResourcePart = DWFCORE_ALLOC_OBJECT( DWFXResourcePart( &rResource ) );
            pDescriptorPart->addResourcePart( pResourcePart );
            _oResourcePartMap.insert( std::make_pair( &rResource, pResourcePart ) );

            if (zRole == DWFXML::kzRole_Font ||
                zRole == DWFXML::kzRole_Graphics2dDictionary ||
                zRole == DWFXML::kzRole_RasterReference)
            {
                pFixedPage->addRequiredResourcePart( pResourcePart );
            }
            else if (zRole == DWFXML::kzRole_Thumbnail ||
                     zRole == DWFXML::kzRole_Preview)
            {
                if (rResource.mime() == DWFMIME::kzMIMEType_JPG ||
                    rResource.mime() == DWFMIME::kzMIMEType_PNG)
                {
                    pFixedPage->setThumbnailResourcePart( pResourcePart );
                }
            }
        }
    }
    else
    {
        DWFXResourcePart* pResourcePart = DWFCORE_ALLOC_OBJECT( DWFXResourcePart( &rResource ) );
        pDescriptorPart->addResourcePart( pResourcePart );
        _oResourcePartMap.insert( std::make_pair( &rResource, pResourcePart ) );
    }
}

void 
DWFXPackageWriter::_addResourceRelationships()
throw( DWFException )
{
    //
    // Process global sections
    //
    DWFGlobalSection::tList::const_iterator iGlobalSection;

    DWFSection* pSection;
    for (iGlobalSection = _oGlobalSections.begin();
         iGlobalSection!= _oGlobalSections.end();
         iGlobalSection++)
    {
        pSection = *iGlobalSection;
        _addRelationships(*pSection);
    }
        //
        // Process the non-global sections
        //
    DWFSection::tList::const_iterator iSection;
    for (iSection = _oSections.begin();
         iSection!= _oSections.end();
         iSection++)
    {
        pSection = *iSection;
        _addRelationships(*pSection);
    }
}

void 
DWFXPackageWriter::_addRelationships( DWFSection& rSection )
throw( DWFException )
{
    DWFResource* pResource = NULL;
    DWFPointer<DWFKVIterator<const wchar_t*, DWFResource*> > piAllResources( rSection.getResourcesByRole(), false );
    if (!piAllResources.isNull())
    {
        for (; piAllResources->valid(); piAllResources->next())
        {
            pResource = piAllResources->value();
            if (pResource->role() != DWFXML::kzRole_Descriptor &&
                pResource->mime() != DWFMIME::kzMIMEType_FIXEDPAGEXML)
            {
                _addRelationships( *pResource );
            }
        }
    }
}

void 
DWFXPackageWriter::_addRelationships( DWFResource& rResource )
throw( DWFException )
{
    //
    // interate through resource relationships and add OPC relationships
    //
    _tResourcePartMap::iterator iSourcePart = _oResourcePartMap.find( &rResource );
    if (iSourcePart == _oResourcePartMap.end())
    {
        _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"A resource could not mapped to its corresponding OPC part" );
    }

    OPCPart* pSourcePart = iSourcePart->second;
    DWFResourceRelationship::tList::ConstIterator* piRels = rResource.getRelationships();
    if (piRels)
    {
        for( ;piRels->valid(); piRels->next() )
        {
            DWFResourceRelationship* pRel = piRels->get();
            _tResourceIDMap::iterator iTargetResource = _oResourceIDMap.find( pRel->resourceID() );
            if (iTargetResource == _oResourceIDMap.end())
            {
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"A relationship resource ID could not be mapped to its corresponding resource" );
            }
            _tResourcePartMap::iterator iTargetPart = _oResourcePartMap.find( iTargetResource->second );
            if (iTargetPart == _oResourcePartMap.end())
            {
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"A resource could not mapped to its corresponding OPC part" );
            }

            OPCPart* pTargetPart = iTargetPart->second;

            pSourcePart->addRelationship( pTargetPart, pRel->type() );
        }

        DWFCORE_FREE_OBJECT( piRels );
    }
}

DWFXFixedPage*
DWFXPackageWriter::_createFixedPage( DWFSection* pSection, bool bAppend )
throw( DWFException )
{
    bool bCreateFixedPage = _fixedPageDesired( pSection );

    DWFXFixedPage* pFixedPage = NULL;

    if (bCreateFixedPage)
    {
        XPSFixedDocumentSequence* pFixedDocSequence = _pPackage->xpsFixedDocumentSequence();
        if (pFixedDocSequence == NULL)
        {
            pFixedDocSequence = DWFCORE_ALLOC_OBJECT( DWFXFixedDocumentSequence() );
            if (pFixedDocSequence == NULL)
            {
                _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate a fixed document sequence for the package" );
            }
            _pPackage->setFixedDocumentSequence( pFixedDocSequence );
        }

        //
        //  Our current DWFX package supports ONLY ONE fixed document for DWF
        //
        DWFXFixedDocument* pFixedDocument = NULL;
        XPSFixedDocument::tConstIterator* piDocs = pFixedDocSequence->fixedDocuments();
        if (piDocs)
        {
            while(piDocs->valid())
            {
                pFixedDocument = dynamic_cast<DWFXFixedDocument*>(piDocs->get());
                if (pFixedDocument == NULL)
                {
                    piDocs->next();
                }
                else
                {
                    break;
                }
            }
            DWFCORE_FREE_OBJECT( piDocs );
        }
        //
        //  Create the fixed document if necessary
        //
        if (pFixedDocument == NULL)
        {
            _ensureManifest();
            pFixedDocument = DWFCORE_ALLOC_OBJECT( DWFXFixedDocument(_pPackageManifest) );
            if (pFixedDocument == NULL)
            {
                _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate a fixed document for the package" );
            }
            pFixedDocSequence->addFixedDocument( pFixedDocument );
        }

        //
        //  Create the fixed page for the section
        //
        pFixedPage = DWFCORE_ALLOC_OBJECT( DWFXFixedPage(pSection) );
        if (pFixedPage == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, L"Failed to allocate a fixed page for the fixed document" );
        }
        if(bAppend)
        {
            //
            // Append to end
            //
            pFixedDocument->addFixedPage( pFixedPage );
        }
        else
        {
            //
            // Insert at front
            //
            pFixedDocument->insertFixedPage( pFixedPage, NULL );
        }
    }

    return pFixedPage;
}

bool 
DWFXPackageWriter::_fixedPageDesired( DWFSection* pSection )
throw( DWFException )
{
        //
        //
        //
    if (dynamic_cast<DWFXProtectedSection*>(pSection) != NULL)
    {
        return false;
    }

    //
    //  <TODO>
    //  We can override what we want for a fixed page at this point. For example if we 
    //  want to create alternate content even if no fixed page content is found this might be
    //  a place to determine that
    //

    bool bCreateFixedPage = false;

    //
    //  If we find any fixed page compatible resource, a fixed page will be created.
    //  Any Graphics2d, Graphics2dMarkup or Graphics2dOverlay resource with 
    //  MIME type equal to kzMIMEType_FIXEDPAGEXML will go into a fixed page. 
    //  Any RasterOverlay or RasterMarkup will also go into a fixed page unless
    //  otherwise noted (TODO).
    //
    //  <TODO> 
    //  If we have a raster overlay or a raster markup, we might want to add an API
    //  to prevent them from going into a fixed page.
    //

    DWFResource::tIterator* piResources = NULL;

    //
    //  Test for Graphics2d
    //
    piResources = pSection->findResourcesByRole( DWFXML::kzRole_Graphics2d );
    if (piResources)
    {
        while (piResources->valid() && !bCreateFixedPage)
        {
            DWFResource* pResource = piResources->get();
            if (pResource->mime() == DWFMIME::kzMIMEType_FIXEDPAGEXML)
            {
                DWFCORE_FREE_OBJECT(piResources);
                return true;
            }
            piResources->next();
        }
        DWFCORE_FREE_OBJECT( piResources );
    }


    //
    // Raster Overlay
    //
    piResources = pSection->findResourcesByRole( DWFXML::kzRole_RasterOverlay );
    if (piResources)
    {
        if (piResources->valid())
        {
            DWFCORE_FREE_OBJECT(piResources);
            return true;
        }
        DWFCORE_FREE_OBJECT( piResources );
    }


    //
    // There might be other Fixed Page XAML resources (Graphics2dOverlay, Graphics2D Markup, Raster Markup)
    // But as long as the primary graphics is not in FPXAML (such as DWFx with DRM) we do not want to
    // generate any FixedPage content
    //
    return false; 
}

void
DWFXPackageWriter::_updateProperties( const DWFString& zSourceProductVendor,
                                      const DWFString& zSourceProductName,
                                      const DWFString& zSourceProductVersion,
                                      const DWFString& zDWFProductVendor,
                                      const DWFString& zDWFProductVersion )
throw( DWFException )
{
    DWFProperty::tMap::Iterator* piProperties = _pPackageManifest->getProperties();
    if (piProperties)
    {
        DWFPropertySet oOPCProperties;
        DWFPropertySet oDWFProperties;
        DWFPropertySet oCustomProperties;

        for (; piProperties->valid(); piProperties->next())
        {
            DWFProperty* pProperty = piProperties->value();

            if (pProperty->category() == OPCXML::kzSchemaID_CoreProperties)
            {
                oOPCProperties.addProperty( pProperty, false );
            }
            else if (pProperty->category() == DWFXXML::kzSchemaID_DWFProperties)
            {
                oDWFProperties.addProperty( pProperty, false );
            }
            else
            {
                oCustomProperties.addProperty( pProperty, false );
            }
        }
    
        if (oOPCProperties.getPropertyCount() > 0)
        {
            oOPCProperties.setSchemaID( OPCXML::kzSchemaID_CoreProperties );
            setCoreProperties( &oOPCProperties );
        }
        if (oDWFProperties.getPropertyCount() > 0)
        {
            oDWFProperties.setSchemaID( DWFXXML::kzSchemaID_DWFProperties );
            setDWFProperties( &oDWFProperties );
        }
        if (oCustomProperties.getPropertyCount() > 0)
        {
            setCustomProperties( &oCustomProperties );
        }

        DWFCORE_FREE_OBJECT( piProperties );
    }


    if (_pDWFProperties == NULL)
    {
        _pDWFProperties = DWFCORE_ALLOC_OBJECT( DWFXDWFProperties() );
    }

    if (zSourceProductVendor.bytes())
    {
        _pDWFProperties->setSourceProductVendor( zSourceProductVendor );
    }
    if (zSourceProductName.bytes())
    {
        _pDWFProperties->setSourceProductName( zSourceProductName );
    }
    if (zSourceProductVersion.bytes())
    {
        _pDWFProperties->setSourceProductVersion( zSourceProductVersion );
    }
    if (zDWFProductVendor.bytes())
    {
        _pDWFProperties->setDWFProductVendor( zDWFProductVendor );
    }
    if (zDWFProductVersion.bytes())
    {
        _pDWFProperties->setDWFProductVersion( zDWFProductVersion );
    }

    //
    //  Set the Toolkit version info.
    //
    _pDWFProperties->setDWFToolkitVersion( _DWFTK_VERSION_STRING );	

    //
    //  Set the Format version info.
    //
    // New: check for versionExtension   
    if (_pVersionExtension != NULL)
    {
        DWFString extVersion = _pVersionExtension->extensionVersionAsWideString();
        if (extVersion.chars() > 0)
        {
            _pDWFProperties->setDWFFormatVersion( extVersion );
        }
        else
        {
            _pDWFProperties->setDWFFormatVersion( _DWFX_FORMAT_DEFAULT_VERSION_STRING );
        }
    }
    else
    {
        _pDWFProperties->setDWFFormatVersion( _DWFX_FORMAT_DEFAULT_VERSION_STRING );	
    }
	//
    //  Set the password protected flag.
    //
	if (_zDWFPassword.bytes())
    {
        _pDWFProperties->setPasswordProtected( L"true" );
    }
}

void 
DWFXPackageWriter::DWFXUndeclaredResourceReceiver::receiveResource( 
    const DWFCore::DWFString &rStoragePath, 
    DWFInputStream* pInStream, 
    DWFZipFileDescriptor::teFileMode eZipMode ) 
throw()
{
    if ( pInStream != NULL && rStoragePath.chars() > 0 )
    {
        _pZipper->write( rStoragePath, pInStream, eZipMode );
    }
}

void
DWFXPackageWriter::DWFXUndeclaredContentTypeReceiver::receiveContentType( 
    const DWFCore::DWFString &rzExtension, 
    const DWFCore::DWFString &rzContentType ) 
throw()
{
    if ( rzExtension.chars() > 0 && rzContentType.chars() > 0 )
    {
        _pPackage->contentTypes()->addContentType( rzExtension, rzContentType );
    }
}

void
DWFXPackageWriter::DWFXUndeclaredPackageRelationshipsReceiver::receiveRelationship( 
    DWFToolkit::OPCRelationship* pRelationship )
throw()
{
    if ( pRelationship != NULL )
    {
        DWFString target = pRelationship->targetURI();

        OPCPart *pPart = DWFCORE_ALLOC_OBJECT( OPCPart );
        _oPartVector.push_back( pPart );

        off_t offset = target.findLast(/*NOXLATE*/'/');
        if (offset == -1)
            offset = target.findLast(/*NOXLATE*/'\\');

        if (offset != -1)
        {
            pPart->setPath( target.substring( 0, offset ) );
            pPart->setName( target.substring( offset+1 ) );

            OPCRelationship* pRel = _pPackage->addRelationship( pPart, pRelationship->relationshipType()  );
            pRel->setId( pRelationship->id() );
            
            //pRel is owned by the _pPackage object
        }
    }
}

DWFXPackageWriter::DWFXUndeclaredPackageRelationshipsReceiver::~DWFXUndeclaredPackageRelationshipsReceiver()
{
    _tPartVector::iterator iterPart;
    for( iterPart = _oPartVector.begin();
         iterPart != _oPartVector.end();
         iterPart++ )
    {
        DWFCORE_FREE_OBJECT( *iterPart );
    }
    _oPartVector.clear();
}

//DNT_End

#endif

