// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//


#ifndef _SIMPLE_W3D_STREAM_PROCESSOR_H
#define _SIMPLE_W3D_STREAM_PROCESSOR_H

//
// We need to try and include stl stuff before the max and min 
// macros get defined an mess up the use of std::max and std::min
// (problem on linux and maybe other platforms).
//
#include "dwfcore/STL.h"
#include <iostream>

#include "dwfcore/File.h"
#include "dwfcore/String.h"

#include "dwf/package/Constants.h"
#include "dwf/package/Manifest.h"
#include "dwf/package/EModelSection.h"
#include "dwf/package/reader/PackageReader.h"

#include "dwf/w3dtk/BStream.h"
#include "dwf/w3dtk/BOpcodeShell.h"

#endif
