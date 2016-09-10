//
//  Copyright (c) 2007 by Autodesk, Inc.
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

#ifdef  DWFCORE_UNIT_TEST

#include "dwfcore/String.h"
using namespace DWFCore;

#include <boost/test/unit_test.hpp>
using boost::unit_test::test_suite;

#include "dwfcore/tests/StringTests.h"

//
// Test the Find() mechanisms
//
static
void
testFind()
{
    DWFCore::DWFString  zTestMeStack;
    zTestMeStack.assign( "small " );
    zTestMeStack.append( "smaller " );
    zTestMeStack.append( L"wide " );

    DWFCore::DWFString  zEmpty;

    BOOST_CHECK(DWFCore::DWFString::Find(zTestMeStack, L's', 0, false)== 0);
    BOOST_CHECK(DWFCore::DWFString::Find(zTestMeStack, L's', 0, true) == 6);
    BOOST_CHECK(DWFCore::DWFString::Find(zTestMeStack, L's', 1, false) == 6);
    BOOST_CHECK(DWFCore::DWFString::Find(zTestMeStack, L's', 1, true) == 6);
    BOOST_CHECK(DWFCore::DWFString::Find(zTestMeStack, L's', 18, false) == -1);
    BOOST_CHECK(DWFCore::DWFString::Find(zTestMeStack, L's', 18, true) == 0);
    BOOST_CHECK(DWFCore::DWFString::Find(zTestMeStack, L's', 19, false) == -1);
    BOOST_CHECK(DWFCore::DWFString::Find(zTestMeStack, L's', 19, true) == -1);
    BOOST_CHECK(DWFCore::DWFString::Find(zTestMeStack, L'X', 0, false) == -1);
    BOOST_CHECK(DWFCore::DWFString::Find(zTestMeStack, L'X', 0, true) == -1);
    BOOST_CHECK(DWFCore::DWFString::Find(zEmpty, L"ABC", 0, true) == -1);
    BOOST_CHECK(DWFCore::DWFString::Find(zTestMeStack, L"", 0, true) == -1);
    BOOST_CHECK(DWFCore::DWFString::Find(zEmpty, L"", 0, true) == -1);
    BOOST_CHECK(DWFCore::DWFString::Find(zEmpty, L'X', 0, true) == -1);
    BOOST_CHECK(DWFCore::DWFString::Find(L"", L"", 0, true) == -1);
    BOOST_CHECK(DWFCore::DWFString::Find(L"", L'X', 0, true) == -1);
    BOOST_CHECK(zEmpty.find( L'X', 0, false) == -1);
    BOOST_CHECK(zEmpty.find( L"ABC", 0, false) == -1);
    BOOST_CHECK(zEmpty.find( L"", 0, false) == -1);
    BOOST_CHECK(zEmpty.find( zTestMeStack, 0, false) == -1);
    BOOST_CHECK(zTestMeStack.find( L"", 0, false) == -1);
    BOOST_CHECK(zTestMeStack.find( zEmpty, 0, false) == -1);
}

//
// Test the substring mechanisms
//
static
void
testSubstring()
{
    DWFCore::DWFString s1(L"This is my string.");
    
    size_t len = s1.chars();

    DWFCore::DWFString s2 = s1.substring(0);
    BOOST_CHECK(s2.chars() == len);

    DWFCore::DWFString s3 = s1.substring(5);
    BOOST_CHECK(s3.chars() == (len-5));

    DWFCore::DWFString s4 = s1.substring(5, 0);
    BOOST_CHECK(s4.chars() == 0);

    DWFCore::DWFString s5 = s1.substring(5, 2);
    BOOST_CHECK(s5.chars() == 2);

    DWFCore::DWFString s6 = s1.substring(5, 100);
    BOOST_CHECK(s6.chars() == (len-5));
}

static
void
testAppend()
{
    DWFCore::DWFString  zTestMeStack;
    zTestMeStack.assign( "small " );
    zTestMeStack.append( "smaller " );
    zTestMeStack.append( L"wide " );
    BOOST_CHECK(zTestMeStack.chars() == 19);

    zTestMeStack.append( zTestMeStack );
    zTestMeStack.append( zTestMeStack );
    zTestMeStack.append( zTestMeStack );
    zTestMeStack.append( zTestMeStack );
    zTestMeStack.append( zTestMeStack );
    zTestMeStack.append( zTestMeStack );
    zTestMeStack.append( zTestMeStack );
    BOOST_CHECK(zTestMeStack.chars() == 128*19);

    unsigned char utf8Buf[] = {0xe6, 0x97, 0xa5, 0xe6, 0x9c, 0xac, 0xe8, 0xaa, 0x9e, 0};  // utf-8 of three japanese characters
    DWFCore::DWFString asianString;
    asianString.assign((char*)utf8Buf);

    zTestMeStack.append( asianString );
    BOOST_CHECK(zTestMeStack.chars() == (128*19)+3);

    zTestMeStack.append( asianString );
    zTestMeStack.append( asianString );
    zTestMeStack.append( asianString );
    zTestMeStack.append( asianString );
    BOOST_CHECK(zTestMeStack.chars() == (128*19)+15);
}

//
// Test != and == operator corner cases.
//
static
void
testEquality()
{
    DWFCore::DWFString  zTestMeStack;
    zTestMeStack.assign( "small " );
    zTestMeStack.append( "smaller " );
    zTestMeStack.append( L"wide " );

    DWFCore::DWFString  zEmpty1;
    DWFCore::DWFString  zEmpty2;

    BOOST_CHECK(zEmpty1 != zTestMeStack);
    BOOST_CHECK(zTestMeStack!= zEmpty1);
    BOOST_CHECK(zEmpty1 != L"ABC");
    BOOST_CHECK(zEmpty1 == zEmpty2);
    BOOST_CHECK(zEmpty1 == L""); // currently fails - this is a defect!

    BOOST_CHECK((zEmpty1 != zEmpty2) == false);
    BOOST_CHECK((zEmpty1 != L"") == false);  // currently fails - this is a defect!
    BOOST_CHECK((zEmpty1 == zTestMeStack) == false);
    BOOST_CHECK((zTestMeStack == zEmpty1) == false);
    BOOST_CHECK((zEmpty1 == L"ABC") == false);
    BOOST_CHECK((zTestMeStack == L"small ") == false);
}

static
void
testUTF8Conversion()
{
    unsigned char utf8Buf[] = {0xe6, 0x97, 0xa5, 0xe6, 0x9c, 0xac, 0xe8, 0xaa, 0x9e, 0};  // utf-8 of three japanese characters
    DWFCore::DWFString s1;
    s1.assign((char*)utf8Buf);
    BOOST_CHECK(s1.chars() == 3);
}

void
DWFStringTest::addTests(boost::unit_test::test_suite* pTestSuite)
{
    pTestSuite->add( BOOST_TEST_CASE( &testFind ), 0 /* expected num of error */, 0 /* timeout */);
    pTestSuite->add( BOOST_TEST_CASE( &testSubstring ), 0 /* expected num of error */, 0 /* timeout */);
    pTestSuite->add( BOOST_TEST_CASE( &testAppend ), 0 /* expected num of error */, 0 /* timeout */);
    pTestSuite->add( BOOST_TEST_CASE( &testEquality ), 2 /* expected num of error */, 0 /* timeout */);
    pTestSuite->add( BOOST_TEST_CASE( &testUTF8Conversion ), 0 /* expected num of error */, 0 /* timeout */);
}

#endif

