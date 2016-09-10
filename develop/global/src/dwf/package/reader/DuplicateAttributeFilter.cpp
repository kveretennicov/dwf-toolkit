//
//  Copyright (c) 2005-2006 by Autodesk, Inc.
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


#include "dwf/package/reader/DuplicateAttributeFilter.h"
using namespace DWFToolkit;


#include "dwfcore/SkipList.h"
using namespace DWFCore;


_DWFTK_API
DWFDuplicateAttributeFilter::DWFDuplicateAttributeFilter( DWFInputStream* pInputStream,
                                                          bool            bOwnStream )
throw()
                           : DWFBufferInputStream( pInputStream, bOwnStream )
{
    ;
}


_DWFTK_API
DWFDuplicateAttributeFilter::~DWFDuplicateAttributeFilter()
throw()
{
    ;
}

_DWFTK_API
size_t
DWFDuplicateAttributeFilter::available() const
throw( DWFException )
{
    return DWFBufferInputStream::available();
}

_DWFTK_API
size_t
DWFDuplicateAttributeFilter::read( void*  pBuffer,
                                   size_t nBytesToRead )
throw( DWFException )
{
    //
    // perform requested read on source stream
    //
    size_t nBytesRead = DWFBufferInputStream::read( pBuffer, nBytesToRead );

    //
    //
    //
    char* pIn = (char*)pBuffer;
    bool bElement = false;
    bool bEraseValue = false;

    size_t iPos = 0;
    size_t iFirst = 0;
    size_t iLast = 0;
    size_t nLen = 0;
    char zTemp[64] = {0};
    DWFStringKeySkipList<bool> oAttributes;

        //
        // scan
        //
    while (iPos < nBytesRead)
    {
        if (bElement)
        {
            switch (pIn[iPos])
            {
                case '>':
                {
                    oAttributes.clear();
                    bElement = false;
                    break;
                }

                case ' ':
                {
                    if (bEraseValue)
                    {
                        bEraseValue = false;
                    }
                    else if ((iFirst != 0) && (iLast == 0))
                    {
                        iLast = iPos - 1;
                    }
                    break;
                }

                case '=':
                {
                    if ((iFirst != 0) && (iLast == 0))
                    {
                        iLast = iPos - 1;
                        nLen = 1 + iLast - iFirst;

                        zTemp[nLen] = 0;
                        while (iFirst <= iLast && iLast>0)
                        {
                            zTemp[iLast - iFirst] = pIn[iLast--];
                        }

                        if (oAttributes.insert(DWFString(zTemp), true, false) == false)
                        {
                            ::memset( &pIn[iFirst], 0x20, nLen );
                            pIn[iPos] = ' ';
                            bEraseValue = true;
                        }

                        iFirst = iLast = nLen = 0;
                        DWFCORE_ZERO_MEMORY( &zTemp, nLen );
                    }

                    break;
                }

                default:
                {
                    if (bEraseValue)
                    {
                        pIn[iPos] = ' ';
                    }
                    else if ((iFirst == 0) || (iLast > 0))
                    {
                        iFirst = iPos;
                        iLast = 0;
                    }
                }
            }
        }
        else if (pIn[iPos] == '<')
        {
            bElement = true;
            iFirst = iLast = 0;
        }

        iPos++;
    }

    return nBytesRead;
}

_DWFTK_API
off_t
DWFDuplicateAttributeFilter::seek( int     eOrigin,
                                   off_t   nOffset )
throw( DWFException )
{
    return DWFBufferInputStream::seek( eOrigin, nOffset );
}

