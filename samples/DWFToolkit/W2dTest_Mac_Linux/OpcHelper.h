#include "dwfcore/Core.h"
#include "dwf/Toolkit.h"
#include "dwfcore/File.h"
#include "XAML/XamlClassFactory.h"
#include "dwf/XAML/pch.h"

#ifndef OpcHelper_HEADER
#define OpcHelper_HEADER

using namespace DWFCore;
using namespace DWFToolkit;

//Define OBFUSCATE if we want odttf font files
//#define OBFUSCATE

class OpcHelper : public WT_OpcResourceSerializer, public WT_OpcResourceMaterializer
{
public:
	OpcHelper();
	virtual ~OpcHelper();

private:
    //WT_OpcResourceMaterializer
    WT_Result getPartInputStream(
		const DWFCore::DWFString& rzPath, 
		DWFCore::DWFInputStream** ppStream);

    //WT_OpcResourceSerializer
    WT_Result getPartOutputStream(
        const DWFCore::DWFString& rzMimeType,
        DWFCore::DWFString& rzPath,
        DWFCore::DWFOutputStream** ppStream);

    int _nPartCount;
};

#endif //XAML_TEST_HEADER