// SimpleEnumReader.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace std;
using namespace DWFCore;
using namespace DWFToolkit;


class CommentHandler : public TK_Comment
{
public:
    CommentHandler() : TK_Comment() {;}
    virtual ~CommentHandler() {;}

    TK_Status Execute( BStreamFileToolkit& rW3DParser )
    {
        TK_Status eStatus = TK_Comment::Execute( rW3DParser );
        if (eStatus == TK_Normal)
        {
            cout << "Found Comment: ";
            cout << this->GetComment() << endl;
        }

        return eStatus;
    }
};

class TextOpcodeHandler : public TK_Text
{
public:
    TextOpcodeHandler() : TK_Text( TKE_Text ) {;}
    virtual ~TextOpcodeHandler() {;}

    TK_Status Execute( BStreamFileToolkit& rW3DParser )
    {
        TK_Status eStatus = TK_Text::Execute( rW3DParser );
        if (eStatus == TK_Normal)
        {
            cout << "Found Text: ";
            cout << this->GetString() << endl;

            FILE* fp = fopen( "text.txt", "a+" );
            fprintf( fp, "%s\n", this->GetString() );
            fclose( fp );
        }

        return eStatus;
    }
};

class TextWithEncodingOpcodeHandler : public TK_Text
{

public:
    TextWithEncodingOpcodeHandler() : TK_Text( TKE_Text_With_Encoding ) {;}
    virtual ~TextWithEncodingOpcodeHandler() {;}

    TK_Status Execute( BStreamFileToolkit& rW3DParser )
    {
        TK_Status eStatus = TK_Text::Execute( rW3DParser );
        if (eStatus == TK_Normal)
        {
            if (this->GetEncoding() == TKO_Enc_Unicode)
            {
                wcout << "Found Unicode Text: ";
                wcout << (const wchar_t*)(this->GetString()) << endl;

                DWFString text( (const wchar_t*)(this->GetString()) );
                char* utf8;
                text.getUTF8( &utf8 );

                FILE* fp = fopen( "text.txt", "a+" );
                fprintf( fp, "%s\n", utf8 );
                fclose( fp );

                delete utf8;

            }
        }

        return eStatus;
    }
};

class ShellHandler : public TK_Shell
{
public:
    ShellHandler() : TK_Shell() {;}
    virtual ~ShellHandler() {;}

    TK_Status Execute( BStreamFileToolkit& parser )
    {
        TK_Status status = TK_Shell::Execute(parser);
        if (status == TK_Normal)
        {
            ;
        }
        return status;
    }
    TK_Status Clone (BStreamFileToolkit & tk, BBaseOpcodeHandler **newhandler) const 
    {
        tk;
        *newhandler = new ShellHandler;
        return TK_Normal;
    }
};

class StartUserDataHandler : public TK_User_Data
{
    FILE* _fp;
    int _stage;
    int _bytes;

public:
    StartUserDataHandler() : TK_User_Data() 
    {
        _stage = 0;
        _bytes = 0;
        _fp = fopen( "userdata.bin", "wb+" );
    }
    virtual ~StartUserDataHandler() 
    {
        fclose(_fp);
    }

    TK_Status   Read (BStreamFileToolkit & tk)
    {
        TK_Status       status = TK_Normal;

    switch (m_stage) {
        case 0: {
            if ((status = GetData (tk, m_size)) != TK_Normal)
                return status;
            set_data (m_size);      // allocate space
            m_stage++;
        }

        case 1: {
            if ((status = GetData (tk, m_data, m_size)) != TK_Normal)
                return status;
            m_stage++;

            fwrite( m_data, m_size, 1, _fp );
        }

        case 2: {
            int loops = 0;
            unsigned char       stop_code = 0;

            while (stop_code != TKE_Stop_User_Data)
            {
                if ((status = GetData (tk, stop_code)) != TK_Normal)
                    return status;
                loops++;
            }

            //if (stop_code != TKE_Stop_User_Data)    // sanity check
            //    return tk.Error();

            m_stage = -1;
        }   break;

        default:
            return tk.Error();
    }

    return status;
}

};

int main(int argc, char* argv[])
{

    if (argc < 2)
    {
        wcout << L"Usage:" << argv[0] << L" file.dwf" << endl;
        return ( 0 );
    }

    try
    {
        DWFFile oDWF( argv[1] );
        DWFToolkit::DWFPackageReader oReader( oDWF );

        DWFPackageReader::tPackageInfo tInfo;
        oReader.getPackageInfo( tInfo );

        wchar_t zBuffer[256] = {0};

        if (tInfo.eType != DWFPackageReader::eDWFPackage)
            
        {
            _DWFCORE_SWPRINTF( zBuffer, 256, L"File is not a DWF package [%s]",
                        (tInfo.eType == DWFPackageReader::eW2DStream) ? L"W2D Stream" :
                        (tInfo.eType == DWFPackageReader::eDWFStream) ? L"DWF Stream (<6.0)" :
                        (tInfo.eType == DWFPackageReader::eZIPFile) ? L"ZIP Archive" : L"Unknown" );

            wcout << zBuffer << endl;
            exit( 0 );
        }
        else if (tInfo.nVersion < _DWF_FORMAT_VERSION_INTRO_3D)
        {
            wcout << L"DWF package specified is not a 3D DWF" << endl;
            exit( 0 );
        }

        //
        // read and parse the manifest
        //
        DWFToolkit::DWFManifest& rManifest = oReader.getManifest();

        //
        // obtain the emodel section
        //
        DWFToolkit::DWFManifest::SectionIterator* piSections = rManifest.findSectionsByType( _DWF_FORMAT_EMODEL_TYPE_WIDE_STRING );

            //
            // make sure we got a section
            //
        if ((piSections == NULL) || (piSections->valid() == false))
        {
            if (piSections)
            {
                DWFCORE_FREE_OBJECT( piSections );
            }

            wcout << L"Malformed or unexpected 3D DWF, cannot continue without an EModel section" << endl;
            exit( 0 );
        }

        //
        // get the EModel Section
        //
        DWFToolkit::DWFEModelSection* pSection = dynamic_cast<DWFEModelSection*>(piSections->get());

        //
        // done with the iterator
        //
        DWFCORE_FREE_OBJECT( piSections );

        if (pSection == NULL)
        {
            wcout << L"Type mismatch - not an EModel Section" << endl;
            exit( 0 );
        }

        //
        // read the descriptor to get all the details
        //
        pSection->readDescriptor();

        //
        // get the graphics stream
        //
        DWFToolkit::DWFResourceContainer::ResourceIterator* piResources = pSection->findResourcesByRole( DWFXML::kzRole_Graphics3d );

        if ((piResources == NULL) || (piResources->valid() == false))
        {
            if (piResources)
            {
                DWFCORE_FREE_OBJECT( piResources );
            }

            wcout << "Illegal EModel section - no graphics" << endl;
            exit( 0 );
        }

        //
        // get the w3d resource
        //
        DWFToolkit::DWFGraphicResource* pW3D = dynamic_cast<DWFGraphicResource*>(piResources->get());

        //
        // done with the iterator
        //
        DWFCORE_FREE_OBJECT( piResources );

        if (pW3D == NULL)
        {
            wcout << L"Type mismatch - not a W3D resource" << endl;
            exit( 0 );
        }

        //
        // get the data stream
        //
        DWFCore::DWFInputStream* pW3DStream = pW3D->getInputStream();

        //
        // Create the HSF toolkit object that does the stream I/O
        //
        BStreamFileToolkit oW3DStreamParser;

        //
        // For this sample, we are interested in those op-codes that might contain
        // some interesting text.  Here is where we hook these handlers.
        // Also note that the parser object will delete this object on it's own destruction
        //
        oW3DStreamParser.SetOpcodeHandler( TKE_Start_User_Data,     new StartUserDataHandler );
        oW3DStreamParser.SetOpcodeHandler( TKE_Stop_User_Data,      new StartUserDataHandler );

        oW3DStreamParser.SetOpcodeHandler( TKE_Shell,               new ShellHandler );
        
        oW3DStreamParser.SetOpcodeHandler( TKE_Comment,             new CommentHandler );
        oW3DStreamParser.SetOpcodeHandler( TKE_Text,                new TextOpcodeHandler );
        oW3DStreamParser.SetOpcodeHandler( TKE_Text_With_Encoding,  new TextWithEncodingOpcodeHandler ); 

        //
        // Attach the stream to the parser
        //
        oW3DStreamParser.OpenStream( *pW3DStream );

        size_t  nBytesRead = 0;
        char    aBuffer[16384] = {0};

            //
            // read and process the stream
            //
        while (pW3DStream->available() > 0)
        {
            //
            // read from the stream ourselves, we could also use ReadBuffer()
            // but it basically just performs this same action.
            //
            nBytesRead = pW3DStream->read( aBuffer, 16384 );

                //
                // use the parser to process the buffer
                //
            if (oW3DStreamParser.ParseBuffer(aBuffer, nBytesRead, TK_Normal) == TK_Error)
            {
                wcout << L"Error occured parsing buffer" << endl;
                break;
            }
        }

        //
        // Done with the stream, we must delete it
        //
        oW3DStreamParser.CloseStream();
        DWFCORE_FREE_OBJECT( pW3DStream );

        wcout << L"OK\n";
    }
    catch (DWFException& ex)
    {
        wcout << ex.type() << endl;
        wcout << ex.message() << endl;
        wcout << ex.function() << endl;
        wcout << ex.file() << endl;
        wcout << ex.line() << endl;
    }

    return 0;
}


