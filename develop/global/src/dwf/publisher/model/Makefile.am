AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../../..

model_includedir = $(pkgincludedir)/publisher/model
model_include_HEADERS =  AttributeHandlerBuilder.h FeatureHandlerBuilder.h GeometryHandlerBuilder.h IncludeSegment.h Model.h Segment.h SegmentHandlerBuilder.h StyleSegment.h
noinst_LTLIBRARIES =  libdwftkpublishermodel.la
libdwftkpublishermodel_la_LDFLAGS = -avoid-version 
libdwftkpublishermodel_la_SOURCES =  IncludeSegment.cpp Model.cpp Segment.cpp StyleSegment.cpp
