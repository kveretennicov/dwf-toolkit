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


#include "stdafx.h"

using namespace std;
using namespace DWFCore;
using namespace DWFToolkit;


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
        DWFPackageReader oReader( oDWF );

        DWFPackageReader::tPackageInfo tInfo;
        oReader.getPackageInfo( tInfo );

        char zBuffer[4000] = {0};

        if (tInfo.eType != DWFPackageReader::eDWFPackage)
        {
            ::sprintf( zBuffer, "File is not a DWF package [%s]", 
                        (tInfo.eType == DWFPackageReader::eW2DStream) ? "W2D Stream" :
                        (tInfo.eType == DWFPackageReader::eDWFStream) ? "DWF Stream (<6.0)" :
                        (tInfo.eType == DWFPackageReader::eZIPFile) ? "ZIP Archive" : "Unknown" );

            cout << zBuffer << endl;
            exit( 0 );
        }

        ::sprintf( zBuffer, "DWF Package version [%0.2f]", (float)(tInfo.nVersion)/100.0f );
        cout << zBuffer << endl;


        cout << "Calibrating...";

#ifdef  _DWFCORE_WIN32_SYSTEM

        size_t i = 0;
        unsigned long ms = ::GetTickCount();
        uint64_t ts = DWFTimer::Tick64();
        for(;i<999999999;i++){;}
        ts = DWFTimer::Tick64() - ts;
        ms = ::GetTickCount() - ms;

        double tpms = ts/(double)(ms);
#else

        size_t i = 0;
        struct timeval tv;
        struct timeval tv2;
        gettimeofday( &tv, NULL );
        unsigned long long ts = DWFTimer::Tick64();
        for (;i<999999999;i++);
        ts = DWFTimer::Tick64() - ts;
        gettimeofday( &tv2, NULL );

        time_t sec = tv2.tv_sec - tv.tv_sec;
        suseconds_t usec = tv2.tv_usec - tv.tv_usec;

        double tpms = ts/( (double)(sec*1000) + (double)(usec/1000) );
#endif

        cout << "using " << tpms << " ticks per ms" << endl << endl;


        DWFTimer oTimer;
        oTimer.start();
        DWFManifest& rManifest = oReader.getManifest();
        oTimer.stop();

#ifdef  _DWFCORE_WIN32_SYSTEM
        uint64_t tick = oTimer.timer64();
#else
        unsigned long long tick = oTimer.timer64();
#endif
        double diff = tick/tpms;

        cout << "Read manifest [" << tick << " ticks] [~" << diff << " ms]" << endl;

        
        DWFSection* pSection = NULL;

        DWFManifest::SectionIterator* piSections = rManifest.getSections();
        
        if (piSections)
        {
            for (; piSections->valid(); piSections->next())
            {
                pSection = piSections->get();


                oTimer.start();
                pSection->readDescriptor();
                oTimer.stop();


                tick = oTimer.timer64();
                diff = tick/tpms;

                cout << endl << "Read section descriptor [" << tick << " ticks] [~" << diff << " ms]" << endl;

                DWFResourceContainer::ResourceIterator* piObjDefs = pSection->findResourcesByRole( DWFXML::kzRole_ObjectDefinition );
                if (piObjDefs && piObjDefs->valid())
                {
                    oTimer.start();
                    DWFObjectDefinition* pDef = pSection->getObjectDefinition();
                    oTimer.stop();

                    tick = oTimer.timer64();
                    diff = tick/tpms;

                    cout << "Processed object definition [" << tick << " ticks] [~" << diff << " ms]" << endl;

                    if (pDef)
                    {
                        DWFCORE_FREE_OBJECT( pDef );
                    }

                    DWFCORE_FREE_OBJECT( piObjDefs );
                }
            }
            DWFCORE_FREE_OBJECT( piSections );
        }

        cout << "OK\n" << endl;
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


