
#include "utf_utils.h"
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <stdarg.h>

#ifdef _MSC_VER
#pragma warning (disable:4127)
#pragma warning (disable:4996)
#endif

#ifdef NEEDS_STR_CASE_CMP
int strcasecmp ( char const * string1, char const * string2 ) {
    char c1, c2;

	while (*string1 && *string2) {
		c1 = tolower(*string1++);
		c2 = tolower(*string2++);
		if (c1 < c2) return -1;
		if (c1 > c2) return 1;
	}

	if (*string1 || *string2) {
		if (*string2)
			return -1;
		if (*string1)
			return 1;
	}

    return 0;
}
#endif
#ifdef NEEDS_WCS_CASE_CMP
int wcscasecmp ( __wchar_t const * string1, __wchar_t const * string2 ) {
	__wchar_t c1, c2;

	while (*string1 && *string2) {
		c1 = towlower(*string1++);
		c2 = towlower(*string2++);
		if (c1 < c2) return -1;
		if (c1 > c2) return 1;
	}

	if (*string1 || *string2) {
		if (*string2)
			return -1;
		if (*string1)
			return 1;
	}

	return 0;
}
#endif

static wchar_t * _va_format_wcs(wchar_t const * const format, va_list va[2]);
static char * _va_format_mbs(char const * const format, va_list va[2]);

#define H_DEFAULT_LOCALE(x) do{char const * const oldloc = setlocale(LC_CTYPE, ""); (x); setlocale(LC_CTYPE, oldloc);}while(0)

template <typename T>
int utf32_char_count(T iter)
{
    int utf32_chars_needed = 0;
    int code_point = 0;
    do {
        code_point = *iter++;
        ++utf32_chars_needed;
    }while(code_point);

    return utf32_chars_needed;
}

template <typename T>
void unicode_to_utf32(utf32_char * utf32, T iter)
{
    int code_point = 0;
    do {
        code_point = *iter++;
        *utf32++ = (utf32_char)code_point;
    }while(code_point);
}

H_UTF32::H_UTF32(H_UTF32 const & rhs) : m_utf32(0), m_size(0) {
  if(rhs.encodedText()){
    m_size = utf32_char_count(rhs.begin());
    m_utf32 = new utf32_char[m_size];
    unicode_to_utf32(m_utf32, rhs.begin());
  }
}

H_UTF32 const & H_UTF32::operator = (H_UTF32 const & rhs) {
    delete [] m_utf32;
    m_utf32 = 0;
    m_size = 0;
    if(rhs.encodedText()){
        m_size = utf32_char_count(rhs.begin());
        m_utf32 = new utf32_char[m_size];
        unicode_to_utf32(m_utf32, rhs.begin());
    }
    return *this;
}

H_UTF32::H_UTF32() : m_utf32(0), m_size(0) {}

H_UTF32::H_UTF32(__wchar_t const * text) : m_utf32(0), m_size(0) {
    if(text){
		if(sizeof(wchar_t) == sizeof(utf32_char)){
            m_size = utf32_char_count(text);
            m_utf32 = new utf32_char[m_size];
            unicode_to_utf32(m_utf32, text);
        }
        else {
            H_WCS u;
	    u.encodedText(text);
            m_size = utf32_char_count(u.begin());
            m_utf32 = new utf32_char[m_size];
            unicode_to_utf32(m_utf32, u.begin());
        }
    }
}



#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
H_UTF32::H_UTF32(unsigned short const * text) : m_utf32(0), m_size(0) {
    if(text){
		if(sizeof(wchar_t) == sizeof(utf32_char)){
            m_size = utf32_char_count((wchar_t const*)text);
            m_utf32 = new utf32_char[m_size];
            unicode_to_utf32(m_utf32, (wchar_t const*)text);
        }
        else {
            H_WCS u;
	    u.encodedText(text);
            m_size = utf32_char_count(u.begin());
            m_utf32 = new utf32_char[m_size];
            unicode_to_utf32(m_utf32, u.begin());
        }
    }
}
#endif

H_UTF32::H_UTF32(char const * text) : m_utf32(0), m_size(0) {
    if(text){
        H_DEFAULT_LOCALE(m_size = mbstowcs(0, text, 0) +1);
        wchar_t * const tmp = new wchar_t[m_size];
        H_DEFAULT_LOCALE(mbstowcs(tmp, text, m_size));
        *this = H_UTF32(tmp);
        delete [] tmp;
    }
}


H_UTF32::H_UTF32(H_UTF16 const & unicode_text) : m_utf32(0), m_size(0) {
    if(unicode_text.encodedText()){
        m_size = utf32_char_count(unicode_text.begin());
        m_utf32 = new utf32_char[m_size];
        unicode_to_utf32(m_utf32, unicode_text.begin());
    }
}

H_UTF32::H_UTF32(H_UTF8 const & unicode_text) : m_utf32(0), m_size(0) {
    if(unicode_text.encodedText()){
        m_size = utf32_char_count(unicode_text.begin());
        m_utf32 = new utf32_char[m_size];
        unicode_to_utf32(m_utf32, unicode_text.begin());
    }
}

H_UTF32::H_UTF32(H_URI const & unicode_text) : m_utf32(0), m_size(0) {
    if(unicode_text.encodedText()){
        m_size = utf32_char_count(unicode_text.begin());
        m_utf32 = new utf32_char[m_size];
        unicode_to_utf32(m_utf32, unicode_text.begin());
    }
}

H_UTF32::~H_UTF32() {delete [] m_utf32;}

size_t H_UTF32::length() const {
	if(m_utf32)
	  return (size_t)(utf32_char_count(begin()) -1);
	return 0;
}

H_UTF32 & H_UTF32::format(wchar_t const * fmt, ...){
  va_list va[2];

  va_start(va[0], fmt);
  va_start(va[1], fmt);
  wchar_t * wcs = _va_format_wcs(fmt, va);
  va_end(va[0]);
  va_end(va[1]);

  H_UTF32 tmp(wcs);
  delete [] wcs;

  *this = tmp;

  return *this;
}


#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
H_UTF32 & H_UTF32::format(unsigned short const * fmt, ...){
  va_list va[2];

  va_start(va[0], fmt);
  va_start(va[1], fmt);
  wchar_t * wcs = _va_format_wcs((wchar_t const *)fmt, va);
  va_end(va[0]);
  va_end(va[1]);

  H_UTF32 tmp(wcs);
  delete [] wcs;

  *this = tmp;

  return *this;
}
#endif

H_UTF32 & H_UTF32::format(char const * fmt, ...){
  va_list va[2];

  va_start(va[0], fmt);
  va_start(va[1], fmt);
  char * mbs = _va_format_mbs(fmt, va);
  va_end(va[0]);
  va_end(va[1]);

  H_UTF32 tmp(mbs);
  delete [] mbs;

  *this = tmp;

  return *this;
}

H_UTF32 & H_UTF32::append(wchar_t const * txt){
  H_WCS orig_utf(*this);
  wchar_t const * orig_wcs = orig_utf.encodedText();
  if(!orig_wcs)
	  orig_wcs = L"";
  format(L"%ls%ls", orig_wcs, txt);
  return *this;
}

#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
H_UTF32 & H_UTF32::append(unsigned short const * txt){
	return append((wchar_t const*) txt);
}
#endif

H_UTF32 & H_UTF32::append(char const * txt){
  H_WCS wcs(txt);
  append(wcs.encodedText());
  return *this;
}

int H_UTF32::iterator::get_and_advance(utf32_char const * & text) const {
    int code_point = *text++;
    return code_point;
}

H_UTF32::iterator::iterator(utf32_char const * text) : m_text(text) {
}

H_UTF32::iterator H_UTF32::iterator::operator ++ () const {
    return iterator(++m_text);
}

H_UTF32::iterator H_UTF32::iterator::operator ++ (int) const {
    return iterator(m_text++);
}
int H_UTF32::iterator::operator * () const {
    utf32_char const * tmp = m_text;
    int code_point = *tmp;
    return code_point;
}
H_UTF32::iterator::operator bool () const {
    return false;
}

H_UTF32::iterator H_UTF32::begin() const {return iterator(m_utf32);}
H_UTF32::iterator H_UTF32::end() const {return iterator(m_utf32 + m_size);}

bool H_UTF32::operator == (H_UTF32 const & rhs) const
{
  if(length() != rhs.length())
    return false;

  return (memcmp(m_utf32, rhs.m_utf32, length() * sizeof(utf32_char)) == 0);
}

template <typename T>
int utf16_char_count(T iter)
{
    int code_point = 0;
    int utf16_chars_needed = 0;
    do {
        code_point = *iter++;
        if(code_point >= 0 && code_point <= 0xffff){
            utf16_chars_needed += 1;
        }
        else if(code_point >= 0x10000 && code_point <= 0x10ffff){
            code_point -= 0x10000;
            utf16_chars_needed += 2;
        }
        else{
            return -1;
        }
    }while(code_point);

    return utf16_chars_needed;
}

template <typename T>
void unicode_to_utf16(utf16_char * utf16, T iter)
{
    int code_point = 0;
    do {
        code_point = *iter++;
        if(code_point >= 0 && code_point <= 0xffff){
            *utf16++ = (utf16_char)code_point;
        }
        else if(code_point >= 0x10000 && code_point <= 0x10ffff){
            code_point -= 0x10000;
            *utf16++ = (utf16_char)(0xd800 | (code_point >> 10));
            *utf16++ = (utf16_char)(0xdc00 | (code_point & 0x3ff));
        }
        else{
            /* throw an exception. */(void)0;
        }
    }while(code_point);
    return;
}


H_UTF16::H_UTF16(H_UTF16 const & rhs) : m_utf16(0), m_size(0) {
    if(rhs.encodedText()){
        m_size = utf16_char_count(rhs.begin());
        m_utf16 = new utf16_char[m_size];
        unicode_to_utf16(m_utf16, rhs.begin());
    }
}

H_UTF16 const & H_UTF16::operator = (H_UTF16 const & rhs) {
    delete [] m_utf16;
    m_utf16 = 0;
    m_size = 0;
    if(rhs.encodedText()){
        m_size = utf16_char_count(rhs.begin());
        m_utf16 = new utf16_char[m_size];
        unicode_to_utf16(m_utf16, rhs.begin());
    }
    return *this;
}

H_UTF16::H_UTF16() : m_utf16(0), m_size(0) {}

H_UTF16::H_UTF16(wchar_t const * text) : m_utf16(0), m_size(0) {
    if(text){
        if(sizeof(wchar_t) == sizeof(utf16_char)){
            m_size = utf16_char_count(text);
            m_utf16 = new utf16_char[m_size];
            unicode_to_utf16(m_utf16, text);
        }
        else {
            H_WCS u;
	    u.encodedText(text);
            m_size = utf16_char_count(u.begin());
            m_utf16 = new utf16_char[m_size];
            unicode_to_utf16(m_utf16, u.begin());
        }
    }
}


#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
H_UTF16::H_UTF16(unsigned short const * text) : m_utf16(0), m_size(0) {
    if(text){
        if(sizeof(wchar_t) == sizeof(utf16_char)){
            m_size = utf16_char_count((wchar_t const*) text);
            m_utf16 = new utf16_char[m_size];
            unicode_to_utf16(m_utf16, (wchar_t const*) text);
        }
        else {
            H_WCS u;
		    u.encodedText(text);
            m_size = utf16_char_count(u.begin());
            m_utf16 = new utf16_char[m_size];
            unicode_to_utf16(m_utf16, u.begin());
        }
    }
}
#endif

H_UTF16::H_UTF16(char const * text) : m_utf16(0), m_size(0) {
    if(text){
        H_DEFAULT_LOCALE(m_size = mbstowcs(0, text, 0) +1);
        wchar_t * const tmp = new wchar_t[m_size];
        H_DEFAULT_LOCALE(mbstowcs(tmp, text, m_size));
        *this = H_UTF16(tmp);
        delete [] tmp;
    }
}

H_UTF16::H_UTF16(H_UTF32 const & unicode_text) : m_utf16(0), m_size(0) {
    if(unicode_text.encodedText()){
        m_size = utf16_char_count(unicode_text.begin());
        m_utf16 = new utf16_char[m_size];
        unicode_to_utf16(m_utf16, unicode_text.begin());
    }
}

H_UTF16::H_UTF16(H_UTF8 const & unicode_text) : m_utf16(0), m_size(0) {
    if(unicode_text.encodedText()){
        m_size = utf16_char_count(unicode_text.begin());
        m_utf16 = new utf16_char[m_size];
        unicode_to_utf16(m_utf16, unicode_text.begin());
    }
}

H_UTF16::H_UTF16(H_URI const & unicode_text) : m_utf16(0), m_size(0) {
    if(unicode_text.encodedText()){
        m_size = utf16_char_count(unicode_text.begin());
        m_utf16 = new utf16_char[m_size];
        unicode_to_utf16(m_utf16, unicode_text.begin());
    }
}

H_UTF16::~H_UTF16() {delete [] m_utf16;}

size_t H_UTF16::length() const {
	if(m_utf16)
	  return (size_t)(utf16_char_count(begin()) -1);
	return 0;
}

H_UTF16 & H_UTF16::format(wchar_t const * fmt, ...){
  va_list va[2];

  va_start(va[0], fmt);
  va_start(va[1], fmt);
  wchar_t * wcs = _va_format_wcs(fmt, va);
  va_end(va[0]);
  va_end(va[1]);

  H_UTF16 tmp(wcs);
  delete [] wcs;

  *this = tmp;

  return *this;
}


#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
H_UTF16 & H_UTF16::format(unsigned short const * fmt, ...){
  va_list va[2];

  va_start(va[0], fmt);
  va_start(va[1], fmt);
  wchar_t * wcs = _va_format_wcs((wchar_t const*) fmt, va);
  va_end(va[0]);
  va_end(va[1]);

  H_UTF16 tmp(wcs);
  delete [] wcs;

  *this = tmp;

  return *this;
}
#endif

H_UTF16 & H_UTF16::format(char const * fmt, ...){
  va_list va[2];

  va_start(va[0], fmt);
  va_start(va[1], fmt);
  char * mbs = _va_format_mbs(fmt, va);
  va_end(va[0]);
  va_end(va[1]);

  H_UTF16 tmp(mbs);
  delete [] mbs;

  *this = tmp;

  return *this;
}

H_UTF16 & H_UTF16::append(wchar_t const * txt){
  H_WCS orig_utf(*this);
  wchar_t const * orig_wcs = orig_utf.encodedText();
  if(!orig_wcs)
	  orig_wcs = L"";
  format(L"%ls%ls", orig_wcs, txt);
  return *this;
}


#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
H_UTF16 & H_UTF16::append(unsigned short const * txt){
	return append((wchar_t const*) txt);
}
#endif

H_UTF16 & H_UTF16::append(char const * txt){
  H_WCS wcs(txt);
  append(wcs.encodedText());
  return *this;
}

int H_UTF16::iterator::get_and_advance(utf16_char const * & text) const {
    int w1 = *text++;
    if(w1 >= 0 && w1 <= 0xffff){
        return w1;
    }
    if(w1 >= 0xd800 && w1 <= 0xdbff){
        int w2 = *text++;
        return ((0x3ff & w1) << 10) | (0x3ff & w2);
    }
    return -1;
}

H_UTF16::iterator::iterator(utf16_char const * text) : m_text(text) {
}

H_UTF16::iterator H_UTF16::iterator::operator ++ () const {
    get_and_advance(m_text);
    return iterator(m_text);
}

H_UTF16::iterator H_UTF16::iterator::operator ++ (int) const {
    iterator retval(m_text);
    get_and_advance(m_text);
    return retval;
}

int H_UTF16::iterator::operator * () const {
    utf16_char const * tmp = m_text;
    return get_and_advance(tmp);
}

H_UTF16::iterator::operator bool () const {
    return false;
}

H_UTF16::iterator H_UTF16::begin() const {return iterator(m_utf16);}
H_UTF16::iterator H_UTF16::end() const {return iterator(m_utf16 + m_size);}

bool H_UTF16::operator == (H_UTF16 const & rhs) const
{
  if(length() != rhs.length())
    return false;

  return (memcmp(m_utf16, rhs.m_utf16, length() * sizeof(utf16_char)) == 0);
}

template <typename T>
int utf8_char_count(T iter)
{
    int code_point = 0;
    int utf8_chars_needed = 0;
    do {
        code_point = *iter++;
        if(code_point >= 0 && code_point <= 0x7f){
            ++utf8_chars_needed;
        }
        else if(code_point >= 0x80 && code_point <= 0x7ff){
            utf8_chars_needed += 2;
        }
        else if(code_point >= 0x800 && code_point <= 0xffff){
            utf8_chars_needed += 3;
        }
        else if(code_point >= 0x10000 && code_point <= 0x10ffff){
            utf8_chars_needed += 4;
        }
        else{
            return -1;
        }
    }while(code_point);

    return utf8_chars_needed;
}

template <typename T>
void unicode_to_utf8(utf8_char * utf8, T iter)
{
    int code_point = 0;
    do {
        code_point = *iter++;
        if(code_point >= 0 && code_point <= 0x7f){
            *utf8++ = (utf8_char)code_point;
        }
        else if(code_point >= 0x80 && code_point <= 0x7ff){
            *utf8++ = (utf8_char)(0xc0 | (code_point >> 6));
            *utf8++ = (utf8_char)(0x80 | (code_point & 0x3f));
        }
        else if(code_point >= 0x800 && code_point <= 0xffff){
            *utf8++ = (utf8_char)(0xe0 | (code_point >> 12));
            *utf8++ = (utf8_char)(0x80 | (code_point >> 6 & 0x3f));
            *utf8++ = (utf8_char)(0x80 | (code_point & 0x3f));
        }
        else if(code_point >= 0x10000 && code_point <= 0x10ffff){
            *utf8++ = (utf8_char)(0xf0 | (code_point >> 18));
            *utf8++ = (utf8_char)(0x80 | (code_point >> 12 & 0x3f));
            *utf8++ = (utf8_char)(0x80 | (code_point >> 6 & 0x3f));
            *utf8++ = (utf8_char)(0x80 | (code_point & 0x3f));
        }
        else{
            /* throw an exception. */(void)0;
        }
    }while(code_point);
    return;
}

H_UTF8::H_UTF8(H_UTF8 const & rhs) : m_utf8(0), m_size(0) {
    if(rhs.encodedText()){
        m_size = utf8_char_count(rhs.begin());
        m_utf8 = new utf8_char[m_size];
        unicode_to_utf8(m_utf8, rhs.begin());
    }
}

H_UTF8 const & H_UTF8::operator = (H_UTF8 const & rhs) {
    delete [] m_utf8;
    m_utf8 = 0;
    m_size = 0;
    if(rhs.encodedText()){
        m_size = utf8_char_count(rhs.begin());
        m_utf8 = new utf8_char[m_size];
        unicode_to_utf8(m_utf8, rhs.begin());
    }
    return *this;
}

H_UTF8::H_UTF8() : m_utf8(0), m_size(0) {}

H_UTF8::H_UTF8(wchar_t const * text) : m_utf8(0), m_size(0) {
    if(text){
        H_WCS u;
		u.encodedText(text);
        m_size = utf8_char_count(u.begin());
        m_utf8 = new utf8_char[m_size];
        unicode_to_utf8(m_utf8, u.begin());
    }
}


#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
H_UTF8::H_UTF8(unsigned short const * text) : m_utf8(0), m_size(0) {
    if(text){
        H_WCS u;
		u.encodedText((wchar_t const*) text);
        m_size = utf8_char_count(u.begin());
        m_utf8 = new utf8_char[m_size];
        unicode_to_utf8(m_utf8, u.begin());
    }
}
#endif

/* This is locale specific multibyte string text. */
H_UTF8::H_UTF8(char const * text) : m_utf8(0), m_size(0) {
    if(text){
        H_DEFAULT_LOCALE(m_size = mbstowcs(0, text, 0) +1);
        wchar_t * const tmp = new wchar_t[m_size];
        H_DEFAULT_LOCALE(mbstowcs(tmp, text, m_size));
        *this = H_UTF8(tmp);
        delete [] tmp;
    }
}

H_UTF8::H_UTF8(H_UTF32 const & unicode_text) : m_utf8(0), m_size(0) {
    if(unicode_text.encodedText()){
        m_size = utf8_char_count(unicode_text.begin());
        m_utf8 = new utf8_char[m_size];
        unicode_to_utf8(m_utf8, unicode_text.begin());
    }
}

H_UTF8::H_UTF8(H_UTF16 const & unicode_text) : m_utf8(0), m_size(0) {
    if(unicode_text.encodedText()){
        m_size = utf8_char_count(unicode_text.begin());
        m_utf8 = new utf8_char[m_size];
        unicode_to_utf8(m_utf8, unicode_text.begin());
    }
}

H_UTF8::H_UTF8(H_URI const & unicode_text) : m_utf8(0), m_size(0) {
    if(unicode_text.encodedText()){
        m_size = utf8_char_count(unicode_text.begin());
        m_utf8 = new utf8_char[m_size];
        unicode_to_utf8(m_utf8, unicode_text.begin());
    }
}

H_UTF8::~H_UTF8() {delete [] m_utf8;}

size_t H_UTF8::length() const {
	if(m_utf8)
	  return (size_t)(utf8_char_count(begin()) -1);
	return 0;
}

H_UTF8 & H_UTF8::format(wchar_t const * fmt, ...){
  va_list va[2];

  va_start(va[0], fmt);
  va_start(va[1], fmt);
  wchar_t * wcs = _va_format_wcs(fmt, va);
  va_end(va[0]);
  va_end(va[1]);

  H_UTF8 tmp(wcs);
  delete [] wcs;

  *this = tmp;

  return *this;
}


#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
H_UTF8 & H_UTF8::format(unsigned short const * fmt, ...){
  va_list va[2];

  va_start(va[0], fmt);
  va_start(va[1], fmt);
  wchar_t * wcs = _va_format_wcs((wchar_t const*) fmt, va);
  va_end(va[0]);
  va_end(va[1]);

  H_UTF8 tmp(wcs);
  delete [] wcs;

  *this = tmp;

  return *this;
}
#endif

H_UTF8 & H_UTF8::format(char const * fmt, ...){
  va_list va[2];

  va_start(va[0], fmt);
  va_start(va[1], fmt);
  char * mbs = _va_format_mbs(fmt, va);
  va_end(va[0]);
  va_end(va[1]);

  H_UTF8 tmp(mbs);
  delete [] mbs;

  *this = tmp;

  return *this;
}

H_UTF8 & H_UTF8::append(wchar_t const * txt){
  H_WCS orig_utf(*this);
  wchar_t const * orig_wcs = orig_utf.encodedText();
  if(!orig_wcs)
	  orig_wcs = L"";
  format(L"%ls%ls", orig_wcs, txt);
  return *this;
}


#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
H_UTF8 & H_UTF8::append(unsigned short const * txt){
  return append((wchar_t const*) txt);
}
#endif

H_UTF8 & H_UTF8::append(char const * txt){
  H_WCS wcs(txt);
  append(wcs.encodedText());
  return *this;
}

int H_UTF8::iterator::get_and_advance(utf8_char const * & text) const {
    int w1 = *text++;
    if(w1 < 0x7f){
        return w1;
    }

    int w2 = *text++;
    w2 &= 0x3f;
    if(w1 >= 0xc0 && w1 <= 0xdf){
        return ((0x1f & w1) << 6) | w2;
    }

    int w3 = *text++;
    w3 &= 0x3f;
    if(w1 >= 0xe0 && w1 <= 0xef){
        return ((0xf & w1) << 12) | (w2 << 6) | w3;
    }

    int w4 = *text++;
    w4 &= 0x3f;

    return ((0x7 & w1) << 18) | (w2 << 12) | (w3 << 6) | w4;
}

H_UTF8::iterator::iterator(utf8_char const * text) : m_text(text) {
}

H_UTF8::iterator H_UTF8::iterator::operator ++ () const {
    get_and_advance(m_text);
    return iterator(m_text);
}

H_UTF8::iterator H_UTF8::iterator::operator ++ (int) const {
    iterator retval(m_text);
    get_and_advance(m_text);
    return retval;
}

int H_UTF8::iterator::operator * () const {
    utf8_char const * tmp = m_text;
    return get_and_advance(tmp);
}

H_UTF8::iterator::operator bool () const {
    return false;
}

H_UTF8::iterator H_UTF8::begin() const {return iterator(m_utf8);}
H_UTF8::iterator H_UTF8::end() const {return iterator(m_utf8 + m_size);}

bool H_UTF8::operator == (H_UTF8 const & rhs) const
{
  if(length() != rhs.length())
    return false;

  return (memcmp(m_utf8, rhs.m_utf8, length() * sizeof(utf8_char)) == 0);
}

template <typename T>
int uri_char_count(T iter)
{
    int code_point = 0;
    int uri_chars_needed = 0;
    do {
        code_point = *iter++;
        if(code_point >= 0 && code_point <= 0x7f){
            if(code_point >= 'a' && code_point <= 'z' ||
               code_point >= 'A' && code_point <= 'Z' ||
               code_point >= '0' && code_point <= '9' ||
               code_point ==  0 )
                ++uri_chars_needed;
            else
                uri_chars_needed += 1 * 3;
        }
        else if(code_point >= 0x80 && code_point <= 0x7ff){
            uri_chars_needed += 2 * 3;
        }
        else if(code_point >= 0x800 && code_point <= 0xffff){
            uri_chars_needed += 3 * 3;
        }
        else if(code_point >= 0x10000 && code_point <= 0x10ffff){
            uri_chars_needed += 4 * 3;
        }
        else{
            return -1;
        }
    }while(code_point);

    return uri_chars_needed;
}

template <typename T>
void unicode_to_uri(utf8_char * uri, T iter)
{
    int code_point = 0, count, b[4];
    char hex_code[3];
    do {
        code_point = *iter++;
        count = 0;
        if(code_point >= 0 && code_point <= 0x7f){
            if(code_point >= 'a' && code_point <= 'z' ||
               code_point >= 'A' && code_point <= 'Z' ||
               code_point >= '0' && code_point <= '9' ||
               code_point ==  0 ){
               count = 0; /* just copy char over. */
               *uri++ = (utf8_char)code_point;
            }
            else {
                count = 1;
                b[0] = code_point;
            }
        }
        else if(code_point >= 0x80 && code_point <= 0x7ff){
            count = 2;
            b[0] = (utf8_char)(0xc0 | (code_point >> 6));
            b[1] = (utf8_char)(0x80 | (code_point & 0x3f));
        }
        else if(code_point >= 0x800 && code_point <= 0xffff){
            count = 3;
            b[0] = (utf8_char)(0xe0 | (code_point >> 12));
            b[1] = (utf8_char)(0x80 | (code_point >> 6 & 0x3f));
            b[2] = (utf8_char)(0x80 | (code_point & 0x3f));
        }
        else if(code_point >= 0x10000 && code_point <= 0x10ffff){
            count = 4;
            b[0] = (utf8_char)(0xf0 | (code_point >> 18));
            b[1] = (utf8_char)(0x80 | (code_point >> 12 & 0x3f));
            b[2] = (utf8_char)(0x80 | (code_point >> 6 & 0x3f));
            b[3] = (utf8_char)(0x80 | (code_point & 0x3f));
        }
        else{
            /* throw an exception. */(void)0;
        }

        for(int i = 0; i < count; ++i){
                sprintf(hex_code, "%02x", b[i]);
                *uri++ = '%';
                *uri++ = hex_code[0];
                *uri++ = hex_code[1];
        }

    }while(code_point);
    return;
}

H_URI::H_URI(H_URI const & rhs) : m_uri(0), m_size(0) {
    if(rhs.encodedText()){
        m_size = uri_char_count(rhs.begin());
        m_uri = new utf8_char[m_size];
        unicode_to_utf8(m_uri, rhs.begin());
    }
}

H_URI const & H_URI::operator = (H_URI const & rhs) {
    delete [] m_uri;
    m_uri = 0;
    m_size = 0;
    if(rhs.encodedText()){
        m_size = uri_char_count(rhs.begin());
        m_uri = new utf8_char[m_size];
        unicode_to_uri(m_uri, rhs.begin());
    }
    return *this;
}

H_URI::H_URI() : m_uri(0), m_size(0) {}

H_URI::H_URI(wchar_t const * text) : m_uri(0), m_size(0) {
    if(text){
        H_WCS u;
	u.encodedText(text);
        m_size = uri_char_count(u.begin());
        m_uri = new utf8_char[m_size];
        unicode_to_uri(m_uri, u.begin());
    }
}


#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
H_URI::H_URI(unsigned short const * text) : m_uri(0), m_size(0) {
    if(text){
        H_WCS u;
	u.encodedText((wchar_t const*) text);
        m_size = uri_char_count(u.begin());
        m_uri = new utf8_char[m_size];
        unicode_to_uri(m_uri, u.begin());
    }
}
#endif

/* This is locale specific multibyte string text. */
H_URI::H_URI(char const * text) : m_uri(0), m_size(0) {
    if(text){
        H_DEFAULT_LOCALE(m_size = mbstowcs(0, text, 0) +1);
        wchar_t * const tmp = new wchar_t[m_size];
        H_DEFAULT_LOCALE(mbstowcs(tmp, text, m_size));
        *this = H_URI(tmp);
        delete [] tmp;
    }
}

H_URI::H_URI(H_UTF32 const & unicode_text) : m_uri(0), m_size(0) {
    if(unicode_text.encodedText()){
        m_size = uri_char_count(unicode_text.begin());
        m_uri = new utf8_char[m_size];
        unicode_to_uri(m_uri, unicode_text.begin());
    }
}

H_URI::H_URI(H_UTF16 const & unicode_text) : m_uri(0), m_size(0) {
    if(unicode_text.encodedText()){
        m_size = uri_char_count(unicode_text.begin());
        m_uri = new utf8_char[m_size];
        unicode_to_uri(m_uri, unicode_text.begin());
    }
}

H_URI::H_URI(H_UTF8 const & unicode_text) : m_uri(0), m_size(0) {
    if(unicode_text.encodedText()){
        m_size = uri_char_count(unicode_text.begin());
        m_uri = new utf8_char[m_size];
        unicode_to_uri(m_uri, unicode_text.begin());
    }
}

H_URI::~H_URI() {delete [] m_uri;}

size_t H_URI::length() const {
	if(m_uri)
	  return (size_t)(uri_char_count(begin()) -1);
	return 0;
}

H_URI & H_URI::format(wchar_t const * fmt, ...){
  va_list va[2];

  va_start(va[0], fmt);
  va_start(va[1], fmt);
  wchar_t * wcs = _va_format_wcs(fmt, va);
  va_end(va[0]);
  va_end(va[1]);

  H_URI tmp(wcs);
  delete [] wcs;

  *this = tmp;

  return *this;
}


#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
H_URI & H_URI::format(unsigned short const * fmt, ...){
  va_list va[2];

  va_start(va[0], fmt);
  va_start(va[1], fmt);
  wchar_t * wcs = _va_format_wcs((wchar_t const*) fmt, va);
  va_end(va[0]);
  va_end(va[1]);

  H_URI tmp(wcs);
  delete [] wcs;

  *this = tmp;

  return *this;
}
#endif

H_URI & H_URI::format(char const * fmt, ...){
  va_list va[2];

  va_start(va[0], fmt);
  va_start(va[1], fmt);
  char * mbs = _va_format_mbs(fmt, va);
  va_end(va[0]);
  va_end(va[1]);

  H_URI tmp(mbs);
  delete [] mbs;

  *this = tmp;

  return *this;
}

H_URI & H_URI::append(wchar_t const * txt){
  H_WCS orig_utf(*this);
  wchar_t const * orig_wcs = orig_utf.encodedText();
  if(!orig_wcs)
	  orig_wcs = L"";
  format(L"%ls%ls", orig_wcs, txt);
  return *this;
}


#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
H_URI & H_URI::append(unsigned short const * txt){
  return append((wchar_t const*) txt);
}
#endif

H_URI & H_URI::append(char const * txt){
  H_WCS wcs(txt);
  append(wcs.encodedText());
  return *this;
}

int H_URI::iterator::get_and_advance(utf8_char const * & text) const {
    int w1 = *text++;
    if(w1 != '%'){
        return w1;
    }

    char hex_code[3];
    hex_code[2] = 0;
    hex_code[0] = *text++;
    hex_code[1] = *text++;
    sscanf(hex_code, "%x", &w1);

    if(w1 <= 0x7f)
        return w1;

    int w2;
    ++text; /* eat the % */
    hex_code[0] = *text++;
    hex_code[1] = *text++;
    sscanf(hex_code, "%x", &w2);

    w2 &= 0x3f;
    if(w1 >= 0xc0 && w1 <= 0xdf){
        return ((0x1f & w1) << 6) | w2;
    }

    int w3;
    ++text; /* eat the % */
    hex_code[0] = *text++;
    hex_code[1] = *text++;
    sscanf(hex_code, "%x", &w3);

    w3 &= 0x3f;
    if(w1 >= 0xe0 && w1 <= 0xef){
        return ((0xf & w1) << 12) | (w2 << 6) | w3;
    }

    int w4;
    ++text; /* eat the % */
    hex_code[0] = *text++;
    hex_code[1] = *text++;
    sscanf(hex_code, "%x", &w4);

    w4 &= 0x3f;

    return ((0x7 & w1) << 18) | (w2 << 12) | (w3 << 6) | w4;
}

H_URI::iterator::iterator(utf8_char const * text) : m_text(text) {
}

H_URI::iterator H_URI::iterator::operator ++ () const {
    get_and_advance(m_text);
    return iterator(m_text);
}

H_URI::iterator H_URI::iterator::operator ++ (int) const {
    iterator retval(m_text);
    get_and_advance(m_text);
    return retval;
}

int H_URI::iterator::operator * () const {
    utf8_char const * tmp = m_text;
    return get_and_advance(tmp);
}

H_URI::iterator::operator bool () const {
    return false;
}

H_URI::iterator H_URI::begin() const {return iterator(m_uri);}
H_URI::iterator H_URI::end() const {return iterator(m_uri + m_size);}

bool H_URI::operator == (H_URI const & rhs) const
{
  if(length() != rhs.length())
    return false;

  return (memcmp(m_uri, rhs.m_uri, length() * sizeof(utf8_char)) == 0);
}

enum DATA_TYPE {
    TYPE_INT,
    TYPE_UINT,

    TYPE_LONG,
    TYPE_ULONG,

    TYPE_DOUBLE,

    TYPE_POINTER,

    TYPE_MBS,
    TYPE_WCS,

    TYPE_PERCENT,
    TYPE_GET_CHAR_COUNT,

    TYPE_UNKNOWN
};

static
DATA_TYPE get_conversion_type(wchar_t const * code)
{
	if(code[1] == L'%'){
		return TYPE_PERCENT;
	}
    DATA_TYPE retval = TYPE_UNKNOWN;

    wchar_t const * specifier = code + wcslen(code) -1;
    wchar_t const * modifier = specifier -1;

    switch(*specifier){
        case L'c':
        case L'C':
        case L'd':
        case L'i':
        case L'x':
        case L'X':
	    case L'o':
            retval = TYPE_INT;
        break;
        case L'e':
        case L'E':
        case L'f':
        case L'g':
        case L'G':
            retval = TYPE_DOUBLE;
        break;
        case L'n':
            retval = TYPE_GET_CHAR_COUNT;
        break;
        case L'p':
            retval = TYPE_POINTER;
        break;
        case L's':
            retval = TYPE_MBS;
        if(*modifier == L'l')
            retval = TYPE_WCS;
        break;
        case L'u':
            retval = TYPE_UINT;
        break;
    }

    return retval;
}

static
void get_conversion_code(wchar_t * code, wchar_t const * buffer)
{
    while(*buffer == L'%')
        *code++ = *buffer++;

    /* flags */
    if(*buffer == L'-' || *buffer == L'+' || *buffer == L'#' || *buffer == L'0' || *buffer == L' ')
        *code++ = *buffer++;

    /* width */
    while(*buffer >= L'0' && *buffer <= L'9')
        *code++ = *buffer++;

    if(*buffer == L'.')
        *code++ = *buffer++;

    /* percision */
    while(*buffer >= L'0' && *buffer <= L'9')
        *code++ = *buffer++;

    /* modifier and specifier */
    while((*buffer >= L'A' && *buffer <= L'Z') || 
			(*buffer >= L'a' && *buffer <= L'z'))
			*code++ = *buffer++;

    *code++ = L'\0';
}


#ifdef HPUX_SYSTEM
/* This is only a partial implmentation!  It will only do one conversion code
   at a time and it doesn't copy any other characters!  This is needed as
   HPUX 11 doesn't have wcs format functions.  */
static void
swprintf(wchar_t * const output, int const size, wchar_t const * wcsfmt, ...){
	char buffer[50] = {""};
	char fmt[20] = {""};
	wcstombs(fmt, wcsfmt, wcslen(wcsfmt) +1);
	va_list va;
	va_start(va, wcsfmt);
	DATA_TYPE type = get_conversion_type(wcsfmt);
	switch(type){
	case TYPE_INT:{
		int arg_int = (int)va_arg(va, int);
		sprintf(buffer, fmt, arg_int);
		mbstowcs(output, buffer, size);
	}break;
	case TYPE_UINT:{
		unsigned int arg_int = (unsigned int)va_arg(va, unsigned int);
		sprintf(buffer, fmt, arg_int);
		mbstowcs(output, buffer, size);
	}break;
	case TYPE_DOUBLE:{
		double arg_double = (double)va_arg(va, double);
		sprintf(buffer, fmt, arg_double);
		mbstowcs(output, buffer, size);
	}break;
	case TYPE_POINTER:{
		void * arg_ptr = (void*)va_arg(va, void*);
		sprintf(buffer, fmt, arg_ptr);
		mbstowcs(output, buffer, size);
	}break;
	}
	va_end(va);
}
#endif

static
wchar_t * _va_format_wcs(wchar_t const * const wformat, va_list va[])
{
    wchar_t * retval = 0;
    for(int i = 0; i < 2; ++i){
        size_t count = 0;
        wchar_t const * tmp = wformat;
        while(*tmp != L'\0'){
            if(*tmp == L'%'){
                wchar_t conversion_code[30] = {L""};
                get_conversion_code(conversion_code, tmp);
                DATA_TYPE type = get_conversion_type(conversion_code);
				if(type != TYPE_PERCENT)
	                tmp += wcslen(conversion_code);
				else do {
					++tmp;
				} while(*tmp == L'%');
                wchar_t buffer[50] = {L""};
                switch(type){
                    case TYPE_INT:{
                        int arg_int = (int)va_arg(va[i], int);
                        swprintf(buffer, 50, conversion_code, arg_int);
                        if(retval){
                            wcscpy(&retval[count], buffer);
                        }
                        count += wcslen(buffer);
                    }break;
                    case TYPE_UINT:{
                        unsigned int arg_int = (unsigned int)va_arg(va[i], unsigned int);
                        swprintf(buffer, 50, conversion_code, arg_int);
                        if(retval){
                            wcscpy(&retval[count], buffer);
                        }
                        count += wcslen(buffer);
                    }break;
                    case TYPE_MBS:{
						wchar_t nullstr[] = {L"(null)"};
                        char * arg_mbs = (char*)va_arg(va[i], char*);
						H_WCS wcs(arg_mbs);
                        if(retval){
							if(wcs.encodedText())
	                            wcscpy(&retval[count], wcs.encodedText());
							else
								wcscpy(&retval[count], nullstr);
                        }
                        count += wcs.length();
                    }break;
                    case TYPE_WCS:{
						wchar_t nullstr[] = {L"(null)"};
                        wchar_t * arg_wcs = (wchar_t*)va_arg(va[i], wchar_t*);
                        if(retval){
							if(arg_wcs)
	                            wcscpy(&retval[count], arg_wcs);
							else
								wcscpy(&retval[count], nullstr);
                        }
						if(arg_wcs)
	                        count += wcslen(arg_wcs);
						else
							count += wcslen(nullstr);
                    }break;
                    case TYPE_DOUBLE:{
                        double arg_float = (double)va_arg(va[i], double);
                        swprintf(buffer, 50, conversion_code, arg_float);
                        if(retval){
                            wcscpy(&retval[count], buffer);
                        }
                        count += wcslen(buffer);
                    }break;
                    case TYPE_PERCENT:{
                        if(retval){
                            wcscpy(&retval[count], L"%");
                        }
                        ++count;
                    }break;
                    case TYPE_POINTER:{
                        void * ptr = (void*)va_arg(va[i], void*);
                        swprintf(buffer, 50, conversion_code, ptr);
                        if(retval){
                            wcscpy(&retval[count], buffer);
                        }
                        count += wcslen(buffer);
                    }break;
                    case TYPE_GET_CHAR_COUNT:{
                        int * return_count = (int*)va_arg(va[i], int*);
                        *return_count = (int)count;
                    }break;
                    default:
                        if(retval)
                            wcscpy(&retval[count], conversion_code);
                        count += wcslen(buffer);
                        break;
                }
            }
            else if(*tmp == L'\\'){
                ++tmp;
                switch(*tmp){
                    case L'a':{
                        if(retval)
                            retval[count] = L'\a';
                        ++count;
                    }break;
                    case L'b':{
                        if(retval)
                            retval[count] = L'\b';
                        ++count;
                    }break;
                    case L'f':{
                        if(retval)
                            retval[count] = L'\f';
                        ++count;
                    }break;
                    case L'n':{
                        if(retval)
                            retval[count] = L'\n';
                        ++count;
                    }break;
                    case L'r':{
                        if(retval)
                            retval[count] = L'\r';
                        ++count;
                    }break;
                    case L't':{
                        if(retval)
                            retval[count] = L'\t';
                        ++count;
                    }break;
                    case L'v':{
                        if(retval)
                            retval[count] = L'\v';
                        ++count;
                    }break;
                    case L'\'':{
                        if(retval)
                            retval[count] = L'\'';
                        ++count;
                    }break;
                    case L'\\':{
                        if(retval)
                            retval[count] = L'\\';
                        ++count;
                    }break;
                    default:{
                    }break;
                }

            }
            else{
                if(retval)
                    retval[count] = *tmp;
                ++count;
                ++tmp;
            }
        }
        if(!retval){
            retval = new wchar_t[count+1];
            count = 0;
        }
        retval[count] = L'\0';
    }
    return retval;
}

static
char * _va_format_mbs(char const * const format, va_list va[])
{
	char const conversion_failed[] = {"INVALID CONVERSION!!!"};
    char * retval = 0;

	size_t format_count = 0;
    H_DEFAULT_LOCALE(format_count = mbstowcs(0, format, 0));
    wchar_t * const wcs_format = new wchar_t[format_count +1];
    H_DEFAULT_LOCALE(mbstowcs(wcs_format, format, format_count +1));
    wchar_t const * const wcs = _va_format_wcs(wcs_format, va);

	int const wcs_count = (int)wcslen(wcs);
    delete [] wcs_format;
    if(wcs_count >= 0){
        int mbs_count = 0;
		H_DEFAULT_LOCALE(mbs_count = (int)wcstombs(0, wcs, 0));
        if(mbs_count >= 0){
			retval = new char[mbs_count +1];
			H_DEFAULT_LOCALE(wcstombs(retval, wcs, mbs_count +1));
		}
		else{
            retval = new char[sizeof(conversion_failed)];
            strcpy(retval, conversion_failed);
        }
    }
    else {
		retval = new char[sizeof(conversion_failed)];
		strcpy(retval, conversion_failed);
    }
    delete [] wcs;
    return retval;
}



#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
HUtilityUnicodeStr::HUtilityUnicodeStr(const unsigned short *p16) : m_wcs((wchar_t const*) p16) {}
#endif

HUtilityUnicodeStr::HUtilityUnicodeStr(const wchar_t *p16) : m_wcs(p16) {}

HUtilityUnicodeStr::operator const wchar_t * (void) const
{
	return m_wcs.encodedText();
}


#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
HUtilityUnicodeStr::operator const unsigned short * (void) const
{
	return (unsigned short const*)m_wcs.encodedText();
}
#endif

HUtilityUnicodeStr::HUtilityUnicodeStr(const char *p8) : m_wcs(p8) {}

HUtilityUnicodeStr::~HUtilityUnicodeStr() {}

HUtilityAsciiStr::HUtilityAsciiStr(const char *p8) : m_sz(0)
{
	if(p8){
		size_t const size = strlen(p8) +1;
		m_sz = new char[size];
		strcpy(m_sz, p8);
	}
}

HUtilityAsciiStr::operator const char * (void) const 
{
	return m_sz;
}


#if defined(_MSC_VER)  &&  defined(_NATIVE_WCHAR_T_DEFINED)
HUtilityAsciiStr::HUtilityAsciiStr(const unsigned short *p16) : m_sz(0)
{
	if(p16)
	{
		// calculate string length
		char const * const old_loc = setlocale(LC_CTYPE, "");
		size_t const size = wcstombs(0, (wchar_t const*)p16, 0) +1;
		m_sz = new char[size];
		wcstombs(m_sz, (wchar_t const*)p16, size);
		setlocale(LC_CTYPE, old_loc);
	}
}
#endif

HUtilityAsciiStr::HUtilityAsciiStr(const wchar_t *p16) : m_sz(0)
{
	if(p16)
	{
		// calculate string length
		char const * const old_loc = setlocale(LC_CTYPE, "");
		size_t const size = wcstombs(0, p16, 0) +1;
		m_sz = new char[size];
		wcstombs(m_sz, p16, size);
		setlocale(LC_CTYPE, old_loc);
	}
}

HUtilityAsciiStr::~HUtilityAsciiStr(void) {
	delete [] m_sz;
}

int const H_FORMAT_TEXT::h_vsnprintf(char * const buf, size_t const n, char const * const format, va_list valist)
{
#ifdef _MSC_VER
	return _vsnprintf(buf, n, format, valist);
#else
	return vsnprintf(buf, n, format, valist);
#endif
}

H_FORMAT_TEXT::H_FORMAT_TEXT(char const * const format, ...) : m_str(0) {
	int result = 0;
	int count = (int)strlen(format) + 256;

	do{
		if(m_str){
			free(m_str);
			count *= 2;
		}
		
		m_str = (char*)malloc(count);

		va_list va1;
		va_start(va1, format);	
		
		H_DEFAULT_LOCALE( (result = H_FORMAT_TEXT::h_vsnprintf(m_str, count, format, va1)) );
		
		va_end(va1);
		
	}while(result < 0 || result > count);
}

H_FORMAT_TEXT & H_FORMAT_TEXT::Append(char const * format, ...) {
	int count = (int)strlen(format) + 256;
	int result = 0;

	H_FORMAT_TEXT mod_format("%s%s", m_str, format);

	do{
		if(m_str){
			free(m_str);
			count *= 2;
		}
		
		m_str = (char*)malloc(count);

		va_list va1;
		va_start(va1, format);	
		
		H_DEFAULT_LOCALE( (result = H_FORMAT_TEXT::h_vsnprintf(m_str, count, mod_format, va1)) );
		
		va_end(va1);
		
	}while(result < 0 || result > count);

	return *this;
}

H_FORMAT_TEXT::~H_FORMAT_TEXT() {
	free(m_str);
}

H_FORMAT_TEXT::operator char * () const {
	return m_str;
}

H_FORMAT_TEXT const & H_FORMAT_TEXT::operator=(H_FORMAT_TEXT const & rhs) {
	free(m_str);
	m_str = strdup(rhs.m_str);
	return *this;
}

H_FORMAT_TEXT::H_FORMAT_TEXT(H_FORMAT_TEXT const & rhs) : m_str(strdup(rhs.m_str)){}

H_FORMAT_TEXT::H_FORMAT_TEXT() : m_str(strdup("")) {}


#ifndef _MSC_VER
FILE * wfopen(wchar_t const * wcfilename, wchar_t const * wcmode) {
    FILE * retval = 0;
    if(wcfilename && wcmode){
        size_t size = 0;
	H_DEFAULT_LOCALE(size = wcstombs(0, wcfilename, 0));
        if(size > 0){
            ++size; /* for the null */
            char * const mbfilename = (char * const)malloc( size * sizeof(char) );
            if(mbfilename){
	      int result = 0;
	      H_DEFAULT_LOCALE(result = wcstombs(mbfilename, wcfilename, size));
                if(result != -1){
                    char mbmode[10] = {""};
                    H_DEFAULT_LOCALE(wcstombs(mbmode, wcmode, 10));
                    retval = fopen(mbfilename, mbmode);
                }
                free(mbfilename);
            }
        }
    }

    return retval;
}

int wremove(wchar_t const * wcfilename) {
	int retval = -1;
	if(wcfilename){
		size_t size = 0;
		H_DEFAULT_LOCALE(size = wcstombs(0, wcfilename, 0));
		if(size > 0){
			++size; /* for the null */
			char * const mbfilename = (char * const)malloc( size * sizeof(char) );
			if(mbfilename){
				int result = 0;
				H_DEFAULT_LOCALE(result = wcstombs(mbfilename, wcfilename, size));
				if(result != -1){
					retval = remove(mbfilename);
				}
				free(mbfilename);
			}
		}
	}

	return retval;
}
#endif

