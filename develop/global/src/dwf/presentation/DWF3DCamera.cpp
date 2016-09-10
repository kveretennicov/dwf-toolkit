//
//  Copyright (c) 2007 by Autodesk, Inc.
//
//  By using this code, you are agreeing to the terms and conditions of
//  the License Agreement included in the documentation for this code.
//
//  AUTODESK MAKES NO WARRANTIES, EXPRESS OR IMPLIED, AS TO THE CORRECTNESS
//  OF THIS CODE OR ANY DERIVATIVE WORKS WHICH INCORPORATE IT. AUTODESK
//  PROVIDES THE CODE ON AN "AS-IS" BASIS AND EXPLICITLY DISCLAIMS ANY
//  LIABILITY, INCLUDING CONSEQUENTIAL AND INCIDENTAL DAMAGES FOR ERRORS,
//  OMISSIONS, AND OTHER PROBLEMS IN THE CODE.
//
//  Use, duplication, or disclosure by the U.S. Government is subject to
//  restrictions set forth in FAR 52.227-19 (Commercial Computer Software
//  Restricted Rights) and DFAR 252.227-7013(c)(1)(ii) (Rights in Technical
//  Data and Computer Software), as applicable.
//


#include "dwf/presentation/DWF3DCamera.h"
#include "dwf/package/Constants.h"
using namespace DWFToolkit;

#ifndef DWFTK_READ_ONLY

void DWF3DCamera::serializeXML( DWFXMLSerializer& rSerializer, unsigned int /*nFlags*/ )
    throw( DWFException )
{
    rSerializer.startElement( DWFXML::kzElement_Camera );
    {
        float anValues[3];
        getPosition(anValues);
        rSerializer.addAttribute( DWFXML::kzAttribute_PositionX, anValues[0] );

        rSerializer.addAttribute( DWFXML::kzAttribute_PositionY, anValues[1] );

        rSerializer.addAttribute( DWFXML::kzAttribute_PositionZ, anValues[2] );

        getTarget(anValues);
        rSerializer.addAttribute( DWFXML::kzAttribute_TargetX, anValues[0] );

        rSerializer.addAttribute( DWFXML::kzAttribute_TargetY, anValues[1] );

        rSerializer.addAttribute( DWFXML::kzAttribute_TargetZ, anValues[2] );

        getUpVector(anValues);
        rSerializer.addAttribute( DWFXML::kzAttribute_UpVectorX, anValues[0] );

        rSerializer.addAttribute( DWFXML::kzAttribute_UpVectorY, anValues[1] );

        rSerializer.addAttribute( DWFXML::kzAttribute_UpVectorZ, anValues[2] );

        getField(anValues);
        rSerializer.addAttribute( DWFXML::kzAttribute_FieldWidth, anValues[0] );

        rSerializer.addAttribute( DWFXML::kzAttribute_FieldHeight, anValues[1] );

        DWFString zTempString = (getProjection() == W3DCamera::eOrthographic) ? /*NOXLATE*/L"Orthographic" : /*NOXLATE*/L"Perspective";
        rSerializer.addAttribute( DWFXML::kzAttribute_ProjectionType, zTempString );
    }
    rSerializer.endElement();
}

#endif