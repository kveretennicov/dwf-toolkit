AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I..
SUBDIRS = ansi crypto digest expat mac utility x86 x64 zip zlib sqlite paging


pkginclude_HEADERS =	BufferInputStream.h \
						BufferOutputStream.h \
						Comparator.h \
						Core.h \
						CountedObject.h \
						DecompressingInputStream.h \
						DWFCompressingInputStream.h \
						DWFEncryptingInputStream.h \
						DWFDecryptingInputStream.h \
						Digest.h \
						DigestInputStream.h \
						DigestOutputStream.h \
						DWFXMLSerializer.h \
						Exception.h \
						File.h \
						FileDescriptor.h \
						FileInputStream.h \
						FileOutputStream.h \
						Hash.h \
						InputStream.h \
						Iterator.h \
						MIME.h \
						MonitoredInputStream.h \
						OutputStream.h \
						Owner.h \
						Pointer.h \
						SkipList.h \
						STL.h \
						StreamFileDescriptor.h \
						StreamOutputBufferDescriptor.h \
						String.h \
						Synchronization.h \
						TempFile.h \
						ThreadPool.h \
						Timer.h \
						UnzippingInputStream.h \
						UTF8EncodingOutputStream.h \
						UUID.h \
						Vector.h \
						Version.h \
						XML.h \
						XMLEncodingOutputStream.h \
						XMLParser.h \
						ZipFileDescriptor.h \
						ZippingOutputStream.h

lib_LTLIBRARIES =  libdwfcore.la

libdwfcore_la_LDFLAGS = -release 1.7.0

libdwfcore_la_SOURCES = BufferInputStream.cpp \
						BufferOutputStream.cpp \
						DecompressingInputStream.cpp \
						DWFCompressingInputStream.cpp \
						DWFEncryptingInputStream.cpp \
						DWFDecryptingInputStream.cpp \
						Digest.cpp \
						DigestInputStream.cpp \
						DigestOutputStream.cpp \
						DWFXMLSerializer.cpp \
						Exception.cpp \
						File.cpp \
						FileInputStream.cpp \
						FileOutputStream.cpp \
						MIME.cpp \
						MonitoredInputStream.cpp \
						Owner.cpp \
						StreamOutputBufferDescriptor.cpp \
						ThreadPool.cpp \
						UnzippingInputStream.cpp \
						UTF8EncodingOutputStream.cpp \
						UUID.cpp \
						XML.cpp \
						XMLEncodingOutputStream.cpp \
						XMLParser.cpp \
						ZipFileDescriptor.cpp \
						ZippingOutputStream.cpp

libdwfcore_la_LIBADD = \
	$(top_builddir)/ansi/libdwfcoreansi.la \
	$(top_builddir)/crypto/libdwfcorecrypto.la \
	$(top_builddir)/digest/libdwfcoredigest.la \
	$(top_builddir)/mac/libdwfcoremac.la  \
	$(top_builddir)/x86/libdwfcorex86.la \
	$(top_builddir)/x64/libdwfcorex64.la \
	$(top_builddir)/zlib/libdwfcorezlib.la \
	$(top_builddir)/zip/libdwfcorezip.la \
	$(top_builddir)/expat/libdwfcoreexpat.la \
	$(top_builddir)/utility/libdwfcoreutility.la \
	$(top_builddir)/sqlite/libdwfcoresqlite.la \
	$(top_builddir)/paging/libdwfcorepaging.la

