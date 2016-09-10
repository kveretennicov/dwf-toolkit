AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS= XPSTest
AM_CPPFLAGS = -DXPSTEST_APP -DDWFCORE_BUILD_ZLIB -DDWFCORE_BUILD_EXPAT -DWHIP_USE_WCHAR_STRINGS -DDWFTK_BUILD_EXPAT
pkginclude_HEADERS = object_list.h \
			OpcHelper.h \
			StdAfx.h \
			TestFramework.h \
			test_list.h \
			XPSTest.h
			
XPSTest_SOURCES = 	obFont.cpp \
			OpcHelper.cpp \
			StdAfx.cpp \
			TestFramework.cpp \
			XPSTest.cpp \
			color_map.cpp \
			contour_set.cpp \
			dash_patterns.cpp \
			delineate.cpp \
			drawing_info.cpp \
			embedded_font.cpp \
			gouraud_polyline.cpp\
			gouraud_polytriangle.cpp \
			image.cpp \
			macro.cpp \			
			object_node.cpp \
			overpost.cpp \
			polygon.cpp \
			polyline.cpp \
			polymarker.cpp \
			text.cpp \
			text_background.cpp \
			text_halign.cpp \
			text_valign.cpp \
			userdata.cpp \
			user_fill_pattern.cpp \
			user_hatch_pattern.cpp

LIBS += -ldwftk -ldwfcore

clean:
	rm XPSTest *.o
test:
	./XPSTest XPSTest.dwfx



