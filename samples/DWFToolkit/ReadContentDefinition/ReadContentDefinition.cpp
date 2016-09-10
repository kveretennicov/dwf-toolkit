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


#include <iomanip>
#include <iostream>
using namespace std;

#include "dwfcore/File.h"
#include "dwfcore/String.h"
using namespace DWFCore;

#include "dwf/package/Constants.h"
#include "dwf/package/Manifest.h"
#include "dwf/package/GlobalSection.h"
#include "dwf/package/ContentManager.h"
#include "dwf/package/reader/PackageReader.h"
using namespace DWFToolkit;


#ifdef  _DWFCORE_WIN32_SYSTEM
#include <crtdbg.h>
#endif


const DWFString zMinIndent( L"    " );

/////////////////////////////////////////////////////////////////////////
//
//  declarations
//

void printTypeInfo( DWFPackageReader::tPackageInfo& tInfo );

void getObjectInfoString( DWFObject* pObject, int nDepth, DWFString& zOut );
void getChildObjectInfo( DWFObject* pObject, int nDepth, DWFString& zOut );

void getPropertyInfo( DWFPropertyContainer* pContainer, int nDepth, DWFString& zOut );
void getGlobalContentInfo( DWFContent* pContent, DWFString& zOut );
void getContentInstances( DWFContent* pContent, DWFString& zOut );

//
//  Functions for interactive queries
//
void doPropertyTest( DWFContent* pContent );
bool performPropertyQueries( DWFContentElement* pElem );

/////////////////////////////////////////////////////////////////////////
//
//  main
//
int main(int argc, char* argv[])
{
#ifdef  _DWFCORE_WIN32_SYSTEM
    //
    // Enable memory leak reporting in Debug mode under Win32.
    //
    int tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
    // Turn on leak-checking bit
    tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
    // Turn off CRT block checking bit
    tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;
    // Set flag to the new value
    _CrtSetDbgFlag( tmpFlag );

    // For mem leak debugging...
    /*
    long foo = 8217;
    _CrtSetBreakAlloc(foo);
    */
#endif

    if (argc < 2)
    {
        wcout << L"Usage:" << argv[0] << L" file.dwf [-q]" << endl;
        wcout << L"-q is used to get into interactive mode to query specific objects" << endl;
        return ( 0 );
    }

    try
    {
        wcout << L"Reading package: " << argv[1] << endl;

        DWFFile oDWF( argv[1] );
        DWFPackageReader oReader( oDWF );

        DWFPackageReader::tPackageInfo tInfo;
        oReader.getPackageInfo( tInfo );

        printTypeInfo( tInfo );

        ///////////////////

        //
        //  Read manifest - this will create/read the manifest, and also create the content manager
        //
        DWFManifest& rManifest = oReader.getManifest();

        DWFContentManager* pContentManager = rManifest.getContentManager();
        DWFContent* pContent = pContentManager->getContent();

        //
        //  This piece is critical - the content still hasn't been loaded. 
        //  The call only loads the global information. To get section specific information loaded into the content
        //  you need to call DWFSection::getContentDefinition, a few lines later.
        //
        pContent->load();

        //
        //  Print out some of the global content information
        //
        DWFString zOut;
        getGlobalContentInfo( pContent, zOut );
        wcout << (const wchar_t*) zOut << endl;

        ///////////////////

        zOut.assign( L"" );

        //
        //  Now get sections, for each section find all section content resources 
        //  and load them.
        //
        DWFSection* pSection = NULL;
        DWFManifest::SectionIterator* piSections = rManifest.getSections();
        if (piSections)
        {
            for (; piSections->valid(); piSections->next())
            {
                pSection = piSections->get();
                pSection->readDescriptor();

                //
                //  This is NOT creating NEW contents - it loading one that was created during the manifest
                //  loading process. Do not delete the content pointer here. In most cases there will be
                //  only one content.
                //
                DWFContent::tIterator* piContent = pSection->getContentDefinition();
                if (piContent)
                {
                    for (; piContent->valid(); piContent->next())
                    {
                        DWFContent* pSectionContent = piContent->get();

                        if (pSectionContent == pContent)
                        {
                            zOut.assign( L"The section returned the global content we were looking at above\n\n" );
                        }

                        getContentInstances( pSectionContent, zOut );

                    }
                    DWFCORE_FREE_OBJECT( piContent );
                }
            }
            if (zOut.chars()>0)
            {
                wcout << (const wchar_t*) zOut << "\n" << endl;
            }
            else
            {
                wcout << L"No instances were found.\n" << endl;
            }

            DWFCORE_FREE_OBJECT( piSections );
        }

        ////
        
        zOut.assign( L"" );

        if (argc>2 && argv[2][0]=='-' && argv[2][1]=='q')
        {
            doPropertyTest( pContent );
        }

        wcout << L"\nDONE\n";
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

/////////////////////////////////////////////////////////////////////////
//
// function definitions
//

std::wostream& operator<<( std::wostream& wos, const DWFString& zString )
{
    if (zString.chars())
    {
        wos << (const wchar_t*)zString;
    }
    return wos;
}

DWFString getLine()
{
    wcin.clear();
    wchar_t szIdentifier[256] = {0};
    if (wcin.peek()==10)
    {
        wcin.get();
    }
    wcin.getline( szIdentifier, 255 );
    return DWFString( szIdentifier );
}

#define ZOUTLABEL( pElem )  zOut.append( pElem->getLabel().chars()>0 ? pElem->getLabel() : pElem->id() );


void printTypeInfo( DWFPackageReader::tPackageInfo& tInfo )
{
    if (tInfo.eType != DWFPackageReader::eDWFPackage)
    {
        cout << "File is not a DWF package ";
        if (tInfo.eType == DWFPackageReader::eW2DStream)
        {
            cout << "[W2D Stream]";
        }
        else if (tInfo.eType == DWFPackageReader::eDWFStream)
        {
            cout << "[DWF Stream (<6.0)]";
        } 
        else if (tInfo.eType == DWFPackageReader::eZIPFile)
        {
            cout << "[ZIP Archive]";
        }
        else
        {
            cout << "[Unknown]";
        }
        cout << endl;
        exit( 0 );
    }

    cout << setprecision(2) << "DWF Package version [" << (float)(tInfo.nVersion)/100.0f << "]\n" << endl;
}

void getObjectInfoString( DWFObject* pObject, int nDepth, DWFString& zOut )
{
    for (int i=0; i<nDepth; i++)
    {
        zOut.append( zMinIndent );   // Two space indentation
    }
    zOut.append( L"Object: " );
    ZOUTLABEL( pObject );
    zOut.append( L" - " );
    zOut.append( pObject->id() );
    zOut.append( L" : Entity: " );
    DWFEntity* pEntity = pObject->getEntity();
    ZOUTLABEL( pEntity );
    zOut.append( L"\n" );

    getPropertyInfo( pObject, nDepth, zOut );
}

void getChildObjectInfo( DWFObject* pObject, int nDepth, DWFString& zOut )
{
    getObjectInfoString( pObject, nDepth, zOut );

    DWFObject::tIterator* piChild = pObject->getChildren();
    if (piChild)
    {
        for (; piChild->valid(); piChild->next())
        {
            getChildObjectInfo( piChild->get(), nDepth+1, zOut );
        }
        DWFCORE_FREE_OBJECT( piChild );
    }
}

void getPropertyInfo( DWFPropertyContainer* pContainer, int nDepth, DWFString& zOut )
{
    {
        //
        // This code doesn't do anything useful. It's only here to help detect memory leaks.
        // Users of this sample should ignore it.
        DWFProperty::tMap::Iterator *piter = pContainer->getProperties();
        DWFCORE_FREE_OBJECT( piter );
    }

    DWFString zIndent;
    for (int i=0; i<nDepth; i++)
    {
        zIndent.append( zMinIndent );   // Two space indentation
    }

    //
    //  The type of container
    //
    if (dynamic_cast<DWFContentElement*>(pContainer))
    {
        //zOut.append( zIndent );
        //zOut.append( L"CE:\n" );
    }
    else
    {
        DWFPropertySet* pSet = dynamic_cast<DWFPropertySet*>(pContainer);
        if (pSet)
        {
            zOut.append( zIndent );
            zOut.append( L"PS:" );
            ZOUTLABEL( pSet );
            zOut.append( L"\n" );
        }
        else
        {
            zOut.append( zIndent );
            zOut.append( L"PC:" );
            zOut.append( pContainer->id() );
            zOut.append( L"\n" );
        }
    }

    DWFProperty::tIterator* piProperty = pContainer->getPropertiesInOrder();
    if (piProperty)
    {
        for (; piProperty->valid(); piProperty->next())
        {
            DWFProperty* pProperty = piProperty->get();

            zOut.append( zIndent );
            zOut.append( L"Property: (" );
            zOut.append( pProperty->name() );
            zOut.append( L" , " );
            zOut.append( pProperty->value() );
            if (pProperty->category().chars()>0)
            {
                zOut.append( L" , " );
                zOut.append( pProperty->category() );
            }
            zOut.append( L" )\n" );
        }
        DWFCORE_FREE_OBJECT( piProperty );
    }

    DWFPropertyContainer::tList oContainers;
    pContainer->getReferencedPropertyContainers( oContainers );
    if (oContainers.size()>0)
    {
        zOut.append( zIndent );
        zOut.append( L"Refs: " );
        DWFPropertyContainer::tList::iterator iCont = oContainers.begin();
        for (; iCont != oContainers.end(); ++iCont)
        {
            zOut.append( (*iCont)->id() );
            zOut.append( L" " );
        }
        zOut.append( L"\n" );
    }

    oContainers.clear();
    pContainer->getOwnedPropertyContainers( oContainers );
    if (oContainers.size()>0)
    {
        DWFPropertyContainer::tList::iterator iCont = oContainers.begin();
        for (; iCont != oContainers.end(); ++iCont)
        {
            getPropertyInfo( *iCont, nDepth+1, zOut );
        }
    }
}

void getGlobalContentInfo( DWFContent* pContent, DWFString& zOut )
{
    //
    // Get shared property set information
    //
    zOut.append( "[SHARED PROPERTY SETS]\n" );
    DWFPropertySet::tMap::Iterator* piSet = pContent->getSharedPropertySets();
    if (piSet)
    {
        if (piSet->valid())
        {
            for (; piSet->valid(); piSet->next())
            {
                DWFPropertySet* pSet = piSet->value();

                if (pSet)
                {
                    zOut.append( L"PropertySet: " );
                    zOut.append( pSet->getLabel().chars()>0 ? pSet->getLabel() : pSet->id() );
                    zOut.append( "   Closed: " );
                    zOut.append( (pSet->isClosed()?"true":"false") );
                    if (pSet->getSchemaID().chars())
                    {
                        zOut.append( "  SchemaID: " );
                        zOut.append( pSet->getSchemaID() );
                    }
                    if (pSet->getSetID().chars())
                    {
                        zOut.append( "  SetID: " );
                        zOut.append( pSet->getSetID() );
                    }
                    zOut.append( L"\n" );
                    getPropertyInfo( pSet, 0, zOut );
                }
            }
            zOut.append( L"\n" );
        }
        else
        {
            zOut.append( L"None\n\n" );
        }
        DWFCORE_FREE_OBJECT( piSet );
    }
    else
    {
        zOut.append( L"None\n\n" );
    }

    //
    // Get class information
    //
    zOut.append( "[CLASSES]\n" );
    DWFClass::tMap::Iterator* piClass = pContent->getClasses();
    if (piClass)
    {
        if (piClass->valid())
        {
            for (; piClass->valid(); piClass->next())
            {
                DWFClass* pClass = piClass->value();

                if (pClass)
                {
                    zOut.append( L"Class: " );
                    ZOUTLABEL( pClass );
                    zOut.append( L"\n" );

                    DWFEntity::tIterator* piEntity = pContent->findEntitiesByClass( pClass );
                    if (piEntity)
                    {
                        if (piEntity->valid())
                        {
                            zOut.append( "Referring entities: ");
                            for (; piEntity->valid(); piEntity->next())
                            {
                                DWFEntity* pEntity = piEntity->get();
                                ZOUTLABEL( pEntity );
                                zOut.append( " " );
                            }
                            zOut.append( "\n" );
                        }
                        DWFCORE_FREE_OBJECT( piEntity );
                    }

                    getPropertyInfo( pClass, 0, zOut );
                }
            }
            zOut.append( L"\n" );
        }
        else
        {
            zOut.append( L"None\n\n" );
        }
        DWFCORE_FREE_OBJECT( piClass );
    }
    else
    {
        zOut.append( L"None\n\n" );
    }

    //
    // Get feature information
    //
    zOut.append( "[FEATURES]\n" );
    DWFFeature::tMap::Iterator* piFeature = pContent->getFeatures();
    if (piFeature)
    {
        if (piFeature->valid())
        {
            for (; piFeature->valid(); piFeature->next())
            {
                DWFFeature* pFeature = piFeature->value();

                if (pFeature)
                {
                    zOut.append( L"Feature: " );
                    ZOUTLABEL( pFeature );
                    zOut.append( L"\n" );
                    getPropertyInfo( pFeature, 0, zOut );
                }
            }
            zOut.append( L"\n" );
        }
        else
        {
            zOut.append( L"None\n\n" );
        }
        DWFCORE_FREE_OBJECT( piFeature );
    }
    else
    {
        zOut.append( L"None\n\n" );
    }

    //
    // Get entity information
    //
    zOut.append( "[ENTITIES]\n" );
    DWFEntity::tMap::Iterator* piEntity = pContent->getEntities();
    if (piEntity)
    {
        if (piEntity->valid())
        {
            for (; piEntity->valid(); piEntity->next())
            {
                DWFEntity* pEntity = piEntity->value();

                if (pEntity)
                {
                    zOut.append( L"Entity: " );
                    ZOUTLABEL( pEntity );
                    zOut.append( L" - " );
                    zOut.append( pEntity->id() );
                    zOut.append( L"\n" );

                    DWFObject::tIterator* piObject = pContent->findObjectsByEntity( pEntity );
                    if (piObject)
                    {
                        if (piObject->valid())
                        {
                            zOut.append( "Rendering objects: ");
                            for (; piObject->valid(); piObject->next())
                            {
                                DWFObject* pObject = piObject->get();
                                ZOUTLABEL( pObject );
                                zOut.append( " " );
                            }
                            zOut.append( "\n" );
                        }
                        DWFCORE_FREE_OBJECT( piObject );
                    }

                    getPropertyInfo( pEntity, 0, zOut );
                }
            }
            zOut.append( L"\n" );
        }
        else
        {
            zOut.append( L"None\n\n" );
        }
        DWFCORE_FREE_OBJECT( piEntity );
    }
    else
    {
        zOut.append( L"None\n\n" );
    }

    //
    // Get object information
    //
    zOut.append( "[OBJECTS]\n" );
    DWFObject::tMap::Iterator* piObject = pContent->getObjects();
    if (piObject)
    {
        if (piObject->valid())
        {
            int nObjectDepth = 0;
            for (; piObject->valid(); piObject->next())
            {
                DWFObject* pObject = piObject->value();

                if (pObject && pObject->getParent() == NULL)
                {
                    getObjectInfoString( pObject, nObjectDepth, zOut );
                }

                DWFObject::tIterator* piChild = pObject->getChildren();
                if (piChild)
                {
                    for (; piChild->valid(); piChild->next())
                    {
                        getChildObjectInfo( piChild->get(), nObjectDepth+1, zOut );
                    }
                    DWFCORE_FREE_OBJECT( piChild );
                }
            }
            zOut.append( L"\n" );
        }
        else
        {
            zOut.append( L"None\n\n" );
        }
        DWFCORE_FREE_OBJECT( piObject );
    }
    else
    {
        zOut.append( L"None\n\n" );
    }

    //
    // Get group information
    //
    zOut.append( "[GROUPS]\n" );
    DWFGroup::tMap::Iterator* piGroup = pContent->getGroups();
    if (piGroup)
    {
        if (piGroup->valid())
        {
            for (; piGroup->valid(); piGroup->next())
            {
                DWFGroup* pGroup = piGroup->value();

                if (pGroup)
                {
                    zOut.append( L"Group: " );
                    zOut.append( pGroup->getLabel().chars()>0 ? pGroup->getLabel() : pGroup->id() );
                    zOut.append( L"\n" );
                    getPropertyInfo( pGroup, 0, zOut );
                }
            }
            zOut.append( L"\n" );
        }
        else
        {
            zOut.append( L"None\n\n" );
        }
        DWFCORE_FREE_OBJECT( piGroup );
    }
    else
    {
        zOut.append( L"None\n\n" );
    }
}

void getContentInstances( DWFContent* pContent, DWFString& zOut )
{
    zOut.append( "[INSTANCES]\n" );
    DWFInstance::tMap::Iterator* piInstance = pContent->getInstances();
    if (piInstance)
    {
        if (piInstance->valid())
        {
            for (; piInstance->valid(); piInstance->next())
            {
                DWFInstance* pInstance = piInstance->value();

                if (pInstance)
                {
                    zOut.append( L"Instance: Renders " );
                    
                    DWFRenderable* pRendered = pInstance->getRenderedElement();
                    if (pRendered->getLabel().chars()>0)
                    {
                        zOut.append( pRendered->getLabel() );
                    }
                    else
                    {
                        zOut.append( pRendered->id() );
                    }

                    zOut.append( (pInstance->getVisibility() ? " : visible" : " : hidden") );
                    zOut.append( (pInstance->getTransparency() ? " : transparent" : " : opaque") );
                    zOut.append( L"\n" );
                }
            }
            zOut.append( L"\n" );
        }
        else
        {
            zOut.append( L"None\n\n" );
        }
        DWFCORE_FREE_OBJECT( piInstance );
    }
    else
    {
        zOut.append( L"None\n\n" );
    }
}

void doPropertyTest( DWFContent* pContent )
{
    bool bDone = false;

    wcout << L"Enter \'-\' by itself to quit" << endl;
    while (!bDone)
    {
        char cElementType = char(0);
        char cIdentifier = char(0);

        //
        //  Get element type to query
        //
        while (cElementType!='e' && cElementType!='o')
        {
            wcout << L"Element type to query - 'e'ntity or 'o'bject ('-' to quit): ";
            cin >> cElementType;
            if (cElementType=='-')
            {
                return;
            }
        }

        //
        //  Get element identifier type
        //
        while (cIdentifier!='l' && cIdentifier!='i')
        {
            wcout << L"Get element by 'l'abel or by 'i'd ('-' to quit): ";
            cin >> cIdentifier;
            if (cIdentifier=='-')
            {
                return;
            }
        }

        //
        //  Get element identifier value
        //
        wcout << L"Enter the " << (cIdentifier=='l'?L"label":L"id") << L": ";
        DWFString zIdentifier = getLine();
        if (zIdentifier==L"-")
        {
            return;
        }
        wcout << endl;

        //
        //  Get element using element type, and identifier type and value
        //
        DWFContentElement* pElem = NULL;
        if(cElementType=='e')
        {
            if (cIdentifier=='l')
            {
                DWFEntity::tMap::Iterator* piEntity = pContent->getEntities();
                if (piEntity)
                {
                    for (; piEntity->valid(); piEntity->next())
                    {
                        if (piEntity->value()->getLabel()==zIdentifier)
                        {
                            pElem = piEntity->value();
                            break;
                        }
                    }
                }
            }
            else
            {
                pElem = pContent->getEntity( zIdentifier );
            }
        }
        else
        {
            if (cIdentifier=='l')
            {
                DWFObject::tMap::Iterator* piObject = pContent->getObjects();
                if (piObject)
                {
                    for (; piObject->valid(); piObject->next())
                    {
                        if (piObject->value()->getLabel()==zIdentifier)
                        {
                            pElem = piObject->value();
                            break;
                        }
                    }
                }
            }
            else
            {
                pElem = pContent->getObject( zIdentifier );
            }
        }

        if (pElem==NULL)
        {
            wcout << L"The requested element was not found in the global content\n\n" << endl;
            continue;
        }

        bDone = performPropertyQueries( pElem );
    }
}

bool performPropertyQueries( DWFContentElement* pElem )
{
    bool bQuitApp = false;

    DWFObject* pObject = dynamic_cast<DWFObject*>(pElem);
    if (pObject)
    {
        wcout << L"Object: " << pObject->id();
        if (pObject->getLabel().chars())
        {
            wcout << "; label: " << pObject->getLabel() << endl;
        }
        else
        {
            wcout << endl;
        }
        DWFEntity* pEntity = pObject->getEntity();
        wcout << L"Object's Entity: " << pEntity->id();
        if (pEntity->getLabel().chars())
        {
            wcout << "; label: " << pEntity->getLabel();
        }
        wcout << endl;
    }
    else
    {
        wcout << L"Entity: " << pElem->id() << endl;
        if (pElem->getLabel().chars())
        {
            wcout << "; label: " << pElem->getLabel();
        }
        wcout << endl;
    }
    wcout << endl;

    {
        wcout << L"Default Properties :-- " << endl;
        DWFProperty::tIterator* piProperty = pElem->getAllProperties();
        if (piProperty)
        {
            for (; piProperty->valid(); piProperty->next())
            {
                DWFProperty* pProperty = piProperty->get();
                wcout << L"Property: ( " << pProperty->name() << L" , " << pProperty->value();
                if (pProperty->category().chars()>0)
                {
                    wcout << L" , " << pProperty->category();
                }
                wcout << L" )" << endl;
            }

            DWFCORE_FREE_OBJECT( piProperty );
        }
    }
    wcout << endl;

    {
        bool bSkip = false;

        wcout << L"Single Property:-- " << endl;
        wcout << "Property Name ('-' to quit, '+' to skip): ";
        DWFString zProperty = getLine();
        if (zProperty == "-")
        {
            return true;
        }
        else if (zProperty == "+")
        {
            bSkip = true;
        }

        if (!bSkip)
        {
            wcout << "Category, this may be empty ('-' to quit): ";
            DWFString zCategory = getLine();
            if (zCategory=="-")
            {
                return true;
            }
            const DWFProperty* pProperty = pElem->getProperty( zProperty, zCategory );
            if (pProperty)
            {
                wcout << "Value: " << pProperty->value() << endl;
            }
            else
            {
                wcout << "Property not found" << endl;
            }
        }
    }
    wcout << endl;

    {
        bool bSkip = false;

        wcout << L"BOM Properties (assuming a BOM which allows defaults):-- " << endl;
        char cIDType = char(0);
        while (cIDType!='t' && cIDType!='a')
        {
            wcout << L"Get by  Se't' or  Schem'a' ID ('-' to quit, '+' to skip): ";
            cin >> cIDType;
            if (cIDType == '-')
            {
                return true;
            }
            else if (cIDType == '+')
            {
                bSkip = true;
                break;
            }
        }

        if (!bSkip)
        {
            wcout << (cIDType=='t'?"Set":"Schema") << " ID : ";
            DWFString zID = getLine();
            if (zID == "-")
            {
                return true;
            }

            DWFProperty::tIterator* piProperty = NULL;
            if (cIDType == 't')
            {
                piProperty = pElem->getAllPropertiesBySetID( zID );
            }
            else
            {
                piProperty = pElem->getAllPropertiesBySchemaID( zID );
            }

            if (piProperty == NULL ||
                piProperty->valid()==false)
            {
                wcout << L"No properties were found with the given " <<  (cIDType=='t'?"Set":"Schema") << " ID" << endl;
            }
            else
            {
                for (; piProperty->valid(); piProperty->next())
                {
                    DWFProperty* pProperty = piProperty->get();
                    wcout << L"Property: ( " << pProperty->name() << L" , " << pProperty->value();
                    if (pProperty->category().chars()>0)
                    {
                        wcout << L" , " << pProperty->category();
                    }
                    wcout << L" )" << endl;
                }
            }

            if (piProperty)
            {
                DWFCORE_FREE_OBJECT( piProperty );
            }
        }
    }
    wcout << endl;

    wcout << endl;
 
    return bQuitApp;
}



