#include "dwfcore/Core.h"
#include "dwf/Toolkit.h"
#include "dwfcore/File.h"
#include "XAML/XamlClassFactory.h"
#include "dwf/XAML/pch.h"

#ifndef OpcHelper_HEADER
#define OpcHelper_HEADER

using namespace DWFCore;
using namespace DWFToolkit;

//Define OBFUSCATE if we want odttf font files
//#define OBFUSCATE

class OpcHelper;

class XamlResourceInputStream : public DWFInputStream
{
    friend class OpcHelper;

public:
    XamlResourceInputStream()
        : _pInputStream( NULL )
        , _pBuffer( NULL )
        , _nBufferBytes( 0 )
    {
        ;
    }

    XamlResourceInputStream( XamlResourceInputStream& rSource)
    {
        _nBufferBytes = rSource.available();
        _pBuffer = DWFCORE_ALLOC_MEMORY( unsigned char, _nBufferBytes );
        rSource.read( _pBuffer, _nBufferBytes );
        rSource.seek( SEEK_SET, 0 );
        _pInputStream = DWFCORE_ALLOC_OBJECT(DWFBufferInputStream( _pBuffer, _nBufferBytes ));
    }
    virtual ~XamlResourceInputStream()throw()
    {
        DWFCORE_FREE_MEMORY( _pBuffer );
        DWFCORE_FREE_OBJECT( _pInputStream );
    }

    void provideBytes( const void* pBuffer, size_t nBufferBytes)
    {
        if (_pBuffer)
        {
            _nBufferBytes = 0;
            DWFCORE_FREE_MEMORY( _pBuffer );
        }

        _nBufferBytes = nBufferBytes;
        _pBuffer = DWFCORE_ALLOC_MEMORY( unsigned char, _nBufferBytes );
        DWFCORE_COPY_MEMORY( _pBuffer, pBuffer, _nBufferBytes );
        if (_pInputStream)
        {
            DWFCORE_FREE_OBJECT( _pInputStream );
        }
        _pInputStream = DWFCORE_ALLOC_OBJECT(DWFBufferInputStream( _pBuffer, _nBufferBytes ));
    }

    virtual size_t available() const
        throw( DWFException )
    {
        if (_pInputStream == NULL)
        {
            return 0;
        }
        else
        {
            return _pInputStream->available();
        }
    }

    virtual size_t read( void*  pBuffer,
                         size_t nBytesToRead )
        throw( DWFException )
    {
        if (_pInputStream == NULL)
        {
            return 0;
        }
        else
        {
            return _pInputStream->read( pBuffer, nBytesToRead );
        }
    }

    virtual off_t seek( int    eOrigin,
                        off_t  nOffset )
        throw( DWFException )
    {
        if (_pInputStream == NULL)
        {
            return 0;
        }
        else
        {
            return _pInputStream->seek( eOrigin, nOffset );
        }
    }

private:
    DWFBufferInputStream* _pInputStream;
    unsigned char*        _pBuffer;
    size_t                _nBufferBytes;
};

class XamlResourceOutputStream : public DWFBufferOutputStream
{
public:
    XamlResourceOutputStream( XamlResourceInputStream* pInputStream)
        : DWFBufferOutputStream( 1024 )
        , _pInputStream( pInputStream )
    {
        ;
    }

    virtual ~XamlResourceOutputStream()throw()
    {
        _pInputStream->provideBytes( buffer(), bytes() );
    }

private:
    XamlResourceInputStream* _pInputStream;
};

class OpcHelper : public WT_OpcResourceSerializer, public WT_OpcResourceMaterializer
{
public:
	OpcHelper();
	virtual ~OpcHelper();

    void setCurrentSection( DWFSection* pSection);

public:
    //WT_OpcResourceMaterializer
    WT_Result getPartInputStream(
		const DWFCore::DWFString& rzPath, 
		DWFCore::DWFInputStream** ppStream);

    //WT_OpcResourceSerializer
    WT_Result getPartOutputStream(
        const DWFCore::DWFString& rzMimeType,
        DWFCore::DWFString& rzPath,
        DWFCore::DWFOutputStream** ppStream);

    WT_Result getSubsetFontName(
        WT_String& zFontName,
        WT_Integer32 nHeight,
        const WT_String &zStr,
        bool  bForEmbeddedFontObject = false);

private:

    int         _nPartCount;
    DWFSection* _pSection;
    DWFUUID     _oUuidGen;
    DWFString   _zNextOutputStreamPath;
    DWFBufferOutputStream* _pStreamCopy;

    typedef std::map<DWFString, XamlResourceInputStream*>  _tPathToStreamMap;
    _tPathToStreamMap _oMap;

    void _addGfxRelationship(const DWFString& zRel, DWFResource* pTarget );
};

#endif //XAML_TEST_HEADER
