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

#ifdef  DWFCORE_UNIT_TEST

#include <boost/test/unit_test.hpp>
using boost::unit_test::test_suite;

#include "dwfcore/tests/StringTests.h"
#include "dwfcore/tests/SkipListTests.h"


//
// This method is invoked by the main() provided by boost's unit test framework.
//
test_suite* init_unit_test_suite( int, char* [] )
{
    test_suite* test= BOOST_TEST_SUITE( "Core library Unit test" );

    // ----
    // List tests here
    DWFStringTest().addTests(test);
    DWFSkipListTest().addTests(test);
    // ----

    return test;
}

#endif
