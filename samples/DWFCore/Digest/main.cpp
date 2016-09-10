//
//  Copyright (c) 2006 by Autodesk, Inc.
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

#include <iostream>
#include <iomanip>

#include "dwfcore/DigestInputStream.h"
#include "dwfcore/DigestOutputStream.h"
#include "dwfcore/FileInputStream.h"
#include "dwfcore/FileOutputStream.h"
#include "dwfcore/StreamFileDescriptor.h"

using namespace std;
using namespace DWFCore;

/*
    This sample shows the use of digests in the DWFCore. To calculate the digest 
    when reading in a file or writing it out, create a digest input stream, with
    the a digest object. NOTE: The digest input stream will own the digest.
    e.g.
        DWFDigest* pDigest = DWFCORE_ALLOC_OBJECT( DWFMD5Digest* );
        DWFDigestInputStream oInDigestStream( pDigest, NULL, false );
    and then chain the source (or target) stream to the digest stream. E.g.
        oInDigestStream.chainInputStream( &oInFilestream, false );

    Now read in (or write out) the data using the digest stream as you would 
    directly with the source (or target) stream. Once the reading (or writing)
    is complete, you can query the digest stream for the digest value.

    Digests that are progressive can be queried for the digest during the read/write
    operations, but this maybe quite a bit slower since a final digest calculation
    can correspond to multiple digest updates depending on the type of digest.
*/


std::wostream& operator<<( std::wostream& os, const DWFString& zString )
{
    os << (const wchar_t*)zString;
    return os;
}

void ShowFileDigest( const DWFString& zFileName,
                     DWFDigestInputStream& rInDigestStream,
                     DWFDigestOutputStream& rOutDigestStream )
{
    //
    //  Create the file and descriptor, for reading.
    //
    DWFFile                 oInFile( zFileName );
    DWFStreamFileDescriptor oInDescriptor( oInFile, L"rb" );
    oInDescriptor.open();

    //
    //  Create the file and descriptor, for writing.
    //
    DWFString               zOutFileName( "Out_" );
    zOutFileName.append( zFileName );
    DWFFile                 oOutFile( zOutFileName );
    DWFStreamFileDescriptor oOutDescriptor( oOutFile, L"wb" );
    oOutDescriptor.open();

    //
    //  Create input/output file streams and attach the descriptors.
    //
    DWFFileInputStream oInFilestream;
    oInFilestream.attach( &oInDescriptor, false );

    DWFFileOutputStream oOutFilestream;
    oOutFilestream.attach( &oOutDescriptor, false );

    //
    //  Chain the input/output file streams to the corresponding digest streams
    //
    rInDigestStream.chainInputStream( &oInFilestream, false );
    rOutDigestStream.chainOutputStream( &oOutFilestream, false );

    #define DATA_BUFFER_BYTES   4096
    unsigned char* pDataBuffer = DWFCORE_ALLOC_MEMORY( unsigned char, DATA_BUFFER_BYTES );

    //
    //  Read in the file. This will compute the digest in the process
    //
    size_t nBytes = 0;
    int nLoopCount = 0;
    while (rInDigestStream.available() > 0)
    {
        nBytes = rInDigestStream.read( pDataBuffer, DATA_BUFFER_BYTES );
        rOutDigestStream.write( pDataBuffer, nBytes );

        if (rInDigestStream.isDigestProgressive() && rInDigestStream.available()>0)
        {
            if (nLoopCount==0)
            {
                wcout << endl;
            }

            if (nLoopCount%2==0)
            {
                wcout << "\tProgressive (input) = " << rInDigestStream.digest() << endl;
            }
        }

        nLoopCount++;
    }

    //
    //  Print out the digest
    //
    DWFString zInDigest = rInDigestStream.digest();
    DWFString zOutDigest = rOutDigestStream.digest();
    wcout << endl
          << "\tFinal (input)  = " << zInDigest << endl
          << "\tFinal (output) = " << zOutDigest << endl;

    DWFString zBase64Digest = rInDigestStream.digestBase64();
    wcout << "\tDigest Base 64 = " << zBase64Digest << endl;

    unsigned char* pCharArray = NULL;
    size_t nRawBytes = rInDigestStream.digestRawBytes( pCharArray );
    if (pCharArray)
    {
        wcout << "\tRaw (" << nRawBytes << " bytes) = ";
        size_t i = 0;
        for (; i<nRawBytes; i++)
        {
            wcout << int(pCharArray[i]) << " ";
        }
        wcout << endl;

        DWFCORE_FREE_MEMORY( pCharArray );
    }

    std::flush( wcout );

    oInDescriptor.close();
    oOutDescriptor.close();
}


int main( int /*argc*/, char* /*argv[]*/ )
{
        //
        //  Create input/output digest streams and chain the input/output file streams
        //  to it, to compute the digests.
        //
        DWFDigestInputStream oInDigestStream( DWFCORE_ALLOC_OBJECT(DWFMD5Digest), NULL, false );
        DWFDigestOutputStream oOutDigestStream( DWFCORE_ALLOC_OBJECT(DWFMD5Digest), NULL, false );

        wcout << endl << "Testing MD5" << endl;
        wcout << "\tString in file = ""12345678901234567890123456789012345678901234567890123456789012345678901234567890""" << endl;
        wcout << "\tDigest expected= ""57edf4a22be3c955ac49da2e2107b67a""" << endl;
        ShowFileDigest( "testMD5.txt", oInDigestStream, oOutDigestStream );
    
        //
        //  Create input/output digest streams and chain the input/output file streams
        //  to it, to compute the digests.
        //
        DWFDigestInputStream oInDigestStream2( DWFCORE_ALLOC_OBJECT(DWFSHA1Digest), NULL, false );
        DWFDigestOutputStream oOutDigestStream2( DWFCORE_ALLOC_OBJECT(DWFSHA1Digest), NULL, false );

        wcout << endl << "Testing SHA1" << endl;
        wcout << "\tString in file = ""abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq""" << endl;
        wcout << "\tDigest expected= ""84983e441c3bd26ebaae4aa1f95129e5e54670f1""" << endl;
        ShowFileDigest( "testSHA1.txt", oInDigestStream2, oOutDigestStream2 );

        return 0;

}


