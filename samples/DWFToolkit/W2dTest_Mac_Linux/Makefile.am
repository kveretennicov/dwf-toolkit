AUTOMAKE_OPTIONS = foreign
bin_PROGRAMS= W2dTest
AM_CPPFLAGS = -DW2DTEST_APP -DDWFCORE_BUILD_ZLIB -DDWFCORE_BUILD_EXPAT -DWHIP_USE_WCHAR_STRINGS -DDWFTK_BUILD_EXPAT
pkginclude_HEADERS = object_list.h \
			fontsub.h \
			OpcHelper.h \
			StdAfx.h \
			t2embapi.h \
			TestFramework.h \
			test_list.h \
			W2dTest.h \
			WhipClassFactoryWrapper.h \
			whip_fill_library.h \
			whip_hatch_library.h 
			
W2dTest_SOURCES = TestFramework.cpp \
			color_map.cpp \
			contour_set.cpp \
			dash_patterns.cpp \
			delineate.cpp \
			drawing_info.cpp \
			embedded_font.cpp \
			FontSubset.cpp \
			gouraud_polyline.cpp\
			gouraud_polytriangle.cpp \
			image.cpp \
			macro.cpp \
			obFont.cpp \
			object_node.cpp \
			OpcHelper.cpp \
			overpost.cpp \
			polygon.cpp \
			polyline.cpp \
			polymarker.cpp \
			StdAfx.cpp \
			text.cpp \
			text_background.cpp \
			text_halign.cpp \
			text_valign.cpp \
			userdata.cpp \
			user_fill_pattern.cpp \
			user_hatch_pattern.cpp \
			W2dTest.cpp \
			WhipClassFactoryWrapper.cpp

LIBS += -ldwftk -ldwfcore

clean:
	rm W2dTest *.o
test:
	./W2dTest 



