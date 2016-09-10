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

#include "StdAfx.h"
#include <fstream>
#include <string>
#include "TestFramework.h"
#include "whiptk/w2d_class_factory.h"
#include "XAML/XamlClassFactory.h"

CTestFramework::CTestFramework() :
    m_classFactoryType(NONE),
    m_bDoReadTests(true),
    m_bDoWriteTests(true),
    m_pClassFactory(NULL)
{
}

CTestFramework::~CTestFramework()
{
    CleanupCurrentClassFactory();
}

CTestFramework &CTestFramework::Instance()
{
    static CTestFramework testFramework;
    return testFramework;
}

bool &CTestFramework::DoReadTests()
{
    return m_bDoReadTests;
}

bool &CTestFramework::DoWriteTests()
{
    return m_bDoWriteTests;
}

const CTestFramework::CLASS_FACTORY_TYPE &CTestFramework::GetClassFactoryType() const
{
    return m_classFactoryType;
}

void CTestFramework::SetClassFactoryType(const CTestFramework::CLASS_FACTORY_TYPE &val)
{
    // Cleanup the currently allocated class factory
    CleanupCurrentClassFactory();

    // Save the type of the class factory to be used
    m_classFactoryType = val;

    // Create an instance of the respective class factory,
    // depending on the class factory type to be used.
    switch( m_classFactoryType )
    {
    case W2D:
        m_pClassFactory = new WT_W2D_Class_Factory();
        break;
    case XAML:
        m_pClassFactory = new WT_XAML_Class_Factory();
        break;

        // Insert support for other class factories just above this line
    }
}

WT_Class_Factory * const CTestFramework::GetClassFactory() const
{
    return m_pClassFactory;
}

void CTestFramework::CleanupCurrentClassFactory()
{
    if (m_pClassFactory != NULL)
    {
        DWFCORE_FREE_OBJECT( m_pClassFactory );
    }

    m_classFactoryType = NONE;
}

std::string CTestFramework::trim(const std::string &str, const char * const whiteSpaceSet)
{
    std::string::size_type const first = str.find_first_not_of(whiteSpaceSet);
    if( first == std::string::npos )
        return std::string();

    std::string::size_type const last  = str.find_last_not_of(whiteSpaceSet);

    return str.substr(first, last-first+1);
}

const bool CTestFramework::ReadSettingsFromConfigFile(const WT_String &configFileName)
{
    ifstream configFile(configFileName.ascii());
    if( !configFile.is_open() )
        return false;

    for(;;)
    {
        string var;
        getline(configFile, var, '=');
        if( configFile.eof() )
            break;

        string val;
        getline(configFile, val, '\n');
        if( configFile.eof() )
            break;

        var = trim(var, " \t\n");
        val = trim(val, " \t\n");

        // Process this variable and value

        // Notes:
        //
        //     Comments can be inserted into the config file by starting
        //     the line with a '=' followed by the comment text.
        //    E.g.:
        //    = This is a comment line
        //
        //    Allowed variables and their values:
        //    -------------------------------------
        //     Var                Values
        //     -------------------------------------
        //    doReadTests        true, false
        //    doWriteTests    true, false
        //    classFactory    W2D
        //

        if( var.compare("doReadTests") == 0 )
        {
            if( val.compare("true") == 0 )
                m_bDoReadTests = true;
            else
                m_bDoReadTests = false;
        }
        else if( var.compare("doWriteTests") == 0 )
        {
            if( val.compare("true") == 0 )
                m_bDoWriteTests = true;
            else
                m_bDoWriteTests = false;
        }
        else if( var.compare("classFactory") == 0 )
        {
            if( val.compare("W2D") == 0 )
                SetClassFactoryType( W2D );
            else if( val.compare("XAML") == 0 )
                SetClassFactoryType( XAML );
        }
    }

    configFile.close();

    return true;
}
