AUTOMAKE_OPTIONS = nostdinc
INCLUDES = -I../..

ansi_includedir = $(pkgincludedir)/crypto

ansi_include_HEADERS =	AsymmetricAlgorithm.h \
						CryptoEngine.h \
						CryptoEngineFactory.h \
						CryptoEngineProvider.h \
						CryptoKey.h \
						CryptoKeyGenerator.h \
						CryptoKeyGeneratorFactory.h \
						CryptoKeyGeneratorProvider.h \
						DigestEngine.h \
						EncryptionEngine.h \
						SymmetricCipher.h \
						X509Certificate.h

noinst_LTLIBRARIES =  libdwfcorecrypto.la

libdwfcorecrypto_la_LDFLAGS = -avoid-version 

libdwfcorecrypto_la_SOURCES =	AsymmetricAlgorithm.cpp \
								CryptoEngine.cpp \
								CryptoEngineProvider.cpp \
								CryptoKeyGeneratorProvider.cpp \
								DigestEngine.cpp \
								SymmetricCipher.cpp \
								X509Certificate.cpp

