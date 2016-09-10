//
//  Copyright (c) 2003-2006 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED,
//  AS TO THE CORRECTNESS OF THIS CODE OR ANY DERIVATIVE
//  WORKS WHICH INCORPORATE IT.
//
//  AUTODESK PROVIDES THE CODE ON AN "AS-IS" BASIS
//  AND EXPLICITLY DISCLAIMS ANY LIABILITY, INCLUDING
//  CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//

#include <math.h>  // for ceil()
#include <cassert>
#include "dwfcore/Core.h"
#include "dwfcore/Synchronization.h"


#ifdef  DWFCORE_USE_ANSI_STRING

#include "dwfcore/String.h"
using namespace DWFCore;



_DWFCORE_API
DWFString::~DWFString()
throw()
{
    try
    {
        _bRetainHeapBuffer = false; // we *really* mean destroy!
        DWFString::destroy();
    }
    catch (...)
    {
        ;
    }
}

_DWFCORE_API
void
DWFString::destroy()
throw( DWFException )
{
    if (_pHeapBuffer)
    {
        if(_bRetainHeapBuffer && (_nBufferChars>0))
        {
            _pHeapBuffer[0] = 0; // Not really needed, but makes things clearer when examining values in the debugger.
        } else
        {
            DWFCORE_FREE_MEMORY( _pHeapBuffer );
            _nBufferChars = 0;
        }
    }

    _nDataChars = 0;

#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
    _bDataIsFixedASCII = false;
#endif

	//
	// Clean up all the append fragments.
	//
	_tBlock* pLast = _pHead;
	_tBlock* pNext = _pHead;
	while (pNext != NULL)
	{
		pLast = pNext;
		pNext = pNext->_next;
		if(pLast->_buffer != NULL)
		{
			DWFCORE_FREE_OBJECT( pLast->_buffer );
		}
		DWFCORE_FREE_OBJECT( pLast );
	}
	_pHead = NULL;
	_pTail = NULL;
	_size = 0;
}

_DWFCORE_API
DWFString::DWFString()
throw()
        : _pHeapBuffer( NULL )
        , _nBufferChars( 0 )
        , _nDataChars( 0 )
#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
        , _bDataIsFixedASCII( false )
#endif
        , _bRetainHeapBuffer( false )
		, _pHead( NULL )
		, _pTail( NULL )
		, _size (0)
{
    ;
}

_DWFCORE_API
DWFString::DWFString( const DWFString& rString )
throw( DWFException )
        : _pHeapBuffer( NULL )
        , _nBufferChars( 0 )
        , _nDataChars( 0 )
#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
        , _bDataIsFixedASCII( false )
#endif
        , _bRetainHeapBuffer( false )
		, _pHead( NULL )
		, _pTail( NULL )
		, _size (0)
{
    if (rString.chars() > 0)
    {
#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
            _store( rString._pHeapBuffer, rString.bytes(), false, rString._bDataIsFixedASCII );
#else
            _store( rString._pHeapBuffer, rString.bytes() );
#endif
    }
}

_DWFCORE_API
DWFString::DWFString( const char*   pBuffer,
                      size_t        nBufferBytes )
throw( DWFException )
        : _pHeapBuffer( NULL )
        , _nBufferChars( 0 )
        , _nDataChars( 0 )
#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
        , _bDataIsFixedASCII( false )
#endif
        , _bRetainHeapBuffer( false )
		, _pHead( NULL )
		, _pTail( NULL )
		, _size (0)
{
    if (pBuffer && nBufferBytes > 0)
    {
        _store( pBuffer, nBufferBytes, true );
    }
}

_DWFCORE_API
DWFString::DWFString( const char* zASCIIString )
throw( DWFException )
        : _pHeapBuffer( NULL )
        , _nBufferChars( 0 )
        , _nDataChars( 0 )
#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
        , _bDataIsFixedASCII( false )
#endif
        , _bRetainHeapBuffer( false )
		, _pHead( NULL )
		, _pTail( NULL )
		, _size (0)
{
    if (zASCIIString && zASCIIString[0] != 0)
    {
        _store( zASCIIString, DWFCORE_ASCII_STRING_LENGTH(zASCIIString), true );
    }
}

_DWFCORE_API
DWFString::DWFString( const wchar_t*    pBuffer,
                      size_t            nBufferBytes )
throw( DWFException )
        : _pHeapBuffer( NULL )
        , _nBufferChars( 0 )
        , _nDataChars( 0 )
#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
        , _bDataIsFixedASCII( false )
#endif
        , _bRetainHeapBuffer( false )
		, _pHead( NULL )
		, _pTail( NULL )
		, _size (0)
{
    if (pBuffer && nBufferBytes > 0)
    {
        _store( pBuffer, nBufferBytes );
    }
}

_DWFCORE_API
DWFString::DWFString( const wchar_t* zWideString )
throw( DWFException )
        : _pHeapBuffer( NULL )
        , _nBufferChars( 0 )
        , _nDataChars( 0 )
#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
        , _bDataIsFixedASCII( false )
#endif
        , _bRetainHeapBuffer( false )
		, _pHead( NULL )
		, _pTail( NULL )
		, _size (0)
{
    if (zWideString && zWideString[0] != 0)
    {
        _store( zWideString, DWFCORE_WIDE_STRING_LENGTH_IN_BYTES(zWideString) );
    }
}

_DWFCORE_API
DWFString::DWFString( size_t nCharsHint )
throw( DWFException )
        : _pHeapBuffer( NULL )
        , _nBufferChars( 0 )
        , _nDataChars( 0 )
#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
        , _bDataIsFixedASCII( false )
#endif
        , _bRetainHeapBuffer( true ) // disallow heap limit logic
		, _pHead( NULL )
{
    //
    // remember that the character type enum equals that char byte size
    //
    _pHeapBuffer = DWFCORE_ALLOC_MEMORY( wchar_t, nCharsHint + 1 );
    if (_pHeapBuffer == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer" );
    }

    _nBufferChars = nCharsHint + 1;
}

_DWFCORE_API
DWFString&
DWFString::operator=( const DWFString& rString )
throw( DWFException )
{
    //
    // Note, we intentionally do not copy the _bRetainHeapBuffer flag. If it had been set, we
    // do not want an assignment to clear it. If it had not been set, we don't want an assignment
    // to propagate it where the user had not asked.
    //

    if (rString.chars() > 0)
    {
#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
            _store( rString._pHeapBuffer, rString.bytes(), false, rString._bDataIsFixedASCII );
#else
            _store( rString._pHeapBuffer, rString.bytes() );
#endif
    }
    else
    {
        destroy();
    }

    return *this;
}

_DWFCORE_API
void
DWFString::assign( const DWFString& rString )
throw( DWFException )
{
    *this = rString;
}

_DWFCORE_API
void
DWFString::assign( const char*  pBuffer,
                   size_t       nBufferBytes )
throw( DWFException )
{
    if (pBuffer && nBufferBytes > 0)
    {
        _store( pBuffer, nBufferBytes, true );
    }
    else
    {
        destroy();
    }
}

_DWFCORE_API
void
DWFString::assign( const char* zASCIIString )
throw( DWFException )
{
    if (zASCIIString && zASCIIString[0] != 0)
    {
        _store( zASCIIString, DWFCORE_ASCII_STRING_LENGTH(zASCIIString), true) ;
    }
    else
    {
        destroy();
    }
}

_DWFCORE_API
void
DWFString::assign( const wchar_t*   pBuffer,
                   size_t           nBufferBytes )
throw( DWFException )
{
    if (pBuffer && nBufferBytes > 0)
    {
        _store( pBuffer, nBufferBytes );
    }
    else
    {
        destroy();
    }
}

_DWFCORE_API
void
DWFString::assign( const wchar_t* zWideString )
throw( DWFException )
{
    if (zWideString && zWideString[0] != 0)
    {
        _store( zWideString, DWFCORE_WIDE_STRING_LENGTH_IN_BYTES(zWideString) );
    }
    else
    {
        destroy();
    }
}

_DWFCORE_API
void
DWFString::append( const DWFString& rString )
throw( DWFException )
{
#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
    if (_bDataIsFixedASCII || rString._bDataIsFixedASCII)
    {
        _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"This method is not available for fixed ASCII strings." ); 
    }
#endif

	//
	// Need to resolve any append fragments before asking if the string's buffer is on the stack.
	//
	const_cast<DWFString*>(&rString)->_affix();

    {
        _append( rString._pHeapBuffer, rString.bytes() );
    }
}

_DWFCORE_API
void
DWFString::append( const char*  pBuffer,
                   size_t       nBufferBytes )
throw( DWFException )
{
    _append( pBuffer, nBufferBytes, true );
}

_DWFCORE_API
void
DWFString::append( const char* zASCIIString )
throw( DWFException )
{
    _append( zASCIIString, DWFCORE_ASCII_STRING_LENGTH(zASCIIString), true );
}

_DWFCORE_API
void
DWFString::append( const wchar_t*   pBuffer,
                   size_t           nBufferBytes )
throw( DWFException )
{
    _append( pBuffer, nBufferBytes );
}

_DWFCORE_API
void
DWFString::append( const wchar_t* zWideString )
throw( DWFException )
{
    _append( zWideString, DWFCORE_WIDE_STRING_LENGTH_IN_BYTES(zWideString) );
}

_DWFCORE_API
bool
DWFString::operator==( const DWFString& rString )
const
throw()
{
    if (chars() != rString.chars())
    {
        return false;
    }

    if ((chars() == 0) && (rString.chars() == 0))
    {
        return true;
    }

#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
        if (_bDataIsFixedASCII || rString._bDataIsFixedASCII)
        {
            if (_bDataIsFixedASCII && rString._bDataIsFixedASCII)
            {
                return (0 == DWFCORE_COMPARE_ASCII_STRINGS((const char*)_pHeapBuffer, (const char*)((const wchar_t*)rString)));
            }
            else
            {
                return false;
            }
        }
        else
#endif
        {
            return (0 == DWFCORE_COMPARE_WIDE_STRINGS(_pHeapBuffer, (const wchar_t*)rString));
        }
}

_DWFCORE_API
bool
DWFString::operator==( const wchar_t* zWideString )
const
throw()
{
    if (zWideString == NULL)
    {
        return (chars() == 0);
    }

    if (chars() == 0)
    {
        return (zWideString == NULL);
    }

#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
    if (_bDataIsFixedASCII)
    {
        return false;
    }
#endif

    {
        return (0 == DWFCORE_COMPARE_WIDE_STRINGS(_pHeapBuffer, zWideString));
    }
}

_DWFCORE_API
bool
DWFString::operator!=( const DWFString& rString )
const
throw()
{
    return !(operator==(rString));
}

_DWFCORE_API
bool
DWFString::operator!=( const wchar_t* zWideString )
const
throw()
{
    return !(operator==(zWideString));
}

_DWFCORE_API
bool
DWFString::operator<( const DWFString& rString )
const
throw()
{
    if (chars() == 0)
    {
        return ((rString.chars() > 0) ? true : false);
    }
    else if (rString.chars() == 0)
    {
        return ((chars() > 0) ? false : true);
    }

#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
    if (_bDataIsFixedASCII)
    {
        return false;
    }
#endif

    {
        return (DWFCORE_COMPARE_WIDE_STRINGS(_pHeapBuffer, (const wchar_t*)rString) < 0);
    }
}

_DWFCORE_API
bool
DWFString::operator<( const wchar_t* zWideString )
const
throw()
{
    if (zWideString == NULL)
    {
        return ((chars() > 0) ? false : true);
    }
    else if (chars() == 0)
    {
        return true;
    }

#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
    if (_bDataIsFixedASCII)
    {
        return false;
    }
#endif

    {
        return (DWFCORE_COMPARE_WIDE_STRINGS(_pHeapBuffer, zWideString) < 0);
    }

}

_DWFCORE_API
bool
DWFString::operator>( const DWFString& rString )
const
throw()
{
    if (chars() == 0)
    {
        return false;
    }
    else if (rString.chars() == 0)
    {
        return ((chars() > 0) ? true : false);
    }

#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
    if (_bDataIsFixedASCII)
    {
        return false;
    }
#endif

    {
        return (DWFCORE_COMPARE_WIDE_STRINGS(_pHeapBuffer, (const wchar_t*)rString) > 0);
    }
}

_DWFCORE_API
bool
DWFString::operator>( const wchar_t* zWideString )
const
throw()
{
    if (chars() == 0)
    {
        return false;
    }
    else if (zWideString == NULL)
    {
        return ((chars() > 0) ? true : false);
    }

#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
    if (_bDataIsFixedASCII)
    {
        return false;
    }
#endif


    {
        return (DWFCORE_COMPARE_WIDE_STRINGS(_pHeapBuffer, zWideString) > 0);
    }
}

_DWFCORE_API
size_t
DWFString::getUTF8( ASCII_char_t*  pBuffer,
                    size_t         nBufferBytes )
const
throw( DWFException )
{
    if (chars() == 0)
    {
        return 0;
    }

    if ((pBuffer == NULL) || (nBufferBytes == 0))
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No output buffer provided" );
    }


    const wchar_t* pSrcBuffer = NULL;

    {
        pSrcBuffer = _pHeapBuffer;
    }

#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
    if (_bDataIsFixedASCII)
    {
        size_t nBytesToCopy = min( nBufferBytes, bytes() );
        DWFCORE_COPY_MEMORY( pBuffer, (const void*)pSrcBuffer, nBytesToCopy );
        return nBytesToCopy;
    }
    else
#endif

    {
        return EncodeUTF8( pSrcBuffer, bytes(), pBuffer, nBufferBytes );
    }
}

_DWFCORE_API
size_t
DWFString::getUTF8( char** ppBuffer )
const
throw( DWFException )
{
    if (ppBuffer == NULL)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"No output buffer provided" );
    }

	//
	// Need to resolve any append fragments before examining character count.
	//
	const_cast<DWFString*>(this)->_affix();

    //
    // maximize
    //
#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
    size_t nBytes = (_bDataIsFixedASCII ? _nDataChars : (_nDataChars * 6)) + 1;
#else
    size_t nBytes = (_nDataChars * 6) + 1;
#endif

    *ppBuffer = DWFCORE_ALLOC_MEMORY( char, nBytes );
    if (*ppBuffer == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer" );
    }

    DWFCORE_ZERO_MEMORY( *ppBuffer, nBytes );
    return getUTF8( *ppBuffer, nBytes );
}

_DWFCORE_API
off_t
DWFString::find( const wchar_t cFind,
                 off_t         iOffset,
                 bool          bReverse ) const
throw( DWFException )
{
	//
	// Need to resolve any append fragments first.
	//
	const_cast<DWFString*>(this)->_affix();

    return Find( this->DWFString::operator const wchar_t*(), cFind, iOffset, bReverse );
}

_DWFCORE_API
off_t
DWFString::find( const wchar_t* zFind,
                 off_t          iOffset,
                 bool           bReverse ) const
throw( DWFException )
{
	//
	// Need to resolve any append fragments first.
	//
	const_cast<DWFString*>(this)->_affix();

    return Find( this->DWFString::operator const wchar_t*(), zFind, iOffset, bReverse );
}

_DWFCORE_API
off_t
DWFString::findFirst( const wchar_t cFind ) const
throw( DWFException )
{
	//
	// Need to resolve any append fragments first.
	//
	const_cast<DWFString*>(this)->_affix();

    return FindFirst( this->DWFString::operator const wchar_t*(), cFind );
}

_DWFCORE_API
off_t
DWFString::findLast( const wchar_t cFind ) const
throw( DWFException )
{
	//
	// Need to resolve any append fragments first.
	//
	const_cast<DWFString*>(this)->_affix();

    return FindLast( this->DWFString::operator const wchar_t*(), cFind );
}

_DWFCORE_API
DWFString
DWFString::substring( size_t iPos,
                      size_t iLen ) const
throw( DWFException )
{
	//
	// Need to resolve any append fragments first.
	//
	const_cast<DWFString*>(this)->_affix();

#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
    if (_bDataIsFixedASCII)
    {
        _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"This method is not available for fixed ASCII strings." ); 
    }
#endif

    size_t          nLen = chars();
    size_t          nCopy = (iLen == (size_t)-1) ? (nLen - iPos) : ((iPos + iLen > nLen) ? nLen - iPos : iLen);
    const wchar_t*  pIn = this->DWFString::operator const wchar_t*();

    if (iPos < 0 || iPos >= nLen)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Invalid starting position" );
    }

    if (nLen > 0)
    {
        DWFPointer<wchar_t> apNew( DWFCORE_ALLOC_MEMORY(wchar_t, nCopy + 1), true );
        if (apNew.isNull())
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate buffer" );
        }

        DWFCORE_ZERO_MEMORY( (wchar_t*)apNew, (nCopy + 1)*sizeof(wchar_t) );
        DWFCORE_COPY_MEMORY( (wchar_t*)apNew, (pIn += iPos), nCopy*sizeof(wchar_t) );

        return DWFString( (wchar_t*)apNew );
    }
    else
    {
        return DWFString( /*NOXLATE*/L"" );
    }
}

void
DWFString::_store( const void*  pBuffer,
                   size_t       nBufferBytes,
                   bool         bDecodeUTF8,
                   bool         bFixedASCII )
throw( DWFException )
{
#ifdef  DWFCORE_STRING_DISABLE_FIXED_ASCII
    if (bFixedASCII)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Usage of this parameter is forbidden in this build configuration." );
    }
#endif

    wchar_t* pSrc = NULL;

    if (bDecodeUTF8)
    {
#ifndef  DWFCORE_STRING_DISABLE_FIXED_ASCII
        if (bFixedASCII)
        {
            _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Usage of these parameters cannot be combined." );
        }
#endif
    
        pSrc = DWFCORE_ALLOC_MEMORY( wchar_t, nBufferBytes + 1 );
        if (pSrc == NULL)
        {
            _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate decoding buffer" );
        }

        nBufferBytes = DecodeUTF8( (const ASCII_char_t*)pBuffer, nBufferBytes,
                                    pSrc,                       (nBufferBytes + 1) * sizeof(wchar_t) );
    }
    else
    {
        pSrc = (wchar_t*)pBuffer;
    }

    {
        if (bFixedASCII)
        {
            //
            // size of new data in ASCII chars
            //
            _nDataChars = nBufferBytes;
        }
        else
        {
            //
            // size of new data in wide chars
            //
            _nDataChars = ((nBufferBytes / sizeof(wchar_t)) + (nBufferBytes % sizeof(wchar_t)));
        }

            //
            // reuse or free previous buffer
            //
        if (_pHeapBuffer)
        {
                //
                // reuse this buffer if the incoming
                // data will fit into the existing buffer
                //
            if (_nDataChars < _nBufferChars)
            {
                    //
                    // if the current buffer size exceeds
                    // the preferred limit then free the buffer and allocate a new one
                    //
                if ( (_nBufferChars > _DWFCORE_STRING_PREFER_HEAP_LIMIT_IN_WCHARS) && (!_bRetainHeapBuffer) )
                {
                    DWFCORE_FREE_MEMORY( _pHeapBuffer );

                    _nBufferChars = 0;
                }
            }
                //
                // must resize current buffer or get a new one
                //
            else
            {
                //
                // free previous buffer
                //
                DWFCORE_FREE_MEMORY( _pHeapBuffer );

                _nBufferChars = 0;
            }
        }

            //
            // allocate a new buffer?
            //
        if (_pHeapBuffer == NULL)
        {
            _nBufferChars = _nDataChars + 1;

            //
            // remember that the character type enum equals that char byte size
            //
            _pHeapBuffer = DWFCORE_ALLOC_MEMORY( wchar_t, _nBufferChars );

            if (_pHeapBuffer == NULL)
            {
                _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate new heap buffer" );
            }
        }

        DWFCORE_ZERO_MEMORY( _pHeapBuffer, _nBufferChars*sizeof(wchar_t));
        DWFCORE_COPY_MEMORY( _pHeapBuffer, pSrc, nBufferBytes );


    }

        //
        // need to clean up the decoding buffer
        //
    if (bDecodeUTF8)
    {
        DWFCORE_FREE_MEMORY( pSrc );
    }

#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII

    else
    {
        _bDataIsFixedASCII = bFixedASCII;
    }

#endif

}

void
DWFString::_append( const void* pBuffer,
                    size_t      nBufferBytes,
                    bool        bDecodeUTF8 )
throw( DWFException )
{
#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
    if (_bDataIsFixedASCII)
    {
        _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"This method is not available for fixed ASCII strings." ); 
    }
#endif

    if (_nDataChars == 0)
    {
        _store( pBuffer, nBufferBytes, bDecodeUTF8 );
    }
    else
    {
        wchar_t* pSrc = NULL;

        if (bDecodeUTF8)
        {
            pSrc = DWFCORE_ALLOC_MEMORY( wchar_t, nBufferBytes + 1 );
            if (pSrc == NULL)
            {
                _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate decoding buffer" );
            }

            nBufferBytes = DecodeUTF8( (const ASCII_char_t*)pBuffer, nBufferBytes,
                                        pSrc,                       (nBufferBytes + 1) * sizeof(wchar_t) );
        }
        else
        {
            pSrc = (wchar_t*)pBuffer;
        }

        //
        // size of new buffer
        //
        size_t nIncomingChars = ((nBufferBytes / sizeof(wchar_t)) + (nBufferBytes % sizeof(wchar_t)));
        size_t nRequiredChars = _nDataChars + nIncomingChars;

            //
            // there is space in the current buffer - use that
            //
        if ( _pHead==NULL && (nRequiredChars + sizeof(wchar_t)) <= _nBufferChars )
        {
            wchar_t* pDest = NULL;

            {
                pDest = &_pHeapBuffer[_nDataChars];
            }

            _nDataChars += nIncomingChars;
            DWFCORE_COPY_MEMORY( pDest, pSrc, nBufferBytes );
        }
        else
        {
            //
            // alloc fragments
            //
			_tBlock* pNext = NULL;
			if (_pHead == NULL)
			{
				_pHead = DWFCORE_ALLOC_OBJECT( _tBlock );
				_pHead->_next = NULL;
				_pHead->_buffer = DWFCORE_ALLOC_MEMORY( wchar_t, nIncomingChars+1 );
				_pHead->size = nIncomingChars;
				_size += nIncomingChars;

				pNext = _pHead;
				_pTail = pNext;
			}
			else
			{
				_pTail->_next = DWFCORE_ALLOC_OBJECT( _tBlock );
				_pTail = _pTail->_next;

				_pTail->_next= NULL;
				_pTail->_buffer = DWFCORE_ALLOC_MEMORY( wchar_t, nIncomingChars+1 );
				_pTail->size = nIncomingChars;
				_size += nIncomingChars;

				pNext = _pTail;				
			}			

			wchar_t* pDest = pNext->_buffer;
            DWFCORE_COPY_MEMORY( pDest, pSrc, nIncomingChars*sizeof(wchar_t) );
			pDest[nIncomingChars] = 0;
        }

            //
            // need to clean up the decoding buffer
            //
        if (bDecodeUTF8)
        {
            DWFCORE_FREE_MEMORY( pSrc );
        }
    }
}

//
// Coalesce fragments
//
_DWFCORE_API
void
DWFString::_affix()
throw( DWFException )
{
	if (_pHead == NULL)
	{
		return;
	}

	size_t nLen = (_nDataChars * sizeof(wchar_t));
	_tBlock* pNext = _pHead;
	
	nLen += _size * sizeof(wchar_t);

	wchar_t* pNew = DWFCORE_ALLOC_MEMORY( wchar_t, nLen + sizeof(wchar_t) );
	wchar_t* pT = pNew;


    {
        DWFCORE_COPY_MEMORY( pT, _pHeapBuffer, (_nDataChars * sizeof(wchar_t)) );
    }

	pT += _nDataChars;

	for (pNext = _pHead; pNext != NULL; pNext = pNext->_next)
	{
		DWFCORE_COPY_MEMORY( pT, pNext->_buffer, pNext->size * sizeof(wchar_t) );
		pT += pNext->size;
	}
	*pT++ = 0;
	*pT = 0;

		//
        // free previous buffer, free append fragments
        //
	destroy();

    //
    // size of new data in wide chars
    //
    _nDataChars = ((nLen / sizeof(wchar_t)) + (nLen % sizeof(wchar_t)));
    _nBufferChars = _nDataChars + 1;

    _pHeapBuffer = pNew;


#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII
    _bDataIsFixedASCII = false;
#endif

}

_DWFCORE_API
size_t
DWFString::EncodeUTF8( const UCS2_char_t*   zUCS2String,
                       size_t               nUCS2StringBytes,
                       ASCII_char_t*        pUTF8Buffer,
                       size_t               nUTF8BufferBytes )
throw( DWFOverflowException )
{
    unsigned char cHi = 0,
                  cLo = 0;
    char* pIn = (char*)zUCS2String;
    char* pOut = (char*)pUTF8Buffer;

    size_t nUsed = 0;
    size_t nAllowed = nUTF8BufferBytes - sizeof(ASCII_char_t);
    size_t nLen = nUCS2StringBytes >> 1;

    for (register size_t i = 0; i < nLen; i++, pIn+=2)
    {
        //
        // check for null terminated string
        //
        if (zUCS2String[i] == 0)
        {
            break;
        }

#ifdef _DWFCORE_LITTLE_ENDIAN_SYSTEM

        cHi = *pIn;
        cLo = *(pIn+1);
#else

        cLo = *pIn;
        cHi = *(pIn+1);
#endif

        if ((cLo == 0) && (cHi < 0x80))
        {
            if ((++nUsed > nAllowed) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small" );
            }

            if (pOut)
            {
                *(pOut++) = cHi;
            }
        }
        else if (cLo < 0x08)
        {
            if (((nUsed+=2) > nAllowed) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small" );
            }

            if (pOut)
            {
                *(pOut++) = (0xc0 | (cLo << 2) | (cHi >> 6));           // 110lllhh
                *(pOut++) = (0x80 | (cHi & 0x3f));                      // 10hhhhhh
            }
        }
        else
        {
            if (((nUsed+=3) > nAllowed) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small" );
            }

            if (pOut)
            {
                *(pOut++) = (0xe0 | (cLo >> 4));                        // 1110llll
                *(pOut++) = (0x80 | ((cLo & 0x0f) << 2) | cHi >> 6);    // 10llllhh
                *(pOut++) = (0x80 | (cHi & 0x3f));                      // 10hhhhhh
            }
        }
    }

    if (pOut)
    {
        *(pOut++) = 0;
    }

    return nUsed;
}

_DWFCORE_API
size_t
DWFString::EncodeUTF8( const UCS4_char_t*   zUCS4String,
                       size_t               nUCS4StringBytes,
                       ASCII_char_t*        pUTF8Buffer,
                       size_t               nUTF8BufferBytes )
throw( DWFOverflowException )
{
    unsigned char a = 0,
                  b = 0,
                  c = 0,
                  d = 0;
    char* pIn = (char*)zUCS4String;
    char* pOut = (char*)pUTF8Buffer;

    size_t nUsed = 0;
    size_t nAllowed = nUTF8BufferBytes - sizeof(ASCII_char_t);
    size_t nLen = nUCS4StringBytes >> 2;

    for (register size_t i = 0; i < nLen; i++, pIn+=4)
    {
        //
        // check for null terminated string
        //
        if (zUCS4String[i] == 0)
        {
            break;
        }

#ifdef  _DWFCORE_LITTLE_ENDIAN_SYSTEM

        d = *pIn;
        c = *(pIn+1);
        b = *(pIn+2);
        a = *(pIn+3);
#else

        a = *pIn;
        b = *(pIn+1);
        c = *(pIn+2);
        d = *(pIn+3);
#endif

        if (a >= 0xd8)
        {
            _DWFCORE_THROW( DWFNotImplementedException, /*NOXLATE*/L"Unicode surrogate pairs not yet supported" );
        }
        else if (a >= 0x80)
        {
            _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Not a valid Unicode string" );
        }
        else if (a >= 0x04)
        {
            if (((nUsed+=6) > nAllowed) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small" );
            }

            if (pOut)
            {
                *(pOut++) = (0xfc | (a >> 6));                          // 1111110a
                *(pOut++) = (0x80 | (a & 0x3f));                        // 10aaaaaa
                *(pOut++) = (0x80 | (b >> 2));                          // 10bbbbbb
                *(pOut++) = (0x80 | ((b & 0x03) << 4) | (c >> 4));      // 10bbcccc
                *(pOut++) = (0x80 | ((c & 0x0f) << 2) | (d >> 6));      // 10ccccdd
                *(pOut++) = (0x80 | (d & 0x3f));                        // 10dddddd
            }
        }
        else if ((a > 0x0) || (b >= 0x20))
        {
            if (((nUsed+=5) > nAllowed) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small" );
            }

            if (pOut)
            {
                *(pOut++) = (0xf8 | a);                                 // 111110aa
                *(pOut++) = (0x80 | (b >> 2));                          // 10bbbbbb
                *(pOut++) = (0x80 | ((b & 0x03) << 4) | (c >> 4));      // 10bbcccc
                *(pOut++) = (0x80 | ((c & 0x0f) << 2) | (d >> 6));      // 10ccccdd
                *(pOut++) = (0x80 | (d & 0x3f));                        // 10dddddd
            }
        }
        else if (b >= 0x01)
        {
            if (((nUsed+=4) > nAllowed) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small" );
            }

            if (pOut)
            {
                *(pOut++) = (0xf0 | (b >> 2));                          // 11110bbb
                *(pOut++) = (0x80 | ((b & 0x03) << 4) | (c >> 4));      // 10bbcccc
                *(pOut++) = (0x80 | ((c & 0x0f) << 2) | (d >> 6));      // 10ccccdd
                *(pOut++) = (0x80 | (d & 0x3f));                        // 10dddddd
            }
        }
            //
            // drop into UCS-2 encoder
            //
        else if (c >= 0x08)
        {
            if (((nUsed+=3) > nAllowed) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small" );
            }

            if (pOut)
            {
                *(pOut++) = (0xe0 | (c >> 4));                          // 1110cccc
                *(pOut++) = (0x80 | ((c & 0x0f) << 2) | d >> 6);        // 10ccccdd
                *(pOut++) = (0x80 | (d & 0x3f));                        // 10dddddd
            }
        }
        else if ((c > 0x0) || (d >= 0x80))
        {
            if (((nUsed+=2) > nAllowed) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small" );
            }

            if (pOut)
            {
                *(pOut++) = (0xc0 | (c << 2) | (d >> 6));               // 110cccdd
                *(pOut++) = (0x80 | (d & 0x3f));                        // 10dddddd
            }
        }
        else 
        {
            if ((++nUsed > nAllowed) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small" );
            }

            if (pOut)
            {
                *(pOut++) = d;
            }
        }
    }

    if (pOut)
    {
        *(pOut++) = 0;
    }

    return nUsed;
}

_DWFCORE_API
size_t
DWFString::DecodeUTF8( const ASCII_char_t*  zUTF8String,
                       size_t               nUTF8StringBytes,
                       UCS2_char_t*         pUCS2Buffer,
                       size_t               nUCS2BufferBytes )
throw( DWFOverflowException )
{
    unsigned char  c1 = 0,
                   c2 = 0,
                   c3 = 0;
    char* pIn = (char*)zUTF8String;
    char* pOut = (char*)pUCS2Buffer;
    size_t nUsed = 0;
    size_t nAllowed = nUCS2BufferBytes - sizeof(UCS2_char_t);

    for (register size_t i = 0; i < nUTF8StringBytes; i++, pIn++)
    {
        if (((nUsed+=2) > nAllowed) && pOut)
        {
            _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small." );
        }

        c1 = *pIn;

            //
            // one byte
            //
        if (c1 < 0x80)
        {

#ifdef  _DWFCORE_LITTLE_ENDIAN_SYSTEM

            *(pOut++) = c1;
            *(pOut++) = 0x00;
#else

            *(pOut++) = 0x00;
            *(pOut++) = c1;
#endif

        }
            //
            // three bytes
            //
        else if ((c1 & 0xf0) == 0xe0)
        {
            c2 = *(pIn+1);
            c3 = *(pIn+2);

#ifdef  _DWFCORE_LITTLE_ENDIAN_SYSTEM

            *(pOut++) = (((c2 & 0x03) << 6) |  (c3 & 0x3f));
            *(pOut++) = (((c1 & 0x0f) << 4) | ((c2 & 0x3c) >> 2));
#else

            *(pOut++) = (((c1 & 0x0f) << 4) | ((c2 & 0x3c) >> 2));
            *(pOut++) = (((c2 & 0x03) << 6) |  (c3 & 0x3f));
#endif

            i += 2;
            pIn += 2;
        }
            //
            // two bytes
            //
        else
        {
            c2 = *(pIn+1);

#ifdef  _DWFCORE_LITTLE_ENDIAN_SYSTEM

            *(pOut++) = (((c1 & 0x03) << 6) | (c2 & 0x3f));
            *(pOut++) = ((c1 & 0x1c) >> 2);
#else

            *(pOut++) = ((c1 & 0x1c) >> 2);
            *(pOut++) = (((c1 & 0x03) << 6) | (c2 & 0x3f));
#endif

            i++;
            pIn++;
        }
    }

    *(pOut++) = 0;
    *(pOut++) = 0;

    return nUsed;
}

_DWFCORE_API
size_t
DWFString::DecodeUTF8( const ASCII_char_t*  zUTF8String,
                       size_t               nUTF8StringBytes,
                       UCS4_char_t*         pUCS4Buffer,
                       size_t               nUCS4BufferBytes )
throw( DWFOverflowException )
{
    unsigned char  a = 0,
                   b = 0,
                   c = 0,
                   d = 0,
                   e = 0,
                   f = 0;

    char* pIn = (char*)zUTF8String;
    char* pOut = (char*)pUCS4Buffer;
    size_t nUsed = 0;
    size_t nAllowed = nUCS4BufferBytes - sizeof(UCS4_char_t);

    for (register size_t i = 0; i < nUTF8StringBytes; i++, pIn++)
    {
        if ((nUsed+=4) > nAllowed)
        {
            _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small." );
        }

        a = *pIn;

            //
            // six bytes
            //
        if ((a & 0xfc) == 0xfc)
        {
            b = *(pIn+1);
            c = *(pIn+2);
            d = *(pIn+3);
            e = *(pIn+4);
            f = *(pIn+5);

#ifdef  _DWFCORE_LITTLE_ENDIAN_SYSTEM

            *(pOut++) = ((e << 6) | (f & 0x3f));
            *(pOut++) = ((d << 4) | ((e & 0x3c) >> 2));
            *(pOut++) = ((c << 2) | ((d & 0x30) >> 4));
            *(pOut++) = ((a << 6) | (b & 0x3f));
#else

            *(pOut++) = ((a << 6) | (b & 0x3f));
            *(pOut++) = ((c << 2) | ((d & 0x30) >> 4));
            *(pOut++) = ((d << 4) | ((e & 0x3c) >> 2));
            *(pOut++) = ((e << 6) | (f & 0x3f));
#endif

            pIn += 5;
        }
            //
            // five bytes
            //
        else if ((a & 0xf8) == 0xf8)
        {
            b = *(pIn+1);
            c = *(pIn+2);
            d = *(pIn+3);
            e = *(pIn+4);

#ifdef  _DWFCORE_LITTLE_ENDIAN_SYSTEM

            *(pOut++) = ((d << 6) | (e & 0x3f));
            *(pOut++) = ((c << 4) | ((d & 0x3c) >> 2));
            *(pOut++) = ((b << 2) | ((c & 0x30) >> 4));
            *(pOut++) = (a & 0x03);
#else

            *(pOut++) = (a & 0x03);
            *(pOut++) = ((b << 2) | ((c & 0x30) >> 4));
            *(pOut++) = ((c << 4) | ((d & 0x3c) >> 2));
            *(pOut++) = ((d << 6) | (e & 0x3f));
#endif

            pIn += 4;
        }
            //
            // four bytes
            //
        else if ((a & 0xf0) == 0xf0)
        {
            b = *(pIn+1);
            c = *(pIn+2);
            d = *(pIn+3);

#ifdef  _DWFCORE_LITTLE_ENDIAN_SYSTEM

            *(pOut++) = ((c << 6) | (d & 0x3f));
            *(pOut++) = ((b << 4) | ((c & 0x3c) >> 2));
            *(pOut++) = (((a & 0x07) << 2) | ((b & 0x30) >> 4));
            *(pOut++) = 0x00;
#else

            *(pOut++) = 0x00;
            *(pOut++) = (((a & 0x07) << 2) | ((b & 0x30) >> 4));
            *(pOut++) = ((b << 4) | ((c & 0x3c) >> 2));
            *(pOut++) = ((c << 6) | (d & 0x3f));
#endif

            pIn += 3;
        }

            //
            // one byte
            //
        if (a < 0x80)
        {

#ifdef  _DWFCORE_LITTLE_ENDIAN_SYSTEM

            *(pOut++) = a;
            *(pOut++) = 0x00;
            *(pOut++) = 0x00;
            *(pOut++) = 0x00;
#else

            *(pOut++) = 0x00;
            *(pOut++) = 0x00;
            *(pOut++) = 0x00;
            *(pOut++) = a;
#endif

        }
            //
            // three bytes
            //
        else if ((a & 0xf0) == 0xe0)
        {
            b = *(pIn+1);
            c = *(pIn+2);

#ifdef  _DWFCORE_LITTLE_ENDIAN_SYSTEM

            *(pOut++) = (((b & 0x03) << 6) |  (c & 0x3f));
            *(pOut++) = (((a & 0x0f) << 4) | ((b & 0x3c) >> 2));
            *(pOut++) = 0x00;
            *(pOut++) = 0x00;
#else

            *(pOut++) = 0x00;
            *(pOut++) = 0x00;
            *(pOut++) = (((a & 0x0f) << 4) | ((b & 0x3c) >> 2));
            *(pOut++) = (((b & 0x03) << 6) |  (c & 0x3f));
#endif

            i += 2;
            pIn += 2;
        }
            //
            // two bytes
            //
        else
        {
            b = *(pIn+1);

#ifdef  _DWFCORE_LITTLE_ENDIAN_SYSTEM

            *(pOut++) = (((a & 0x03) << 6) | (b & 0x3f));
            *(pOut++) = ((a & 0x1c) >> 2);
            *(pOut++) = 0x00;
            *(pOut++) = 0x00;
#else

            *(pOut++) = 0x00;
            *(pOut++) = 0x00;
            *(pOut++) = ((a & 0x1c) >> 2);
            *(pOut++) = (((a & 0x03) << 6) | (b & 0x3f));
#endif

            i++;
            pIn++;
        }
    }

    *(pOut++) = 0;
    *(pOut++) = 0;
    *(pOut++) = 0;
    *(pOut++) = 0;

    return nUsed;
}

_DWFCORE_API
size_t
DWFString::EncodeBase64( const void*    pRawBuffer,
                         size_t         nRawBufferBytes,
                         void*          pOutBuffer,
                         size_t         nOutBufferBytes,
                         bool           bRFC1521Compliant )
throw( DWFOverflowException )
{
    static char kzBase64Digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static char kzBase64DWFDigits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_";

    char* zBase64Digits = (bRFC1521Compliant ? kzBase64Digits : kzBase64DWFDigits);

    unsigned char* pRaw = (unsigned char*)pRawBuffer;
    unsigned char* pOut = (unsigned char*)pOutBuffer;
    size_t nOut = 0;
    size_t nInputGroups = nRawBufferBytes / 3;
    size_t nRemainderChars = nRawBufferBytes % 3;

        //
        // encode 6-bit quartets
        //
    size_t i = 0;
    for (; i<nInputGroups; i++)
    {
        if (((nOut += 4) > nOutBufferBytes) && pOut)
        {
            _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small" );
        }

        if (pOut)
        {
            *(pOut++) = zBase64Digits[(unsigned char)*pRaw >> 2];
            *(pOut++) = zBase64Digits[((unsigned char)*pRaw & 0x03) << 4 | ((unsigned char)*(pRaw+1) & 0xF0) >> 4];
            *(pOut++) = zBase64Digits[((unsigned char)*(pRaw+1) & 0x0F) << 2 | ((unsigned char)*(pRaw+2) & 0xC0) >> 6];
            *(pOut++) = zBase64Digits[(unsigned char)*(pRaw+2) & 0x3F];
            
            pRaw += 3;
        }
    }

        //
        // encode and pad remaining input groups
        //
    if (nRemainderChars > 0)
    {
        if (bRFC1521Compliant)
        {
            if (((nOut += 4)> nOutBufferBytes) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small" );
            }
        }
            //
            // at least two...
            //
        else
        {
            if (((nOut += 2)> nOutBufferBytes) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small" );
            }
        }

            //
            // always will have the first input group
            //
        if (pOut)
        {
            *(pOut++) = zBase64Digits[(unsigned char)*pRaw >> 2];
        }

        if (nRemainderChars == 1)
        {
            if (pOut)
            {
                //
                // second (abbreviated) input group
                //
                *(pOut++) = zBase64Digits[((unsigned char)*pRaw & 0x03) << 4];

                    //
                    // double pad
                    //
                if (bRFC1521Compliant)
                {
                    *(pOut++) = 0x3D;
                    *(pOut++) = 0x3D;
                }
            } 
        }
        else
        {
            if (pOut)
            {
                //
                // second input group
                //
                *(pOut++) = zBase64Digits[((unsigned char)*pRaw & 0x03) << 4 | ((unsigned char)*(pRaw+1) & 0xF0) >> 4];

                //
                // third (abbreviated) input group
                //
                *(pOut++) = zBase64Digits[((unsigned char)*(pRaw+1) & 0x0F) << 2];

                    //
                    // single pad
                    //
                if (bRFC1521Compliant)
                {
                    *(pOut++) = 0x3D;
                }
                else
                  // in case of not RFC1521 Compliant encoding, we need to take into 
                  // account of the extra byte that is needed to encode the intput string
                  // when input string length modulo of 3 is 1
                  nOut++;

            }
        }
    }

    return nOut;
}

_DWFCORE_API
size_t
DWFString::DecodeBase64( const void*        pRawBuffer,
                         size_t             nRawBufferBytes,
                         void*              pOutBuffer,
                         size_t             nOutBufferBytes,
                         bool               bRFC1521Compliant )
throw( DWFOverflowException )
{
    static char kzBase64Digits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static char kzBase64DWFDigits[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+_";

    // select the right decoding string according to whether it is encoded with
    // RFC1521 compliant
    char* zBase64Digits = (bRFC1521Compliant ? kzBase64Digits : kzBase64DWFDigits);

    if(pOutBuffer)
    {
        DWFCORE_ZERO_MEMORY(pOutBuffer, nOutBufferBytes*sizeof(unsigned char));
    }

    unsigned char *pD =  (unsigned char *)pOutBuffer;
    unsigned char *pOb = (unsigned char*) pRawBuffer;
    char *pEncodedChar;

    size_t nOut = 0;

    // for every 6 bits work, decode the word back to original byte values
    int i = 0;
    int nBitOffset, nByteOffset, nIndex;
    while (i<(int)nRawBufferBytes && ((pEncodedChar=strchr(zBase64Digits,*pOb))!=0) ) 
    {
        nIndex = (int)(pEncodedChar - zBase64Digits);
        nByteOffset = (i*6)/8;
        nBitOffset = (i*6)%8;

        if(pOutBuffer)
        {
            pD[nByteOffset] &= ~((1<<(8-nBitOffset))-1);
        }

        if (nBitOffset < 3) 
        {
            if(pOutBuffer)
            {
                if ((size_t)nByteOffset >= nOutBufferBytes)
                {
                    _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small" );
                }
                pD[nByteOffset] |= (nIndex << (2-nBitOffset));
            }

            nOut = nByteOffset+1;
        } 
        else 
        {
            if(pOutBuffer)
            {
                if ((size_t)(nByteOffset+1) > nOutBufferBytes)
                {
                    _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Buffer too small" );
                }
                pD[nByteOffset] |= (nIndex >> (nBitOffset-2));
                pD[nByteOffset+1] = 0;
                pD[nByteOffset+1] |= (nIndex << (8-(nBitOffset-2))) & 0xFF;
            }
            nOut = nByteOffset+2;
        }
        pOb++; i++;
    } 

    // return the correct number of bytes
    if (bRFC1521Compliant && i != (int)nRawBufferBytes)
    {
        nOut--;
    }
    else if (!bRFC1521Compliant)    
    {
        // note that for RFC1521 non compliant decoding, we need to calculate
        // the correct decoded bytes length
        int tempInt = (int)ceil((float)nRawBufferBytes/4.0);
        nOut = tempInt*3 - (4*tempInt-nRawBufferBytes);
    } 

    return nOut;
}

_DWFCORE_API
size_t
DWFString::EncodeXML( const ASCII_char_t*   zASCIIString,
                      size_t                nASCIIStringBytes,
                      ASCII_char_t*         pXMLBuffer,
                      size_t                nXMLBufferBytes )
throw( DWFException )
{
    if ((zASCIIString == NULL) || (nASCIIStringBytes == 0))
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Invalid argument" );
    }

    if (pXMLBuffer && (nXMLBufferBytes < nASCIIStringBytes))
    {
        _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
    }

    size_t          iIn = 0;
    size_t          iOut = 0;
    ASCII_char_t*   pIn = (ASCII_char_t*)zASCIIString;
    ASCII_char_t*   pOut = pXMLBuffer;
    unsigned char   zIn = 0;

    for (; iIn < nASCIIStringBytes; iIn++)
    {
        zIn = (unsigned char)*pIn++;
        if (zIn && ((zIn < 0x20) || (zIn > 0x7f)))
        {
            size_t nRemaining = nXMLBufferBytes - iOut;
            if (((iOut += 6)> nXMLBufferBytes) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
            }

            if (pOut)
            {
                _DWFCORE_SPRINTF( pOut, nRemaining, "&#x%02X;", zIn );
                pOut += 6;
            }
        }
        else
        {
            if (zIn == 0x3c)        // '<'
            {
                if (((iOut += 4)> nXMLBufferBytes) && pOut)
                {
                    _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
                }

                if (pOut)
                {
                    *pOut++ = 0x26;     // '&'
                    *pOut++ = 0x6c;     // 'l'
                    *pOut++ = 0x74;     // 't'
                    *pOut++ = 0x3b;     // ';'
                }
            }
            else if (zIn == 0x3e)   // '>'
            {
                if (((iOut += 4)> nXMLBufferBytes) && pOut)
                {
                    _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
                }

                if (pOut)
                {
                    *pOut++ = 0x26;     // '&'
                    *pOut++ = 0x67;     // 'g'
                    *pOut++ = 0x74;     // 't'
                    *pOut++ = 0x3b;     // ';'
                }
            }
            else if (zIn == 0x26)   // '&'
            {
                if (((iOut += 5) > nXMLBufferBytes) && pOut)
                {
                    _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
                }

                if (pOut)
                {
                    *pOut++ = 0x26;     // '&'
                    *pOut++ = 0x61;     // 'a'
                    *pOut++ = 0x6D;     // 'm'
                    *pOut++ = 0x70;     // 'p'
                    *pOut++ = 0x3b;     // ';'
                }
            }
            else if (zIn == 0x22)   // '"'
            {
                if (((iOut += 6) > nXMLBufferBytes) && pOut)
                {
                    _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
                }

                if (pOut)
                {
                    *pOut++ = 0x26;     // '&'
                    *pOut++ = 0x71;     // 'q'
                    *pOut++ = 0x75;     // 'u'
                    *pOut++ = 0x6f;     // 'o'
                    *pOut++ = 0x74;     // 't'
                    *pOut++ = 0x3b;     // ';'
                }
            }
            else if (zIn == 0x27)   // '''
            {
                if (((iOut += 6) > nXMLBufferBytes) && pOut)
                {
                    _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
                }

                if (pOut)
                {
                    *pOut++ = 0x26;     // '&'
                    *pOut++ = 0x61;     // 'a'
                    *pOut++ = 0x70;     // 'p'
                    *pOut++ = 0x6f;     // 'o'
                    *pOut++ = 0x73;     // 's'
                    *pOut++ = 0x3b;     // ';'
                }
            }
            else if (*pIn == 0xa)      // Newline
            {
                if (((iOut += 5) > nXMLBufferBytes) && pOut)
                {
                    _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
                }

                if (pOut)
                {
                    *pOut++ = 0x26;     // '&'
                    *pOut++ = 0x23;     // '#'
                    *pOut++ = 0x78;     // 'x'
                    *pOut++ = 0x61;     // 'a'
                    *pOut++ = 0x3b;     // ';'
                }
            }
            else if (*pIn == 0xd)      // Carriage return
            {
                if (((iOut += 5) > nXMLBufferBytes) && pOut)
                {
                    _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
                }

                if (pOut)
                {
                    *pOut++ = 0x26;     // '&'
                    *pOut++ = 0x23;     // '#'
                    *pOut++ = 0x78;     // 'x'
                    *pOut++ = 0x64;     // 'd'
                    *pOut++ = 0x3b;     // ';'
                }
            }
            else if (zIn != 0x00)
            {
                if (((++iOut) > nXMLBufferBytes) && pOut)
                {
                    _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
                }

                if (pOut)
                {
                    *pOut++ = zIn;
                }
            }
        }
    }

    if (pOut)
    {
        *pOut = 0x00;
    }

    return iOut;
}

_DWFCORE_API
size_t
DWFString::EncodeXML( const wchar_t*        zWideString,
                      size_t                nWideStringBytes,
                      wchar_t*              pXMLBuffer,
                      size_t                nXMLBufferBytes,
                      bool                  bHTMLEncodeNonASCII)
throw( DWFException )
{
    if ((zWideString == NULL) || (nWideStringBytes == 0))
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Invalid argument" );
    }

    if (pXMLBuffer && (nXMLBufferBytes < nWideStringBytes))
    {
        _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
    }

    size_t      iIn = 0;
    size_t      iOut = 0;
    wchar_t*    pIn = (wchar_t*)zWideString;
    wchar_t*    pOut = pXMLBuffer;

    for (; *pIn != 0; iIn++, pIn++)
    {
            //
            // Encode non-ascii characters as &#x<unicode code point>; ?
            //
        if ( bHTMLEncodeNonASCII && ((*pIn < 0x20) || (*pIn > 0x7f)) )
        {
            //
            // Check for utf-16 surrogate pairs.
            //
            static const unsigned long UNI_SUR_HIGH_START    = 0xD800;
            static const unsigned long UNI_SUR_HIGH_END      = 0xDBFF;
            static const unsigned long UNI_SUR_LOW_START     = 0xDC00;
            static const unsigned long UNI_SUR_LOW_END       = 0xDFFF;
            static const unsigned int  halfShift             = 10; /* used for shifting by 10 bits */
            static const unsigned long halfBase              = 0x0010000UL;

            unsigned long ch1 = *pIn;
            if ( (ch1 >= UNI_SUR_HIGH_START) && (ch1 <= UNI_SUR_HIGH_END) )
            {
                pIn++; // Move on to the other half of the pair.
                unsigned long ch2 = *pIn;

                    // bad surrogate pair data?
                if ( !(ch2 >= UNI_SUR_LOW_START && ch2 <= UNI_SUR_LOW_END) )
                {
                    _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Not a valid Unicode string" );
                }

                // 12 bytes for full string
                if (((iOut += (12*sizeof(wchar_t)))> nXMLBufferBytes) && pOut)
                {
                    _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
                }

                if (pOut)
                {

                    //
                    // Combine the two halves to recover the true unicode value.
                    //
                    unsigned long unicodeCodePoint = ((ch1 - UNI_SUR_HIGH_START) << halfShift) + (ch2 - UNI_SUR_LOW_START) + halfBase;

                    //
                    // output as 8 character hex
                    //
                    _DWFCORE_SWPRINTF( pOut, 128, L"&#x%08X;", unicodeCodePoint );
                    pOut += 12;
                }
            }
            else
            {

                if (((iOut += (8*sizeof(wchar_t)))> nXMLBufferBytes) && pOut)
                {
                    _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
                }

                if (pOut)
                {
                    _DWFCORE_SWPRINTF( pOut, 128, L"&#x%04X;", *pIn );
                    pOut += 8;
                }
            }
        }
        else if (*pIn == 0x3c)           // '<'
        {
            if (((iOut += (4*sizeof(wchar_t)))> nXMLBufferBytes) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
            }

            if (pOut)
            {
                *pOut++ = 0x26;     // '&'
                *pOut++ = 0x6c;     // 'l'
                *pOut++ = 0x74;     // 't'
                *pOut++ = 0x3b;     // ';'
            }
        }
        else if (*pIn == 0x3e)      // '>'
        {
            if (((iOut += (4*sizeof(wchar_t)))> nXMLBufferBytes) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
            }

            if (pOut)
            {
                *pOut++ = 0x26;     // '&'
                *pOut++ = 0x67;     // 'g'
                *pOut++ = 0x74;     // 't'
                *pOut++ = 0x3b;     // ';'
            }
        }
        else if (*pIn == 0x26)      // '&'
        {
            if (((iOut += (5*sizeof(wchar_t))) > nXMLBufferBytes) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
            }

            if (pOut)
            {
                *pOut++ = 0x26;     // '&'
                *pOut++ = 0x61;     // 'a'
                *pOut++ = 0x6D;     // 'm'
                *pOut++ = 0x70;     // 'p'
                *pOut++ = 0x3b;     // ';'
            }
        }
        else if (*pIn == 0x22)      // '"'
        {
            if (((iOut += (6*sizeof(wchar_t))) > nXMLBufferBytes) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
            }

            if (pOut)
            {
                *pOut++ = 0x26;     // '&'
                *pOut++ = 0x71;     // 'q'
                *pOut++ = 0x75;     // 'u'
                *pOut++ = 0x6f;     // 'o'
                *pOut++ = 0x74;     // 't'
                *pOut++ = 0x3b;     // ';'
            }
        }
        else if (*pIn == 0x27)      // '''
        {
            if (((iOut += (6*sizeof(wchar_t))) > nXMLBufferBytes) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
            }

            if (pOut)
            {
                *pOut++ = 0x26;     // '&'
                *pOut++ = 0x61;     // 'a'
                *pOut++ = 0x70;     // 'p'
                *pOut++ = 0x6f;     // 'o'
                *pOut++ = 0x73;     // 's'
                *pOut++ = 0x3b;     // ';'
            }
        }
        else if (*pIn == 0xa)      // Newline
        {
            if (((iOut += (5*sizeof(wchar_t))) > nXMLBufferBytes) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, L"XML buffer provided is too small" );
            }

            if (pOut)
            {
                *pOut++ = 0x26;     // '&'
                *pOut++ = 0x23;     // '#'
                *pOut++ = 0x78;     // 'x'
                *pOut++ = 0x61;     // 'a'
                *pOut++ = 0x3b;     // ';'
            }
        }
        else if (*pIn == 0xd)      // Carriage return
        {
            if (((iOut += (5*sizeof(wchar_t))) > nXMLBufferBytes) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, L"XML buffer provided is too small" );
            }

            if (pOut)
            {
                *pOut++ = 0x26;     // '&'
                *pOut++ = 0x23;     // '#'
                *pOut++ = 0x78;     // 'x'
                *pOut++ = 0x64;     // 'd'
                *pOut++ = 0x3b;     // ';'
            }
        }
        else if (*pIn != 0x00)
        {
            if (((iOut+=sizeof(wchar_t)) > nXMLBufferBytes) && pOut)
            {
                _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"XML buffer provided is too small" );
            }

            if (pOut)
            {
                *pOut++ = *pIn;
            }
        }
    }

    if (pOut)
    {
        *pOut = 0x00;
    }

    return iOut;
}

_DWFCORE_API
size_t
DWFString::DecodeXML( const ASCII_char_t*   zXMLString,
                      size_t                nXMLStringBytes,
                      ASCII_char_t*         pASCIIBuffer,
                      size_t                nASCIIBufferBytes )
throw( DWFException )
{
    if ((zXMLString == NULL)      ||
        (nXMLStringBytes == 0)    ||
        (pASCIIBuffer == NULL))
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Invalid argument" );
    }

    size_t          iIn = 0;
    size_t          iOut = 0;
    unsigned int    nByteIn = 0;
    ASCII_char_t*   pIn = (ASCII_char_t*)zXMLString;
    ASCII_char_t*   pOut = pASCIIBuffer;

    for (; iIn < nXMLStringBytes; iIn++)
    {
        if (++iOut > nASCIIBufferBytes)
        {
            _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Output buffer too small" );
        }

        if (*pIn == 0x26)                   // '&'
        {
            pIn++;

            if ((*pIn == 0x23)    &&        // '#'
                (*(++pIn) == 0x78))         // 'x'
            {
                //
                // nByteIn is used here to comply with strict format checking
                // in sscanf() by the gcc compiler
                //
                ::sscanf( ++pIn, "%02X;", &nByteIn );
                *pOut = (ASCII_char_t)(0x000000ff & nByteIn);

                pOut++;
                pIn += 3;
            }
            else if ((*pIn == 0x6c) &&      // 'l'
                     (*(++pIn) == 0x74) &&  // 't'
                     (*(++pIn) == 0x3b))    // ';'
            {
                pIn++;
                *pOut++ = 0x3c;             // '<'
            }
            else if ((*pIn == 0x67) &&      // 'g'
                     (*(++pIn) == 0x74) &&  // 't'
                     (*(++pIn) == 0x3b))    // ';'
            {
                pIn++;
                *pOut++ = 0x3e;             // '>'
            }
            else if ((*pIn == 0x61) &&      // 'a'
                     (*(pIn+1) == 0x6D) &&  // 'm'
                     (*(pIn+2) == 0x70) &&  // 'p'
                     (*(pIn+3) == 0x3b))    // ';'
            {
                pIn+=4;
                *pOut++ = 0x26;             // '&'
            }
            else if ((*pIn == 0x71) &&      // 'q'
                     (*(++pIn) == 0x75) &&  // 'u'
                     (*(++pIn) == 0x6f) &&  // 'o'
                     (*(++pIn) == 0x74) &&  // 't'
                     (*(++pIn) == 0x3b))    // ';'
            {
                pIn++;
                *pOut++ = 0x22;             // '"'
            }
            else if ((*pIn == 0x61) &&      // 'a'
                     (*(pIn+1) == 0x70) &&  // 'p'
                     (*(pIn+2) == 0x6f) &&  // 'o'
                     (*(pIn+3) == 0x73) &&  // 's'
                     (*(pIn+4) == 0x3b))    // ';'
            {
                pIn+=5;
                *pOut++ = 0x27;             // '''
            }
            else
            {
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Invalid XML encoding" );
            }
        }
        else
        {
            *pOut++ = *pIn++;
        }
    }

    *pOut = 0x00;
    return iOut;
}

_DWFCORE_API
size_t
DWFString::DecodeXML(   const wchar_t*        zXMLString,
                        size_t                nXMLStringBytes,
                        wchar_t*              pWideStringBuffer,
                        size_t                nWideStringBufferBytes )
    throw( DWFException )
{
    if ((zXMLString == NULL)      ||
        (nXMLStringBytes == 0)    ||
        (pWideStringBuffer == NULL))
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"Invalid argument" );
    }

    size_t          iIn = 0;
    size_t          iOut = 0;
    size_t          nXMLStringChars = nXMLStringBytes / sizeof(wchar_t);
    unsigned int    nWordIn = 0;
    wchar_t*        pIn = (wchar_t*)zXMLString;
    wchar_t*        pOut = pWideStringBuffer;

    for (; iIn < nXMLStringChars; iIn++)
    {
        if ((iOut+=sizeof(wchar_t)) > nWideStringBufferBytes)
        {
            _DWFCORE_THROW( DWFOverflowException, /*NOXLATE*/L"Output buffer too small" );
        }

        if (*pIn == 0x26)                   // '&'
        {
            pIn++;

            if ((*pIn == 0x23)    &&        // '#'
                (*(++pIn) == 0x78))         // 'x'
            {
                //
                // nWordIn is used here to comply with strict format checking
                // in sscanf() by the gcc compiler
                //
                ::swscanf( ++pIn, L"%04X;", &nWordIn );
                *pOut = (wchar_t)nWordIn;

                pOut++;
                pIn += 5; // four hex chars plus the semicolon
            }
            else if ((*pIn == 0x6c) &&      // 'l'
                     (*(++pIn) == 0x74) &&  // 't'
                     (*(++pIn) == 0x3b))    // ';'
            {
                pIn++;
                *pOut++ = 0x3c;             // '<'
            }
            else if ((*pIn == 0x67) &&      // 'g'
                     (*(++pIn) == 0x74) &&  // 't'
                     (*(++pIn) == 0x3b))    // ';'
            {
                pIn++;
                *pOut++ = 0x3e;             // '>'
            }
            else if ((*pIn == 0x61) &&      // 'a'
                     (*(pIn+1) == 0x6D) &&  // 'm'
                     (*(pIn+2) == 0x70) &&  // 'p'
                     (*(pIn+3) == 0x3b))    // ';'
            {
                pIn+=4;
                *pOut++ = 0x26;             // '&'
            }
            else if ((*pIn == 0x71) &&      // 'q'
                     (*(++pIn) == 0x75) &&  // 'u'
                     (*(++pIn) == 0x6f) &&  // 'o'
                     (*(++pIn) == 0x74) &&  // 't'
                     (*(++pIn) == 0x3b))    // ';'
            {
                pIn++;
                *pOut++ = 0x22;             // '"'
            }
            else if ((*pIn == 0x61) &&      // 'a'
                     (*(pIn+1) == 0x70) &&  // 'p'
                     (*(pIn+2) == 0x6f) &&  // 'o'
                     (*(pIn+3) == 0x73) &&  // 's'
                     (*(pIn+4) == 0x3b))    // ';'
            {
                pIn+=5;
                *pOut++ = 0x27;             // '''
            }
            else
            {
                _DWFCORE_THROW( DWFUnexpectedException, /*NOXLATE*/L"Invalid XML encoding" );
            }
        }
        else
        {
            *pOut++ = *pIn++;
        }
    }

    *pOut = 0x00;
    return iOut;
}

_DWFCORE_API
double
DWFString::StringToDouble( const char* zString )
throw( DWFException )
{
    bool bSign = false;
    bool bPowerSign = false;
    bool bExponent = true;
    bool bMantissa = false;

    int nPower = 0;
    double nExponent = 0.0;
    double nMantissa = 0.0;
    double nScale = 1.0;
    char* pIn = (char*)zString;

    if (*pIn == '-')
    {
        *pIn++;
        bSign = true;
    }

    while (*pIn != 0)
    {
            //
            // capture digits
            //
        if ((*pIn >= '0') && (*pIn <= '9'))
        {
            if (bExponent)
            {
                nExponent *= 10.0;
                nExponent += (*pIn - '0');
            }
            else if (bMantissa)
            {
                nMantissa *= 10.0;
                nMantissa += (*pIn - '0');

                nScale *= 10.0;
            }
            else
            {
                nPower *= 10;
                nPower += (*pIn - '0');
            }
        }
            //
            // power
            //
        else if ((*pIn == 'e') || (*pIn == 'E'))
        {
                //
                // sign
                //
            if (*(++pIn) == '-')
            {
                bPowerSign = true;
            }

            bMantissa = false;
        }
            //
            // the first character that hasn't been accounted for already,
            // is assumed to be the separator
            //
        else
        {
                //
                // cross over
                //
            if (bExponent)
            {
                bExponent = false;
                bMantissa = true;
            }
                //
                // white space (including null) or bad char
                // we're done either way
                //
            else
            {
                break;
            }
        }

        pIn++;
    }

    double nVal = nExponent + (nMantissa / nScale);


    if (nPower > 0)
    {
        for (; nPower > 0; nPower--)
        {
            if (bPowerSign)
            {
                nVal /= 10.0;
            }
            else
            {
                nVal *= 10.0;
            }
        }
    }

    if (bSign)
    {
        nVal *= -1.0;
    }

    return nVal;
}

_DWFCORE_API
double
DWFString::StringToDouble( const wchar_t* zString )
throw( DWFException )
{
    bool bSign = false;
    bool bPowerSign = false;
    bool bExponent = true;
    bool bMantissa = false;

    int nPower = 0;
    double nExponent = 0.0;
    double nMantissa = 0.0;
    double nScale = 1.0;
    wchar_t* pIn = (wchar_t*)zString;

    if (*pIn == L'-')
    {
        *pIn++;
        bSign = true;
    }

    while (*pIn != 0)
    {
            //
            // capture digits
            //
        if ((*pIn >= L'0') && (*pIn <= L'9'))
        {
            if (bExponent)
            {
                nExponent *= 10.0;
                nExponent += (*pIn - L'0');
            }
            else if (bMantissa)
            {
                nMantissa *= 10.0;
                nMantissa += (*pIn - L'0');

                nScale *= 10.0;
            }
            else
            {
                nPower *= 10;
                nPower += (*pIn - L'0');
            }
        }
            //
            // power
            //
        else if ((*pIn == L'e') || (*pIn == L'E'))
        {
                //
                // sign
                //
            if (*(++pIn) == L'-')
            {
                bPowerSign = true;
            }

            bExponent = false;
            bMantissa = false;
        }
            //
            // the first character that hasn't been accounted for already,
            // is assumed to be the separator
            //
        else
        {
                //
                // cross over
                //
            if (bExponent)
            {
                bExponent = false;
                bMantissa = true;
            }
                //
                // white space (including null) or bad char
                // we're done either way
                //
            else
            {
                break;
            }
        }

        pIn++;
    }

    double nVal = nExponent + (nMantissa / nScale);


    if (nPower > 0)
    {
        for (; nPower > 0; nPower--)
        {
            if (bPowerSign)
            {
                nVal /= 10.0;
            }
            else
            {
                nVal *= 10.0;
            }
        }
    }

    if (bSign)
    {
        nVal *= -1.0;
    }

    return nVal;
}

_DWFCORE_API
DWFString 
DWFString::DoubleToString( double dValue,short nPrecision,short nWidth)
        throw( DWFException )
{
    wchar_t wcFormattedBuf[64];
    DoubleToString( wcFormattedBuf, 64, dValue, nPrecision, nWidth );
    return wcFormattedBuf;
}

_DWFCORE_API
void
DWFString::DoubleToString( char* pBuffer, size_t nBufferChars, double dValue,short nPrecision,short nWidth)
        throw( DWFException )
{
    if(nPrecision > 17)
    {
        nPrecision = 17;
    }

    if(nWidth != -1)
    {
        _DWFCORE_SPRINTF(pBuffer, nBufferChars, "%*.*f", nWidth, nPrecision, dValue);
    }
    else
    {
        _DWFCORE_SPRINTF(pBuffer, nBufferChars, "%.*G", nPrecision, dValue);
    }

    RepairDecimalSeparators(pBuffer);
}

_DWFCORE_API
void
DWFString::DoubleToString( wchar_t* pBuffer, size_t nBufferChars, double dValue,short nPrecision,short nWidth)
        throw( DWFException )
{
    if(nPrecision > 17)
    {
        nPrecision = 17;
    }

    if(nWidth != -1)
    {
        _DWFCORE_SWPRINTF(pBuffer, nBufferChars, L"%*.*f", nWidth,nPrecision, dValue);
    }
    else
    {
        _DWFCORE_SWPRINTF(pBuffer, nBufferChars, L"%.*G", nPrecision, dValue);
    }

    RepairDecimalSeparators(pBuffer);
}

_DWFCORE_API
wchar_t*
DWFString::RepairDecimalSeparators( wchar_t* zString )
throw( DWFException )
{
    wchar_t* pIn = (wchar_t*)zString;
    wchar_t* pNext = pIn;

    while (*pIn != 0)
    {
        if (++pNext != 0)
        {
            if ((*pIn >= L'0') && (*pIn <= '9'))
            {
                if (((*pNext < L'0') || (*pNext > '9'))     &&
                    ((*pNext > 0x20) && (*pNext != 0x7F))   &&
                    ((*(pNext+1) >= L'0') && (*(pNext+1) <= L'9')))
                {
                    *pNext++ = L'.';
                    pIn = pNext++;
                }
            }
        }

        pIn++;
    }

    return zString;
}

_DWFCORE_API
char*
DWFString::RepairDecimalSeparators( char* zString )
throw( DWFException )
{
    char* pIn = zString;
    char* pNext = pIn;

    while (*pIn != 0)
    {
        if (++pNext != 0)
        {
            if ((*pIn >= '0') && (*pIn <= '9'))
            {
                if (((*pNext < '0') || (*pNext > '9'))     &&
                    ((*pNext > 0x20) && (*pNext != 0x7F))   &&
                    ((*(pNext+1) >= '0') && (*(pNext+1) <= '9')))
                {
                    *pNext++ = '.';
                    pIn = pNext++;
                }
            }
        }

        pIn++;
    }

    return zString;
}

_DWFCORE_API
off_t
DWFString::Find( const wchar_t* zString,
                 const wchar_t  cFind,
                 off_t          iOffset,
                 bool           bReverse )
throw( DWFInvalidArgumentException )
{
    if (iOffset < 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"iOffset must be greater than or equal to zero." );
    }

    if(zString == NULL)
    {
        return -1;
    }

    // iFind is our return value. -1 means no match.
    off_t iFind = -1;

    // nlen has the number of characters in the string.
    off_t nLen = (off_t)DWFCORE_WIDE_STRING_LENGTH_IN_WCHARS( zString );

        //
        // Only do the work if:
        //    - the string has length
        //    - the offset isn't so large that we would walk off the end
        //
    if ( (nLen > 0) && (nLen > iOffset) )
    {
        // iNext controls stepping forward (1) or backwards (-1)
        off_t           iNext = 0;

        // iStart is the position in the string to start looking from, always relative to the start of the string.
        off_t           iStart = 0;

        // nCharsToSearch controls how many characters should be searched.
        off_t           nCharsToSearch = nLen - iOffset;

        if (bReverse)
        {
            // walk backwards
            iNext = -1;

            // start at the last character in the string, moving backward by the given offset
            iStart = (nLen - 1) - iOffset;
        }
        else
        {
            // walk forwards
            iNext = 1;

            // start at the first character in the string, moving forward by the given offset
            iStart = iOffset;
        }


        // pNext points to the current character to be compared. This moves through the string in the loop, below.
        const wchar_t*  pNext = zString + iStart;

        //
        // Check each of the characters, moving in the direction specified by bReverse (expressed in iNext).
        //
        for (;
             nCharsToSearch > 0;
             nCharsToSearch--,
             pNext += iNext)
        {
            if (*pNext == cFind)
            {
                // Match. Compute the offset from the start of the given string.
                iFind = (off_t)(pNext - zString);

                // We are only looking for one char, so if we've see it, we're done.
                break;
            }
        }
    }

    return iFind;
}

_DWFCORE_API
off_t
DWFString::Find( const wchar_t* zString,
                 const wchar_t* zFind,
                 off_t          iOffset,
                 bool           bReverse )
throw( DWFInvalidArgumentException )
{
    if (iOffset < 0)
    {
        _DWFCORE_THROW( DWFInvalidArgumentException, /*NOXLATE*/L"iOffset must be greater than or equal to zero." );
    }

    if(zString == NULL)
    {
        return -1;
    }

    if(zFind == NULL)
    {
        return -1;
    }

    size_t nFindByteLen   = DWFCORE_WIDE_STRING_LENGTH_IN_BYTES( zFind );
    size_t nFindCharLen   = DWFCORE_WIDE_STRING_LENGTH_IN_WCHARS( zFind );
    size_t nStringCharLen = DWFCORE_WIDE_STRING_LENGTH_IN_WCHARS( zString );

    //
    // find first character in substring
    //
    off_t iFound = DWFString::Find( zString, zFind[0], iOffset, bReverse );
    
        //
        // find next occurence
        //
    for (;iFound != -1;
          iFound = DWFString::Find(zString, zFind[0], iFound+1, bReverse))
    {
        //
        // ensure we have enough bytes in the string so that we don't overrun memory
        //
        if (nStringCharLen - iFound >= nFindCharLen)
        {
                //
                // match substring
                //
            if (DWFCORE_COMPARE_MEMORY(&zString[iFound], zFind, nFindByteLen) == 0)
            {
                break;
            }
        }
        else
        {
            iFound = -1;
            break;
        }
    }

    return iFound;
}

_DWFCORE_API
off_t
DWFString::FindFirst( const wchar_t* zString,
                      const wchar_t  cFind )
throw()
{
    return Find( zString, cFind );
}

_DWFCORE_API
off_t
DWFString::FindLast( const wchar_t* zString,
                     const wchar_t  cFind )
throw()
{
    return Find( zString, cFind, 0, true );
}

#ifndef DWFCORE_STRING_DISABLE_FIXED_ASCII

_DWFCORE_API
DWFString*
DWFString::BuildFixedASCIIString( const char* zASCIIString )
throw( DWFException )
{
        //
        // create a new, empty DWFString
        //
    DWFString* pASCIIString = DWFCORE_ALLOC_OBJECT( DWFString );
    if (pASCIIString == NULL)
    {
        _DWFCORE_THROW( DWFMemoryException, /*NOXLATE*/L"Failed to allocate memory" );
    }

    //
    // invoke special storage for fixed ASCII
    // this will place the resultant DWFString in a special state
    // forbidding almost all operations on the object
    //
    pASCIIString->_store( zASCIIString,
                          DWFCORE_ASCII_STRING_LENGTH(zASCIIString),
                          false,
                          true );

    return pASCIIString;
}

#endif

///
///
///


struct _scoped_lock
{
    _scoped_lock( DWFThreadMutex* pMutex ) : _pMutex(pMutex) { _pMutex->lock(); }
    ~_scoped_lock() { _pMutex->unlock(); }
    DWFThreadMutex* _pMutex;
};




DWFStringTable& 
DWFStringTable::Instance()
{
	static DWFStringTable koSingleton;
	return koSingleton;
}

void 
DWFStringTable::Purge()
{
    _scoped_lock oLock( Instance()._pMutex );

    Instance()._oTable.clear();
    Instance()._oIndex.clear();

    Instance()._oTable.resize( 1 );

	Instance()._oIndex.insert( &Instance()._oTable[0]);
}

DWFStringTable::DWFStringTable() 
              : _pMutex( 0 )
{
    _pMutex = DWFCORE_ALLOC_OBJECT( DWFThreadMutex );
    _pMutex->init();
	_oTable.resize( 1 );
	_oIndex.insert(&_oTable[0]);
}

DWFStringTable::~DWFStringTable()
{
    if (_pMutex)
    {
        _pMutex->destroy();
        DWFCORE_FREE_OBJECT( _pMutex );
    }
}

const DWFString*
DWFStringTable::insert( const DWFString& zText )
{
    _scoped_lock oLock( _pMutex );

	_tIndex::const_iterator itr = _oIndex.find( &zText );
	
    if (itr != _oIndex.end())
    {
		return *itr;
    }

	_oTable.push_back( zText );
	_oIndex.insert( &_oTable.back());

	return &_oTable.back();
}

#endif

