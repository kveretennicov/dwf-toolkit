#include <dwfcore/Core.h>
#include <dwfcore/ZipFileDescriptor.h>
#include <dwfcore/StreamFileDescriptor.h>
#include <dwfcore/FileInputStream.h>
#include <dwfcore/FileOutputStream.h>
#include <dwfcore/BufferInputStream.h>

//
//  This sample shows how to use the zip descriptor to create an archive in two different ways. 
//  In the first case the archive is created by directly creating the archive file on disk. 
//  The second method creates the archive in memory. This is useful for compressing data chunks
//  in memory. We output the data to disk at the end for validating the compression to a buffer.
//


int main( int /*argc*/, char* /*argv[]*/ )
{
    DWFString inpFileName( /*NOXLATE*/L"SomeText.txt" );

    DWFFile inpFile( inpFileName );
    DWFStreamFileDescriptor inpFD( inpFile, "rb" );
    inpFD.open();

    DWFFileInputStream inpStream;
    inpStream.attach( &inpFD, false );

    //
    //  Write directly to a file output stream
    //
    {
        DWFFile zipFileDirect( "DirectToZip.zip" );
        DWFZipFileDescriptor zipDirect( zipFileDirect, DWFZipFileDescriptor::eZip );
        zipDirect.open();

        DWFOutputStream* pZipOutStream = zipDirect.zip( inpFileName );

        const size_t nBufferSize = 16384;
        unsigned char aBuffer[nBufferSize] = {0};
        size_t nBytesRead = 0;

        while (inpStream.available()>0)
        {
            nBytesRead = inpStream.read( aBuffer, nBufferSize );
            pZipOutStream->write( aBuffer, nBytesRead );
        }
        pZipOutStream->flush();
        DWFCORE_FREE_OBJECT( pZipOutStream );

        zipDirect.close();
    }

    inpStream.seek( SEEK_SET, 0 );

    //
    //  Write via a buffered archive in memory
    //
    {
        DWFZipFileDescriptor zipBuffered( DWFZipFileDescriptor::eZip );
        zipBuffered.open();

        DWFOutputStream* pZipOutStream = zipBuffered.zip( inpFileName );

        const size_t nBufferSize = 16384;
        unsigned char aBuffer[nBufferSize] = {0};
        size_t nBytesRead = 0;

        while (inpStream.available()>0)
        {
            nBytesRead = inpStream.read( aBuffer, nBufferSize );
            pZipOutStream->write( aBuffer, nBytesRead );
        }
        pZipOutStream->flush();
        //
        //  This needs to be destroyed here, before closing the
        //  descriptor, to close the file in the zip archive.
        //
        DWFCORE_FREE_OBJECT( pZipOutStream );
        zipBuffered.close();

        DWFBufferInputStream zipBufferedInp( zipBuffered.buffer(), zipBuffered.size() );

        DWFFile outFileName( "BufferedToZip.zip" );
        DWFStreamFileDescriptor outDescriptor( outFileName, "wb" );
        outDescriptor.open();

        DWFFileOutputStream outStream;
        outStream.attach( &outDescriptor, false );

        while (zipBufferedInp.available()>0)
        {
            nBytesRead = zipBufferedInp.read( aBuffer, nBufferSize );
            outStream.write( aBuffer, nBytesRead );
        }
        outStream.flush();

        outStream.detach();

    }

    inpStream.detach();
}
