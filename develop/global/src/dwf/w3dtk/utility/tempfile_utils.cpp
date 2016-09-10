
/*
* $Id: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/w3dtk/utility/tempfile_utils.cpp#1 $
*/

#include "tempfile_utils.h"

#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#pragma warning(disable: 4996) // don't complain about standard string functions
#include <windows.h>
#include <assert.h>
#else
#include <unistd.h>
#include <errno.h>
#ifndef OSX_SYSTEM
#include <stdio.h>
int mkstemps ( char alter * tempFileNameOut, int suffixlen ) {
	int tempFileNameOutLen = strlen(tempFileNameOut);
	char *tempFileNameCopy = new char [tempFileNameOutLen + 1];
	char *suffixCopy = new char [suffixlen + 1];
	strcpy(tempFileNameCopy, tempFileNameOut);
	strcpy(suffixCopy, tempFileNameCopy + tempFileNameOutLen - suffixlen);
	suffixCopy[suffixlen] = 0;
	tempFileNameCopy[tempFileNameOutLen - suffixlen] = 0;
	
	int fileDescriptor = mkstemp(tempFileNameCopy);
	if (fileDescriptor != -1)
		sprintf(tempFileNameOut, "%s%s", tempFileNameCopy, suffixCopy);

	delete [] tempFileNameCopy;
	delete [] suffixCopy;

	return fileDescriptor;
}
#endif
#endif

#include "utf_utils.h"

void GenerateTempFileName ( char alter * tempFileNameOut, char const * extension ) {
#ifdef _MSC_VER
	char temp_dir[_MAX_DIR];
	DWORD dir_len = 0;
	
	dir_len = GetTempPathA(_MAX_DIR, temp_dir);
	assert(dir_len != 0);	
	assert(dir_len <= _MAX_DIR);
	UINT res = 0;
	res = GetTempFileNameA(temp_dir, "HOOPS", 0, tempFileNameOut);
	assert(res != 0);
	// if extension specified, replace .tmp with user-provided value
	if (extension) {
		char *old_extension = strrchr(tempFileNameOut, '.');
		if (extension[0] == '.')
			old_extension[0] = 0;
		else
			old_extension[1] = 0;
		strcat(tempFileNameOut, extension);
	}
#else
	strcpy(tempFileNameOut, "/tmp/tmpXXXXXX");
	int ext_len = 0;
	
	if (extension) {
		if (extension[0] != '.') {
			strcat(tempFileNameOut, ".");
			ext_len += 1;
		}
		strcat(tempFileNameOut, extension);
		ext_len += strlen(extension);
	}
	else {
		strcat(tempFileNameOut, ".tmp");
		ext_len += 4;
	}
	
	int fileDescriptor = mkstemps(tempFileNameOut, ext_len);
	if (fileDescriptor == -1) {
		printf("mkstemps call failed.\nerrno: %d\t%s\n", errno, strerror(errno));
		tempFileNameOut[0] = 0;
	}
	else
		close(fileDescriptor);
#endif
}

void GenerateTempFileName ( wchar_t alter * tempFileNameOut, wchar_t const * extension ) {
#ifdef _MSC_VER
	wchar_t temp_dir[_MAX_DIR];
	DWORD dir_len = 0;
	dir_len = GetTempPathW(_MAX_DIR,  temp_dir);
	assert(dir_len != 0);	
	assert(dir_len <= _MAX_DIR);
	UINT res = 0;
	res = GetTempFileNameW(temp_dir, L"HOOPS", 0, tempFileNameOut);
	assert(res != 0);
	// if extension is specified replace .tmp with user-specified value
	if (extension) {
		wchar_t *old_extension = wcsrchr(tempFileNameOut, L'.');
		if (extension[0] == L'.')
			old_extension[0] = 0;
		else
			old_extension[1] = 0;
		wcscat(tempFileNameOut, extension);
	}
#else
	char temp_template[TEMPFILE_UTILS_BUFFER_SIZE];
	
	if (extension)
		GenerateTempFileName(temp_template, reinterpret_cast<char const *>(H_UTF8(extension).encodedText()));
	else
		GenerateTempFileName(temp_template);
	
	if (temp_template[0] == 0)
		tempFileNameOut[0] = 0;
	else
		wcscpy(tempFileNameOut, H_WCS(temp_template).encodedText());
#endif
}

#if 0
void GenerateTempDirectoryName ( wchar_t alter * tempDirectoryNameOut ) {
#ifdef _MSC_VER
	
#else
	
#endif
}
#endif
