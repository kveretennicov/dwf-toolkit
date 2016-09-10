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

#include "whiptk/class_factory.h"

class CTestFramework
{

    // Enumerations
public:
    enum CLASS_FACTORY_TYPE
    {
        NONE = 0, //Default
        W2D,
        XAML
    };

    // Members
private:
    CLASS_FACTORY_TYPE m_classFactoryType;

    bool m_bDoReadTests;        // True by default
    bool m_bDoWriteTests;        // True by default

    WT_Class_Factory  *m_pClassFactory;


    // Copy Constructor / Assignment operator
private:
    CTestFramework(const CTestFramework &);
    const CTestFramework &operator = (const CTestFramework &);


    // Constructor / Destructor
private:
    explicit CTestFramework();
public:
    virtual ~CTestFramework();


    // Private Functions
private:
    void CleanupCurrentClassFactory();
    std::string trim(const std::string &str, const char * const whiteSpaceSet);

    // Public Functions
public:
    static CTestFramework &Instance();

    bool &DoReadTests();
    bool &DoWriteTests();

    const CLASS_FACTORY_TYPE &GetClassFactoryType() const;
    void SetClassFactoryType(const CLASS_FACTORY_TYPE &val);

    WT_Class_Factory * const GetClassFactory() const;

    const bool ReadSettingsFromConfigFile(const WT_String &configFileName);
};
