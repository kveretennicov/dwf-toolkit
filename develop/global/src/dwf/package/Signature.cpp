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
//  $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/package/Signature.cpp#1 $
//  $DateTime: 2011/02/14 01:16:30 $
//  $Author: caos $
//  $Change: 197964 $
//  $Revision: #1 $
//

#include "dwfcore/DWFXMLSerializer.h"
#include "dwfcore/MIME.h"
#include "dwfcore/BufferInputStream.h"
#include "dwfcore/BufferOutputStream.h"
#include "dwfcore/crypto/CryptoEngineProvider.h"
#include "dwfcore/crypto/AsymmetricAlgorithm.h"
using namespace DWFCore;


#include "dwf/Version.h"
#include "dwf/package/XML.h"
#include "dwf/package/Constants.h"
#include "dwf/package/Signature.h"
#include "dwf/package/reader/PackageReader.h"
using namespace DWFToolkit;


#if defined(DWFTK_STATIC) || !defined(_DWFCORE_WIN32_SYSTEM)
//DNT_Start
const wchar_t* const DWFSignature::DigestMethodSHA1::kzAlgorithmNameSHA1         = L"http://www.w3.org/2000/09/xmldsig#sha1";
const wchar_t* const DWFSignature::DigestMethodMD5::kzAlgorithmNameMD5           = L"http://www.w3.org/2000/09/xmldsig#md5";
const wchar_t* const DWFSignature::SignatureMethodDSA::kzAlgorithmNameDSA_SHA1   = L"http://www.w3.org/2000/09/xmldsig#dsa-sha1";
const wchar_t* const DWFSignature::SignatureMethodRSA::kzAlgorithmNameRSA_SHA1   = L"http://www.w3.org/2000/09/xmldsig#rsa-sha1";
//DNT_End
#endif



_DWFTK_API
DWFSignature::DWFSignature()
    throw()
    : _nVersion( 0.1f /* TODO: _DWF_FORMAT_SIGNATURE_VERSION_CURRENT_FLOAT */ )
    , _oReferences()
    , _pDigestMethod(NULL)
    , _pSignatureValue(NULL)
    , _pSignatureMethod(NULL)
    , _oX509DataVector()
    , _pCurrentReference(NULL)
    , _pPublicKey(NULL)
    , _zKeyName()
{
}

_DWFTK_API
DWFSignature::~DWFSignature()
throw()
{
    if(_pDigestMethod!=NULL)
    {
        DWFCORE_FREE_OBJECT(_pDigestMethod);
    }

    if(_pSignatureValue!=NULL)
    {
        DWFCORE_FREE_OBJECT(_pSignatureValue);
    }

    if(_pSignatureMethod!=NULL)
    {
        DWFCORE_FREE_OBJECT(_pSignatureMethod);
    }

    // Free any & all the X509Data elements
    tX509DataVector::Iterator *piX509Datas = _oX509DataVector.iterator();
    for(; piX509Datas->valid(); piX509Datas->next())
    {
        X509Data* pData = piX509Datas->get();
        if(pData != NULL)
        {
            DWFCORE_FREE_OBJECT(pData);
        }
    }
    DWFCORE_FREE_OBJECT( piX509Datas );


    // Free any & all references
    DWFSignature::tReferenceVector::Iterator* piRef = references();
    for( ; piRef->valid(); piRef->next())
    {
        DWFSignature::Reference* pRef = piRef->get();
        if(pRef != NULL)
        {
            DWFCORE_FREE_OBJECT(pRef);
        }
    }
    DWFCORE_FREE_OBJECT( piRef );

}

_DWFTK_API
void DWFSignature::addReference(Reference* pReference)
    throw( DWFException )
{
    if(pReference == NULL )
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No Reference provided" );
    }

    pReference->setSignature(this);

    _oReferences.push_back(pReference);
}

_DWFTK_API
void DWFSignature::setDigestMethod(const DigestMethod* pDigestMethod)
    throw()
{
    _pDigestMethod = pDigestMethod;
}

_DWFTK_API
void DWFSignature::setSignatureValue(const SignatureValue* pSignatureValue)
    throw()
{
    _pSignatureValue = pSignatureValue;
}

_DWFTK_API
void DWFSignature::setSignatureMethod(SignatureMethod* pSignatureMethod)
    throw()
{
    _pSignatureMethod = pSignatureMethod;
}

_DWFTK_API
void DWFSignature::addX509Data(X509Data* pX509Data)
    throw()
{
    _oX509DataVector.push_back(pX509Data);
}



_DWFTK_API
const DWFString&
DWFSignature::provideSignatureMethod( const DWFString& zSigMethod )
throw( DWFException )
{
    SignatureMethodDSA dsa;
    SignatureMethodRSA rsa;

    SignatureMethod* pMethod = NULL;
    if(DWFCORE_COMPARE_WIDE_STRINGS(zSigMethod, dsa.algorithmName()) == 0)
    {
        pMethod = DWFCORE_ALLOC_OBJECT(SignatureMethodDSA);
    }
    else if(DWFCORE_COMPARE_WIDE_STRINGS(zSigMethod, rsa.algorithmName()) == 0)
    {
        pMethod = DWFCORE_ALLOC_OBJECT(SignatureMethodRSA);
    }

    setSignatureMethod(pMethod);

    return zSigMethod;
}

_DWFTK_API
const DWFString&
DWFSignature::provideSignatureValue( const DWFString& zSigValue )
throw( DWFException )
{
    SignatureValue* pValue = DWFCORE_ALLOC_OBJECT(SignatureValue);
    pValue->setValueBase64(zSigValue);
    setSignatureValue(pValue);
    return zSigValue;
}

_DWFTK_API
X509Data *
DWFSignature::provideX509Data( X509Data *pX509Data )
throw( DWFException )
{
    _oX509DataVector.push_back(pX509Data);
    return pX509Data;
}


_DWFTK_API
const DWFString&
DWFSignature::provideReference( const DWFString& zReferenceURI )
throw( DWFException )
{
    _pCurrentReference = DWFCORE_ALLOC_OBJECT(Reference);
    if(_pCurrentReference != NULL)
    {
        _pCurrentReference->setURI(zReferenceURI);
        addReference(_pCurrentReference);
    }
    return zReferenceURI;
}

_DWFTK_API
const DWFString&
DWFSignature::provideDigestMethod( const DWFString& zDigestMethod )
throw( DWFException )
{
    if(_pDigestMethod != NULL)
    {
        // We only allow one digest method for all references in the entire signature.

        // Optional TODO: Check this here, complain if it's not true.
    }
    else
    {
        DigestMethodSHA1 sha1;
        DigestMethodMD5  md5;

        DigestMethod* pMethod = NULL;
        if(DWFCORE_COMPARE_WIDE_STRINGS(zDigestMethod, sha1.algorithmName()) == 0)
        {
            pMethod = DWFCORE_ALLOC_OBJECT(DigestMethodSHA1);
        }
        else if(DWFCORE_COMPARE_WIDE_STRINGS(zDigestMethod, md5.algorithmName()) == 0)
        {
            pMethod = DWFCORE_ALLOC_OBJECT(DigestMethodMD5);
        }

        setDigestMethod(pMethod);
    }

    return zDigestMethod;
}

_DWFTK_API
const DWFString&
DWFSignature::provideDigestValue( const DWFString& zDigestValue )
throw( DWFException )
{
    if(_pCurrentReference == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No Reference active." );
    }

    DigestValue* pValue = DWFCORE_ALLOC_OBJECT(DigestValue);
    pValue->setValueBase64(zDigestValue);
    _pCurrentReference->setDigestValue(pValue);

    return zDigestValue;
}

_DWFTK_API
const DWFSignatureReader::RSAKeyValueStrings&
DWFSignature::provideRSAKeyValueStrings( const DWFSignatureReader::RSAKeyValueStrings& rRSAKeyValueStrings )
throw( DWFException )
{
    //
    //  TODO: ...
    //
    return rRSAKeyValueStrings;
}

_DWFTK_API
const DWFSignatureReader::DSAKeyValueStrings&
DWFSignature::provideDSAKeyValueStrings( const DWFSignatureReader::DSAKeyValueStrings& rDSAKeyValueStrings )
throw( DWFException )
{
    //
    //  TODO: ...
    //
    return rDSAKeyValueStrings;
}

_DWFTK_API
const DWFString&
DWFSignature::provideKeyName( const DWFString& zKeyName )
throw( DWFException )
{
    _zKeyName = zKeyName;
    return _zKeyName;
}


#ifndef DWFTK_READ_ONLY

_DWFTK_API
bool
DWFSignature::validateSignedInfo(DWFCryptoKey* pPublicKey)
    throw( DWFException )
{
    //
    // Start off with some sanity checks...
    //
    const SignatureValue* pSigValue = signatureValue();
    if(pSigValue==NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No signature value set." );
    }

    const DigestMethod* pDigestMethod = digestMethod();
    if(pDigestMethod==NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No digest method set." );
    }

    SignatureMethod* pSigMethod = signatureMethod();
    if(pSigMethod==NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No signing method set." );
    }


    //
    // Recreate a SignedInfo block based on our current data. Compute its DigestValue.
    //
    DWFSignature::DigestValue currentSignedInfoDigest;
    DWFString zNamespace(L"" /*DWFXML::kzNamespace_Signatures*/);
    computeSignedInfoDigestValue(currentSignedInfoDigest, zNamespace);

    //
    // Ask the SignatureMethod to verify.
    //
    return pSigMethod->verifyDigest(&currentSignedInfoDigest, pDigestMethod, pPublicKey, pSigValue);
}

#endif


_DWFTK_API
bool
DWFSignature::validateReferences(DWFPackageReader* pReader)
    throw( DWFException )
{
    const DigestMethod* pDigestMethod = digestMethod();
    if(pDigestMethod==NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No digest method set." );
    }

    DWFSignature::tReferenceVector::Iterator* piRef = references();
    for( ; piRef->valid(); piRef->next())
    {
        DWFSignature::Reference* pRef = piRef->get();
        if(pRef != NULL)
        {
            DWFString zRefURI = pRef->URI();
            DWFInputStream* pDirectStream = pReader->extract(zRefURI);
            DWFDigestInputStream* pDigestStream = pDigestMethod->chainInputStream(pDirectStream, true);
            char buf[1024];
            size_t nBytesToRead(1024);
            while(pDigestStream->available() > 0)
            {
                pDigestStream->read(buf, nBytesToRead);
                // Don't care about the bytes. Don't care how many were read.
            }

            DWFString zCurrentDigest = pDigestStream->digestBase64();
            DWFString zStoredDigest = pRef->digestValue()->valueBase64();

            // cleanup before we might return
            DWFCORE_FREE_OBJECT(pDigestStream);

            if(zCurrentDigest != zStoredDigest)
            {
                return false;
            }

        }
    }

    return true;
}



// We need a useless output stream, because DigestOutputStream can't chain to nothing.
class _NullOutputStream : public DWFOutputStream
{
public:
    _NullOutputStream()
        throw()
    {;}

    _DWFTK_API
    virtual ~_NullOutputStream()
        throw()
    {;}

    virtual void flush()
        throw( DWFException )
    {;}

    _DWFTK_API
    virtual size_t write( const void*   /*pBuffer*/,
                            size_t        nBytesToWrite )
        throw( DWFException )
    {
        return nBytesToWrite;
    }
};


#ifndef DWFTK_READ_ONLY

_DWFTK_API
void
DWFSignature::computeSignedInfoDigestValue(DWFSignature::DigestValue& signedInfoDigest, const DWFString& zNamespace)
    throw( DWFException )
{
    if(_pDigestMethod != NULL)
    {
        // Create a null output stream to chain to.
        _NullOutputStream oNull;
        DWFDigestOutputStream* pDigestStream = _pDigestMethod->chainOutputStream(&oNull, false);
        if(pDigestStream != NULL)
        {
            // create a new serializer
            DWFUUID oUUID;
            DWFXMLSerializer* pXMLSerializer = DWFCORE_ALLOC_OBJECT( DWFXMLSerializer(oUUID) );
            if (pXMLSerializer != NULL)
            {
                //
                // bind the digest stream to the new serializer.
                //
                pXMLSerializer->attach( *pDigestStream );

                //
                // tell the SignedInfo to serialize itself as an XML document
                //
                _serializeSignedInfo(*pXMLSerializer, zNamespace);

                //
                // unbind the stream and finalize the digest
                //
                pXMLSerializer->detach();

                //
                // release the serializer
                //
                DWFCORE_FREE_OBJECT( pXMLSerializer );

                //
                // Extract out the digest of the signed info...
                //
                signedInfoDigest.setValueBase64(pDigestStream->digestBase64());
            }

            //
            // release the file stream
            //
            DWFCORE_FREE_OBJECT( pDigestStream );
        }
    }
}

void
DWFSignature::_serializeSignedInfo( DWFXMLSerializer& rSerializer, const DWFString& zNamespace)
throw( DWFException )
{
    // !!! VERY IMPORTANT !!!
    //
    // Signature generation will be incorrect if anything in here uses
    // the serializer's UUID generator, as the process won't be exactly
    // repeatable, and will produce an incorrect SignatureValue.
    //
    // !!! VERY IMPORTANT !!!

    rSerializer.startElement( DWFXML::kzElement_SignedInfo, zNamespace );
    {
        rSerializer.startElement( DWFXML::kzElement_CanonicalizationMethod, zNamespace );
        rSerializer.endElement(); // end of CanonicalizationMethod


        rSerializer.startElement( DWFXML::kzElement_SignatureMethod, zNamespace );
        rSerializer.addAttribute( DWFXML::kzAttribute_Algorithm , signatureMethod()->algorithmName() );
        rSerializer.endElement(); // end of SignatureMethod

        DWFSignature::tReferenceVector::Iterator* piRef = references();
        for( ; piRef->valid(); piRef->next())
        {
            DWFSignature::Reference* pRef = piRef->get();
            if(pRef != NULL)
            {
                rSerializer.startElement( DWFXML::kzElement_Reference, zNamespace );
                rSerializer.addAttribute( DWFXML::kzAttribute_URI , pRef->URI() );
                {
                    rSerializer.startElement( DWFXML::kzElement_DigestMethod, zNamespace );
                    rSerializer.addAttribute( DWFXML::kzAttribute_Algorithm , digestMethod()->algorithmName() ); // Note: Each request always uses the same digest for all references.
                    rSerializer.endElement(); // end of DigestMethod

                    rSerializer.startElement( DWFXML::kzElement_DigestValue, zNamespace );
                    const DWFSignature::DigestValue* digestValue = pRef->digestValue();
                    if(digestValue != NULL)
                    {
                        rSerializer.addCData( digestValue->valueBase64() );
                    }
                    else
                    {
                        // TODO: Anything better???
                        rSerializer.addCData( L"No digest value available" );
                    }
                    rSerializer.endElement(); // end of DigestValue
                }
                rSerializer.endElement(); // end of Reference
            }
        }
        DWFCORE_FREE_OBJECT( piRef );
    }
    rSerializer.endElement(); // end of SignedInfo
}

#endif

_DWFTK_API
void
DWFSignature::DigestValue::getValueBytes(void*& pBytes, size_t& nBytes) const
    throw()
{
    char *valueAscii = DWFCORE_ALLOC_MEMORY(char, _zValue.chars()*3);
    _zValue.getUTF8(valueAscii, _zValue.chars()*3);

    nBytes = DWFString::DecodeBase64(
                        valueAscii, strlen(valueAscii),
                        NULL, 0,
                        true
                    );
    pBytes = DWFCORE_ALLOC_MEMORY(char, (int)(nBytes+1));
    DWFString::DecodeBase64(
                        valueAscii, strlen(valueAscii),
                        pBytes, nBytes+1,
                        true
                    );
    DWFCORE_FREE_MEMORY(valueAscii);
}

_DWFTK_API
void
DWFSignature::DigestValue::setValueBytes(const void *pBytes, const size_t nBytes)
    throw()
{
    // Base64 encode
    size_t nAsciiBytes = DWFString::EncodeBase64(
                        pBytes, nBytes,
                        NULL, 0,
                        true
                    );
    char *encodedBase64 = DWFCORE_ALLOC_MEMORY(char, nAsciiBytes+1);
    DWFString::EncodeBase64(
                        pBytes, nBytes,
                        encodedBase64, nAsciiBytes+1,
                        true
                    );

    _zValue.assign(encodedBase64, nAsciiBytes);

    DWFCORE_FREE_MEMORY(encodedBase64);
}

_DWFTK_API
void
DWFSignature::SignatureValue::getValueBytes(void*& pBytes, size_t& nBytes) const
    throw()
{
    char *valueAscii = DWFCORE_ALLOC_MEMORY(char, _zValue.chars()*3);
    _zValue.getUTF8(valueAscii, _zValue.chars()*3);

    nBytes = DWFString::DecodeBase64(
                        valueAscii, strlen(valueAscii),
                        NULL, 0,
                        true
                    );
    pBytes = DWFCORE_ALLOC_MEMORY(char, (int)(nBytes+1));
    DWFString::DecodeBase64(
                        valueAscii, strlen(valueAscii),
                        pBytes, nBytes+1,
                        true
                    );
    DWFCORE_FREE_MEMORY(valueAscii);
}

_DWFTK_API
void
DWFSignature::SignatureValue::setValueBytes(const void *pBytes, const size_t nBytes)
    throw()
{
    // Base64 encode
    size_t nAsciiBytes = DWFString::EncodeBase64(
                        pBytes, nBytes,
                        NULL, 0,
                        true
                    );
    char *encodedBase64 = DWFCORE_ALLOC_MEMORY(char, nAsciiBytes+1);
    DWFString::EncodeBase64(
                        pBytes, nBytes,
                        encodedBase64, nAsciiBytes+1,
                        true
                    );

    _zValue.assign(encodedBase64, nAsciiBytes);

    DWFCORE_FREE_MEMORY(encodedBase64);
}

_DWFTK_API
DWFAsymmetricAlgorithm*
DWFSignature::SignatureMethodDSA::createAlgorithm()
throw()
{
    DWFCryptoEngine* pEngine = DWFCryptoEngineProvider::ProvideEngine( DWFCryptoEngine::eDSA );  
    return dynamic_cast<DWFAsymmetricAlgorithm*>(pEngine);
}

_DWFTK_API
DWFAsymmetricAlgorithm*
DWFSignature::SignatureMethodRSA::createAlgorithm()
throw()
{
    DWFCryptoEngine* pEngine = DWFCryptoEngineProvider::ProvideEngine( DWFCryptoEngine::eRSA );  
    return dynamic_cast<DWFAsymmetricAlgorithm*>(pEngine);
}


_DWFTK_API
DWFSignature::SignatureMethod::SignatureMethod()
throw()
                             : _pCachedAlgorithm( NULL )
{
    ;
}

_DWFTK_API
DWFSignature::SignatureMethod::~SignatureMethod()
throw()
{
    if (_pCachedAlgorithm)
    {
        DWFCORE_FREE_OBJECT( _pCachedAlgorithm );
    }
}

_DWFTK_API
DWFAsymmetricAlgorithm*
DWFSignature::SignatureMethod::algorithm()
throw()
{
    if (_pCachedAlgorithm == NULL)
    {
        _pCachedAlgorithm = createAlgorithm();
    }

    return _pCachedAlgorithm;
}

_DWFTK_API
void
DWFSignature::SignatureMethod::signDigest(const DigestValue* pDigestValue, const DigestMethod* pDigestMethod, DWFCryptoKey* pPrivateKey, DWFSignature::SignatureValue* pSignatureValue)
throw(DWFException)
{
    if (pDigestValue == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No DigestValue given." );
    }
    if (pDigestMethod == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No DigestMethod given." );
    }
    if (pPrivateKey == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No PrivateKey given." );
    }
    if (pSignatureValue == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No SignatureValue given." );
    }

    if (algorithm() == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No signing algorithm available." );
    }

    char *pDigestBytes = NULL;
    size_t nBytes;
    pDigestValue->getValueBytes((void*&)pDigestBytes, nBytes); // remember to free the bytes, below
    DWFBufferInputStream DataStream( pDigestBytes, nBytes );
    DWFBufferOutputStream Encrypted( 256 );

    algorithm()->setKey( *pPrivateKey, DWFCryptoKey::eAsymmetricPrivateKey );
    algorithm()->sign( DataStream, Encrypted, pDigestMethod->algorithmIdentifier() );

    pSignatureValue->setValueBytes(Encrypted.buffer(), Encrypted.bytes());

    DWFCORE_FREE_MEMORY(pDigestBytes);
}

_DWFTK_API
bool
DWFSignature::SignatureMethod::verifyDigest(const DigestValue* pDigestValue, const DigestMethod* pDigestMethod, DWFCryptoKey* pPublicKey, const SignatureValue* pSignatureValue)
    throw(DWFException)
{
    //
    // Start off with some sanity checks...
    //
    if(pSignatureValue == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No signature value given." );
    }

    if(pDigestValue == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No digest value given." );
    }

    if(pDigestMethod == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No digest method given." );
    }

    if(algorithm() == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/ L"No DWFAsymmetricAlgorithm algorithm available." );
    }


    //
    // Build a stream out of the supplied digest.
    //
    char *pDigestBytes = NULL;
    size_t nBytes;
    pDigestValue->getValueBytes((void*&)pDigestBytes, nBytes); // remember to free the bytes, below
    DWFBufferInputStream pDigestStream( pDigestBytes, nBytes );

    //
    // Build a stream out of the supplied SignatureValue.
    //
    char *pSignatureBytes = NULL;
    pSignatureValue->getValueBytes((void*&)pSignatureBytes, nBytes);  // remember to free the bytes, below
    DWFBufferInputStream pSignatureStream( pSignatureBytes, nBytes );

    // Let the engine know the public key
    algorithm()->setKey( *pPublicKey, DWFCryptoKey::eAsymmetricPublicKey );
    
    // Check if the streams match
    bool retVal = algorithm()->verify( pDigestStream, pSignatureStream, pDigestMethod->algorithmIdentifier() );

    // cleanup memory allocated by getValueBytes() methods.
    DWFCORE_FREE_MEMORY(pDigestBytes);
    DWFCORE_FREE_MEMORY(pSignatureBytes);

    return retVal;
}

_DWFTK_API
DWFDigestOutputStream*
DWFSignature::DigestMethodSHA1::chainOutputStream(DWFOutputStream* pStream, bool bOwnStream) const
throw( DWFException )
{
    DWFCryptoEngine* pEngine = DWFCryptoEngineProvider::ProvideEngine( DWFCryptoEngine::eSHA1 );
    DWFDigest* pDigest = dynamic_cast<DWFDigest*>( pEngine );
    if (pDigest)
    {
        return DWFCORE_ALLOC_OBJECT(DWFDigestOutputStream( pDigest, pStream, bOwnStream ));
    }
    else
    {
        _DWFCORE_THROW( DWFInvalidTypeException, /*NOXLATE*/L"The cryptoengine provider did not return an engine with a digest interface" );
    }
}

_DWFTK_API
DWFDigestInputStream*
DWFSignature::DigestMethodSHA1::chainInputStream(DWFInputStream* pStream, bool bOwnStream) const
throw( DWFException )
{
    DWFCryptoEngine* pEngine = DWFCryptoEngineProvider::ProvideEngine( DWFCryptoEngine::eSHA1 );
    DWFDigest* pDigest = dynamic_cast<DWFDigest*>( pEngine );
    if (pDigest)
    {
        return DWFCORE_ALLOC_OBJECT(DWFDigestInputStream( pDigest, pStream, bOwnStream));
    }
    else
    {
        _DWFCORE_THROW( DWFInvalidTypeException, /*NOXLATE*/L"The cryptoengine provider did not return an engine with a digest interface" );
    }
}

_DWFTK_API
DWFDigestOutputStream*
DWFSignature::DigestMethodMD5::chainOutputStream(DWFOutputStream* pStream, bool bOwnStream) const
throw( DWFException )
{
    DWFCryptoEngine* pEngine = DWFCryptoEngineProvider::ProvideEngine( DWFCryptoEngine::eMD5 );
    DWFDigest* pDigest = dynamic_cast<DWFDigest*>( pEngine );
    if (pDigest)
    {
        return DWFCORE_ALLOC_OBJECT(DWFDigestOutputStream( pDigest, pStream, bOwnStream ));
    }
    else
    {
        _DWFCORE_THROW( DWFInvalidTypeException, /*NOXLATE*/L"The cryptoengine provider did not return an engine with a digest interface" );
    }
}

_DWFTK_API
DWFDigestInputStream*
DWFSignature::DigestMethodMD5::chainInputStream(DWFInputStream* pStream, bool bOwnStream) const
throw( DWFException )
{
    DWFCryptoEngine* pEngine = DWFCryptoEngineProvider::ProvideEngine( DWFCryptoEngine::eMD5 );
    DWFDigest* pDigest = dynamic_cast<DWFDigest*>( pEngine );
    if (pDigest)
    {
        return DWFCORE_ALLOC_OBJECT(DWFDigestInputStream( pDigest, pStream, bOwnStream));
    }
    else
    {
        _DWFCORE_THROW( DWFInvalidTypeException, /*NOXLATE*/L"The cryptoengine provider did not return an engine with a digest interface" );
    }
}
