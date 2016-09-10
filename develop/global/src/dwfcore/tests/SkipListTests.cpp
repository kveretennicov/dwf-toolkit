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

#include "dwfcore/SkipList.h"
using namespace DWFCore;

#include <boost/test/unit_test.hpp>
using boost::unit_test::test_suite;

#include "dwfcore/tests/SkipListTests.h"


template<typename KeyType, typename ValueType>
static
void
skipList_specific_type_test(KeyType key, ValueType value, ValueType value2 )
{
    DWFSkipList<KeyType, ValueType> oSkipList;
    BOOST_CHECK(oSkipList.insert(key, value));
    BOOST_CHECK(oSkipList.size() == 1);

    ValueType* pItem = oSkipList.find(key);
    BOOST_CHECK(pItem != NULL);
    BOOST_CHECK(*pItem == value);

    // test clearing
    oSkipList.clear();
    pItem = oSkipList.find(key);
    BOOST_CHECK(pItem == NULL);
    BOOST_CHECK(oSkipList.size() == 0);

    // test duplicate insertion with replacement
    oSkipList.insert(key, value);
    BOOST_CHECK(oSkipList.size() == 1);
    oSkipList.insert(key, value2, true);
    BOOST_CHECK(oSkipList.size() == 1);
    pItem = oSkipList.find(key);
    BOOST_CHECK(pItem != NULL);
    BOOST_CHECK(*pItem == value2);

    oSkipList.clear();

    // test duplicate insertion without replacement
    oSkipList.insert(key, value);
    BOOST_CHECK(oSkipList.size() == 1);
    oSkipList.insert(key, value2, false);
    BOOST_CHECK(oSkipList.size() == 1);
    pItem = oSkipList.find(key);
    BOOST_CHECK(pItem != NULL);
    BOOST_CHECK(*pItem == value);

    oSkipList.clear();
}

static
void
testSkipLists()
{
    skipList_specific_type_test<int, int>(1000000, 0, 1);
    skipList_specific_type_test<int, DWFCore::DWFString>(-1, L"Woof", L"Meow");
    skipList_specific_type_test<wchar_t*, DWFCore::DWFString>(L"Arf", L"Woof", L"Meow");
}

void
DWFSkipListTest::addTests(boost::unit_test::test_suite* pTestSuite)
{
    pTestSuite->add( BOOST_TEST_CASE( &testSkipLists ), 0 /* expected num of error */, 0 /* timeout */);
}

#endif

