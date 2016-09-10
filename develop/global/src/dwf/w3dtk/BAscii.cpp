
// Copyright (c) 2000 by Tech Soft 3D, LLC.
// The information contained herein is confidential and proprietary to
// Tech Soft 3D, LLC., and considered a trade secret as defined under
// civil and criminal statutes.  Tech Soft 3D shall pursue its civil
// and criminal remedies in the event of unauthorized use or misappropriation
// of its trade secrets.  Use of this information by anyone other than
// authorized employees of Tech Soft 3D, LLC. is granted only under a
// written non-disclosure agreement, expressly prescribing the scope and
// manner of such use.
//
// $Header: //DWF/Development/Components/Internal/DWF Toolkit/v7.7/develop/global/src/dwf/w3dtk/BAscii.cpp#2 $
//


#include "dwf/w3dtk/BStream.h"
#include "dwf/w3dtk/BOpcodeHandler.h"
#include "dwf/w3dtk/BInternal.h"

#include "dwf/w3dtk/utility/vlist.h"
#include <ctype.h>
#include <limits.h>

#define ASSERT(x)
#define MVO_BUFFER_SIZE 4096

static bool strni_equal (
				register	char const		*a,
				register	char const		*b,
				register	int				max)
{

	do {
		if (--max < 0) return true;
		if (tolower(*a) != tolower(*b)) return false;
		++a;
	}
	until (*b++ == '\0');

	return true;
}

TK_Status BBaseOpcodeHandler::ReadAsciiLine(BStreamFileToolkit & tk, unsigned int* readSize) alter
{
#ifndef BSTREAM_DISABLE_ASCII
	TK_Status status = TK_Normal;
	char cur_char = '\0';

	if (readSize)
		*readSize = 0;

	while (1) {	// LF character
		if ((status = GetData (tk, cur_char)) != TK_Normal)
			return status;

		if (cur_char == 13)
		{
			// we are expecting this sequence "\x0D\x0A"
			if ((status = GetData (tk, cur_char)) != TK_Normal)
				return status;
			ASSERT(cur_char == 10);
		}
		if (cur_char == 10)
		{
			if (readSize)
				*readSize = m_ascii_length;
			m_ascii_length = 0;
			break;
		}

		if (m_ascii_length+1 >= m_ascii_size)
		{
			// we are full, need to reallocate
			char * old_buffer = m_ascii_buffer;
			m_ascii_size += 4096;
			m_ascii_buffer = new char[m_ascii_size];
			strcpy(m_ascii_buffer, old_buffer);
			delete[] old_buffer;
		}

		m_ascii_buffer[m_ascii_length++] = cur_char;
		m_ascii_buffer[m_ascii_length] = '\0';
	}

	return status;
#else
	UNREFERENCED (readSize);
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status BBaseOpcodeHandler::ReadAsciiWord(BStreamFileToolkit & tk, unsigned int* readSize) alter
{
#ifndef BSTREAM_DISABLE_ASCII
	TK_Status status = TK_Normal;
	char cur_char = '\0';

	if (readSize)
		*readSize = 0;


	while (1) {
		if ((status = GetData (tk, cur_char)) != TK_Normal)
			return status;

		// We will expect to get 10 only if we return from here after reading 13 and status TK_Pending
		if (cur_char == ' ' || cur_char == 10) {
			if (readSize)
				*readSize = m_ascii_length;
			m_ascii_length = 0;
			break;
		}
		else if (cur_char == 13)
		{
			// we are expecting this sequence "\x0D\x0A"
			if ((status = GetData (tk, cur_char)) != TK_Normal)
				return status;

			if (readSize)
				*readSize = m_ascii_length;
			m_ascii_length = 0;
			ASSERT(cur_char == 10);
			break;
		}

		if (m_ascii_length+1 >= m_ascii_size)
		{
			// we are full, need to reallocate
			char * old_buffer = m_ascii_buffer;
			m_ascii_size += 4096;
			m_ascii_buffer = new char[m_ascii_size];
			if (old_buffer)
			{
				strcpy(m_ascii_buffer, old_buffer);
				delete[] old_buffer;
			}
		}

		m_ascii_buffer[m_ascii_length++] = cur_char;
		m_ascii_buffer[m_ascii_length] = '\0';
	}

	return status;
#else
	UNREFERENCED (readSize);
	return tk.Error(stream_disable_ascii);
#endif 
}

// skips Carriage return, line feed and tab characters
TK_Status BBaseOpcodeHandler:: SkipNewlineAndTabs(BStreamFileToolkit & tk, unsigned int* readSize) alter
{
#ifndef BSTREAM_DISABLE_ASCII
	TK_Status status = TK_Normal;
	unsigned char cur_char = '\0';

	LookatData(tk, cur_char);
	while (cur_char == 13 || cur_char == 10 || cur_char == '\0' || cur_char == '\t') { // read CR,LF
		status = GetData (tk, cur_char);
		if( status != TK_Normal )
			break;
		LookatData(tk, cur_char);
	}

	return status;
	UNREFERENCED (readSize);
#else
	UNREFERENCED (readSize);
	return tk.Error(stream_disable_ascii);
#endif 
}

bool BBaseOpcodeHandler::RemoveQuotes(char* string)
{
	char * is = string;
	char * os = string;

	if (*is == '\"')
		is++;

	while (*is != '\"' && *is != '\0' /* && *is != ' ' */)
		*os++ = *is++;

	*os = '\0';

	return true;
}

// returns the string with it's angular brackets removed. 
// NOTE: If the string inside the brackets has a space or /, then the search for '>'
// will be terminated and the read string will be returned.
bool BBaseOpcodeHandler::RemoveAngularBrackets(char* string)
{
	char * is = string;
	char * os = string;

	if (*is == '<')
		is++;

	while (*is != '>' && *is != ' ' && *is != '/' && *is != '\0')
		*os++ = *is++;

	*os = '\0';

	return true;

}

// returns the string with it's angular brackets removed. 
static bool RemoveOnlyAngularBrackets(char* string)
{
	char * is = string;
	char * os = string;

	if (*is == '<')
		is++;

	while (*is != '>' && *is != '\0')
		*os++ = *is++;

	*os = '\0';

	return true;

}

TK_Status BBaseOpcodeHandler::ReadEndOpcode(BStreamFileToolkit & tk) 
{
	TK_Status status = TK_Normal;
	unsigned int read_size = 0;
	status = ReadAsciiLine(tk, &read_size); 
	return status;
}

TK_Status BBaseOpcodeHandler::Read_Referenced_Segment(BStreamFileToolkit & tk, int &i_progress)
{
	TK_Status status = TK_Normal;

	switch (m_ascii_stage) {

		case 0: {
				if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
					return status;
				m_ascii_stage++;
		}	nobreak;

		case 1: {
				if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
					 return status;
				if (!(RemoveAngularBrackets(m_ascii_buffer)))
						status = TK_Error;		
				if ((strni_equal("Has_Condition_Length_Flag", m_ascii_buffer, 28) != true) &&
				   (strni_equal("Length", m_ascii_buffer, 7) != true)) {
					   return tk.Error("TK_Referenced_Segment::expected Has_Condition_Length_Flag: or Length");
				}
			   m_ascii_stage++;
		}	nobreak;

		case 2:{
				if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
					return status;
				 m_ascii_stage++;   
			}	nobreak;
			
		case 3:{
				if ((status =  ReadAsciiWord(tk, 0)) != TK_Normal)
					return status;

				if (!(RemoveQuotes(m_ascii_buffer)))
					status = TK_Error;

				if (sscanf(m_ascii_buffer," %d", &i_progress) != 1)
					return TK_Error;
				 m_ascii_stage++;   
			}	nobreak;
		
		 case 4:{
			if ((status = ReadAsciiLine(tk, 0)) != TK_Normal)
				return status;
				m_ascii_stage = 0;  
			}	break;
			   
		default:
            return tk.Error();			
		
	 }
		return status;
 }

TK_Status   BBaseOpcodeHandler::PutStartXMLTag (BStreamFileToolkit & tk, char const *tag) alter 
{
#ifndef BSTREAM_DISABLE_ASCII
	TK_Status status = TK_Normal;
	int tag_len = (int)strlen(tag);
	char * buffer1 = new char[tag_len+512];
	int	n_tabs = tk.GetTabs();

	buffer1[0] = '\0';
	for (int t=0; t<n_tabs; t++)
		strcat(buffer1,"\t");
	strcat(buffer1, "<");
	strcat(buffer1, tag);
	strcat(buffer1,">\x0D\x0A");
	status = PutData(tk, buffer1, (int)strlen(buffer1));	// no null term
	delete[] buffer1;

	return status;
#else
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status   BBaseOpcodeHandler::PutEndXMLTag (BStreamFileToolkit & tk, char const *tag) alter 
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status = TK_Normal;
	int tag_len = (int)strlen(tag);
	char * buffer1 = new char[tag_len+512];
	int	n_tabs = tk.GetTabs();

	buffer1[0] = '\0';
	for (int t=0; t<n_tabs; t++)
		strcat(buffer1,"\t");

	strcat(buffer1, "</");
	strcat(buffer1, tag);
	strcat(buffer1, ">\x0D\x0A");
	status = PutData(tk, buffer1, (int)strlen(buffer1));	// no null term
	delete[] buffer1;

	return status;
#else
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

/* Puts n characters from buffer b into the toolkit buffer as ASCII; returns result of function call. */
TK_Status   BBaseOpcodeHandler::PutAsciiData (BStreamFileToolkit & tk, char const *tag, char const * b, int n) alter 
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status = TK_Normal;
	int tag_len = (int)strlen(tag);
	char * buffer1 = new char[tag_len + n*sizeof(char) + 512];
	int n_tabs = tk.GetTabs();

	buffer1[0] = '\0';
	for (int t=0; t<n_tabs; t++)
		strcat(buffer1,"\t");

	strcat(buffer1, "<");
	strcat(buffer1, tag);
	strcat(buffer1, ">\x20\"");
	if ( b )		//guard against NULL ptr, if no segment name given
		strncat(buffer1, b, n);
	strcat(buffer1, "\" </");
	strcat(buffer1, tag);
	strcat(buffer1,">\x0D\x0A");

	status = PutData(tk, buffer1, (int)strlen(buffer1));	// no null term
	delete[] buffer1;

	return status;
#else
	UNREFERENCED (b);
	UNREFERENCED (n);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

/* Puts n short integers from buffer s into the toolkit buffer as ASCII; returns result of function call.*/
TK_Status   BBaseOpcodeHandler::PutAsciiData (BStreamFileToolkit & tk, char const *tag, short const * s, int n) alter 
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status = TK_Normal;
	int tag_len = (int)strlen(tag);

	// a maximum short is 32767 which is 5 chars plus we need 1 for blank space
	// that means potentially we need atleast 6 times n.
	char * buffer = new char[tag_len + 6*n*sizeof(char) + 512];
	char * p = buffer;
	int np = 0;
	
	int	n_tabs = tk.GetTabs();
	for (int t=0; t<n_tabs; t++)
	{
		*p = '\t';
		p++;
	}
	*p = '<';						p++;
	np = sprintf(p, "%s", tag);		p+=np;
	np = sprintf(p, "%s", "> \"");	p+=np;

	for (int i=0; i<n; i++)
	{
		np = sprintf(p,"%hd ", s[i]);
		p+=np;
	}
	p--;	// don't want the space at the end

	np = sprintf(p, "\" </");		p+=np;
	np = sprintf(p, "%s", tag);		p+=np;
	np = sprintf(p, ">\x0D\x0A");	p+=np;

    // cast for x64 build warning
	status = PutData(tk, buffer, (int)(p-buffer));	//no null term
	delete[] buffer;

	return status;
#else
	UNREFERENCED (n);
	UNREFERENCED (s);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

/* Puts n integers from buffer i into the toolkit buffer as ASCII; returns result of function call.  */      
TK_Status   BBaseOpcodeHandler::PutAsciiData (BStreamFileToolkit & tk, char const *tag, int const * i, int n) alter 
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status = TK_Normal;
	int tag_len = (int)strlen(tag);

	// a maximum int is 2147483647 which is 10 chars plus we need 1 for blank space
	// that means potentially we need atleast 11 times n.
	char * buffer  = new char[tag_len + 11*n*sizeof(char) + 512];
	char * p = buffer;
	int np = 0;

	int	n_tabs = tk.GetTabs();
	for (int t=0; t<n_tabs; t++)
	{
		*p = '\t';
		p++;
	}
	*p = '<';						p++;
	np = sprintf(p, "%s", tag);		p+=np;
	np = sprintf(p, "%s", "> \"");	p+=np;

	for (int d=0; d<n; d++)
	{
		np = sprintf(p,"%d ", i[d]);
		p+=np;
	}
	p--;	// don't want the space at the end

	np = sprintf(p, "\" </");		p+=np;
	np = sprintf(p, "%s", tag);		p+=np;
	np = sprintf(p, ">\x0D\x0A");	p+=np;

	status = PutData(tk, buffer, (int)(p-buffer));	//no null term
	delete[] buffer;

	return status;
#else
	UNREFERENCED (i);
	UNREFERENCED (n);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 

}

/* Puts n floats from buffer f into the toolkit buffer as ASCII; returns result of function call. */             
TK_Status   BBaseOpcodeHandler::PutAsciiData (BStreamFileToolkit & tk, char const *tag, float const * f, int n) alter 
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status = TK_Normal;
	int tag_len = (int)strlen(tag);
	char * buffer = new char[tag_len + 14*n*sizeof(char) + 512];
	char * p = buffer;
	int np = 0;

	int	n_tabs = tk.GetTabs();
	for (int t=0; t<n_tabs; t++)
	{
		*p = '\t';
		p++;
	}
	*p = '<';						p++;
	np = sprintf(p, "%s", tag);		p+=np;
	np = sprintf(p, "%s", "> \"");	p+=np;

	for (int i=0; i<n; i++)
	{
		np = sprintf(p,"%.6g ", f[i]);
		p+=np;
	}
	p--;	// don't want the space at the end

	np = sprintf(p, "\" </");		p+=np;
	np = sprintf(p, "%s", tag);		p+=np;
	np = sprintf(p, ">\x0D\x0A");	p+=np;

	status = PutData(tk, buffer, (int)(p-buffer));	//no null term
	delete[] buffer;

	return status;
#else
	UNREFERENCED (f);
	UNREFERENCED (n);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

/* Puts n unsigned characters from buffer b into the toolkit buffer as ASCII; returns result of function call. */
TK_Status   BBaseOpcodeHandler::PutAsciiData (BStreamFileToolkit & tk, char const *tag, unsigned char const * b, int n) alter 
{
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status status = TK_Normal;
	int tag_len = (int)strlen(tag);

    switch (m_ascii_stage) {
        case 0: {

			int	n_tabs = tk.GetTabs();
			char * tag_buffer = new char[tag_len+MVO_BUFFER_SIZE];

			tag_buffer[0] = '\0';
			for (int t=0; t<n_tabs; t++)
				strcat(tag_buffer,"\t");

			strcat(tag_buffer, "<");
			strcat(tag_buffer, tag);
			strcat(tag_buffer, ">\x20");
			if ((status = PutData(tk, tag_buffer, (int)strlen(tag_buffer))) != TK_Normal)
			{
				delete[] tag_buffer;
				return status;
			}

			delete[] tag_buffer;
			
            m_ascii_stage++;
        }   nobreak;

        case 1: {

			char * buffer = new char[3*n + 2];
			char * p = buffer;
	
			p[0] = '"';
			p++;

			for (int i = 0; i < n; i++) {
				sprintf (p, "%02X ", *b++);
				p+=3;
			}
		
			p--;
			p[0] = '"';
			p++;

			status = PutData(tk, buffer, (int)(p-buffer));	//no null term

			delete[] buffer;

			m_ascii_stage++;
        }   nobreak;

		case 2: {

			char * tag_buffer = new char[tag_len+MVO_BUFFER_SIZE];
			sprintf(tag_buffer," </%s", tag);
			strcat(tag_buffer, ">\x0D\x0A");
			if ((status = PutData(tk, tag_buffer, (int)strlen(tag_buffer))) != TK_Normal)
			{
				delete[] tag_buffer;
				return status;
			}
			delete[] tag_buffer;
			m_ascii_stage = 0;

		}   break;

        default:
            return tk.Error();			
    }

	return status;
#else
	UNREFERENCED (b);
	UNREFERENCED (n);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 

}

/* Puts n unsigned short integers from buffer s into the toolkit buffer as ASCII; returns result of function call. */
TK_Status   BBaseOpcodeHandler::PutAsciiData (BStreamFileToolkit & tk, char const *tag, unsigned short const * s, int n) alter 
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status;
	int tag_len = (int)strlen(tag);
	char * buffer = new char[tag_len + 11*n*sizeof(char) + 512];
	char * p = buffer;
	int np = 0;

	int	n_tabs = tk.GetTabs();
	for (int t=0; t<n_tabs; t++)
	{
		*p = '\t';
		p++;
	}
	*p = '<';						p++;
	np = sprintf(p, "%s", tag);		p+=np;
	np = sprintf(p, "%s", "> \"");	p+=np;

	for (int i=0; i<n; i++)
	{
		np = sprintf(p,"%hu ", s[i]);
		p+=np;
	}
	p--;	// don't want the space at the end

	np = sprintf(p, "\" </");		p+=np;
	np = sprintf(p, "%s", tag);		p+=np;
	np = sprintf(p, ">\x0D\x0A");	p+=np;

	status = PutData(tk, buffer, (int)(p-buffer));	//no null term
	delete[] buffer;

	return status;
#else
	UNREFERENCED (n);
	UNREFERENCED (s);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}
/* Puts n unsigned integers from buffer i into the toolkit buffer as ASCII; returns result of function call. */
TK_Status   BBaseOpcodeHandler::PutAsciiData (BStreamFileToolkit & tk, char const *tag, unsigned int const * i, int n) alter
{ 
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status;	
	int tag_len = (int)strlen(tag);
	char * buffer = new char[tag_len + 11*n*sizeof(char) + 512];
	char * p = buffer;
	int np = 0;

	int	n_tabs = tk.GetTabs();
	for (int t=0; t<n_tabs; t++)
	{
		*p = '\t';
		p++;
	}
	*p = '<';						p++;
	np = sprintf(p, "%s", tag);		p+=np;
	np = sprintf(p, "%s", "> \"");	p+=np;

	for (int d=0; d<n; d++)
	{
		np = sprintf(p,"%u ", i[d]);
		p+=np;
	}
	p--;	// don't want the space at the end

	np = sprintf(p, "\" </");		p+=np;
	np = sprintf(p, "%s", tag);		p+=np;
	np = sprintf(p, ">\x0D\x0A");	p+=np;

	status = PutData(tk, buffer, (int)(p-buffer));	//no null term
	delete[] buffer;
	
	return status;
#else
	UNREFERENCED (i);
	UNREFERENCED (n);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

/* Puts a single character c into the toolkit buffer as ASCII; returns result of function call.*/
TK_Status   BBaseOpcodeHandler::PutAsciiData (BStreamFileToolkit & tk, char const *tag, char const & c) alter 
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status;
	char buffer1[MVO_BUFFER_SIZE];
	char buffer2[256];
    int n_tabs = tk.GetTabs();

	buffer1[0] = '\0';
	for (int t=0; t<n_tabs; t++)
		strcat(buffer1,"\t");

	strcat(buffer1, "<");
	strcat(buffer1, tag);
	strcat(buffer1, ">\x20\"");

	sprintf(buffer2,"%c", c);

	strcat(	buffer1, buffer2);	
	strcat(buffer1, "\" </");
	strcat(buffer1, tag);
	strcat(buffer1,">\x0D\x0A");

	status = PutData(tk, buffer1, (int)strlen(buffer1));
	
	return status;
#else
	UNREFERENCED (c);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}
/* Puts a single short integer s into the toolkit buffer as ASCII; returns result of function call.*/
TK_Status   BBaseOpcodeHandler::PutAsciiData (BStreamFileToolkit & tk, char const *tag, short const & s) alter 
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status;
	char buffer1[MVO_BUFFER_SIZE];
	char buffer2[256];
			
	int	n_tabs = tk.GetTabs();

	buffer1[0] = '\0';
	for (int t=0; t<n_tabs; t++)
		strcat(buffer1,"\t");

	strcat(buffer1, "<");
	strcat(buffer1, tag);
	strcat(buffer1, ">\x20\"");

	sprintf(buffer2,"%hi", s);

	strcat(	buffer1, buffer2);	
	strcat(buffer1, "\" </");
	strcat(buffer1, tag);
	strcat(buffer1,">\x0D\x0A");

	status = PutData(tk, buffer1, (int)strlen(buffer1));	//no null term
	return status;
#else
	UNREFERENCED (s);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

/* Puts a single integer i into the toolkit buffer as ASCII; returns result of function call.*/
TK_Status   BBaseOpcodeHandler::PutAsciiData (BStreamFileToolkit & tk, char const *tag, int const & i) alter 
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status;
	char buffer1[MVO_BUFFER_SIZE];
	char buffer2[256];
			
	int	n_tabs = tk.GetTabs();

	buffer1[0] = '\0';
	for (int t=0; t<n_tabs; t++)
		strcat(buffer1,"\t");

	strcat(buffer1, "<");
	strcat(buffer1, tag);
	strcat(buffer1, ">\x20\"");

	sprintf(buffer2,"%d", i);

	strcat(	buffer1, buffer2);	
	strcat(buffer1, "\" </");
	strcat(buffer1, tag);
	strcat(buffer1,">\x0D\x0A");

	status = PutData(tk, buffer1, (int)strlen(buffer1));	//no null term
	return status;
#else
	UNREFERENCED (i);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status   BBaseOpcodeHandler::PutAsciiFlag (BStreamFileToolkit & tk, char const *tag, int const & i) alter
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status;
	char buffer1[MVO_BUFFER_SIZE];
	char buffer2[256];
	int	n_tabs = tk.GetTabs();

	buffer1[0] = '\0';
	for (int t=0; t<n_tabs; t++)
		strcat(buffer1,"\t");

	strcat(buffer1, "<");
	strcat(buffer1, tag);
	strcat(buffer1, ">\x20\"");

	sprintf (buffer2, "0x%08X", i);

	strcat(	buffer1, buffer2);	
	strcat(buffer1, "\" </");
	strcat(buffer1, tag);
	strcat(buffer1,">\x0D\x0A");

	status = PutData(tk, buffer1, (int)strlen(buffer1));	//no null term
	return status;
#else
	UNREFERENCED (i);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status   BBaseOpcodeHandler::PutAsciiMask (BStreamFileToolkit & tk, char const *tag, int const & i) alter
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status;
	int	n_tabs = tk.GetTabs();

	char buffer1[MVO_BUFFER_SIZE];
	char buffer2[256];
			
	buffer1[0] = '\0';
	for (int t=0; t<n_tabs; t++)
		strcat(buffer1,"\t");

	strcat(buffer1, "<");
	strcat(buffer1, tag);
	strcat(buffer1, ">\x20\"");

	sprintf (buffer2, "0x%08X", i);

	strcat(	buffer1, buffer2);	
	strcat(buffer1, "\" </");
	strcat(buffer1, tag);
	strcat(buffer1,">\x0D\x0A");

	status = PutData(tk, buffer1, (int)strlen(buffer1));	//no null term

	return status;
#else
	UNREFERENCED (i);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status   BBaseOpcodeHandler::PutAsciiHex (BStreamFileToolkit & tk, char const *tag, int const & i) alter
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status;
	int	n_tabs = tk.GetTabs();

	char buffer1[MVO_BUFFER_SIZE];
	char buffer2[256];
			
	buffer1[0] = '\0';
	for (int t=0; t<n_tabs; t++)
		strcat(buffer1,"\t");

	strcat(buffer1, "<");
	strcat(buffer1, tag);
	strcat(buffer1, ">\x20\"");

	sprintf (buffer2, "0x%08X", i);

	strcat(	buffer1, buffer2);	
	strcat(buffer1, "\" </");
	strcat(buffer1, tag);
	strcat(buffer1,">\x0D\x0A");

	status = PutData(tk, buffer1, (int)strlen(buffer1));	//no null term

	return status;
#else
	UNREFERENCED (i);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

/* Puts a single unsigned character b into the toolkit buffer as ASCII; returns result of function call. */
TK_Status   BBaseOpcodeHandler::PutAsciiData (BStreamFileToolkit & tk, char const *tag, unsigned char const & b) alter 
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status;
	int	n_tabs = tk.GetTabs();

	char buffer1[MVO_BUFFER_SIZE];
	char buffer2[256];
			

	buffer1[0] = '\0';
	for (int t=0; t<n_tabs; t++)
		strcat(buffer1,"\t");

	strcat(buffer1, "<");
	strcat(buffer1, tag);
	strcat(buffer1, ">\x20\"");

	sprintf(buffer2, "%c", b);

	strcat(	buffer1, buffer2);	
	strcat(buffer1, "\" </");
	strcat(buffer1, tag);
	strcat(buffer1,">\x0D\x0A");

	status = PutData(tk, buffer1, (int)strlen(buffer1));	//no null term
	
	return status;
#else
	UNREFERENCED (b);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

/* Puts a single unsigned short s into the toolkit buffer as ASCII; returns result of function call. */
TK_Status   BBaseOpcodeHandler::PutAsciiData (BStreamFileToolkit & tk, char const *tag, unsigned short const & s) alter 
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status;
    int	n_tabs = tk.GetTabs();

	char buffer1[MVO_BUFFER_SIZE];
	char buffer2[256];
			
	buffer1[0] = '\0';
	for (int t=0; t<n_tabs; t++)
		strcat(buffer1,"\t");

	strcat(buffer1, "<");
	strcat(buffer1, tag);
	strcat(buffer1, ">\x20\"");

	sprintf(buffer2,"%hu", s);

	strcat(	buffer1, buffer2);	
	strcat(buffer1, "\" </");
	strcat(buffer1, tag);
	strcat(buffer1,">\x0D\x0A");

	status = PutData(tk, buffer1, (int)strlen(buffer1));	//no null term
	return status;
#else
	UNREFERENCED (s);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

/* Puts a single unsigned integer i into the toolkit buffer as ASCII; returns result of function call. */
TK_Status   BBaseOpcodeHandler::PutAsciiData (BStreamFileToolkit & tk, char const *tag, unsigned int const & i) alter 
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status;
    int	n_tabs = tk.GetTabs();

	char buffer1[MVO_BUFFER_SIZE];
	char buffer2[256];

	buffer1[0] = '\0';
	for (int t=0; t<n_tabs; t++)
		strcat(buffer1,"\t");

	strcat(buffer1, "<");
	strcat(buffer1, tag);
	strcat(buffer1, ">\x20\"");

	sprintf(buffer2,"%u", i);

	strcat(	buffer1, buffer2);	
	strcat(buffer1, "\" </");
	strcat(buffer1, tag);
	strcat(buffer1,">\x0D\x0A");

	status = PutData(tk, buffer1, (int)strlen(buffer1));	//no null term
	return status;
#else
	UNREFERENCED (i);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

/* Puts a single float f into the toolkit buffer as ASCII; returns result of function call. */
TK_Status   BBaseOpcodeHandler::PutAsciiData (BStreamFileToolkit & tk, char const *tag, float const & f) alter  
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status;
	int	n_tabs = tk.GetTabs();

	char buffer1[MVO_BUFFER_SIZE];
	char buffer2[256];

	buffer1[0] = '\0';
	for (int t=0; t<n_tabs; t++)
		strcat(buffer1,"\t");

	strcat(buffer1, "<");
	strcat(buffer1, tag);
	strcat(buffer1, ">\x20\"");

	sprintf(buffer2,"%f", f);

	strcat(	buffer1, buffer2);	
	strcat(buffer1, "\" </");
	strcat(buffer1, tag);
	strcat(buffer1,">\x0D\x0A");

	status = PutData(tk, buffer1, (int)strlen(buffer1));	//no null term
	return status;
#else
	UNREFERENCED (f);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status BBaseOpcodeHandler::GetAsciiData(BStreamFileToolkit & tk, const char * tag,
											  float * rFloats, unsigned int n) alter
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	char			err_msg[MVO_BUFFER_SIZE];

	switch (m_ascii_stage) 
	{
	case 0: 
		{
			if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
				return status;

				m_ascii_stage++;
		}   
		nobreak;

	case 1: 
		{	
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			if (!(RemoveAngularBrackets(m_ascii_buffer)))
				status = TK_Error;		
			
			if (streq(tag, m_ascii_buffer) != true) {
				sprintf(err_msg, "expected %s not found", tag);
				return tk.Error(err_msg);
			}

			m_ascii_stage++;
		}   
		nobreak;

	case 2: 
		{
		    float temp;
			for(; m_ascii_progress <((int)n); m_ascii_progress++)
			{
				if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
					return status;
				if (!(RemoveQuotes(m_ascii_buffer)))
					status = TK_Error;
				if (sscanf(m_ascii_buffer," %f", &temp) != 1)
					return TK_Error;
				rFloats[m_ascii_progress] = temp;
			}

			m_ascii_stage++;
		}   nobreak;

	case 3: 
		{
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			m_ascii_stage = 0;
		}  	break;


	default:
		return tk.Error();
	}
	m_ascii_progress = 0;
	return status;
#else
	UNREFERENCED (n);
	UNREFERENCED (rFloats);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status BBaseOpcodeHandler::GetAsciiData(BStreamFileToolkit & tk, const char * tag,
											  int * rInts, unsigned int n) alter
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	char			            err_msg[MVO_BUFFER_SIZE];

	switch (m_ascii_stage) 
	{
	case 0: 
		{
			if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
				return status;

				m_ascii_stage++;
		}   
		nobreak;

	case 1: 
		{	
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			if (!(RemoveAngularBrackets(m_ascii_buffer)))
				status = TK_Error;

			if (streq(tag, m_ascii_buffer) != true) {
				sprintf(err_msg, "expected %s not found", tag);
				return tk.Error(err_msg);
			}

			m_ascii_stage++;
		}   
		nobreak;

	case 2: 
		{
			int temp;
			for(; m_ascii_progress <((int)n); m_ascii_progress++)
			{
				if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
					return status;
				if (!(RemoveQuotes(m_ascii_buffer)))
					status = TK_Error;
				if (sscanf(m_ascii_buffer," %d", &temp) != 1)
					return TK_Error;
				rInts[m_ascii_progress] =  temp;
		
			}

			m_ascii_stage++;
		}   nobreak;

	case 3: 
		{
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			m_ascii_stage = 0;
		}  	break;


	default:
		return tk.Error();
	}
	m_ascii_progress = 0;
	return status;
#else
	UNREFERENCED (n);
	UNREFERENCED (rInts);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 

}


TK_Status BBaseOpcodeHandler::GetAsciiData(BStreamFileToolkit & tk, const char * tag,
											  short * rShorts, unsigned int n) alter
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	char			            err_msg[MVO_BUFFER_SIZE];

	switch (m_ascii_stage) 
	{
	case 0: 
		{
			if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
				return status;

				m_ascii_stage++;
		}   
		nobreak;

	case 1: 
		{	
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			if (!(RemoveAngularBrackets(m_ascii_buffer)))
				status = TK_Error;

			if (streq(tag, m_ascii_buffer) != true) {
				sprintf(err_msg, "expected %s not found", tag);
				return tk.Error(err_msg);
			}

			m_ascii_stage++;
		}   
		nobreak;

	case 2: 
		{
			short temp;
			for(; m_ascii_progress <((int)n); m_ascii_progress++)
			{
				if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
					return status;
				if (!(RemoveQuotes(m_ascii_buffer)))
					status = TK_Error;
				if (sscanf(m_ascii_buffer," %hd", &temp) != 1)
					return TK_Error;
				rShorts[m_ascii_progress] = temp;
				
			}
			m_ascii_stage++;
		}   nobreak;

	case 3: 
		{
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			m_ascii_stage = 0;
		}  	break;


	default:
		return tk.Error();
	}
	m_ascii_progress = 0;
	return status;
#else
	UNREFERENCED (n);
	UNREFERENCED (rShorts);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
	
}

TK_Status BBaseOpcodeHandler::GetAsciiData(BStreamFileToolkit & tk, const char * tag,
										   unsigned short * rShorts, unsigned int n) alter
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	char			            err_msg[MVO_BUFFER_SIZE];

	switch (m_ascii_stage) 
	{
	case 0: 
		{
			if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
				return status;

				m_ascii_stage++;
		}   
		nobreak;

	case 1: 
		{	
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			if (!(RemoveAngularBrackets(m_ascii_buffer)))
				status = TK_Error;

			if (streq(tag, m_ascii_buffer) != true) {
				sprintf(err_msg, "expected %s not found", tag);
				return tk.Error(err_msg);
			}

			m_ascii_stage++;
		}   
		nobreak;

	case 2: 
		{
			unsigned short temp = 0;
			for(; m_ascii_progress <((int)n); m_ascii_progress++)
				{
				if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
						return status;
				if (!(RemoveQuotes(m_ascii_buffer)))
					status = TK_Error;
				if (sscanf(m_ascii_buffer," %hu", &temp) != 1)
					return TK_Error;
				rShorts[m_ascii_progress] = temp;

			}
			m_ascii_stage++;
		}   nobreak;

	case 3: 
		{
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			m_ascii_stage = 0;
		}  	break;


	default:
		return tk.Error();
	}
	m_ascii_progress = 0;
	return status;
#else
	UNREFERENCED (n);
	UNREFERENCED (rShorts);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status BBaseOpcodeHandler::GetAsciiData(BStreamFileToolkit & tk, const char * tag,
											  char * rValues, unsigned int n) alter
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	char			            err_msg[MVO_BUFFER_SIZE];

	switch (m_ascii_stage) 
	{
	case 0: 
		{
			if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
				return status;

				m_ascii_stage++;
		}   
		nobreak;

	case 1: 
		{	
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			if (!(RemoveAngularBrackets(m_ascii_buffer)))
				status = TK_Error;

			if (streq(tag, m_ascii_buffer) != true) {
				sprintf(err_msg, "expected %s not found", tag);
				return tk.Error(err_msg);
			}

			m_ascii_stage++;
		}   
		nobreak;

	case 2: 
		{
			char quote;
			if ((status = GetData(tk, quote)) != TK_Normal)
				return status;
			ASSERT( quote == '"');

			m_ascii_stage++;
		} nobreak;

	case 3:
		{
			if ((status = GetData(tk, rValues, n)) != TK_Normal)
				return status;

			m_ascii_stage++;
		} nobreak;

	case 4:
		{
			char quote_and_space[2];
			if ((status = GetData(tk, quote_and_space, 2)) != TK_Normal)
				return status;

			if (quote_and_space[0] != '"' || quote_and_space[1] != ' ') {
				strcpy(err_msg, "expected \"  not found");
				return tk.Error(err_msg);
			}

			m_ascii_stage++;
		} nobreak;

	case 5:
		{
			// read end tag
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			if (!(RemoveOnlyAngularBrackets(m_ascii_buffer)))
				status = TK_Error;

			if (streq(tag, m_ascii_buffer+1) != true) {
				sprintf(err_msg, "expected %s not found", tag);
				return tk.Error(err_msg);
			}

/*			unsigned int read_size = 0;
			
			if ((status = ReadAsciiLine(tk, &read_size)) != TK_Normal)
				return status;

			if (!(RemoveQuotes(m_ascii_buffer)))
				status = TK_Error;

			char * p = m_ascii_buffer;
			char temp;
			for(int i = 0;  i <((int)n); i++)
			{
				if (sscanf(p,"%c", &temp) != 1)
					return TK_Error;
				rValues[i] = temp;
				p++;
			}
*/
			m_ascii_stage = 0;
		}  	break;


	default:
		return tk.Error();
	}
	return status;
#else
	UNREFERENCED (n);
	UNREFERENCED (rValues);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 

}


TK_Status BBaseOpcodeHandler::GetAsciiData(BStreamFileToolkit & tk, const char * tag,
											  unsigned char * rValues, unsigned int n) alter

{
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	char			            err_msg[MVO_BUFFER_SIZE];

    switch (m_ascii_stage) 
	{
        case 0: 
		{
			if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
				return status;

				m_ascii_stage++;
		}   
		nobreak;

		case 1: 
			{	

			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			if (!(RemoveAngularBrackets(m_ascii_buffer)))
				status = TK_Error;
		
			if (streq(tag, m_ascii_buffer) != true) {
				sprintf(err_msg, "expected %s not found", tag);
				return tk.Error(err_msg);
			}

			m_ascii_stage++;
        }   
		nobreak;

        case 2: 
		{
			unsigned int read_size = 0;
			int temp;
			for (; m_ascii_progress < (int)n; m_ascii_progress++)
			{
				if ((status = ReadAsciiWord(tk, &read_size)) != TK_Normal)
					return status;

				if (!(RemoveQuotes(m_ascii_buffer)))
					status = TK_Error;

				if (sscanf(m_ascii_buffer,"%d", &temp) != 1)
					return TK_Error;
				rValues[m_ascii_progress] = (unsigned char)temp;
			}

			m_ascii_stage++;
		}   nobreak;

		case 3: 
			{
				if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
					return status;

				m_ascii_stage = 0;
			}  	break;


        default:
            return tk.Error();
    }

	m_ascii_progress = 0;
    return status;
#else
	UNREFERENCED (n);
	UNREFERENCED (rValues);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status BBaseOpcodeHandler::GetAsciiImageData(BStreamFileToolkit & tk, const char * tag,
											  unsigned char * rValues, unsigned int n) alter

{
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	char			            err_msg[MVO_BUFFER_SIZE];

    switch (m_ascii_stage) 
	{
        case 0: 
		{
			if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
				return status;

				m_ascii_stage++;
		}   
		nobreak;

		case 1: 
			{	
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			if (!(RemoveAngularBrackets(m_ascii_buffer)))
				status = TK_Error;
		
			if (streq(tag, m_ascii_buffer) != true) {
				sprintf(err_msg, "expected %s not found", tag);
				return tk.Error(err_msg);
			}

			m_ascii_stage++;
        }   
		nobreak;

        case 2: 
		{
			char format[8];
			strcpy(format, "%02X");

			unsigned int read_size = 0;
			
			int temp;
			for (; m_ascii_progress < (int)n; m_ascii_progress++)
			{
				if ((status = ReadAsciiWord(tk, &read_size)) != TK_Normal)
					return status;

				if (!(RemoveQuotes(m_ascii_buffer)))
					status = TK_Error;

				if (sscanf(m_ascii_buffer, format, &temp) != 1)
					return TK_Error;
				rValues[m_ascii_progress] = (unsigned char)temp;
			
			}

			m_ascii_stage++;
		}   nobreak;

		case 3: 
			{
				if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
					return status;

				m_ascii_stage = 0;
			}  	break;


        default:
            return tk.Error();
    }

	m_ascii_progress = 0;
    return status;
#else
	UNREFERENCED (n);
	UNREFERENCED (rValues);
	UNREFERENCED (tag);
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status BBaseOpcodeHandler::GetAsciiHex(BStreamFileToolkit & tk, const char * tag, unsigned char &value) alter
{
#ifndef BSTREAM_DISABLE_ASCII
	
   TK_Status       status = TK_Normal;
	char			            err_msg[MVO_BUFFER_SIZE];

	switch (m_ascii_stage) 
	{
	case 0: 
		{
			if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
				return status;

				m_ascii_stage++;
		}   
		nobreak;

	case 1: 
		{				
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			if (!(RemoveAngularBrackets(m_ascii_buffer)))
				status = TK_Error;

			if (streq(tag, m_ascii_buffer) != true) {
				sprintf(err_msg, "expected %s not found", tag);
				return tk.Error(err_msg);
			}

			m_ascii_stage++;
		}   
		nobreak;

	case 2: 
		{
			unsigned int read_size = 0;
			if ((status = ReadAsciiWord(tk, &read_size)) != TK_Normal)
				return status;

			char format[8];
			strcpy(format, "0x%08X");

			if (!(RemoveQuotes(m_ascii_buffer)))
				status = TK_Error;

			int int_value;
			if (sscanf(m_ascii_buffer, format, &int_value) != 1)
				status = TK_Error;

			value = (unsigned char) int_value;

			m_ascii_stage++;
		}   nobreak;

	case 3: 
		{
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			m_ascii_stage = 0;
		}  	break;



	default:
		return tk.Error();
	}

	return status;
#else
	UNREFERENCED (tag);
	UNREFERENCED (value);
	return tk.Error(stream_disable_ascii);
#endif 
	
}

TK_Status BBaseOpcodeHandler::GetAsciiHex(BStreamFileToolkit & tk, const char * tag, int &value) alter
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	char			            err_msg[MVO_BUFFER_SIZE];

	switch (m_ascii_stage) 
	{
	case 0: 
		{
			if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
				return status;

				m_ascii_stage++;
		}   
		nobreak;

	case 1: 
		{				

			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			if (!(RemoveAngularBrackets(m_ascii_buffer)))
				status = TK_Error;

			if (streq(tag, m_ascii_buffer) != true) {
				sprintf(err_msg, "expected %s not found", tag);
				return tk.Error(err_msg);
			}

			m_ascii_stage++;
		}   
		nobreak;

	case 2: 
		{
			unsigned int read_size = 0;
			
			if ((status = ReadAsciiWord(tk, &read_size)) != TK_Normal)
				return status;

			char format[8];
			strcpy(format, "0x%08X");

			if (!(RemoveQuotes(m_ascii_buffer)))
				status = TK_Error;
			int int_value;
			if (sscanf(m_ascii_buffer, format, &int_value) != 1)
				status = TK_Error;

			value = int_value;

			m_ascii_stage++;
		}   nobreak;

	case 3: 
		{
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			m_ascii_stage = 0;
		}  	break;


	default:
		return tk.Error();
	}

	return status;
#else
	UNREFERENCED (tag);
	UNREFERENCED (value);
	return tk.Error(stream_disable_ascii);
#endif 

}
TK_Status BBaseOpcodeHandler::GetAsciiHex(BStreamFileToolkit & tk, const char * tag,  char &value) alter
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	char			            err_msg[MVO_BUFFER_SIZE];

	switch (m_ascii_stage) 
	{
	case 0: 
		{
			if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
				return status;

				m_ascii_stage++;
		}   
		nobreak;

	case 1: 
		{				
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			if (!(RemoveAngularBrackets(m_ascii_buffer)))
				status = TK_Error;

			if (streq(tag, m_ascii_buffer) != true) {
				sprintf(err_msg, "expected %s not found", tag);
				return tk.Error(err_msg);
			}

			m_ascii_stage++;
		}   
		nobreak;

	case 2: 
		{
			unsigned int read_size = 0;
			if ((status = ReadAsciiWord(tk, &read_size)) != TK_Normal)
				return status;

			char format[8];
			strcpy(format, "0x%08X");

			if (!(RemoveQuotes(m_ascii_buffer)))
				status = TK_Error;

			int int_value;

			if (sscanf(m_ascii_buffer, format, &int_value) != 1)
				status = TK_Error;

			value = (char) int_value;

			m_ascii_stage++;
		}   nobreak;

	case 3: 
		{
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			m_ascii_stage = 0;
		}  	break;

	default:
		return tk.Error();
	}

	return status;
#else
	UNREFERENCED (tag);
	UNREFERENCED (value);
	return tk.Error(stream_disable_ascii);
#endif 

}
TK_Status BBaseOpcodeHandler::GetAsciiHex(BStreamFileToolkit & tk, const char * tag, unsigned short &value) alter
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	char			            err_msg[MVO_BUFFER_SIZE];

	switch (m_ascii_stage) 
	{
	case 0: 
		{
			if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
				return status;

				m_ascii_stage++;
		}   
		nobreak;

	case 1: 
		{
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			if (!(RemoveAngularBrackets(m_ascii_buffer)))
				status = TK_Error;

			if (streq(tag, m_ascii_buffer) != true) {
				sprintf(err_msg, "expected %s not found", tag);
				return tk.Error(err_msg);
			}

			m_ascii_stage++;
		}   
		nobreak;

	case 2: 
		{
			unsigned int read_size = 0;
			if ((status = ReadAsciiWord(tk, &read_size)) != TK_Normal)
				return status;

			char format[8];
			strcpy(format, "0x%08X");

			if (!(RemoveQuotes(m_ascii_buffer)))
				status = TK_Error;

			int int_value;

			if (sscanf(m_ascii_buffer, format, &int_value) != 1)
				status = TK_Error;

			value = (unsigned short) int_value;

			m_ascii_stage++;
		}   nobreak;

	case 3: 
		{
			if ((status = ReadAsciiWord(tk, 0)) != TK_Normal)
				return status;

			m_ascii_stage = 0;
		}  	break;

	default:
		return tk.Error();
	}

	return status;
#else
	UNREFERENCED (tag);
	UNREFERENCED (value);
	return tk.Error(stream_disable_ascii);
#endif 

}
TK_Status BBaseOpcodeHandler::GetAsciiData(BStreamFileToolkit & tk, int *rInts, unsigned int n) alter
{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status status = TK_Normal;

	unsigned int readSize = 0;
	for(; m_ascii_progress <((int)n); m_ascii_progress++)
	{
		char format[8];
		strcpy(format, "%d");
	switch(m_ascii_stage)
	{
	case 0: 
		{
			if ((status =  SkipNewlineAndTabs(tk, 0)) != TK_Normal)
				return status;

				m_ascii_stage++;
		}   
		nobreak;

	case 1: 
		{				
		if ((status = ReadAsciiWord(tk, &readSize)) != TK_Normal)
			return status;
		
		if (!(RemoveQuotes(m_ascii_buffer)))
			status = TK_Error;

		if ((readSize >= 2) && (m_ascii_buffer[0] == '0') && (m_ascii_buffer[1] == 'x'))
			strcpy(format, "0x%08X");

		if (sscanf(m_ascii_buffer, format, &rInts[m_ascii_progress]) != 1)
			status = TK_Error;
			
			m_ascii_stage = 0;
		
		}	break;
		
	 default:
		return tk.Error();	
		}
	}
	m_ascii_progress = 0;

	return status;
#else
	
	UNREFERENCED (n);
	UNREFERENCED (rInts);
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status BBaseOpcodeHandler::GetAsciiData(BStreamFileToolkit & tk, const char * tag, int& value)alter
{
	return GetAsciiData(tk, tag, &value, 1);
}

TK_Status BBaseOpcodeHandler::GetAsciiData(BStreamFileToolkit & tk, const char * tag, float& value)alter
{
	return GetAsciiData(tk, tag, &value, 1);
}

TK_Status BBaseOpcodeHandler::GetAsciiData(BStreamFileToolkit & tk, const char * tag, unsigned char& value)alter
{
	return GetAsciiData(tk, tag, &value, 1);
}

TK_Status BBaseOpcodeHandler::GetAsciiData(BStreamFileToolkit & tk, const char * tag, char& value)alter
{
	return GetAsciiData(tk, tag, &value, 1);
}

TK_Status BBaseOpcodeHandler::GetAsciiData(BStreamFileToolkit & tk, const char * tag, short& value)
{
	return GetAsciiData(tk, tag, &value, 1);
}


TK_Status BBaseOpcodeHandler::GetAsciiData(BStreamFileToolkit & tk, const char * tag, unsigned short& value)
{
	return GetAsciiData(tk, tag, &value, 1);
}


TK_Status TK_Default::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    return tk.Error();
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_Header::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	char            root_node[MVO_BUFFER_SIZE];

    switch (m_stage) {
		case 0:	{
				strcpy(root_node,"<HSX>\x0D\x0A");
				PutData(tk,root_node,7);

				m_stage++;
				} nobreak;
        case 1: {
            if (m_current_object == null) {
                char        buffer[32];

                sprintf (buffer, "; HSF V%d.%02d ",
                         tk.GetTargetVersion() / 100,
                         tk.GetTargetVersion() % 100);
                m_current_object = new TK_Comment (buffer);
            }

            if ((status = m_current_object->Write (tk)) != TK_Normal)
                return status;
            delete m_current_object;
            m_current_object = null;

            m_stage++;
        }   nobreak;

        case 2: {
            if (m_current_object == null) {
                m_current_object = new TK_File_Info;
                m_current_object->Interpret (tk, -1);
            }

            if ((status = m_current_object->Write (tk)) != TK_Normal)
                return status;
            delete m_current_object;
            m_current_object = null;
            tk.m_header_comment_seen = true;

            m_stage = -1;
        }   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Header::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	return tk.Error();
#else
	return tk.Error(stream_disable_ascii);
#endif 
}



////////////////////////////////////////

TK_Status TK_File_Info::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: { 
			
			PutTab t(&tk);
					
            if ((status = PutAsciiFlag (tk,"Flags", m_flags)) != TK_Normal)
                return status;

            if (tk.GetLogging()) {
                char        buffer[32];

                sprintf (buffer, "%08X", m_flags);
                tk.LogEntry (buffer);
            }

            m_stage++;
        }   nobreak;

		case 2: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_File_Info::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Flags", m_flags)) != TK_Normal)
                return status;

            if (tk.GetVersion() < 105)      // meaning of Disctionary bit reversed at 1.05
                m_flags ^= TK_Generate_Dictionary;

            if (tk.GetLogging()) {
                char        buffer[32];

                sprintf (buffer, "%08X", m_flags);
                tk.LogEntry (buffer);
            }

            m_stage++;
        }   nobreak;
		
		case 1: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
				return status;

				m_stage = -1;
	}   break;
        
		default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Comment::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {  
        case 0: {
            if ((status = PutAsciiOpcode (tk,1,false,false)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
		   PutTab t(&tk);
		   if ((status = PutData (tk, m_comment, m_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;
		
		case 2: {
			if ((status = PutAsciiOpcode (tk,1,true,false)) != TK_Normal)
				return status;
			m_stage++;
		}   nobreak;

        case 3: {
			// Not required in Ascii because we do that anyway in PutAsciiData
			//if (m_length == 0 || m_comment[m_length-1] != '\n')
            //    if ((status = PutData (tk,'\n')) != TK_Normal)
            //        return status;

            if (tk.GetLogging()) {
                char        buffer[256];

                buffer[0] = '\0';
                strncat (buffer, m_comment, 64);
                tk.LogEntry (buffer);
            }

            m_stage = -1;
        }   break;

        default:
            return tk.Error();
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Comment::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
   
	TK_Status       status = TK_Normal;
    unsigned char   byte;

    if (m_stage != 0)
        return tk.Error();

    while (1) {
		if ((status = GetData (tk, byte)) != TK_Normal)
            return status;

        if (m_progress == m_length) {
            char *      old_buffer = m_comment;
            m_comment = new char [m_length + 32];
            memcpy (m_comment, old_buffer, m_length);
            m_length += 32;         // use m_length here to store current allocation size
            delete [] old_buffer;
        }

        if (byte == '\n') {
            m_comment[m_progress++] = '\0';
            break;
        }

        m_comment[m_progress++] = byte;
    }

    m_length = m_progress;

    if (tk.GetLogging()) {
        char        buffer[256];

        buffer[0] = '\0';
        strncat (buffer, m_comment, 64);
        tk.LogEntry (buffer);
    }

    m_stage = -1;

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Comment::ExecuteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	if (!tk.m_header_comment_seen) {
        tk.m_header_comment_seen = true;

		if (strncmp (m_comment, "; HSF V", 7))
            return tk.Error ("file does not appear to be HSF format");

        char const *    cp = &m_comment[7];
        int             version = 0;

        while (cp) {
            if ('0' <= *cp && *cp <= '9')
                version = 10 * version  +  (int)(*cp++ - '0');
            else if (*cp == '.')
                ++cp;       // ignore '.'
            else if (*cp == ' ')
                cp = null;  // all done
            else
                return tk.Error ("error reading version number");   // unexpected character
        }

        tk.m_file_version = version;    // save it

        if (!tk.GetReadFlags(TK_Ignore_Version)) {
            if (version > TK_File_Format_Version) {
                // generate error message here...
                return TK_Version;
            }
        }
    }
    else {
        // normal comment, we don't do anything with it
    }
	
    return TK_Normal;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_Terminator::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status;
	char            root_node[1024];
	PutTab t(&tk);

	status = PutAsciiOpcode (tk, 0, false,true);

    if (status == TK_Normal && Opcode() == TKE_Pause)
        tk.RecordPause (tk.GeneratedSoFar());

    if (tk.GetLogging())
        tk.LogEntry ("\n");
	
	if (status == TK_Normal && Opcode() != TKE_Pause && m_terminate_file == true)
	{
		strcpy(root_node,"</HSX>\x0D\x0A");
		status = PutData(tk,root_node,7);
	}
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Terminator::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;

	if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

	// UNREFERENCED(tk);
    if (Opcode() == TKE_Termination)
        return TK_Complete;
    else
        return TK_Pause;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Compression::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk, 0, false,true)) != TK_Normal)
                return status;
            m_stage = -1;
        }   break;

        default:
            return tk.Error();
    }
	
    return TK_Normal;
#else
return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Compression::ExecuteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
	UNREFERENCED(tk);
    TK_Status       status = TK_Normal;
	return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_Compression::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
	TK_Status       status = TK_Normal;
	UNREFERENCED(tk);
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Open_Segment::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            // need code here to handle length > 255 ?
            if ((status = PutAsciiData (tk, "Length" , m_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if(m_length > 0)
			{
				PutTab t(&tk);
				if ((status = PutAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
					return status;
			}

            if (tk.GetLogging() &&tk.GetLoggingOptions (TK_Logging_Segment_Names)) {
                LogDebug (tk, "(");
                LogDebug (tk, m_string);
                LogDebug (tk, ")");
            }

           m_stage++;
        }   nobreak;

		case 3: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Open_Segment::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
   
	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Length", m_length)) != TK_Normal)
                return status;
            set_segment (m_length);    // allocate space & set length
            m_stage++;
        }   nobreak;

        case 1: {
			if(m_length > 0)
			{
				if ((status = GetAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
					return status;
			}

            if (tk.GetLogging() &&tk.GetLoggingOptions (TK_Logging_Segment_Names)) {
                LogDebug (tk, "(");
                LogDebug (tk, m_string);
                LogDebug (tk, ")");
            }

            increase_nesting(tk);
              m_stage++;
        }   nobreak;
		
		case 2: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Close_Segment::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

	switch(m_stage)
	{
	case 0:
		{
			if ((status = PutAsciiOpcode (tk, 1, false,true)) != TK_Normal)
				return status;
			m_stage++;
		}	nobreak;
	
	case 1:
		{
			if (Tagging (tk))
			{
				if((status = Tag (tk, 0)) != TK_Normal)
					return status;
			}
			m_stage = -1;
		}	break;

	default:
		return tk.Error();
	}

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Close_Segment::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	decrease_nesting(tk);
    return TK_Normal;       // no associated data...
#else
return tk.Error(stream_disable_ascii);
#endif 
}


////////////////////////////////////////

TK_Status TK_Reopen_Segment::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Index", m_index)) != TK_Normal)
                return status;

            if (tk.GetLogging() &&tk.GetLoggingOptions (TK_Logging_Tagging)) {
                char        buffer[32];

                sprintf (buffer, "[%d]", m_index);
                LogDebug (tk, buffer);
            }

            m_stage++;
        }   nobreak;

		case 2: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Reopen_Segment::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Index", m_index)) != TK_Normal)
                return status;

            if (tk.GetLogging() &&tk.GetLoggingOptions (TK_Logging_Tagging)) {
                char        buffer[32];

                sprintf (buffer, "[%d]", m_index);
                LogDebug (tk, buffer);
            }

            increase_nesting(tk);
             
			m_stage++;
        }   nobreak;
		
		case 1: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

////////////////////////////////////////

TK_Status TK_Renumber::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk, 0)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            int         value = (int)m_key;
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Value", value)) != TK_Normal)
                return status;

            m_stage++;
        }   nobreak;

		case 2: {
			if ((status = PutAsciiOpcode (tk,0,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Renumber::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	switch (m_stage) {
	case 0:
		{
			if ((status = GetAsciiData (tk,"Value", m_int)) != TK_Normal)
				return status;
			m_key = m_int;

		m_stage++;
	}   nobreak;

	case 1:
		{
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
				return status;
			
				m_stage = -1;
	}   break;

	default:
		return tk.Error();
	}

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

////////////////////////////////////////

TK_Status TK_Geometry_Attributes::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t(&tk);

    switch (m_stage) {
        case 0: {
            
            if ((status = PutAsciiOpcode (tk, 0, false,true)) != TK_Normal)
                return status;
			m_stage = -1;
		}	break;

        default:
            return tk.Error();
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Geometry_Attributes::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	 TK_Status       status = TK_Normal;
    if (m_stage != 0)
        return tk.Error();

    m_stage = -1;

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status	TK_Close_Geometry_Attributes::WriteAscii(BStreamFileToolkit & tk) alter{
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t(&tk);

    switch (m_stage) {
        case 0: {
            
            if ((status = PutAsciiOpcode (tk, 0, false,true)) != TK_Normal)
                return status;
			m_stage = -1;
		}	break;

        default:
            return tk.Error();
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status	TK_Close_Geometry_Attributes::ReadAscii(BStreamFileToolkit & tk) alter{
TK_Status       status = TK_Normal;
tk;
return status;
}

TK_Status TK_Tag::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk, 0, false,true)) != TK_Normal)
                return status;

            LogDebug (tk);
      
			m_stage = -1;
		}	break;

        default:
            return tk.Error();
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Tag::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
   
	TK_Status       status = TK_Normal;
	UNREFERENCED(tk);
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Dictionary::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t1(&tk);

            m_format = (unsigned char)tk.GetDictionaryFormat();
            if (m_format == 3) {
                if (tk.GetTargetVersion() < 1155 || tk.GetWriteFlags (TK_Disable_Global_Compression))
                    m_format = 2;
            }
            if (tk.GetTargetVersion() < 710)
                m_format = 0;

            // A little odd... if the format is (at least) 3, we require (at least) version 1155.
            // Over time, there might be checks for higher numbers...
            if (m_format > 2)
            {
                _W3DTK_REQUIRE_VERSION( 1155 );
            }

            if ((status = PutAsciiData (tk,"Format" ,(int)m_format)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {

			PutTab t1(&tk);

            if (m_format >= 2) {
                unsigned short          pauses = (unsigned short)tk.GetPauseCount();

                if ((status = PutAsciiData (tk,"Pauses", pauses)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t1(&tk);
            if (m_format >= 2) {
                if ((status = PutAsciiData (tk, "Pause_Table", tk.GetPauseTable(), tk.GetPauseCount())) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
            /*if (m_format == 3)
                if ((status = tk.start_compression()) != TK_Normal)
                    return status;*/

            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t1(&tk);
            int             item_count = 0;
            int             i;

            for (i=0; i<tk.m_translator.m_size; ++i) {
                if (tk.m_translator.m_pairs[i].m_index == i)
                    ++item_count;
            }
            if ((status = PutAsciiData (tk,"Count" ,item_count)) != TK_Normal)
                return status;

            m_placeholder = 0;  // item index
            m_progress = -1;    // internal index
            m_stage++;
        }   nobreak;

        case 6: {
			
            // m_placeholder keeps track of the position in the table
            while (m_placeholder < tk.m_translator.m_size) {
                Internal_Translator::Index_Key_Pair const &     item = tk.m_translator.m_pairs[m_placeholder];

                if (item.m_index == m_placeholder) {    // real item
                    switch (m_progress) {
                        case -1: {
							PutTab t(&tk);
                            if ((status = PutAsciiData (tk,"PlaceHolder", m_placeholder)) != TK_Normal)
                                return status;
                            m_progress--;
                        }   nobreak;

                        case -2: {
							PutTab t(&tk);
                            unsigned char       present = 0;
                            int                 i;

                            if (item.m_extra != null) {
                                for (i=0; i<8; ++i)
                                    if (item.m_extra->m_variants[i][0] != 0)
                                        present |= 1 << i;
                            }
							
                            if ((status = PutAsciiData (tk,"Present", (int)present)) != TK_Normal)
                                return status;
                            m_progress = 0;
                        }   nobreak;

                        default: {
							PutTab t(&tk);
                            int         num_offsets = m_format == 0 ? 1 : 2;
                            if (m_progress < 0 || m_progress > 7)
                                return tk.Error();

                            if (item.m_extra != null) {
                                while (m_progress < 8) {
                                    if (item.m_extra->m_variants[m_progress][0] != 0) {
									
                                        if ((status = PutAsciiData (tk,"Extra_Variants", item.m_extra->m_variants[m_progress],
                                                                   num_offsets)) != TK_Normal)
                                            return status;
                                    }
                                    ++m_progress;
                                }
                            }
                            m_progress = -3;
                        }   nobreak;

                        case -3: {
							PutTab t(&tk);
                            if (m_format > 0) {
                                unsigned char       byte = 0;
                                if (item.m_extra != null)
                                    byte = (unsigned char)(item.m_extra->m_options & tk.m_dictionary_options);

                                if ((status = PutAsciiHex (tk,"Extra_Options_And_Dictionary_Options", byte)) != TK_Normal)
                                    return status;
                            }
                            m_progress--;
                        }   nobreak;

                        case -4: {
							PutTab t(&tk);
                            if (m_format > 0 && item.m_extra != null &&
                                (item.m_extra->m_options & tk.m_dictionary_options & Internal_Translator::Bounds_Valid) != 0) {
                               
									if ((status = PutAsciiData (tk,"Bounds", item.m_extra->m_bounds, 6)) != TK_Normal)
                                    return status;
                            }
                            m_progress = 666;
                        }   break;
                    }
                }

                ++m_placeholder;
                m_progress = -1;
            }
            m_stage++;
        }   nobreak;

        case 7: {
          /*  if (m_format == 3)
                if ((status = tk.stop_compression()) != TK_Normal)
                    return status;*/

            m_stage++;
        }   nobreak;

        case 8: {
			PutTab t1(&tk);
            m_format = (unsigned char)tk.GetDictionaryFormat();
            if (m_format == 1) {
                int         offset = tk.GetFirstPause();
				
                if ((status = PutAsciiData (tk,"First_Pause_Offset", offset)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 9: {
			PutTab t1(&tk);
            // last thing is the start position
            int         offset = tk.GetDictionaryOffset();
			
            if ((status = PutAsciiData (tk,"Dictionary_Offset", offset)) != TK_Normal)
                return status;
                 m_stage++;
        }   nobreak;

		case 10: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Dictionary::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Format", m_int)) != TK_Normal)
                return status;
			m_format = (unsigned char)m_int;
            if (m_format > 3)
                return tk.Error();
            m_stage++;
        }   nobreak;

        case 1: {
            if (m_format >= 2) {
				if ((status = GetAsciiData (tk,"Pauses", m_progress)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 2: {
            if (m_format >= 2) {
                int *       table = new int [m_progress];
                register    int         i;

				if ((status = GetAsciiData (tk,"Pause_Table" ,table, m_progress)) != TK_Normal) {
                    delete [] table;
                    return status;
                }

                tk.ClearPauses();
                for (i=0; i<m_progress; i++)
                    tk.RecordPause(table[i]);
                delete [] table;
            }
            m_stage++;
           
        }   nobreak;

        case 3: {
            /*if (m_format == 3)
                if ((status = tk.start_decompression()) != TK_Normal)
                    return status;*/
			 m_stage++;
        }   nobreak;

        case 4: {
			if ((status = GetAsciiData (tk,"Count", m_number_of_items)) != TK_Normal)
                return status;
            m_placeholder = 0;  // current item
            m_progress = -1;    // item index
            m_stage++;
        }   nobreak;

        case 5: {
            // m_placeholder keeps track of the position in the table
            while (m_placeholder < m_number_of_items) {
                switch (m_progress) {
                    case -1: {
						if ((status = GetAsciiData (tk,"PlaceHolder", m_int)) != TK_Normal)
                            return status;

                        m_int += tk.m_index_base;   // adjust for current file in case of ExRefs
                        m_item = &tk.m_translator.m_pairs[m_int];
                        m_progress--;
                    }   nobreak;

                    case -2: {
						if ((status = GetAsciiData (tk,"Present", m_present)) != TK_Normal)
                            return status;
                        m_progress = 0;
                    }   nobreak;

                    default: {
                        int         num_offsets = m_format == 0 ? 1 : 2;

                        if (m_progress < 0 || m_progress > 7)
                            return tk.Error();

                        while (m_progress < 8) {
                            if ((m_present & (1 << m_progress)) != 0) {
                                int         value[2];

                                value[1] = 0;
								if ((status = GetAsciiData (tk,"Extra_Variants", value, num_offsets)) != TK_Normal)
                                    return status;
                                // but we do record variant
                                tk.AddVariant (m_item->m_key, m_progress, value[0], value[1]);
                            }
                            ++m_progress;
                        }
                        m_progress = -3;
                    }   nobreak;

                    case -3: {
                        if (m_format > 0) {

							if ((status = GetAsciiHex (tk,"Extra_Options_And_Dictionary_Options", m_byte)) != TK_Normal)
                                return status;

                            if (m_byte != 0) {
                                if (m_item->m_extra == null) {
                                    m_item->m_extra = new IT_Index_Key_Extra;
                                    memset (m_item->m_extra, 0, sizeof (IT_Index_Key_Extra));
                                }

                                m_item->m_extra->m_options = (unsigned char) m_byte;
                            }
                        }

                        m_progress--;
                    }   nobreak;

                    case -4: {
                        if (m_format > 0) {
                            if (m_item->m_extra != null &&
                                (m_item->m_extra->m_options & Internal_Translator::Bounds_Valid) != 0) {
									if ((status = GetAsciiData (tk, "Bounds", m_item->m_extra->m_bounds, 6)) != TK_Normal)
                                    return status;
                            }
                        }

                        m_item = null;
                        m_progress = 666;
                    }   break;
                }

                ++m_placeholder;
                m_progress = -1;
            }
            m_stage++;
        }   nobreak;

        case 6: {
            /*if (m_format == 3)
                if ((status = tk.stop_decompression()) != TK_Normal)
                    return status;*/

            m_stage++;
        }   nobreak;

        case 7: {
            if (m_format == 1) {
				if ((status = GetAsciiData (tk,"First_Pause_Offset", m_int)) != TK_Normal)
                    return status;

                tk.SetFirstPause (m_int);
            }
            m_stage++;
        }   nobreak;

        case 8: {
            // last thing is the starting offset
            int             offset;

			if ((status = GetAsciiData (tk,"Dictionary_Offset", offset)) != TK_Normal)
                return status;

            tk.SetDictionaryOffset (offset);
               m_stage++;
        }   nobreak;
		
		case 9: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    status = TK_Normal;
    m_stage = -1;

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Dictionary_Locater::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Size", m_size)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk, "Offset",m_offset)) != TK_Normal)
                return status;
               m_stage++;
        }   nobreak;

		case 3: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;
        default:
            return tk.Error();
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Dictionary_Locater::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Size", m_size)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Offset", m_offset)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;
		
		case 2: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_Referenced_Segment::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            unsigned char       byte = 0;
            if (tk.GetTargetVersion() >= 1150 && m_cond_length > 0)
            {
                if ((status = PutAsciiData (tk,"Has_Condition_Length_Flag", (int) byte)) != TK_Normal)
                    return status;

                _W3DTK_REQUIRE_VERSION( 1150 );
            }

            m_stage++;
        }   nobreak;

        case 2: {
            // need code here to handle length > 255 ?
            unsigned char       byte = (unsigned char)m_length;
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Length", (int)byte)) != TK_Normal)
                return status;

            m_stage++;
        }   nobreak;

        case 3: {
			 PutTab t(&tk);
             if ((status = PutAsciiData (tk, "String", m_string, m_length)) != TK_Normal)
                return status;

            if (tk.GetLogging() && tk.GetLoggingOptions (TK_Logging_Segment_Names)) {
                LogDebug (tk, "<");
                LogDebug (tk, m_string);
                LogDebug (tk, ">");
            }

            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            // need code here to handle length > 255 ?
            if (tk.GetTargetVersion() >= 1150 && m_cond_length > 0) {
                if ((status = PutAsciiData (tk,"Condition_Length", m_cond_length)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1150 );
            }

            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if (tk.GetTargetVersion() >= 1150 && m_cond_length > 0) {
                 if ((status = PutAsciiData (tk,"Condition", m_condition, m_cond_length)) != TK_Normal)
                    return status;

                if (tk.GetLogging() && tk.GetLoggingOptions (TK_Logging_Segment_Names)) {
                    LogDebug (tk, "<");
                    LogDebug (tk, m_condition);
                    LogDebug (tk, ">");
                }

                _W3DTK_REQUIRE_VERSION( 1150 );
            }

            m_stage++;
        }   nobreak;

        case 6: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;

             m_stage++;
        }   nobreak;

		case 7: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Referenced_Segment::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	TK_Status       status = TK_Normal;
//	auto		unsigned int	*readSize = 0;

    switch (m_stage) {
        case 0: {
		  if ((status = Read_Referenced_Segment(tk, m_progress)) != TK_Normal)
			     return status;
            m_stage++;
        }   nobreak;

        case 1: {
            if (m_progress != 0)
                set_segment (m_progress);   // was simple length
            else {
                // condition flagged, read real length nmext
				if ((status = GetAsciiData (tk,"Length", m_int)) != TK_Normal)
                    return status;
                set_segment (m_int);    // allocate space & set length
            }
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
                return status;

            if (tk.GetLogging() && tk.GetLoggingOptions (TK_Logging_Segment_Names)) {
                LogDebug (tk, "<");
                LogDebug (tk, m_string);
                LogDebug (tk, ">");
            }

            m_stage++;
        }   nobreak;

        case 3: {
            if (m_progress == 0) {
				if ((status = GetAsciiData (tk,"Condition_Length", m_int)) != TK_Normal)
                    return status;
                SetCondition (m_int);    // allocate space & set length
            }
            else
                SetCondition (0);
            m_stage++;
        }   nobreak;

        case 4: {
            if (m_progress == 0) {
				if ((status = GetAsciiData (tk,"Condition", m_condition, m_cond_length)) != TK_Normal)
                    return status;

                if (tk.GetLogging() && tk.GetLoggingOptions (TK_Logging_Segment_Names)) {
                    LogDebug (tk, "<");
                    LogDebug (tk, m_condition);
                    LogDebug (tk, ">");
                }
            }

               m_stage++;
        }   nobreak;
		
		case 5: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Reference::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Index", m_index)) != TK_Normal)
                return status;

            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            // need code here to handle length > 255 ?
            if ((status = PutAsciiData (tk,"Condition_Length", m_cond_length)) != TK_Normal)
                return status;

            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if (m_cond_length > 0) {
                 if ((status = PutAsciiData (tk,"Condition", m_condition, m_cond_length)) != TK_Normal)
                    return status;

                if (tk.GetLogging() && tk.GetLoggingOptions (TK_Logging_Segment_Names)) {
                    LogDebug (tk, "<");
                    LogDebug (tk, m_condition);
                    LogDebug (tk, ">");
                }
            }

            m_stage++;
        }   nobreak;

        case 6: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;

             m_stage++;
        }   nobreak;

		case 7: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Reference::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	TK_Status       status = TK_Normal;
//	auto		unsigned int	*readSize = 0;

    switch (m_stage) {
        case 0: {
            if ((status = GetAsciiData (tk,"Index", m_index)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Condition_Length", m_int)) != TK_Normal)
                return status;
            SetCondition (m_int);    // allocate space & set length
            m_stage++;
        }   nobreak;

        case 2: {
            if (m_cond_length != 0) {
				if ((status = GetAsciiData (tk,"Condition", m_condition, m_cond_length)) != TK_Normal)
                    return status;

                if (tk.GetLogging() && tk.GetLoggingOptions (TK_Logging_Segment_Names)) {
                    LogDebug (tk, "<");
                    LogDebug (tk, m_condition);
                    LogDebug (tk, ">");
                }
            }

               m_stage++;
        }   nobreak;
		
		case 3: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

}
TK_Status TK_Instance::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk, "From_Index", m_from_index)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			 PutTab t(&tk);
             if ((status = PutAsciiData (tk,"From_Variant" ,m_from_variant)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"To_Index", m_to_index)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			 PutTab t(&tk);
             if ((status = PutAsciiData (tk,"To_Variant", m_to_variant)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Options", m_options)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 6: {
			 PutTab t(&tk);
             if ((status = PutAsciiData (tk,"Matrix_0", &m_matrix[0], 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 7: {
			 PutTab t(&tk);
             if ((status = PutAsciiData (tk,"Matrix_4",&m_matrix[4], 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 8: {
			PutTab t(&tk);
             if ((status = PutAsciiData (tk,"Matrix_8", &m_matrix[8], 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 9: {
			 PutTab t(&tk);
             if ((status = PutAsciiData (tk,"Matrix_12",&m_matrix[12], 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 10: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage++;
        }   nobreak;

		case 11: {
#if 0       // HoopsStream overloaded tagging currently requires the unspecified variant
            // occur before any specific ones, so we recreate the functionality used
            // by the BaseStream
            if ((status = Tag (tk, m_to_variant)) != TK_Normal)
                return status;
#else
            int                 i;
            int                 index;
            ID_Key              key;
			Outdent				one_tab(&tk);

            for (i = 0; i < tk.m_last_keys_used; i++) { 
                key = tk.m_last_keys[i];
                status = tk.KeyToIndex (key, index);
                if (status == TK_NotFound) {
                    BBaseOpcodeHandler *    tagop = tk.GetOpcodeHandler (TKE_Tag);
                    if ((status = tagop->Write (tk)) != TK_Normal)
                        return status;
                    tagop->Reset();
                    // must be a new item
                    tk.AddIndexKeyPair (index = tk.NextTagIndex(), key);
                }
                if (m_to_variant != -1) {
                    // and now record the file offset to associate with the variant (if any)
                    tk.AddVariant (key, m_to_variant, tk.GetPosition() + tk.GetFileOffset());
                }
            }
#endif
            if (tk.GetLogging() && tk.GetLoggingOptions (TK_Logging_Tagging)) {
                char            buffer[64];

                sprintf (buffer, "[%d:%d - %d:%d]", m_from_index, m_from_variant, m_to_index, m_to_variant);
                tk.LogEntry (buffer);
            }

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }
	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Instance::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"From_Index", m_from_index)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"From_Variant", m_from_variant)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"To_Index", m_to_index)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			if ((status = GetAsciiData (tk, "To_Variant", m_to_variant)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

		case 4: {
			if ((status = GetAsciiData (tk,"Options", m_options)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 5: {
			if ((status = GetAsciiData (tk,"Matrix_0", &m_matrix[0], 3)) != TK_Normal)
                return status;
            m_matrix[3] = 0.0f;
            m_stage++;
        }   nobreak;

        case 6: {
			if ((status = GetAsciiData (tk,"Matrix_4", &m_matrix[4], 3)) != TK_Normal)
                return status;
            m_matrix[7] = 0.0f;
            m_stage++;
        }   nobreak;

        case 7: {
			if ((status = GetAsciiData (tk, "Matrix_8",&m_matrix[8], 3)) != TK_Normal)
                return status;
            m_matrix[11] = 0.0f;
            m_stage++;
        }   nobreak;

        case 8: {
			if ((status = GetAsciiData (tk,"Matrix_12", &m_matrix[12], 3)) != TK_Normal)
                return status;
            m_matrix[15] = 1.0f;

            if (tk.GetLogging() && tk.GetLoggingOptions (TK_Logging_Tagging)) {
                char            buffer[64];

                sprintf (buffer, "[%d:%d - %d:%d]", m_from_index, m_from_variant, m_to_index, m_to_variant);
                tk.LogEntry (buffer);
            }
             m_stage++;
        }   nobreak;
		
		case 9: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

}


TK_Status TK_Delete_Object::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Index", m_index)) != TK_Normal)
                return status;

            if (tk.GetLogging() &&tk.GetLoggingOptions (TK_Logging_Tagging)) {
                char        buffer[32];

                sprintf (buffer, "[%d]", m_index);
                LogDebug (tk, buffer);
            }

                 m_stage++;
        }   nobreak;

		case 2: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Delete_Object::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Index", m_index)) != TK_Normal)
                return status;

            if (tk.GetLogging() &&tk.GetLoggingOptions (TK_Logging_Tagging)) {
                char        buffer[32];

                sprintf (buffer, "[%d]", m_index);
                LogDebug (tk, buffer);
            }

             m_stage++;
        }   nobreak;
		
		case 1: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

////////////////////////////////////////

TK_Status TK_Bounding::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Type", (int)m_type)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            bool is_sphere = (m_type == TKO_Bounding_Type_Sphere);
            if ((status = PutAsciiData (tk,"Values", m_values, is_sphere ? 4 : 6)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
            if (Opcode() == TKE_Bounding_Info) {
                if (m_type == TKO_Bounding_Type_Sphere)
                    tk.SetWorldBoundingBySphere(m_values, m_values[3]);
                else
                    tk.SetWorldBounding(m_values);
            }
                 m_stage++;
        }   nobreak;

		case 4: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;
        default:
            return tk.Error("internal error in TK_Bounding::Write");
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Bounding::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Type", m_int)) != TK_Normal)
                return status;
			m_type = (char)m_int;
			
            m_stage++;
        }   nobreak;

        case 1: {
            bool is_sphere = (m_type == TKO_Bounding_Type_Sphere);
			if ((status = GetAsciiData (tk,"Values", m_values, is_sphere ? 4 : 6)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
            if (Opcode() == TKE_Bounding_Info) {
                if (m_type == TKO_Bounding_Type_Sphere)
                    tk.SetWorldBoundingBySphere(m_values, m_values[3]);
                else
                    tk.SetWorldBounding(m_values);
            }
               m_stage++;
        }   nobreak;
		
		case 3: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error("internal error in TK_Bounding::Read");
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

////////////////////////////////////////

TK_Status TK_Streaming::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

  #if 0   // no longer generated
    if (m_flag)
        return PutAsciiOpcode (tk);
    else
  #else
    UNREFERENCED(tk);
  #endif
        return TK_Normal;       // don't bother recording "off"
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Streaming::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	UNREFERENCED(tk);
    return TK_Pause;
#else
return tk.Error(stream_disable_ascii);
#endif 
}


////////////////////////////////////////


TK_Status TK_LOD::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    TK_Terminator   terminate(TKE_Termination, false);
    if (m_primitives == null)
        return TK_Normal;

    if (tk.GetTargetVersion() < 907)
        return TK_Normal;

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        } nobreak;

        case 1: {
			PutTab t(&tk);
            while (m_substage <= m_highest_level) {
                if (m_num_primitives[m_substage]) {
                    if (m_progress == 0) {
                        if ((status = PutAsciiData (tk,"LOD_Level", m_substage)) != TK_Normal)
                            return status;
                        m_progress++;
                    }
                    while (m_progress < m_num_primitives[m_substage] + 1) {
                        if ((status = m_primitives[m_substage][m_progress-1]->Write(tk)) != TK_Normal)
                            return status;
                        m_progress++;
                    }
                    if (m_progress == m_num_primitives[m_substage] + 1) {
                        if ((status = terminate.Write(tk)) != TK_Normal)
                            return status;
                        m_progress++;
                    }
                    m_progress = 0;
                }
                m_substage++;
            }
            m_substage = 0;
            m_stage++;
        } nobreak;

        case 2: {
			PutTab t(&tk);
            unsigned char byte = TKLOD_ESCAPE;
            if ((status = PutAsciiData (tk,"LOD_Escape", (int) byte)) != TK_Normal)
                return status;
                 m_stage++;
        }   nobreak;

		case 3: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;

        default:
            return tk.Error("unrecognized case in TK_LOD");

    }

	
    return status;

#else
return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_LOD::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	TK_Status       status = TK_Normal;
    int j, count;
    BBaseOpcodeHandler *handler;

    if (m_current_working == null)
        m_current_working = new_vlist(malloc, free);
    
    forever {
        switch(m_stage) {
            case 0: {
				if ((status = GetAsciiData (tk,"LOD_Level", m_byte)) != TK_Normal)
                    return status;
                m_current_level = m_byte;
                if (m_current_level == TKLOD_ESCAPE) {
                    return TK_Normal;
                }
                if (m_current_level >= m_levels_allocated) {
                    int old_levels_allocated;
                    BBaseOpcodeHandler ***temp_primitives;
                    int *temp_num_primitives;

                    old_levels_allocated = m_levels_allocated;
                    m_levels_allocated += 10;
                    temp_primitives = new BBaseOpcodeHandler **[m_levels_allocated];
                    temp_num_primitives = new int[m_levels_allocated];
                    if (!temp_primitives || !temp_num_primitives)
                        return tk.Error("memory allocation failed in TK_LOD::Read");
                    if (old_levels_allocated) {
                        memcpy(temp_primitives, m_primitives, old_levels_allocated * sizeof(BBaseOpcodeHandler **));
                        memcpy(temp_num_primitives, m_num_primitives, old_levels_allocated * sizeof(int));
                        delete [] m_primitives;
                        delete [] m_num_primitives;
                    }
                    memset(&temp_primitives[old_levels_allocated], 0, (m_levels_allocated - old_levels_allocated) * sizeof(BBaseOpcodeHandler **));
                    memset(&temp_num_primitives[old_levels_allocated], 0, (m_levels_allocated - old_levels_allocated) * sizeof(int));
                    m_primitives = temp_primitives;
                    m_num_primitives = temp_num_primitives;
                }
                if (m_current_level > m_highest_level)
                    m_highest_level = m_current_level;
                m_stage++;
            } nobreak;
            case 1: {

		        ASSERT(m_primitives[m_current_level] == null); //this should be the first time we try to read this m_current_level
                if ((status = ReadOneList(tk)) != TK_Normal)
					return status;
                // transfer the array of pointers from the vlist to a regular array
                count = vlist_count(m_current_working);
                m_num_primitives[m_current_level] = count;
                m_primitives[m_current_level] = new BBaseOpcodeHandler *[count];
                for (j = 0; j < count; j++) {
                    handler = (BBaseOpcodeHandler *)vlist_remove_first(m_current_working);
                    m_primitives[m_current_level][j] = handler;
                }
                m_substage = 0;
                m_stage++;
        }   nobreak;
		
		case 2: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = 0;
		}   break;

            default:
                return tk.Error("unrecognized case of m_stage during TK_LOD::Read");
        }
    }
	return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Point::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Point", m_point, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if (tk.GetTargetVersion() >= 1170 && // camera relative lights were first added in 11.70
                (m_opcode == TKE_Distant_Light || m_opcode == TKE_Local_Light)) {
                if ((status = PutAsciiData (tk,"Options", (int)m_options)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1170 );
            }
            m_stage++;
        }   nobreak;

        case 3: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
            m_stage++;
        }   nobreak;

		case 4: {
			if (Tagging (tk) && m_opcode != TKE_Text_Path)
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;

        default:
            return tk.Error("internal error in TK_Point::Write");
    }

	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Point::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Point", m_point, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            if (tk.GetVersion() >= 1170 &&
                (m_opcode == TKE_Local_Light || m_opcode == TKE_Distant_Light)) {
					if ((status = GetAsciiData (tk,"Options", m_options)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;
		
		case 2: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error("internal error in TK_Point::Read");
    }

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

////////////////////////////////////////////////////////////////////////////////

TK_Status TK_Line::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Points", m_points, 6)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
              m_stage++;
        }   nobreak;

		case 3: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;
        default:
            return tk.Error();
    }

	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Line::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Points", m_points, 6)) != TK_Normal)
                return status;
              m_stage++;
        }   nobreak;
		
		case 1: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Infinite_Line::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Points", m_points, 6)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
              m_stage++;
        }   nobreak;

		case 3: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;
        default:
            return tk.Error();
    }

	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

}
TK_Status TK_Infinite_Line::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Points", m_points, 6)) != TK_Normal)
                return status;
              m_stage++;
        }   nobreak;
		
		case 1: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}
TK_Status TK_Polypoint::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Count", m_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Points", m_points, 3*m_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
            m_stage++;
        }   nobreak;

		case 4: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Polypoint::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Count", m_count)) != TK_Normal)
                return status;
            if (!validate_count (m_count))
                return tk.Error("bad Polypoint count");
            set_points (m_count);   // allocated space
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Points", m_points, 3*m_count)) != TK_Normal)
                return status;
               m_stage++;
        }   nobreak;
		
		case 2: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_NURBS_Curve::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiHex (tk,"Optionals", m_optionals)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Degree", (int)m_degree)) != TK_Normal)
                return status;
            m_progress = 0;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Control_Point_Count", m_control_point_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        //control points
        case 4: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Control_Points", m_control_points, 3 * m_control_point_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        //weights
        case 5: {
			PutTab t(&tk);
            if (m_optionals & NC_HAS_WEIGHTS) {
                if ((status = PutAsciiData (tk,"Weights", m_weights, m_control_point_count)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        //knots
        case 6: {
			PutTab t(&tk);
            if (m_optionals & NC_HAS_KNOTS) {
                if ((status = PutAsciiData (tk,"Knots", m_knots, m_knot_count_implicit)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 7: {
			PutTab t(&tk);
            if (m_optionals & NC_HAS_START) {	
                if ((status = PutAsciiData (tk,"Start", m_start)) != TK_Normal)
                    return status;
            }
            m_progress = 0;
            m_stage++;
        }   nobreak;

        case 8: {
			PutTab t(&tk);
            if (m_optionals & NC_HAS_END) {
                if ((status = PutAsciiData (tk,"End", m_end)) != TK_Normal)
                    return status;
            }
            m_progress = 0;
            m_stage++;
        }   nobreak;

        case 9: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
                m_stage++;
        }   nobreak;

		case 10: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} // end function TK_NURBS_Curve::Write


TK_Status TK_NURBS_Curve::ReadAscii (BStreamFileToolkit & tk) alter  // end function TK_NURBS_Curve::Read
{
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Optionals", m_optionals)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Degree", m_degree)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"Control_Point_Count", m_control_point_count)) != TK_Normal)
                return status;
            if (!validate_count (m_control_point_count))
                return tk.Error("bad NURBS Curve count");
            set_curve (m_degree, m_control_point_count);    // allocate space
            m_stage++;
        }   nobreak;


        case 3: {
			if ((status = GetAsciiData (tk,"Control_Points", m_control_points, 3 * m_control_point_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
            if (m_optionals & NC_HAS_WEIGHTS) {
				if ((status = GetAsciiData (tk,"Weights", m_weights, m_control_point_count)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
            if (m_optionals & NC_HAS_KNOTS) {
				if ((status = GetAsciiData (tk,"Knots",m_knots, m_knot_count_implicit)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 6: {
            if (m_optionals & NC_HAS_START) {
				if ((status = GetAsciiData (tk,"Start", m_start)) != TK_Normal)
                    return status;
            }
            else
                m_start = 0.0;
            m_stage++;
        }   nobreak;

        case 7: {
            if (m_optionals & NC_HAS_END) {
				if ((status = GetAsciiData (tk,"End", m_end)) != TK_Normal)
                    return status;
            }
            else
                m_end = 1.0;
               m_stage++;
        }   nobreak;
		
		case 8: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}
TK_Status HT_NURBS_Trim::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	PutTab t0(&tk);

    switch (m_stage) {
		case 0: {
			PutTab t(&tk);
            if ((status = PutAsciiHex (tk,"Type", m_type)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiHex (tk,"Options", m_options)) != TK_Normal)
                return status;
            m_progress = 0;
            m_stage++;
        }   nobreak;

        case 3: {
            if (m_type == NS_TRIM_COLLECTION) {
                if ((status = write_collection(tk)) != TK_Normal)
                    return status;
				 m_stage = -1;
					return status;          
            }
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Count", m_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Points", m_points, 2*m_count)) != TK_Normal)
                return status;
            if (m_type == NS_TRIM_POLY) {
                m_stage = -1;
                return status;
            }
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Degree",(int) m_degree)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 7: {
			PutTab t(&tk);
            if (m_options & NS_TRIM_HAS_WEIGHTS) {
                if ((status = PutAsciiData (tk,"Weights", m_weights, m_count)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 8: {
			PutTab t(&tk);
            if (m_options & NS_TRIM_HAS_KNOTS) {
                if ((status = PutAsciiData (tk,"Knots", m_knots, m_degree + m_count + 1)) != TK_Normal)
                    return status;
            }
               	m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}


TK_Status HT_NURBS_Trim::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {

		case 0: {
			/*if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;*/
			   m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiHex (tk,"Options", m_options)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
            if (m_type == NS_TRIM_COLLECTION) {
                if ((status = read_collection(tk)) != TK_Normal)
                    return status;
                m_stage = -1;
				/*status = ReadAsciiWord(tk,0);*/
					return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			if ((status = GetAsciiData (tk,"Count", m_count)) != TK_Normal)
                return status;
            if (!validate_count (m_count))
                return tk.Error("bad NURBS Trim count");
            m_points = new float [2*m_count];
            m_stage++;
        }   nobreak;

        case 4: {
			if ((status = GetAsciiData (tk,"Points", m_points, 2*m_count)) != TK_Normal)
                return status;
            if (m_type == NS_TRIM_POLY) {
                m_stage = -1;
                return status;
            }
            m_progress = 0;
            m_stage++;
        }   nobreak;

        case 5: {
			if ((status = GetAsciiData (tk,"Degree", m_degree)) != TK_Normal)
                return status;
            if ((m_options & NS_TRIM_HAS_WEIGHTS) != 0)
                m_weights = new float [m_count];
            if ((m_options & NS_TRIM_HAS_KNOTS) != 0)
                m_knots = new float [m_degree + m_count + 1];
            m_stage++;
        }   nobreak;

        case 6: {
            if (m_options & NS_TRIM_HAS_WEIGHTS) {
				if ((status = GetAsciiData (tk,"Weights", m_weights, m_count)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 7: {
            if (m_options & NS_TRIM_HAS_KNOTS) {
				if ((status = GetAsciiData (tk,"Knots", m_knots, m_degree + m_count + 1)) != TK_Normal)
                    return status;
            }
               m_stage++;
        }   nobreak;
		
		case 8: {

			/*if ((status = ReadAsciiWord(tk,0)) != TK_Normal)
					return status;*/

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status HT_NURBS_Trim::write_collection_ascii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_READONLY
    TK_Status       status = TK_Normal;
    int             i;
    HT_NURBS_Trim   *ptr;

    ptr = m_list;
    for (i = 0; i < m_progress; i++) {
        ptr = ptr->m_next;
    }

    switch(m_substage) {

        case 0: {
            while (ptr != null) {
                if ((status = ptr->Write(tk)) != TK_Normal)
                    return status;
                ptr = ptr->m_next;
                m_progress++;
            }
            m_progress = 0;
            m_substage++;
        } nobreak;

        case 1: {
            unsigned char       end = NS_TRIM_END;

            if ((status = PutAsciiHex (tk,"Type", end)) != TK_Normal)
                return status;
            m_substage = 0;
        } break;

        default:
            return tk.Error();

    }

    return status;
#else
    return tk.Error (stream_readonly);
#endif
}

TK_Status HT_NURBS_Trim::read_collection_ascii(BStreamFileToolkit & tk) alter {
    TK_Status       status = TK_Normal;
    HT_NURBS_Trim   *tail;

    tail = m_list;
    while (tail != null && tail->m_next != null) {
        tail = tail->m_next;
    }
    while (1) {
        if (m_current_trim == null) {
            unsigned char       type;

            if ((status = GetAsciiHex (tk,"Type", type)) != TK_Normal)
                return status;

            if (type == NS_TRIM_END)
                break;
            else if (type > NS_TRIM_LAST_KNOWN_TYPE)
                return tk.Error ("Unknown trim type");

            m_current_trim = new HT_NURBS_Trim;
            m_current_trim->m_type = type;
        }
        if ((status = m_current_trim->Read (tk)) != TK_Normal)
            return status;
        if (m_list == null) {
            m_list = tail = m_current_trim;
        }
        else {
            tail->m_next = m_current_trim;
            tail = m_current_trim;
        }
        m_current_trim = null;
    }

    return status;
}
////////////////////////////////////////

TK_Status TK_NURBS_Surface::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if (tk.GetTargetVersion() < 650)    // first added in 6.50, so just skip it
                return TK_Normal;
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiHex (tk,"Optionals", m_optionals)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			int i_degree[2];
			i_degree[0] = (int)m_degree[0];
			i_degree[1] = (int)m_degree[1];
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Degree", i_degree, 2)) != TK_Normal)
                return status;
            m_progress = 0;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Size", m_size, 2)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        //control points
        case 4: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Control_Points", m_control_points, 3 * m_size[0] * m_size[1])) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        //weights
        case 5: {
			PutTab t(&tk);
            if (m_optionals & NS_HAS_WEIGHTS) {
                if ((status = PutAsciiData (tk,"Weights", m_weights, m_size[0] * m_size[1])) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        //knots
        case 6: {
			PutTab t(&tk);
            if (m_optionals & NS_HAS_KNOTS) {
                if ((status = PutAsciiData (tk,"U_Knots", m_u_knots, m_degree[0] + m_size[0] + 1)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 7: {
			PutTab t(&tk);
            if (m_optionals & NS_HAS_KNOTS) {
                if ((status = PutAsciiData (tk,"V_Knots", m_v_knots, m_degree[1] + m_size[1] + 1)) != TK_Normal)
                    return status;
            }
            m_current_trim = m_trims;
            m_stage++;
        }   nobreak;

        case 8: {
            if (m_optionals & NS_HAS_TRIMS) {
                while (m_current_trim != null) {
                    if ((status = m_current_trim->Write (tk)) != TK_Normal)
                        return status;
                    m_current_trim = m_current_trim->m_next;
                }
            }
            m_stage++;
        }   nobreak;

        case 9: {
			PutTab t(&tk);
            if (m_optionals & NS_HAS_TRIMS) {
        //        unsigned char       end = NS_TRIM_END;

                if ((status = PutAsciiHex (tk,"Type", NS_TRIM_END)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 10: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
                m_stage++;
        }   nobreak;

		case 11: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

} // end function TK_NURBS_Surface::Write

TK_Status TK_NURBS_Surface::ReadAscii (BStreamFileToolkit & tk) alter  // end function TK_NURBS_Surface::Read
{
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Optionals", m_optionals)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			int degree[2];
			if ((status = GetAsciiData (tk,"Degree", degree, 2)) != TK_Normal)
                return status;
			m_degree[0] = (unsigned char) degree[0];
			m_degree[1] = (unsigned char) degree[1];
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"Size", m_size, 2)) != TK_Normal)
                return status;
            if (!validate_count (m_size[0]*m_size[1]))
                return tk.Error("bad NURBS Surface count");
            SetSurface (m_degree[0], m_degree[1], m_size[0], m_size[1]);    // allocate space
            m_stage++;
        }   nobreak;


        case 3: {
			if ((status = GetAsciiData (tk,"Control_Points", m_control_points, 3 * m_size[0] * m_size[1])) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
            if (m_optionals & NS_HAS_WEIGHTS) {
				if ((status = GetAsciiData (tk,"Weights", m_weights, m_size[0] * m_size[1])) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
            if (m_optionals & NS_HAS_KNOTS) {
				if ((status = GetAsciiData (tk,"U_Knots", m_u_knots, m_degree[0] + m_size[0] + 1)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 6: {
            if (m_optionals & NS_HAS_KNOTS) {
				if ((status = GetAsciiData (tk,"V_Knots", m_v_knots, m_degree[1] + m_size[1] + 1)) != TK_Normal)
                    return status;
            }
            m_current_trim = null;
            m_stage++;
        }   nobreak;

        case 7: {
            if (m_optionals & NS_HAS_TRIMS) {
                while (1) {
                    if (m_current_trim == null) {
                        int       type;

						if ((status = GetAsciiHex (tk,"Type", type)) != TK_Normal)
                            return status;

                        if (type == NS_TRIM_END)
                            break;
                        else if (type > NS_TRIM_LAST_KNOWN_TYPE)
                            return tk.Error ("Unknown trim type");

                        m_current_trim = NewTrim (type);
                    }

                    if ((status = m_current_trim->Read (tk)) != TK_Normal)
                        return status;
                    m_current_trim = null;
                }
            }
               m_stage++;
        }   nobreak;
		
		case 8: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Area_Light::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

   switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Count", m_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Points", m_points, 3*m_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiHex (tk,"Options", m_options)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
                m_stage++;
        }   nobreak;

		case 5: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

    
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Area_Light::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Count", m_count)) != TK_Normal)
                return status;
            if (!validate_count (m_count))
                return tk.Error("bad Area Light count");
            set_points (m_count);
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Points", m_points, 3*m_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiHex (tk,"Options", m_options)) != TK_Normal)
                return status;
             m_stage++;
        }   nobreak;
		
		case 3: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_Spot_Light::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Position", m_position, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Target", m_target, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiHex (tk,"Options", m_options)) != TK_Normal)
                return status;
			
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((m_options & TKO_Spot_Outer_Mask) != 0) {
                if ((status = PutAsciiData (tk,"Outer", m_outer)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if ((m_options & TKO_Spot_Inner_Mask) != 0) {
                if ((status = PutAsciiData (tk,"Inner", m_inner)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t(&tk);
            if ((m_options & TKO_Spot_Concentration) != 0) {
                if ((status = PutAsciiData (tk,"Concentration", m_concentration)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 7: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
                m_stage++;
        }   nobreak;

		case 8: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Spot_Light::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Position", m_position, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Target", m_target, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiHex (tk,"Options", m_options)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
            if ((m_options & TKO_Spot_Outer_Mask) != 0) {
				if ((status = GetAsciiData (tk,"Outer", m_outer)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
            if ((m_options & TKO_Spot_Inner_Mask) != 0) {
				if ((status = GetAsciiData (tk,"Inner", m_inner)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
            if ((m_options & TKO_Spot_Concentration) != 0) {
				if ((status = GetAsciiData (tk,"Concentration", m_concentration)) != TK_Normal)
                    return status;
            }
               m_stage++;
        }   nobreak;
		
		case 6: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Cutting_Plane::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
    int				count = m_count;
	
	PutTab t0(&tk);

	if (tk.GetTargetVersion() < 1305)
    {
		count = 1;
    }
    else
    {
        //
        // plane count was introduced in 1305
        //
        _W3DTK_REQUIRE_VERSION( 1305 );
    }

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
			if (tk.GetTargetVersion() >= 1305)
            {
				if ((status = PutAsciiData (tk,"Count", count)) != TK_Normal)
					return status;
                _W3DTK_REQUIRE_VERSION( 1305 );
            }
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Planes", m_planes, 4*count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
               m_stage++;
        }   nobreak;

		case 4: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Cutting_Plane::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	TK_Status       status = TK_Normal;
	switch (m_stage) {
        case 0: {
			if (tk.GetVersion() >= 1305) {
				if ((status = GetAsciiData (tk,"Count", m_int)) != TK_Normal)
					return status;
			}
			else
				m_int = 1;
			SetPlanes (m_int);
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Planes", m_planes, 4*m_count)) != TK_Normal)
                return status;
               m_stage++;
        }   nobreak;
		
		case 2: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

////////////////////////////////////////////////////////////////////////////////

TK_Status TK_Circle::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Start", m_start, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Middle", m_middle, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"End", m_end, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if (tk.GetTargetVersion() >= 1215) {
				if ((status = PutAsciiFlag (tk,"Flags", m_flags)) != TK_Normal)
					return status;
                _W3DTK_REQUIRE_VERSION( 1215 );
			}
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if (tk.GetTargetVersion() >= 1215 &&
				(m_flags & TKO_Circular_Center) != 0) {
				if ((status = PutAsciiData (tk,"Center", m_center, 3)) != TK_Normal)
					return status;
                _W3DTK_REQUIRE_VERSION( 1215 );
			}
            m_stage++;
        }   nobreak;

        case 6: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage++;
        }   nobreak;

		case 7: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}


TK_Status TK_Circle::ReadAscii (BStreamFileToolkit & tk) alter{
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Start", m_start, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Middle", m_middle, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"End", m_end, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
          if (tk.GetVersion() >= 1215) {
			if ((status = GetAsciiHex (tk,"Flags", m_flags)) != TK_Normal)
					return status;
			}
			else
				m_flags = 0;
            m_stage++;
        }   nobreak;

        case 4: {
			if ((m_flags & TKO_Circular_Center) != 0) {
				if ((status = GetAsciiData (tk,"Center", m_center, 3)) != TK_Normal)
					return status;
			}
               m_stage++;
        }   nobreak;
		
		case 5: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
} 

TK_Status TK_Ellipse::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Center", m_center, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Major", m_major, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
			if ((status = PutAsciiData (tk,"Minor", m_minor, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if (Opcode() == TKE_Elliptical_Arc) {
                if ((status = PutAsciiData (tk,"Limits", m_limits, 2)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
             m_stage++;
        }   nobreak;

		case 6: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			   m_stage = -1;
		}   break;		
			
        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Ellipse::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk, "Center",m_center, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Major", m_major, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"Minor", m_minor, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
            if (Opcode() == TKE_Elliptical_Arc) {
				if ((status = GetAsciiData (tk, "Limits", m_limits, 2)) != TK_Normal)
                    return status;
            }
               m_stage++;
        }   nobreak;
		
		case 4: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


////////////////////////////////////////

TK_Status TK_Sphere::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if (tk.GetTargetVersion() < 1155)    // first added in 11.55, so just skip it
                return TK_Normal;

            _W3DTK_REQUIRE_VERSION( 1155 );

            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiFlag (tk,"Flags", m_flags)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Center", m_center, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Radius", m_radius)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if (!(m_flags & TKSPH_NULL_AXIS)) {
                if ((status = PutAsciiData (tk,"Axis", m_axis, 3)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if (!(m_flags & TKSPH_NULL_AXIS)) {
                if ((status = PutAsciiData (tk,"Ortho", m_ortho, 3)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 6: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
             m_stage++;
        }   nobreak;

		case 7: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;

        default:
            return tk.Error("internal error in TK_Sphere::Write");
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Sphere::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Flags", m_flags)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk, "Center", m_center, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"Radius", m_radius)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
            if (!(m_flags & TKSPH_NULL_AXIS)) {
				if ((status = GetAsciiData (tk,"Axis", m_axis, 3)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
            if (!(m_flags & TKSPH_NULL_AXIS)) {
				if ((status = GetAsciiData (tk,"Ortho", m_ortho, 3)) != TK_Normal)
                    return status;
            }
               m_stage++;
        }   nobreak;
		
		case 5: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error("internal error in TK_Sphere::Read");
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Cylinder::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if (tk.GetTargetVersion() < 650)    // first added in 6.50, so just skip it
                return TK_Normal;
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Axis", m_axis, 6)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
			if ((status = PutAsciiData (tk,"Radius", m_radius)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			if ((status = PutAsciiFlag (tk,"Flags", m_flags)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
            m_stage++;
        }   nobreak;

		case 5: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;
        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Cylinder::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

  TK_Status       status = TK_Normal;
  switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Axis", m_axis, 6)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Radius", m_radius)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiHex (tk,"Flags", m_flags)) != TK_Normal)
                return status;
                m_stage++;
        }   nobreak;
		
		case 3: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_PolyCylinder::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
    bool            do_attributes = (tk.GetTargetVersion() >= 805);
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if (tk.GetTargetVersion() < 705)    // first added in 7.05, so just skip it
                return TK_Normal;
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Count", m_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Points", m_points, 3*m_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Radius_Count", m_radius_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Radii", m_radii, m_radius_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 5: {

			PutTab t(&tk);
            unsigned char       flags = m_flags;

            if (!do_attributes)
                flags &= 0x03;  // only caps settings
			if ((status = PutAsciiFlag (tk,"Flags", flags)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t(&tk);
            if (do_attributes && (m_flags & TKCYL_NORMAL_FIRST))              
				if ((status = PutAsciiData (tk,"Normals", &m_normals[0], 3)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 7: {
			PutTab t(&tk);
            if (do_attributes && (m_flags & TKCYL_NORMAL_SECOND))
                if ((status = PutAsciiData (tk,"Normals", &m_normals[3], 3)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 8: {
            if (do_attributes && (m_flags & TKCYL_OPTIONALS))
                if ((status = TK_Polyhedron::Write (tk)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 9: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
            m_stage++;
        }   nobreak;

		case 10: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_PolyCylinder::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
  
	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Count", m_count)) != TK_Normal)
                return status;
            if (!validate_count (m_count))
                return tk.Error("bad TK_PolyCylinder count");
            SetPoints (m_count);   // allocate space
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Points", m_points, 3*m_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"Radius_Count", m_radius_count)) != TK_Normal)
                return status;
            if (!validate_count (m_radius_count))
                return tk.Error("bad TK_PolyCylinder radius_count");
            SetRadii (m_radius_count);   // allocate space
            m_stage++;
        }   nobreak;

        case 3: {
			if ((status = GetAsciiData (tk,"Radii", m_radii, m_radius_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			if ((status = GetAsciiHex (tk,"Flags", m_flags)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 5: {
            if (m_flags & TKCYL_NORMAL_FIRST)
				if ((status = GetAsciiData (tk,"Normals", &m_normals[0], 3)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 6: {
            if (m_flags & TKCYL_NORMAL_SECOND)
				if ((status = GetAsciiData (tk,"Normals", &m_normals[3], 3)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 7: {
            if (m_flags & TKCYL_OPTIONALS)
                if ((status = TK_Polyhedron::Read (tk)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;
		
		case 8: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Grid::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Type", (int)m_type)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Origin", m_origin, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Ref1", m_ref1, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Ref2" ,m_ref2, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;
	
        case 5: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Counts", m_counts, 2)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 6: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
              m_stage++;
        }   nobreak;

		case 7: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Grid::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Type", m_type)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Origin", m_origin, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"Ref1", m_ref1, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			if ((status = GetAsciiData (tk,"Ref2", m_ref2, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			if ((status = GetAsciiData (tk,"Counts", m_counts, 2)) != TK_Normal)
                return status;
               m_stage++;
        }   nobreak;
		
		case 5: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}


TK_Status TK_Text::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status			status = TK_Normal;
    unsigned char		options = m_options;
	
	PutTab t0(&tk);

	if (tk.GetTargetVersion() < 1002)
		options &= ~TKO_Text_Option_Region;
	if (tk.GetTargetVersion() < 1210)
		options &= ~TKO_Text_Option_Character_Attributes;

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t1(&tk);
            if ((status = PutAsciiData (tk,"Position", m_position, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t1(&tk);
            if (Opcode() == TKE_Text_With_Encoding) {
				
				if ((status = PutAsciiData (tk,"Encoding", (int)m_encoding)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
            unsigned char       byte;
			PutTab t1(&tk);
            if (tk.GetTargetVersion() > 600) {  // extended format added in 6.01
                if (m_length > 65535)
                    byte = 255;                 // flag length stored as int following
                else if (m_length > 253)
                    byte = 254;                 // flag length stored as short following
                else
                    byte = (unsigned char)m_length;
            }
            else {
                if (m_length > 255)
                    byte = 255;                 // maximum length supported by old readers
                else
                    byte = (unsigned char)m_length;
            }
			
            if ((status = PutAsciiData (tk,"Length", m_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t1(&tk);
            if (tk.GetTargetVersion() > 600) {
                if (m_length > 65535) {
                    if ((status = PutAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                        return status;
                }
                else if (m_length > 253) {
                    if ((status = PutAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                        return status;
                }
            }
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t1(&tk);
            if (m_encoding == TKO_Enc_Unicode32) {
				
                if ((status = PutAsciiData (tk,"String", (int const *)m_string, m_length/4)) != TK_Normal)
                    return status;
            }
            else if (m_encoding == TKO_Enc_Unicode) {
				
                if ((status = PutAsciiData (tk,"String", (short const *)m_string, m_length/2)) != TK_Normal)
                    return status;
            }
            else {
				
                if ((status = PutAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t1(&tk);
            if (tk.GetTargetVersion() >= 1002) {
				
                if ((status = PutAsciiHex (tk,"Options", options)) != TK_Normal)
                    return status;

                _W3DTK_REQUIRE_VERSION( 1002 );
            }
            m_stage++;
        }   nobreak;

        case 7: {
			PutTab t1(&tk);
            if ((options & TKO_Text_Option_Region) != 0) {
                
				if ((status = PutAsciiData (tk,"Region_Options", m_region_options)) != TK_Normal)
                    return status;

                _W3DTK_REQUIRE_VERSION( 1002 );
            }
            m_stage++;
        }   nobreak;

        case 8: {
			PutTab t1(&tk);
            if ((options & TKO_Text_Option_Region) != 0) {
				
				if ((status = PutAsciiData (tk,"Region_Count", m_region_count)) != TK_Normal)
                    return status;

                _W3DTK_REQUIRE_VERSION( 1002 );
            }
            m_stage++;
        }   nobreak;

        case 9: {
			PutTab t1(&tk);
            if ((options & TKO_Text_Option_Region) != 0) {
                
				if ((status = PutAsciiData (tk,"Region", m_region, 3*m_region_count)) != TK_Normal)
                    return status;

                _W3DTK_REQUIRE_VERSION( 1002 );
            }
            m_stage++;
        }   nobreak;

        case 10: {
			PutTab t1(&tk);
            if ((options & TKO_Text_Option_Character_Attributes) != 0) {
                
				if ((status = PutAsciiData (tk,"Count", m_count)) != TK_Normal)
                    return status;
				m_progress = 0;
				m_substage = 0;

                _W3DTK_REQUIRE_VERSION( 1210 );
            }
            m_stage++;
        }   nobreak;

        case 11: {
            if ((options & TKO_Text_Option_Character_Attributes) != 0) {
				while (m_progress < m_count) {
					TK_Character_Attribute *	ca = &m_character_attributes[m_progress];

					switch (m_substage) {
						case 0: {
							PutTab t(&tk);
							if ((status = PutAsciiData (tk,"Mask", ca->mask)) != TK_Normal)
								return status;
							m_substage++;
						}   nobreak;

						case 1: {
							PutTab t(&tk);
							if ((ca->mask & TKO_Character_Name) != 0) {
								int				len = (int)strlen (ca->name);
								
								if ((status = PutAsciiData (tk,"Len", len)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 2: {
							PutTab t(&tk);
							if ((ca->mask & TKO_Character_Name) != 0) {
								int				len = (int)strlen (ca->name);
								
								if ((status = PutAsciiData (tk,"Name", ca->name, len)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 3: {
							PutTab t(&tk);
							if ((ca->mask & TKO_Character_Size) != 0) {
															
								if ((status = PutAsciiData (tk,"Size", ca->size)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 4: {
							PutTab t(&tk);
							if ((ca->mask & TKO_Character_Size) != 0) {
								
								if ((status = PutAsciiData (tk,"Size_units", ca->size_units)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 5: {
							PutTab t(&tk);
							if ((ca->mask & TKO_Character_Vertical_Offset) != 0) {
								
								if ((status = PutAsciiData (tk,"Vertical_Offset", ca->vertical_offset)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 6: {
							PutTab t(&tk);
							if ((ca->mask & TKO_Character_Vertical_Offset) != 0) {
								
								if ((status = PutAsciiData (tk,"Vertical_Offset_Unit", ca->vertical_offset_units)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 7: {
							PutTab t(&tk);
							if ((ca->mask & TKO_Character_Slant) != 0) {
								
								if ((status = PutAsciiData (tk,"Slant", ca->slant)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 8: {
							PutTab t(&tk);
							if ((ca->mask & TKO_Character_Rotation) != 0) {
								
								if ((status = PutAsciiData (tk,"Rotation", ca->rotation)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 9: {
							PutTab t(&tk);
							if ((ca->mask & TKO_Character_Width_Scale) != 0) {
															
								if ((status = PutAsciiData (tk,"Width_Scale", ca->width_scale)) != TK_Normal)
									return status;
							}
							m_substage = 0;
						}   break;
					}

					++m_progress;
				}

                _W3DTK_REQUIRE_VERSION( 1210 );
            }
			m_progress = 0;
            m_stage++;
        }   nobreak;

        case 12: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
                m_stage++;
        }   nobreak;

		case 13: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Text::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Position", m_position, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            if (Opcode() == TKE_Text_With_Encoding) {
				if ((status = GetAsciiData (tk,"Encoding", m_int)) != TK_Normal)
                    return status;
				m_encoding = (unsigned char)m_int;
            }
            m_stage++;
        }   nobreak;

        case 2: {
            if ((status = GetAsciiData (tk,"Length", m_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
            if (tk.GetVersion() > 600) {        // extended text length added after 6.00
                if (m_length == 255) {                  // real length is a following int
					if ((status = GetAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                        return status;
                }
                else if (m_length == 254) {             // real length is a following short
					if ((status = GetAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                        return status;
                }
            }
            
            set_string (m_length);      // allocate space & (re)set length
            m_stage++;
        }   nobreak;

        case 4: {
            if (m_encoding == TKO_Enc_Unicode32) {
				if ((status = GetAsciiData (tk, "String",(int*) (m_string), m_length/4)) != TK_Normal)
                    return status;
            }
            else if (m_encoding == TKO_Enc_Unicode) {
				if ((status = GetAsciiData (tk, "String",(short*) (m_string), m_length/2)) != TK_Normal)
                    return status;
            }
            else {
				if ((status = GetAsciiData (tk, "String", m_string, m_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak; 

        case 5: {
            if (tk.GetVersion() >= 1002) {
				if ((status = GetAsciiHex (tk,"Options", m_options)) != TK_Normal)
                    return status;
            }
			else
				m_options = 0;
            m_stage++;
        }   nobreak;

        case 6: {
            if ((m_options & TKO_Text_Option_Region) != 0) {
				if ((status = GetAsciiData (tk,"Region_Options", m_region_options)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 7: {
            if ((m_options & TKO_Text_Option_Region) != 0) {
				if ((status = GetAsciiData (tk,"Region_Count", m_region_count)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 8: {
            if ((m_options & TKO_Text_Option_Region) != 0) {
				if ((status = GetAsciiData (tk,"Region", m_region, 3*m_region_count)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 9: {
            if ((m_options & TKO_Text_Option_Character_Attributes) != 0) {
				if ((status = GetAsciiData (tk,"Count", m_count)) != TK_Normal)
                    return status;

				m_character_attributes = new TK_Character_Attribute [m_count];
				memset (m_character_attributes, 0, m_count*sizeof(TK_Character_Attribute));
				m_progress = 0;
				m_substage = 0;
            }
            m_stage++;
        }   nobreak;

        case 10: {
            if ((m_options & TKO_Text_Option_Character_Attributes) != 0) {
				while (m_progress < m_count) {
					TK_Character_Attribute *	ca = &m_character_attributes[m_progress];

					switch (m_substage) {
						case 0: {
							if ((status = GetAsciiData (tk,"Mask", ca->mask)) != TK_Normal)
								return status;
							m_substage++;
						}   nobreak;

						case 1: {
							if ((ca->mask & TKO_Character_Name) != 0) {
								if ((status = GetAsciiData (tk,"Len", m_tmp)) != TK_Normal)
									return status;
								ca->name = new char [m_tmp+1];
								ca->name[m_tmp] = '\0';
							}
							m_substage++;
						}   nobreak;

						case 2: {
							if ((ca->mask & TKO_Character_Name) != 0) {
								if ((status = GetAsciiData (tk,"Name", ca->name, m_tmp)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 3: {
							if ((ca->mask & TKO_Character_Size) != 0) {
								if ((status = GetAsciiData (tk,"Size", ca->size)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 4: {
							if ((ca->mask & TKO_Character_Size) != 0) {
								if ((status = GetAsciiData (tk,"Size_units", ca->size_units)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 5: {
							if ((ca->mask & TKO_Character_Vertical_Offset) != 0) {
								if ((status = GetAsciiData (tk, "Vertical_Offset",ca->vertical_offset)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 6: {
							if ((ca->mask & TKO_Character_Vertical_Offset) != 0) {
								if ((status = GetAsciiData (tk, "Vertical_Offset_Unit", ca->vertical_offset_units)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 7: {
							if ((ca->mask & TKO_Character_Slant) != 0) {
								if ((status = GetAsciiData (tk, "Slant",ca->slant)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 8: {
							if ((ca->mask & TKO_Character_Rotation) != 0) {
								if ((status = GetAsciiData (tk,"Rotation", ca->rotation)) != TK_Normal)
									return status;
							}
							m_substage++;
						}   nobreak;

						case 9: {
							if ((ca->mask & TKO_Character_Width_Scale) != 0) {
								if ((status = GetAsciiData (tk,"Width_Scale", ca->width_scale)) != TK_Normal)
									return status;
							}
							m_substage = 0;
						}   break;
					}

					++m_progress;
				}
            }
			m_progress = 0;
               m_stage++;
        }   nobreak;
		
		case 11: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Font::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

	if (m_encoding == TKO_Enc_Unicode32 && tk.GetTargetVersion() < 1210)
		return TK_Normal;

    _W3DTK_REQUIRE_VERSION( 1210 );

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiHex (tk,"Type", m_type)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
            // need handling for string > 255 ?
            PutTab t(&tk);
			if ((status = PutAsciiData (tk,"Name_Length", m_name_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Name", m_name, m_name_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Encoding", (int)m_encoding)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 5: {
            // need handling for string > 255 ?
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"LookUp_Length", m_lookup_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t(&tk); 
			if (m_lookup_length > 0) {
				if ((status = PutAsciiData (tk,"LookUp", m_lookup, m_lookup_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 7: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Length", m_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 8: {
			if(m_length)
			{
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Bytes", m_bytes, m_length)) != TK_Normal)
                return status;
			}
               m_stage++;
        }   nobreak;

		case 9: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;
        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Font::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Type", m_type)) != TK_Normal)
                return status;
            if (m_type != TKO_Font_HOOPS_Stroked)
                return tk.Error ("Unknown font type");
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Name_Length", m_int)) != TK_Normal)
                return status;
            set_name (m_int);
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"Name", m_name, m_name_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			if ((status = GetAsciiData (tk,"Encoding", m_encoding)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
     		if ((status = GetAsciiData (tk,"LookUp_Length", m_lookup_length)) != TK_Normal)
                return status;
            if (m_lookup_length > 0)
                set_lookup (m_lookup_length);
            m_stage++;
        }   nobreak;

        case 5: {
            if (m_lookup_length > 0) {
				if ((status = GetAsciiData (tk,"LookUp", m_lookup, m_lookup_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 6: {
			if ((status = GetAsciiData (tk,"Length", m_length)) != TK_Normal)
                return status;
            set_bytes (m_length);
            m_stage++;
        }   nobreak;

        case 7: {
			if(m_length)
			{
			if ((status = GetAsciiData (tk,"Bytes", m_bytes, m_length)) != TK_Normal)
                return status;
			}
                m_stage++;
        }   nobreak;
		
		case 8: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

// based on the format
 

TK_Status TK_Image::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Position", m_position, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
                unsigned char       byte = (unsigned char)(m_format | m_options);
			PutTab t(&tk);
            if (m_format == TKO_Image_Invalid)
                return tk.Error ("bad (perhaps uninitialized) image format");

            if (tk.GetTargetVersion() < 1001)
                byte &= ~TKO_Image_Explicit_Size;

            if (m_name_length > 0)
                byte |= TKO_Image_Is_Named;

			
			if ((status = PutAsciiHex (tk,"Format_Options", byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if (m_name_length > 0) {
                // need handling for string > 255 ?
				if ((status = PutAsciiData (tk,"Name_Length" ,m_name_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if (m_name_length > 0) {
                if ((status = PutAsciiData (tk,"Name", m_name, m_name_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Size", m_size, 2)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t(&tk);
            if (m_compression != TKO_Compression_None &&
                m_compression != TKO_Compression_Reference &&
                m_compression != m_bytes_format)
                compress_image (tk);    // see if we can compress it, or fall back
			
            if ((status = PutAsciiData (tk,"Compression", (int)m_compression)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 7: {
			PutTab t(&tk);
            if ((int)m_compression == TKO_Compression_Reference) {
            //    unsigned short      word = (unsigned short)m_reference_length;
				
                if ((status = PutAsciiData (tk,"Reference_Length_Or_Used_Work_Area", m_reference_length)) != TK_Normal)
                    return status;
            }
            else if (m_compression != TKO_Compression_None) {
                // if compressed, we must have a size
				
                if ((status = PutAsciiData (tk,"Reference_Length_Or_Used_Work_Area", m_work_area[0].Used())) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 8: {
			PutTab t(&tk);
            if ((int)m_compression == TKO_Compression_Reference) {
                if ((status = PutAsciiData (tk,"Data", m_reference, m_reference_length)) != TK_Normal)
                    return status;
            }
            else if ((int)m_compression == TKO_Compression_None) {
                int     total = m_size[0] * m_size[1] * TK_Image_Bytes_Per_Pixel[(int)m_format];
				
                if ((status = PutAsciiData (tk,"Data",(unsigned char*)m_bytes, total)) != TK_Normal)
                    return status;
            }
            else {
                if ((status = PutAsciiData (tk,"Data", m_work_area[0].Buffer(), m_work_area[0].Used())) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 9: {
            if (m_work_area[1].Used() > 0) {
				unsigned char		byte;

				if (m_work_area[1].Used() > 1)
					byte = TKO_Compression_RLE;
				else
					byte = TKO_Compression_None;

				PutTab t(&tk);
				if ((status = PutAsciiData (tk,"Alpha_Format", (int)byte)) != TK_Normal)
					return status;
            }
            m_stage++;
        }   nobreak;

        case 10: {
            if (m_work_area[1].Used() > 1) {
				PutTab t(&tk);
                if ((status = PutAsciiData (tk,"Alpha_RLE_Length", m_work_area[1].Used())) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 11: {
            if (m_work_area[1].Used() > 0) {
				PutTab t(&tk);
                if ((status = PutAsciiData (tk,"Alpha_Data", m_work_area[1].Buffer(), m_work_area[1].Used())) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 12: {
			PutTab t(&tk);
            if ((m_options & TKO_Image_Explicit_Size) != 0 && tk.GetTargetVersion() >= 1001) {
    			if ((status = PutAsciiData (tk,"Explicit_Size", m_explicit_size, 2)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1001 );
            }
            m_stage++;
        }   nobreak;

        case 13: {
			PutTab t(&tk);
            if ((m_options & TKO_Image_Explicit_Size) != 0 && tk.GetTargetVersion() >= 1001) {
				if ((status = PutAsciiData (tk,"Explicit_Units", m_explicit_units, 2)) != TK_Normal)
                    return status;
                _W3DTK_REQUIRE_VERSION( 1001 );
            }
            m_stage++;
        }   nobreak;

        case 14: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
                 m_stage++;
        }   nobreak;

		case 15: {
			if (Tagging (tk))
			{
				if((status = Tag (tk)) != TK_Normal)
				return status;
			}
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Image::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
	
	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Position", m_position, 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
   			if ((status = GetAsciiHex (tk,"Format_Options", m_byte)) != TK_Normal)
                return status;

            m_format = (unsigned char)(m_byte & TKO_Image_Format_Mask);
            m_options = (unsigned char)(m_byte & TKO_Image_Options_Mask);

            m_stage++;
        }   nobreak;

        case 2: {
            if ((m_options & TKO_Image_Is_Named) != 0) {
           		if ((status = GetAsciiData (tk,"Name_Length", m_name_length)) != TK_Normal)
                    return status;
                set_name (m_name_length);
            }
            m_stage++;
        }   nobreak;

        case 3: {
            if ((m_options & TKO_Image_Is_Named) != 0) {
				if ((status = GetAsciiData (tk,"Name", m_name, m_name_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			if ((status = GetAsciiData (tk,"Size", m_size, 2)) != TK_Normal)
                return status;
            int     total = m_size[0] * m_size[1] * TK_Image_Bytes_Per_Pixel[(int)m_format];
            set_data (total);
            m_stage++;
        }   nobreak;

        case 5: {
			if ((status = GetAsciiData (tk,"Compression", m_int)) != TK_Normal) // Instead of member variable unsigned char m_compression I have used char temp_char coz it makes the typecasting to integer easier.- Pavan 
                return status;
			m_compression  = (unsigned char)m_int;			
            m_stage++;
        }   nobreak;

        case 6: {
			
            if (m_compression == TKO_Compression_Reference) {
				if ((status = GetAsciiData (tk,"Reference_Length_Or_Used_Work_Area", m_int)) != TK_Normal)
                    return status;
                SetReference (m_int);
            }
            else if (m_compression != TKO_Compression_None) {
				if ((status = GetAsciiData (tk,"Reference_Length_Or_Used_Work_Area", m_int)) != TK_Normal)
                    return status;
                m_work_area[0].Resize (m_int);
            }
            ++m_stage;
        }   nobreak;

        case 7: {
            if (m_compression == TKO_Compression_Reference) {
				if ((status = GetAsciiData (tk,"Data", m_reference, m_reference_length)) != TK_Normal)
                    return status;

                // generates dummy checkerboard
                if ((status = decompress_image (tk)) != TK_Normal)
                    return status;
            }
            else if (m_compression == TKO_Compression_None) {
                int     total = m_size[0] * m_size[1] * TK_Image_Bytes_Per_Pixel[(int)m_format];

				if ((status = GetAsciiImageData (tk,"Data",(unsigned char*)m_bytes, total)) != TK_Normal)
                    return status;
            }
            else {
                // read compressed data & decompress it
				if ((status = GetAsciiImageData (tk,"Data", m_work_area[0].Buffer(), m_work_area[0].Size())) != TK_Normal)
                    return status;
                m_work_area[0].Used() = m_work_area[0].Size();

				if (m_compression == TKO_Compression_JPEG &&
					(m_format == TKO_Image_RGBA || m_format == TKO_Image_BGRA)) {
					// delay decompression until we have also read extra alpha data
				}
				else {
					if ((status = decompress_image (tk)) != TK_Normal)
						return status;
				}

            }
            m_stage++;
        }   nobreak;

        case 8: {
			if (m_compression == TKO_Compression_JPEG &&
				(m_format == TKO_Image_RGBA || m_format == TKO_Image_BGRA)) {
				int		alpha_format = 0;

				if ((status = GetAsciiData (tk,"Alpha_Format", alpha_format)) != TK_Normal)
					return status;

				if (alpha_format == TKO_Compression_None)
					m_work_area[1].Resize (1);	// single alpha
				else
					m_work_area[1].Resize (2);	// RLE alpha, real size read next
			}
        }   nobreak;

        case 9: {
			if (m_work_area[1].Size() == 2) {
                int			         size;

				if ((status = GetAsciiData (tk,"Alpha_RLE_Length", size)) != TK_Normal)
					return status;

				m_work_area[1].Resize (size);
			}
        }   nobreak;

        case 10: {
			if (m_work_area[1].Size() > 0) {
				if ((status = GetAsciiImageData (tk,"Alpha_Data", m_work_area[1].Buffer(), m_work_area[1].Size())) != TK_Normal)
                    return status;
                m_work_area[1].Used() = m_work_area[1].Size();

				// now we have everything for jpeg with alpha
                if ((status = decompress_image (tk)) != TK_Normal)
                    return status;
			}
        }   nobreak;

        case 11: {
            if ((m_options & TKO_Image_Explicit_Size) != 0) {
				if ((status = GetAsciiData (tk,"Explicit_Size", m_explicit_size, 2)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 12: {
            if ((m_options & TKO_Image_Explicit_Size) != 0) {
				if ((status = GetAsciiData (tk,"Explicit_Units", m_explicit_units, 2)) != TK_Normal)
                    return status;
            }
               m_stage++;
        }   nobreak;
		
		case 13: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_Image::compress_image_ascii (BStreamFileToolkit & tk) {
    m_compression = TKO_Compression_None;
	UNREFERENCED (tk);
    return TK_Normal;
}

// based on the format
static int const thumbnail_bytes_per_pixel[] = {
    3, 4
};


TK_Status TK_Thumbnail::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if (m_format != 0 && m_format != 1)
                return tk.Error ("invalid thumbnail format");
			if ((status = PutAsciiData (tk,"Format", m_format)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
            unsigned char       size[2];
			PutTab t(&tk);
            if (m_size[0] < 1 || m_size[0] > 256 || m_size[1] < 1 || m_size[1] > 256)
                return tk.Error ("invalid thumbnail size");

            // note: 256 encodes as 0
            size[0] = (unsigned char)(m_size[0] & 0x00ff);
            size[1] = (unsigned char)(m_size[1] & 0x00ff);
			
            if ((status = PutAsciiData (tk,"Size", size, 2)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
            int     total = m_size[0] * m_size[1] * thumbnail_bytes_per_pixel[(int)m_format];
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"ThumbNail_Bytes_Per_Pixel", m_bytes, total)) != TK_Normal)
                return status;
                 m_stage++;
        }   nobreak;

		case 4: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;
        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Thumbnail::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Format", m_int)) != TK_Normal)
                return status;
			m_format = (unsigned char)m_int;
            m_stage++;
        }   nobreak;

        case 1: {
            unsigned char       size[2];

			if ((status = GetAsciiData (tk, "Size",size, 2)) != TK_Normal)
                return status;

            if ((m_size[0] = (int)size[0]) == 0) m_size[0] = 256;
            if ((m_size[1] = (int)size[1]) == 0) m_size[1] = 256;

            int     total = m_size[0] * m_size[1] * thumbnail_bytes_per_pixel[(int)m_format];

            SetBytes (total);
            m_stage++;
        }   nobreak;

        case 2: {
            int     total = m_size[0] * m_size[1] * thumbnail_bytes_per_pixel[(int)m_format];

			if ((status = GetAsciiData (tk,"ThumbNail_Bytes_Per_Pixel", m_bytes, total)) != TK_Normal)
                return status;
               m_stage++;
        }   nobreak;
		
		case 3: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_Color::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    int				length;
    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            unsigned char       byte = (unsigned char)(m_mask & 0x000000FF);
				PutTab t(&tk);
            if ((status = PutAsciiMask (tk,"Mask", byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>8) & 0x000000FF);
                if ((status = PutAsciiMask (tk,"Extended_Mask", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended_Colors) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>16) & 0x000000FF);
                if ((status = PutAsciiMask (tk,"Extended_Colors", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended2) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>24) & 0x000000FF);
                if ((status = PutAsciiMask (tk,"Extended2", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            unsigned char       byte = (unsigned char)(m_channels & 0x000000FF);
            if ((status = PutAsciiMask( tk,"Channels", byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Extended)) != 0) {
				unsigned char       byte = (unsigned char)((m_channels>>8) & 0x000000FF);
                if ((status = PutAsciiMask (tk,"Channels_Extended", byte)) != TK_Normal)
                    return status;
            }
			m_progress = 0; //prepare for next stage
            m_stage++;
        }   nobreak;

        case 7: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Diffuse)) != 0) {

				if (m_diffuse.m_name) {
					int length;
                    length = (int) strlen (m_diffuse.m_name);
						
					if ((status = PutAsciiData (tk,"Length", length)) != TK_Normal)
						return status;
					
				}
				else {
					// if no string, zero indicates by-value
					
					if ((status = PutAsciiData (tk,"Length", (int)0)) != TK_Normal)
						return status;
				}
            }
            m_stage++;
        }   nobreak;

        case 8: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Diffuse)) != 0) {
                if (m_diffuse.m_name != null) {
					
					if ((status = PutAsciiData (tk,"Diffuse_Name", m_diffuse.m_name, (int) strlen (m_diffuse.m_name))) != TK_Normal)
                        return status;
                }
                else {
					
                    if ((status = PutAsciiData (tk,"Diffuse_RGB", m_diffuse.m_rgb, 3)) != TK_Normal)
                        return status;
                }
            }
            m_stage++;
        }   nobreak;

        case 9: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Specular)) != 0) {
                length = 0;   // if no string, zero indicates by-value

				if (m_specular.m_name != null) {
					length = (int)strlen (m_specular.m_name);
					if (length > 255)
						return tk.Error(">255 specular texture not yet supported");
				}
				
                if ((status = PutAsciiData (tk,"Length", length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 10: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Specular)) != 0) {
                if (m_specular.m_name != null) {
					
                    if ((status = PutAsciiData (tk,"Specular_Name", m_specular.m_name, (int) strlen (m_specular.m_name))) != TK_Normal)
                        return status;
                }
                else {
					
                    if ((status = PutAsciiData (tk,"Specular_RGB", m_specular.m_rgb, 3)) != TK_Normal)
                        return status;
                }
            }
            m_stage++;
        }   nobreak;

        case 11: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Mirror)) != 0) {
                length = 0;   // if no string, zero indicates by-value

				if (m_mirror.m_name != null) {
					length = (int) strlen (m_mirror.m_name);
					if (length > 255)
						return tk.Error(">255 mirror texture not yet supported");
				}
				
                if ((status = PutAsciiData (tk,"Length", length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 12: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Mirror)) != 0) {
                if (m_mirror.m_name != null) {
					
                    if ((status = PutAsciiData (tk,"Mirror_Name", m_mirror.m_name, (int) strlen (m_mirror.m_name))) != TK_Normal)
                        return status;
                }
                else {
					
                    if ((status = PutAsciiData (tk,"Mirror_RGB", m_mirror.m_rgb, 3)) != TK_Normal)
                        return status;
                }
            }
            m_stage++;
        }   nobreak;

        case 13: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Transmission)) != 0) {
                length = 0;
				if (m_transmission.m_name != null) {
                    length = (int) strlen (m_transmission.m_name);
					if (length > 255)
						return tk.Error(">255 transmission texture not yet supported");
				}
				
                if ((status = PutAsciiData (tk,"Length", length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 14: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Transmission)) != 0) {
                if (m_transmission.m_name != null) {
					
                    if ((status = PutAsciiData (tk,"Transmission_Name", m_transmission.m_name, (int) strlen (m_transmission.m_name))) != TK_Normal)
                        return status;
                }
                else {
					
                    if ((status = PutAsciiData (tk,"Transmission_RGB", m_transmission.m_rgb, 3)) != TK_Normal)
                        return status;
                }
            }
            m_stage++;
        }   nobreak;

        case 15: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Emission)) != 0) {
                length = 0;   // if no string, zero indicates by-value

				if (m_emission.m_name != null) {
                    length = (int) strlen (m_emission.m_name);
					if (length > 255)
						return tk.Error(">255 emission texture not yet supported");
				}
				
                if ((status = PutAsciiData (tk,"Length", length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 16: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Emission)) != 0) {
                if (m_emission.m_name != null) {
					
                    if ((status = PutAsciiData (tk,"Emission_Name", m_emission.m_name, (int) strlen (m_emission.m_name))) != TK_Normal)
                        return status;
                }
                else {
					
                    if ((status = PutAsciiData (tk,"Emission_RGB", m_emission.m_rgb, 3)) != TK_Normal)
                        return status;
                }
            }
            m_stage++;
        }   nobreak;

        case 17: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Gloss)) != 0) {
				
                if ((status = PutAsciiData (tk,"Gloss", m_gloss)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 18: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Index)) != 0) {
				
                if ((status = PutAsciiData (tk,"Index", m_index)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 19: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Environment)) != 0) {
				length = 0;

				if (m_environment.m_name != null) {
                    length = (int) strlen (m_environment.m_name);
					if (length > 255)
						return tk.Error(">255 environment texture not yet supported");
				}
				
                if ((status = PutAsciiData (tk,"Length", length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 20: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Environment)) != 0) {
				
                if ((status = PutAsciiData (tk,"Environment", m_environment.m_name, (int) strlen (m_environment.m_name))) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 21: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Bump)) != 0) {
				length = 0;

				if (m_bump.m_name != null) {
                    length = (int) strlen (m_bump.m_name);
					if (length > 255)
						return tk.Error(">255 bump texture not yet supported");
				}
				
                if ((status = PutAsciiData (tk,"Length", length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 22: {
			PutTab t(&tk);
            if ((m_channels & (1 << TKO_Channel_Bump)) != 0) {
				
                if ((status = PutAsciiData (tk,"Bump", m_bump.m_name, (int) strlen (m_bump.m_name))) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

		case 23: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Color::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Mask", m_mask)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            if ((m_mask & TKO_Geo_Extended) != 0) {
             
				if ((status = GetAsciiHex (tk,"Extended_Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 8;
            }
            m_stage++;
        }   nobreak;

        case 2: {
            if ((m_mask & TKO_Geo_Extended_Colors) != 0) {
                
				if ((status = GetAsciiHex (tk,"Extended_Colors", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 16;
            }
            m_stage++;
        }   nobreak;

        case 3: {
            if ((m_mask & TKO_Geo_Extended2) != 0) {
                
				if ((status = GetAsciiHex (tk,"Extended2", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 24;
            }
            m_stage++;
        }   nobreak;

           
        case 4: {
			if ((status = GetAsciiHex (tk,"Channels", m_byte)) != TK_Normal)
                return status;
			m_channels = m_byte;
            m_stage++;
        }   nobreak;

        case 5: {
            if ((m_channels & (1 << TKO_Channel_Extended)) != 0) {
				if ((status = GetAsciiHex (tk,"Channels_Extended", m_byte)) != TK_Normal)
                    return status;
                m_channels |= m_byte << 8;
            }
            m_stage++;
        }   nobreak;

        case 6: {
            if ((m_channels & (1 << TKO_Channel_Diffuse)) != 0) {
       
				if ((status = GetAsciiData (tk,"Length", m_progress)) != TK_Normal)
						return status;
	
				//(ab)use m_progress as a length here
				if (m_progress != 0)
                	set_channel_name (m_diffuse, m_progress);  
            }
            m_stage++;
        }   nobreak;

        case 7: {
            if ((m_channels & (1 << TKO_Channel_Diffuse)) != 0) {
                if (m_progress != 0) {
					if ((status = GetAsciiData (tk,"Diffuse_Name", m_diffuse.m_name, m_progress)) != TK_Normal)
                        return status;
                }
                else {

					if ((status = GetAsciiData (tk,"Diffuse_RGB", m_diffuse.m_rgb, 3)) != TK_Normal)
                        return status;
                }
            }
            m_stage++;
        }   nobreak;

        case 8: {
            if ((m_channels & (1 << TKO_Channel_Specular)) != 0) {
				if ((status = GetAsciiData (tk,"Length", m_progress)) != TK_Normal)
                    return status;

                if (m_progress != 0)
                    set_channel_name (m_specular, m_progress);
            }
            m_stage++;
        }   nobreak;

        case 9: {
            if ((m_channels & (1 << TKO_Channel_Specular)) != 0) {
                if (m_progress != 0) {
					if ((status = GetAsciiData (tk,"Specular_Name", m_specular.m_name, m_progress)) != TK_Normal)
                        return status;
                }
                else {
       
					if ((status = GetAsciiData (tk,"Specular_RGB", m_specular.m_rgb, 3)) != TK_Normal)
                        return status;
                }
            }
            m_stage++;
        }   nobreak;

        case 10: {
            if ((m_channels & (1 << TKO_Channel_Mirror)) != 0) {
				if ((status = GetAsciiData (tk,"Length", m_progress)) != TK_Normal)
                    return status;

                if (m_progress != 0)
                    set_channel_name (m_mirror, m_progress);
            }
            m_stage++;
        }   nobreak;

        case 11: {
            if ((m_channels & (1 << TKO_Channel_Mirror)) != 0) {
                if (m_progress != 0) {
					if ((status = GetAsciiData (tk,"Mirror_Name", m_mirror.m_name, m_progress)) != TK_Normal)
                        return status;
                }
                else {
           		if ((status = GetAsciiData (tk,"Mirror_RGB", m_mirror.m_rgb, 3)) != TK_Normal)
                        return status;

                      }
            }
            m_stage++;
        }   nobreak;

        case 12: {
            if ((m_channels & (1 << TKO_Channel_Transmission)) != 0) {
				if ((status = GetAsciiData (tk,"Length", m_progress)) != TK_Normal)
                    return status;

                if (m_progress != 0)
                    set_channel_name (m_transmission, m_progress);
            }
            m_stage++;
        }   nobreak;

        case 13: {
            if ((m_channels & (1 << TKO_Channel_Transmission)) != 0) {
                if (m_progress != 0) {
					if ((status = GetAsciiData (tk,"Transmission_Name", m_transmission.m_name, m_progress)) != TK_Normal)
                        return status;
                }
                else {
      			if ((status = GetAsciiData (tk,"Transmission_RGB", m_transmission.m_rgb, 3)) != TK_Normal)
                        return status;

                   }
            }
            m_stage++;
        }   nobreak;

        case 14: {
            if ((m_channels & (1 << TKO_Channel_Emission)) != 0) {
				if ((status = GetAsciiData (tk,"Length", m_progress)) != TK_Normal)
                    return status;

                if (m_progress != 0)
                    set_channel_name (m_emission, m_progress);
            }
            m_stage++;
        }   nobreak;

        case 15: {
            if ((m_channels & (1 << TKO_Channel_Emission)) != 0) {
                if (m_progress != 0) {
					if ((status = GetAsciiData (tk,"Emission_Name", m_emission.m_name, m_progress)) != TK_Normal)
                        return status;
                }
                else {
           		if ((status = GetAsciiData (tk,"Emission_RGB", m_emission.m_rgb, 3)) != TK_Normal)
                        return status;
                }
            }
            m_stage++;
        }   nobreak;

        case 16: {
            if ((m_channels & (1 << TKO_Channel_Gloss)) != 0) {
				if ((status = GetAsciiData (tk,"Gloss", m_gloss)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 17: {
            if ((m_channels & (1 << TKO_Channel_Index)) != 0) {
				if ((status = GetAsciiData (tk,"Index", m_index)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 18: {
            if ((m_channels & (1 << TKO_Channel_Environment)) != 0) {
				if ((status = GetAsciiData (tk,"Length", m_progress)) != TK_Normal)
                    return status;

                set_channel_name (m_environment, m_progress);
            }
            m_stage++;
        }   nobreak;

        case 19: {
            if ((m_channels & (1 << TKO_Channel_Environment)) != 0) {
				if ((status = GetAsciiData (tk,"Environment", m_environment.m_name, m_progress)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 20: {
            if ((m_channels & (1 << TKO_Channel_Bump)) != 0) {
				if ((status = GetAsciiData (tk,"Length", m_progress)) != TK_Normal)
                    return status;

                set_channel_name (m_bump, m_progress);
            }
            m_stage++;
        }   nobreak;

        case 21: {
            if ((m_channels & (1 << TKO_Channel_Bump)) != 0) {
				if ((status = GetAsciiData (tk,"Bump", m_bump.m_name, m_progress)) != TK_Normal)
                    return status;
            }
               m_stage++;
        }   nobreak;
		
		case 22: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Color_RGB::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            unsigned char       byte = (unsigned char)(m_mask & 0x000000FF);
               PutTab t(&tk);
            if ((status = PutAsciiMask (tk,"Mask", byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>8) & 0x000000FF);
                    
                if ((status = PutAsciiMask (tk,"Mask", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended_Colors) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>16) & 0x000000FF);
				
                if ((status = PutAsciiMask (tk,"Mask",byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended2) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>24) & 0x000000FF);
				
                if ((status = PutAsciiMask (tk,"Mask",byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"RGB", m_rgb, 3)) != TK_Normal)
                return status;
                
			m_stage++;
        }   nobreak;

		case 6: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;
        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Color_RGB::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Mask" ,m_mask)) != TK_Normal)
					return status;
			
            m_stage++;
        }   nobreak;

        case 1: {
            if ((m_mask & TKO_Geo_Extended) != 0) {
				if ((status = GetAsciiHex (tk,"Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 8;
            }
            m_stage++;
        }   nobreak;

        case 2: {
			if ((m_mask & TKO_Geo_Extended_Colors) != 0) {
				if ((status = GetAsciiHex (tk,"Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 16;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			if ((m_mask & TKO_Geo_Extended2) != 0) {
				if ((status = GetAsciiHex (tk,"Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 24;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			if ((status = GetAsciiData (tk,"RGB", m_rgb, 3)) != TK_Normal)
                return status;

              m_stage++;
        }   nobreak;
		
		case 5: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


////////////////////////////////////////


TK_Status TK_Color_By_Value::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            unsigned char       byte = (unsigned char)(m_mask & 0x000000FF);
			PutTab t(&tk);
            if ((status = PutAsciiMask (tk,"Mask", byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>8) & 0x000000FF);
				
                if ((status = PutAsciiMask (tk,"Mask", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended_Colors) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>16) & 0x000000FF);
				
                if ((status = PutAsciiMask (tk,"Mask",byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended2) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>24) & 0x000000FF);
				
                if ((status = PutAsciiMask (tk,"Mask",byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Space", m_space)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Value", m_value, 3)) != TK_Normal)
                return status;
              
			m_stage++;
        }   nobreak;

		case 7: {
			PutTab t(&tk);
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Color_By_Value::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Mask", m_mask)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            if ((m_mask & TKO_Geo_Extended) != 0) {
				if ((status = GetAsciiHex (tk,"Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 8;
            }
            m_stage++;
        }   nobreak;

        case 2: {
            if ((m_mask & TKO_Geo_Extended_Colors) != 0) {
				if ((status = GetAsciiHex (tk,"Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 16;
            }
            m_stage++;
        }   nobreak;

        case 3: {
            if ((m_mask & TKO_Geo_Extended2) != 0) {
				if ((status = GetAsciiHex (tk,"Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 24;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			if ((status = GetAsciiData (tk,"Space", m_byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 5: {
			if ((status = GetAsciiData (tk,"Value", m_value, 3)) != TK_Normal)
                return status;
             m_stage++;
        }   nobreak;
		
		case 6: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


////////////////////////////////////////

TK_Status TK_Color_By_Index::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            unsigned char       byte = (unsigned char)(m_mask & 0x000000FF);
              PutTab t(&tk);
            if ((status = PutAsciiMask (tk,"Mask",byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>8) & 0x000000FF);
                       
                if ((status = PutAsciiMask (tk,"Mask",byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended_Colors) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>16) & 0x000000FF);
                      
                if ((status = PutAsciiMask (tk,"Mask", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended2) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>24) & 0x000000FF);
                      
                if ((status = PutAsciiMask (tk,"Mask", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
            switch (Opcode()) {
                case TKE_Color_By_Index: {
					PutTab t(&tk);
                    unsigned char       byte = (unsigned char)m_index;
                    if ((status = PutAsciiData (tk,"Index", m_index)) != TK_Normal)
                        return status;

                    m_index = byte;
                }   break;

                case TKE_Color_By_Index_16: {
                    unsigned short      word = (unsigned short)m_index;
					PutTab t(&tk);
                    if ((status = PutAsciiData (tk,"Index", m_index)) != TK_Normal)
                        return status;

                    m_index = word;
                }   break;

                default:
                    return tk.Error();
            }

            m_stage++;
        }   nobreak;

		case 6: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Color_By_Index::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Mask", m_mask)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            if ((m_mask & TKO_Geo_Extended) != 0) {
				if ((status = GetAsciiHex (tk,"Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 8;
            }
            m_stage++;
        }   nobreak;

        case 2: {
            if ((m_mask & TKO_Geo_Extended_Colors) != 0) {
				if ((status = GetAsciiHex (tk,"Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 16;
            }
            m_stage++;
        }   nobreak;

        case 3: {
            if ((m_mask & TKO_Geo_Extended2) != 0) {
				if ((status = GetAsciiHex (tk,"Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 24;
            }
            m_stage++;
        }   nobreak;

        case 4: {
            switch (Opcode()) {
                case TKE_Color_By_Index: {
					if ((status = GetAsciiData (tk,"Index", m_index)) != TK_Normal)
                        return status;
                }   break;

                case TKE_Color_By_Index_16: {
					if ((status = GetAsciiData (tk,"Index", m_index)) != TK_Normal)
                        return status;
                }   break;

                default:
                    return tk.Error();
            }

              m_stage++;
        }   nobreak;
		
		case 5: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


////////////////////////////////////////

TK_Status TK_Color_By_FIndex::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            unsigned char       byte = (unsigned char)(m_mask & 0x000000FF);
                 PutTab t(&tk);
            if ((status = PutAsciiMask (tk,"Mask",byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>8) & 0x000000FF);
                       
                if ((status = PutAsciiMask (tk,"Mask",byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended_Colors) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>16) & 0x000000FF);
				
                if ((status = PutAsciiMask (tk,"Mask", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended2) != 0) {
                unsigned char       byte = (unsigned char)((m_mask>>24) & 0x000000FF);
				
                if ((status = PutAsciiMask (tk,"Mask", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Index", m_index)) != TK_Normal)
                return status;

              m_stage++;
        }   nobreak;

		case 6: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Color_By_FIndex::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Mask", m_mask)) != TK_Normal)
                return status;
        }   nobreak;

        case 1: {
            if ((m_mask & TKO_Geo_Extended) != 0) {
				if ((status = GetAsciiHex (tk,"Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 8;
            }
            m_stage++;
        }   nobreak;

        case 2: {
            if ((m_mask & TKO_Geo_Extended_Colors) != 0) {
				if ((status = GetAsciiHex (tk,"Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 16;
            }
            m_stage++;
        }   nobreak;

        case 3: {
            if ((m_mask & TKO_Geo_Extended2) != 0) {
				if ((status = GetAsciiHex (tk,"Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 24;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			if ((status = GetAsciiData (tk,"Index", m_index)) != TK_Normal)
                return status;
                m_stage++;
        }   nobreak;
		
		case 5: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


////////////////////////////////////////

TK_Status TK_Color_Map::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Format", (int)m_format)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Length", m_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
             if ((status = PutAsciiData (tk,"Values", m_values, 3*m_length)) != TK_Normal)
                return status;

              m_stage++;
        }   nobreak;

		case 4: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;

		
        default:
            return tk.Error();
    }
	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Color_Map::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Format", m_format)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Length", m_length)) != TK_Normal)
                return status;
            if (!validate_count (m_length, 1<<16))
                return tk.Error("bad Color Map length");
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"Values", m_values, 3*m_length)) != TK_Normal)
                return status;

               m_stage++;
        }   nobreak;
		
		case 3: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_Callback::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if (m_length > 0) {
                if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if (m_length > 0) {
                // need code here to handle length > 65535 ?
                short               word = (short)m_length;
				
                if ((status = PutAsciiData (tk,"Length", word)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if (m_length > 0) {
				
                if ((status = PutAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
                    return status;
            }
               
			m_stage++;
        }   nobreak;

		case 3: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Callback::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
    short           word;

    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Length", word)) != TK_Normal)
                return status;
            set_callback ((int)word);
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
                return status;
                m_stage++;
        }   nobreak;
		
		case 2: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}



TK_Status TK_Rendering_Options::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
    int             mask = m_mask[0], emask = m_mask[1];
    int             nurbs_mask = m_nurbs_options_mask;
    int             hlr_mask = m_hlr_options;


    //
    // these are tied to the constraint check below
    //
    if (emask & (TKO_Rendo_Mask_Transform|TKO_Rendo_Image_Scale))
    {
        _W3DTK_REQUIRE_VERSION( 1001 );
    }
    if (emask & (TKO_Rendo_Mask_Transform|TKO_Rendo_Simple_Shadow))
    {
        _W3DTK_REQUIRE_VERSION( 1002 );
    }
    if (hlr_mask & (TKO_Hidden_Line_Extended2_Mask|TKO_Hidden_Line_Extended2))
    {
        _W3DTK_REQUIRE_VERSION( 1150 );
    }
    if (emask & TKO_Rendo_Geometry_Options)
    {
        _W3DTK_REQUIRE_VERSION( 1205 );
    }
    if (emask & TKO_Rendo_Image_Tint)
    {
        _W3DTK_REQUIRE_VERSION( 1210 );
    }

	PutTab t0(&tk);


    if (tk.GetTargetVersion() < 650) {
        // these options added in 6.50
        mask &= ~(TKO_Rendo_Buffer_Options          | TKO_Rendo_Hidden_Line_Options     |
                  TKO_Rendo_LOD                     | TKO_Rendo_LOD_Options             |
                  TKO_Rendo_NURBS_Curve_Options     | TKO_Rendo_NURBS_Surface_Options   |
                  TKO_Rendo_Stereo                  | TKO_Rendo_Stereo_Separation       |
                  TKO_Rendo_Extended);
        emask = 0;
    }
    if (tk.GetTargetVersion() < 710) {
        emask &= ~(TKO_Rendo_Transparency_Style | TKO_Rendo_Transparency_Hardware);
    }
    if (tk.GetTargetVersion() < 806) {
        emask &= ~(TKO_Rendo_Cut_Geometry|TKO_Rendo_Depth_Range);
        nurbs_mask &= ~(TKO_NURBS_Surface_Max_Facet_Width|
                        TKO_NURBS_Extended|
                        TKO_NURBS_Surface_Max_Facet_Angle|
                        TKO_NURBS_Surface_Max_Facet_Deviation|
                        TKO_NURBS_Surface_Max_Trim_Curve_Deviation);
    }
    if (tk.GetTargetVersion() < 1001) {
        emask &= ~(TKO_Rendo_Mask_Transform|TKO_Rendo_Image_Scale);
    }
    if (tk.GetTargetVersion() < 1002) {
        emask &= ~(TKO_Rendo_Mask_Transform|TKO_Rendo_Simple_Shadow);
    }
    if (tk.GetTargetVersion() < 1150) {
        hlr_mask &= ~(TKO_Hidden_Line_Extended2_Mask|TKO_Hidden_Line_Extended2);
    }
    if (tk.GetTargetVersion() < 1205) {
        emask &= ~(TKO_Rendo_Geometry_Options);
    }
	if (tk.GetTargetVersion() < 1210) {
        emask &= ~(TKO_Rendo_Image_Tint);
    }
	if (tk.GetTargetVersion() < 1310) {
        emask &= ~(TKO_Rendo_Transparency_Options);
    }
	if (tk.GetTargetVersion() < 1335) {
		emask &= ~(TKO_Rendo_General_Displacement);
	}

	if (tk.GetTargetVersion() < 1405) {
		emask &= ~(TKO_Rendo_Join_Cutoff_Angle);
	}

    if (emask == 0)
        mask &= ~TKO_Rendo_Extended;

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_progress = 0;     // make sure
            m_stage++;
        }   nobreak;


        case 1: {
			PutTab t1(&tk);
            if ((status = PutAsciiMask (tk,"Mask", mask)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t1(&tk);
            if ((mask & TKO_Rendo_Extended) != 0) {
				
                if ((status = PutAsciiMask (tk,"Extended_Mask", emask)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t1(&tk);
            if ((status = PutAsciiData (tk,"Value", m_value[0])) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t1(&tk);
            if ((mask & TKO_Rendo_Extended) != 0) {
				
                if ((status = PutAsciiData (tk,"Extended_Value", m_value[1])) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;


        case 5: {
			PutTab t1(&tk);
            if ((mask & TKO_Rendo_Any_HSR) != 0) {
				
                if ((status = PutAsciiData (tk,"HSR", (int)m_hsr)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t1(&tk);
            if ((mask & TKO_Rendo_TQ) != 0) {
				
                if ((status = PutAsciiData (tk,"TQ", (int)m_tq)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 7: {
			PutTab t1(&tk);
            if ((mask & m_value[0] & TKO_Rendo_Face_Displacement) != 0) {
                
				if ((status = PutAsciiData (tk,"Face_Displacement", m_face_displacement)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        // Attribute Locks
        case 8: {
			PutTab t1(&tk);
            if ((mask & m_value[0] & TKO_Rendo_Attribute_Lock) != 0) {
                
				if ((status = PutAsciiData (tk,"Lock_Mask", m_lock_mask)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 9: {
			PutTab t1(&tk);
            if ((mask & m_value[0] & TKO_Rendo_Attribute_Lock) != 0) {
				
                if ((status = PutAsciiData (tk,"Lock_Value", m_lock_value)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 10: {
			PutTab t1(&tk);
            if ((mask & m_value[0] & TKO_Rendo_Attribute_Lock) != 0 &&
                (m_lock_mask & m_lock_value & TKO_Lock_Color) != 0) {
				
                if ((status = PutAsciiMask (tk,"Lock_Color_Mask", m_lock_color_mask)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

		case 11: {
			PutTab t1(&tk);
			if ((mask & m_value[0] & TKO_Rendo_Attribute_Lock) != 0 &&
				(m_lock_mask & m_lock_value & TKO_Lock_Color) != 0) {
					if (tk.GetTargetVersion() <= 1100) {
						if ((status = PutAsciiData (tk,"Lock_Color_Value", m_lock_color_value)) != TK_Normal)
							return status;
					}
                    else {
                        //
                        // this follows from the above check right?
                        //
                        _W3DTK_REQUIRE_VERSION( 1101 );

                        switch (m_progress) {
						case 0:{
							PutTab t(&tk);
							if ((status = PutAsciiData (tk,"Lock_Color_Value", m_lock_color_value)) != TK_Normal)
								return status;
							m_progress++;
							   }
						case 1:{
							PutTab t(&tk);
							if ((m_lock_color_value & (TKO_Geo_Face|TKO_Geo_Front)) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Face_Mask", m_lock_color_face_mask)) != TK_Normal)
									return status;
							m_progress++;
							   }
						case 2:{
							PutTab t(&tk);
							if ((m_lock_color_value & (TKO_Geo_Face|TKO_Geo_Front)) != 0)

								if ((status = PutAsciiData (tk,"Lock_Color_Face_Value", m_lock_color_face_value)) != TK_Normal)
									return status;
							m_progress++;
							   }
						case 3:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Back) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Back_Mask", m_lock_color_back_mask)) != TK_Normal)
									return status;
							m_progress++;
							   }
						case 4:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Back) != 0)

								if ((status = PutAsciiData (tk,"Lock_Color_Back_Value", m_lock_color_back_value)) != TK_Normal)
									return status;
							m_progress++;
							   }
						case 5:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Edge) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Edge_Mask", m_lock_color_edge_mask)) != TK_Normal)
									return status;
							m_progress++;
							   }
						case 6:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Edge) != 0)

								if ((status = PutAsciiData (tk,"Lock_Color_Edge_Value", m_lock_color_edge_value)) != TK_Normal)
									return status;
							m_progress++;
							   }
						case 7:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Line) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Line_Mask", m_lock_color_line_mask)) != TK_Normal)
									return status;
							m_progress++;
							   }
						case 8:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Line) != 0)

								if ((status = PutAsciiData (tk, "Lock_Color_Line_Value",m_lock_color_line_value)) != TK_Normal)
									return status;
							m_progress++;
							   }
						case 9:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Text) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Text_Mask", m_lock_color_text_mask)) != TK_Normal)
									return status;
							m_progress++;
							   } 
						case 10:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Text) != 0)

								if ((status = PutAsciiData (tk,"Lock_Color_Text_Value", m_lock_color_text_value)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 11:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Marker) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Marker_Mask", m_lock_color_marker_mask)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 12:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Marker) != 0)

								if ((status = PutAsciiData (tk,"Lock_Color_Marker_Value", m_lock_color_marker_value)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 13:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Vertex) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Vertex_Mask", m_lock_color_vertex_mask)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 14:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Vertex) != 0)

								if ((status = PutAsciiData (tk,"Lock_Color_Vertex_Value", m_lock_color_vertex_value)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 15:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Window) != 0)

								if ((status = PutAsciiMask (tk, "Lock_Color_Window_Mask",m_lock_color_window_mask)) != TK_Normal)
									return status;
							m_progress++;
								}                 
						case 16:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Window) != 0)

								if ((status = PutAsciiData (tk,"Lock_Color_Window_Value", m_lock_color_window_value)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 17:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Face_Contrast) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Face_Contrast_Mask", m_lock_color_face_contrast_mask)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 18:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Face_Contrast) != 0)

								if ((status = PutAsciiData (tk, "Lock_Color_Face_Contrast_Value",m_lock_color_face_contrast_value)) != TK_Normal)
									return status;
							m_progress++;
								}

						case 19:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Edge_Contrast) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Edge_Contrast_Mask", m_lock_color_edge_contrast_mask)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 20:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Edge_Contrast) != 0)

								if ((status = PutAsciiData (tk,"Lock_Color_Edge_Contrast_Value", m_lock_color_edge_contrast_value)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 21:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Line_Contrast) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Line_Contrast_Mask", m_lock_color_line_contrast_mask)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 22:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Line_Contrast) != 0)

								if ((status = PutAsciiData (tk,"Lock_Color_Line_Contrast_Value", m_lock_color_line_contrast_value)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 23:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Text_Contrast) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Text_Contrast_Mask", m_lock_color_text_contrast_mask)) != TK_Normal)
									return status;
							m_progress++;
								}       
						case 24:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Text_Contrast) != 0)

								if ((status = PutAsciiData (tk,"Lock_Color_Text_Contrast_Value", m_lock_color_text_contrast_value)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 25:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Marker_Contrast) != 0)

								if ((status = PutAsciiMask (tk, "Lock_Color_Marker_Contrast_Mask",m_lock_color_marker_contrast_mask)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 26:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Marker_Contrast) != 0)

								if ((status = PutAsciiData (tk,"Lock_Color_Marker_Contrast_Value", m_lock_color_marker_contrast_value)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 27:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Vertex_Contrast) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Vertex_Contrast_Mask", m_lock_color_vertex_contrast_mask)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 28:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Vertex_Contrast) != 0)

								if ((status = PutAsciiData (tk, "Lock_Color_Vertex_Contrast_Value",m_lock_color_vertex_contrast_value)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 29:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Window_Contrast) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Window_Contrast_Mask", m_lock_color_window_contrast_mask)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 30:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Window_Contrast) != 0)

								if ((status = PutAsciiData (tk, "Lock_Color_Window_Contrast_Value",m_lock_color_window_contrast_value)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 31:{
							if (tk.GetTargetVersion() < 1550)
								m_lock_value &= TKO_Geo_Simple_Reflection;

							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Simple_Reflection) != 0)
								if ((status = PutAsciiMask (tk,"Lock_Color_Simple_Reflection_Mask", m_lock_color_simple_reflection_mask)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 32:{         
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Simple_Reflection) != 0)

								if ((status = PutAsciiData (tk, "Lock_Color_Simple_Reflection_Value",m_lock_color_simple_reflection_value)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 33:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Cut_Face) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Cut_Face_Mask", m_lock_color_cut_face_mask)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 34:{         
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Cut_Face) != 0)

								if ((status = PutAsciiData (tk, "Lock_Color_Cut_Face_Value",m_lock_color_cut_face_value)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 35:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Cut_Edge) != 0)

								if ((status = PutAsciiMask (tk,"Lock_Color_Cut_Edge_Mask", m_lock_color_cut_edge_mask)) != TK_Normal)
									return status;
							m_progress++;
								}
						case 36:{
							PutTab t(&tk);
							if ((m_lock_color_value & TKO_Geo_Cut_Edge) != 0)

								if ((status = PutAsciiData (tk,"Lock_Color_Cut_Edge_Value", m_lock_color_cut_edge_value)) != TK_Normal)
									return status;
							m_progress = 0;
							break;
								}
						default:
							return tk.Error ("lost track processing color locks");
                        }
                    }
                }
			    m_stage++;
        } nobreak;
        case 12: {
            if ((mask & m_value[0] & TKO_Rendo_Attribute_Lock) != 0 &&
                (m_lock_mask & m_lock_value & TKO_Lock_Visibility) != 0) {
				switch (m_progress) {
				case 0:{
								PutTab t(&tk);
								if ((status = PutAsciiMask (tk,"Lock_Visibility_Mask", m_lock_visibility_mask)) != TK_Normal)
									return status;
								m_progress++;
					   }
				case 1:{
								PutTab t(&tk);
								if ((status = PutAsciiMask (tk,"Lock_Visibility_Mask" , m_lock_visibility_value)) != TK_Normal)
									return status;
								m_progress = 0;
					   }        break;

							default:
								return tk.Error ("lost track processing visibility locks");
						}
            }
            m_stage++;
        }   nobreak;


        case 13: {
			PutTab t1(&tk);
            if ((mask & m_value[0] & TKO_Rendo_Fog) != 0) {
				
                if ((status = PutAsciiData (tk,"Fog_Limits", m_fog_limits, 2)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 14: {
			PutTab t1(&tk);
            if ((mask & m_value[0] & TKO_Rendo_Debug) != 0) {
				
                if ((status = PutAsciiData (tk,"Rendering_Options_Debug", m_debug)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;


        // Note, these check mask instead of m_mask for version checking
        case 15: {
			PutTab t1(&tk);
            if ((mask & m_value[0] & TKO_Rendo_Stereo_Separation) != 0) {
				
                if ((status = PutAsciiData (tk,"Stereo_Seperation", m_stereo_separation)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        // sfb options
        case 16: {
            if ((mask & m_value[0] & TKO_Rendo_Buffer_Options) != 0) {
                switch (m_progress) {
					case 0:{
						PutTab t(&tk);
                        if ((status = PutAsciiMask (tk,"Buffer_Options_Mask", m_buffer_options_mask)) != TK_Normal)
                            return status;
                        m_progress++;
							}
					case 1:{
						PutTab t(&tk);
                        if ((status = PutAsciiData (tk,"Buffer_Options_Value", m_buffer_options_value)) != TK_Normal)
                            return status;
                        m_progress++;
							}
					case 2:{
			             PutTab t(&tk);
                        if ((m_buffer_options_mask & m_buffer_options_value & TKO_Buffer_Size_Limit) != 0) {
							
                            if ((status = PutAsciiData (tk,"Buffer_Size_Limit", m_buffer_size_limit)) != TK_Normal)
                                return status;
							}
                        m_progress = 0;
					}
				}
            }
            m_stage++;
        }   nobreak;

        // hlr options
        case 17: {
            if ((mask & m_value[0] & TKO_Rendo_Hidden_Line_Options) != 0) {
                switch (m_progress) {
                    case 0: {
                        unsigned char       byte = (unsigned char)(hlr_mask & 0xff);
						PutTab t(&tk);
                        if ((status = PutAsciiMask (tk,"HLR_Mask", byte)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 1: {
						PutTab t(&tk);
                        if ((hlr_mask & TKO_Hidden_Line_Extended) != 0) {
                            unsigned char       byte = (unsigned char)(hlr_mask >> 8);
							
                            if ((status = PutAsciiMask (tk,"HLR_Mask", byte)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 2: {
						PutTab t(&tk);
                        if ((hlr_mask & TKO_Hidden_Line_Extended2) != 0) {
                            unsigned short      word = (unsigned short)(hlr_mask >> 16);
							
                            if ((status = PutAsciiData (tk,"HLR_Mask", word)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 3: {
						PutTab t(&tk);
                        if ((hlr_mask & TKO_Hidden_Line_Pattern) != 0) {
							
                            if ((status = PutAsciiData (tk,"HLR_Line_Pattern", m_hlr_line_pattern)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 4: {
						PutTab t(&tk);
                        if ((hlr_mask & TKO_Hidden_Line_Dim_Factor) != 0) {
							
                            if ((status = PutAsciiData (tk,"HLR_Dim_Factor", m_hlr_dim_factor)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 5: {
						PutTab t(&tk);
                        if ((hlr_mask & TKO_Hidden_Line_Face_Displacement) != 0) {
							
                            if ((status = PutAsciiData (tk,"HLR_Face_Displacement", m_hlr_face_displacement)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 6: {
						PutTab t(&tk);
                        if ((hlr_mask & TKO_Hidden_Line_Color) != 0) {
							
                            if ((status = PutAsciiData (tk,"Hidden_Line_Color", m_hlr_color, 3)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 7: {
						PutTab t(&tk);
                        if ((hlr_mask & TKO_Hidden_Line_Weight) != 0) {
							
                            if ((status = PutAsciiData (tk,"HLR_Weight", m_hlr_weight)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 8: {
						PutTab t(&tk);
                        if ((hlr_mask & TKO_Hidden_Line_Weight) != 0 &&
                            m_hlr_weight != -1.0f) {
								
                            if ((status = PutAsciiData (tk,"HLR_Weight_Units", m_hlr_weight_units)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 9: {
						PutTab t(&tk);
                        if ((hlr_mask & TKO_Hidden_Line_HSR_Algorithm) != 0) {
							
                            if ((status = PutAsciiData (tk,"HLR_HSR_Algorithm", m_hlr_hsr_algorithm)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                    }
                }
            }
            m_stage++;
        }   nobreak;


        // NURBS curve and surface options
        case 18: {
            if (mask & TKO_Rendo_NURBS_Options) {
                switch (m_progress) {
                    case 0: {
                        unsigned char       byte = (unsigned char)(nurbs_mask & 0xff);
						PutTab t(&tk);
                        if ((status = PutAsciiMask (tk,"Nurbs_Mask", byte)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 1: {
						PutTab t(&tk);
                        if ((nurbs_mask & TKO_NURBS_Extended)) {
                            unsigned char       byte = (unsigned char)((m_nurbs_options_mask>>8) & 0xff);
							
                            if ((status = PutAsciiMask (tk,"Nurbs_Options_Mask", byte)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 2: {
                        unsigned char       byte = (unsigned char)(m_nurbs_options_value & 0xff);
						PutTab t(&tk);
                        if ((status = PutAsciiData (tk,"Nurbs_Options_Value", (int) byte)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 3: {
						PutTab t(&tk);
                        if ((nurbs_mask & TKO_NURBS_Extended)) {
                            unsigned char       byte = (unsigned char)((m_nurbs_options_value>>8) & 0xff);
							
                            if ((status = PutAsciiData (tk,"Nurbs_Options_Value", (int) byte)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 4: {
						PutTab t(&tk);
                        if ((mask & m_value[0] & TKO_Rendo_NURBS_Curve_Options) &&
                            (nurbs_mask & m_nurbs_options_value & TKO_NURBS_Curve_Budget)) {
								
                            if ((status = PutAsciiData (tk,"Curve_Budget", m_curve_budget)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 5: {
						PutTab t(&tk);
                        if ((mask & m_value[0] & TKO_Rendo_NURBS_Curve_Options) != 0 &&
                            (nurbs_mask & m_nurbs_options_value & TKO_NURBS_Curve_Continued_Budget) != 0) {
								
                            if ((status = PutAsciiData (tk,"Curve_Continued_Budget", m_curve_continued_budget)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 6: {
						PutTab t(&tk);
                        if ((mask & m_value[0] & TKO_Rendo_NURBS_Surface_Options) != 0 &&
                            (nurbs_mask & m_nurbs_options_value & TKO_NURBS_Surface_Budget) != 0) {
								
                            if ((status = PutAsciiData (tk,"Surface_Budget", m_surface_budget)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 7: {
						PutTab t(&tk);
                        if ((mask & m_value[0] & TKO_Rendo_NURBS_Surface_Options) != 0 &&
                            (nurbs_mask & m_nurbs_options_value & TKO_NURBS_Surface_Trim_Budget) != 0) {
								
                            if ((status = PutAsciiData (tk,"Surface_Trim_Budget", m_surface_trim_budget)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 8: {
						PutTab t(&tk);
                        if ((mask & m_value[0] & TKO_Rendo_NURBS_Surface_Options) != 0 &&
                            (nurbs_mask & m_nurbs_options_value & TKO_NURBS_Surface_Max_Trim_Curve_Deviation) != 0) {
								
                            if ((status = PutAsciiData (tk,"Surface_Max_Trim_Curve_Deviation", m_surface_max_trim_curve_deviation)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 9: {
						PutTab t(&tk);
                        if ((mask & m_value[0] & TKO_Rendo_NURBS_Surface_Options) != 0 &&
                            (nurbs_mask & m_nurbs_options_value & TKO_NURBS_Surface_Max_Facet_Angle) != 0) {
								
                            if ((status = PutAsciiData (tk,"Surface_Max_Facet_Angle", m_surface_max_facet_angle)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 10: {
						PutTab t(&tk);
                        if ((mask & m_value[0] & TKO_Rendo_NURBS_Surface_Options) != 0 &&
                            (nurbs_mask & m_nurbs_options_value & TKO_NURBS_Surface_Max_Facet_Deviation) != 0) {
								
                            if ((status = PutAsciiData (tk,"Surface_Max_Facet_Deviation", m_surface_max_facet_deviation)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 11: {
						PutTab t(&tk);
                        if ((mask & m_value[0] & TKO_Rendo_NURBS_Surface_Options) != 0 &&
                            (nurbs_mask & m_nurbs_options_value & TKO_NURBS_Surface_Max_Facet_Width) != 0) {
								
                            if ((status = PutAsciiData (tk,"Surface_Max_Facet_Width", m_surface_max_facet_width)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 12: {
						PutTab t(&tk);
                        if ((mask & m_value[0] & TKO_Rendo_NURBS_Curve_Options) != 0 &&
                            (nurbs_mask & m_nurbs_options_value & TKO_NURBS_Curve_Max_Angle) != 0) {
								
                            if ((status = PutAsciiData (tk,"Curve_Max_Angle", m_curve_max_angle)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 13: {
						PutTab t(&tk);
                        if ((mask & m_value[0] & TKO_Rendo_NURBS_Curve_Options) != 0 &&
                            (nurbs_mask & m_nurbs_options_value & TKO_NURBS_Curve_Max_Deviation) != 0) {
								
                            if ((status = PutAsciiData (tk,"Curve_Max_Deviation", m_curve_max_deviation)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 14: {
						PutTab t(&tk);
                        if ((mask & m_value[0] & TKO_Rendo_NURBS_Curve_Options) != 0 &&
                            (nurbs_mask & m_nurbs_options_value & TKO_NURBS_Curve_Max_Length) != 0) {
								
                            if ((status = PutAsciiData (tk,"Curve_Max_Length", m_curve_max_length)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                    }
                }
            }
            m_stage++;
        }   nobreak;


        // LOD Options
        case 19: {
            if ((mask & m_value[0] & TKO_Rendo_LOD_Options) != 0) {
                switch (m_progress) {
                    case 0: {
						PutTab t(&tk);
                        if ((status = PutAsciiHex (tk,"LOD_Options_Mask", m_lod_options_mask)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 1: {
						PutTab t(&tk);
                        if ((status = PutAsciiHex (tk,"LOD_Options_Value", m_lod_options_value)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 2: {
						PutTab t(&tk);
                        if ((m_lod_options_mask & TKO_LOD_Algorithm) != 0) {
							
                            if ((status = PutAsciiData (tk,"LOD_Algorithm", m_lod_algorithm)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 3: {
						PutTab t(&tk);
                        if ((m_lod_options_mask & m_lod_options_value & TKO_LOD_Bounding_Explicit) != 0) {
							
                            if ((status = PutAsciiData (tk,"LOD_Bounding", m_bounding, 6)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 4: {
						PutTab t(&tk);
                        if ((m_lod_options_mask & m_lod_options_value & TKO_LOD_Calculation_Cutoff) != 0) {
							
                            if ((status = PutAsciiData (tk,"LOD_Calculation_Cutoff", m_num_cutoffs)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 5: {
						PutTab t(&tk);
                        if ((m_lod_options_mask & m_lod_options_value & TKO_LOD_Calculation_Cutoff) != 0) {
							
                            if ((status = PutAsciiData (tk,"LOD_Calculation_Cutoff", m_cutoff, m_num_cutoffs)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 6: {
						PutTab t(&tk);
                        if ((m_lod_options_mask & m_lod_options_value & TKO_LOD_Clamp) != 0) {
							
                            if ((status = PutAsciiData (tk,"LOD_Clamp", m_clamp)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 7: {
						PutTab t(&tk);
                        if ((m_lod_options_mask & m_lod_options_value & TKO_LOD_Fallback) != 0) {
							if (tk.GetTargetVersion() < 1315 && 
								m_fallback == TKO_LOD_Fallback_Bounding_None)
								m_fallback = TKO_LOD_Fallback_Bounding;
							
                            if ((status = PutAsciiData (tk,"LOD_FallBack", (int)m_fallback)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 8: {
						PutTab t(&tk);
                        if ((m_lod_options_mask & m_lod_options_value & TKO_LOD_Max_Degree) != 0) {
							
                            if ((status = PutAsciiData (tk,"LOD_Max_Degree", m_max_degree)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 9: {
						PutTab t(&tk);
                        if ((m_lod_options_mask & TKO_LOD_Min_Triangle_Count) != 0) {
							
                            if ((status = PutAsciiData (tk,"LOD_Min_Triangle_Count", m_min_triangle_count)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 10: {
						PutTab t(&tk);
                        if ((m_lod_options_mask & TKO_LOD_Num_Levels) != 0) {
							
                            if ((status = PutAsciiData (tk,"LOD_Num_Levels", (int)m_num_levels)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 11: 
						{
							PutTab t(&tk);
                        if ((m_lod_options_mask & TKO_LOD_Ratio) != 0) {
							
                            if ((status = PutAsciiData (tk,"LOD_Ratio", (int)m_num_ratios)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 12: {
						PutTab t(&tk);
                        if ((m_lod_options_mask & TKO_LOD_Ratio) != 0) {
							
                            if ((status = PutAsciiData (tk,"LOD_Ratio", m_ratio, (int)m_num_ratios)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 13: {
						PutTab t(&tk);
						if (tk.GetTargetVersion() < 1315 && 
							m_threshold_type == TKO_LOD_Threshold_Distance)
							m_lod_options_mask &= ~TKO_LOD_Threshold;

                        if ((m_lod_options_mask & TKO_LOD_Threshold) != 0) {
							
                            if ((status = PutAsciiData (tk,"LOD_Threshold", m_num_thresholds)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 14: {
						PutTab t(&tk);
                        if ((m_lod_options_mask & TKO_LOD_Threshold) != 0) {
							
                            if ((status = PutAsciiData (tk,"LOD_Threshold", m_threshold, m_num_thresholds)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 15: {
						PutTab t(&tk);
                        if ((m_lod_options_mask & TKO_LOD_Threshold) != 0 &&
                            tk.GetTargetVersion() >= 908) {
								
                            if ((status = PutAsciiData (tk,"Threshold_Type", m_threshold_type)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 16: {
						PutTab t(&tk);
                        if ((m_lod_options_mask & m_lod_options_value & TKO_LOD_Usefulness_Heuristic) != 0) {
							
                            if ((status = PutAsciiData (tk,"LOD_Usefullness_Heuristic", m_heuristic)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                    }
                }
            }
            m_stage++;
        }   nobreak;

        // tessellation options
        case 20: {
            if ((emask & m_value[1] & TKO_Rendo_Tessellation) != 0) {
                switch (m_progress) {
                    case 0: {
						PutTab t(&tk);
                        if ((status = PutAsciiData (tk,"Rendo_Tessellation", m_tessellations)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 1: {
						PutTab t(&tk);
                        if ((m_tessellations & TKO_Tessellation_Cylinder) != 0) {
							
                            if ((status = PutAsciiData (tk,"Tessellation_Cylinder", m_num_cylinder)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 2: {
						PutTab t(&tk);
                        if ((m_tessellations & TKO_Tessellation_Cylinder) != 0) {
							
                            if ((status = PutAsciiData (tk,"Tessellation_Cylinder", m_cylinder, m_num_cylinder)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 3: {
						PutTab t(&tk);
                        if ((m_tessellations & TKO_Tessellation_Sphere) != 0) {
							
                            if ((status = PutAsciiData (tk,"Tessellation_Sphere", m_num_sphere)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 4: {
						PutTab t(&tk);
                        if ((m_tessellations & TKO_Tessellation_Sphere) != 0) {
							
                            if ((status = PutAsciiData (tk,"Tessellation_Sphere", m_sphere, m_num_sphere)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                    }
                }
            }
            m_stage++;
        }   nobreak;

        // transparency options
        case 21: {
            if ((emask & m_value[1] & (TKO_Rendo_Transparency_Style|TKO_Rendo_Transparency_Options)) != 0) {
                switch (m_progress) {
                    case 0: {
						PutTab t(&tk);
						if ((status = PutAsciiHex (tk,"Rendo_Transparency_Options", m_transparency_options)) != TK_Normal)
							return status;
                        m_progress++;
                    }

                    case 1: {
						PutTab t(&tk);
                        if ((m_transparency_options & TKO_Transparency_Peeling_Layers) != 0) {
							
                            if ((status = PutAsciiHex (tk,"Transparency_Peeling_Layers", m_depth_peeling_layers)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 2: {
						PutTab t(&tk);
                        if ((m_transparency_options & TKO_Transparency_Peeling_Min_Area) != 0) {
							
                            if ((status = PutAsciiData (tk,"Transparency_Peeling_Min_Area", m_depth_peeling_min_area)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                    }
                }
            }
            m_stage++;
        }   nobreak;

        // cut geometry
        case 22: {
            if ((emask & m_value[1] & TKO_Rendo_Cut_Geometry) != 0) {
                switch (m_progress) {
                    case 0: {
						PutTab t(&tk);
                        if ((status = PutAsciiData (tk,"Rendo_Cut_Geometry", (int)m_cut_geometry)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 1: {
						PutTab t(&tk);
                        if ((m_cut_geometry & TKO_Cut_Geometry_Level) != 0) {
							
                            if ((status = PutAsciiData (tk,"Cut_Geometry_Level", (int)m_cut_geometry_level)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 2: {
						PutTab t(&tk);
                        if ((m_cut_geometry & TKO_Cut_Geometry_Tolerance) != 0) {
							
                            if ((status = PutAsciiData (tk,"Cut_Geometry_Tolerance", m_cut_geometry_tolerance)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 3: {
						PutTab t(&tk);
                        if ((m_cut_geometry & TKO_Cut_Geometry_Match_Color) != 0) {
							
                            if ((status = PutAsciiData (tk,"Cut_Geometry_Match_Color", m_cut_geometry_match)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                    }
                }
            }
            m_stage++;
        }   nobreak;

        // depth range
        case 23: {
			PutTab t1(&tk);
            if ((emask & m_value[1] & TKO_Rendo_Depth_Range) != 0) {
					
                if ((status = PutAsciiData (tk,"Rendo_Depth_Range", m_depth_range, 2)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        // mask transform
        case 24: {
			PutTab t1(&tk);
            if ((emask & m_value[1] & TKO_Rendo_Mask_Transform) != 0) {
					
                if ((status = PutAsciiHex (tk,"Rendo_Mask_Transform", m_mask_transform)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        // image scale
        case 25: {
			PutTab t1(&tk);
            if ((emask & m_value[1] & TKO_Rendo_Image_Scale) != 0) {
					
                if ((status = PutAsciiData (tk,"Rendo_Image_Scale", m_image_scale, 2)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        // simple shadow
        case 26: {
            if ((emask & m_value[1] & TKO_Rendo_Simple_Shadow) != 0) {
                unsigned short          smask = m_simple_shadow;

                if (smask & (TKO_Simple_Shadow_Extended|TKO_Simple_Shadow_Auto))
                {
                    _W3DTK_REQUIRE_VERSION( 1170 )
                }

                if (tk.GetTargetVersion() < 1170) {
                    smask &= ~(TKO_Simple_Shadow_Extended|TKO_Simple_Shadow_Auto);
                }
                switch (m_progress) {
                    case 0: {
                        unsigned char       byte = (unsigned char)smask;
						PutTab t(&tk);
                        if ((status = PutAsciiMask (tk,"Rendo_Simple_Shadow_Mask", byte)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 1: {
						PutTab t(&tk);
                        if ((smask & TKO_Simple_Shadow_Extended) != 0) {
                                unsigned char       byte = (unsigned char)(smask >> 8);
							
                            if ((status = PutAsciiMask (tk,"Simple_Shadow_Extended", byte)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 2: {
						PutTab t(&tk);
                        if ((m_simple_shadow & TKO_Simple_Shadow_Plane) != 0) {
							
                            if ((status = PutAsciiData (tk,"Simple_Shadow_Plane", m_simple_shadow_plane, 4)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 3: {
						PutTab t(&tk);
                        if ((m_simple_shadow & TKO_Simple_Shadow_Light_Direction) != 0) {
							
                            if ((status = PutAsciiData (tk,"Simple_Shadow_Light_Direction", m_simple_shadow_light, 3)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 4: {
						PutTab t(&tk);
                        if ((m_simple_shadow & TKO_Simple_Shadow_Color) != 0) {
							
                            if ((status = PutAsciiData (tk,"Simple_Shadow_RGB", m_simple_shadow_color, 3)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 5: {
						PutTab t(&tk);
                        if ((m_simple_shadow & TKO_Simple_Shadow_Blur) != 0) {
							
                            if ((status = PutAsciiData (tk,"Simple_Shadow_Blur", m_simple_shadow_blur)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 6: {
						PutTab t(&tk);
                        if ((m_simple_shadow & TKO_Simple_Shadow_Resolution) != 0) {
							
                            if ((status = PutAsciiData (tk,"Simple_Shadow_Resolution", m_simple_shadow_resolution)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                    }
                }
           }
            m_stage++;
        }   nobreak;

		// Geometry options
        case 27: {
            if ((emask & m_value[1] & TKO_Rendo_Geometry_Options) != 0 &&
				tk.GetTargetVersion() > 1200) {
                _W3DTK_REQUIRE_VERSION( 1200 );
				switch (m_progress) {
					case 0: {
						PutTab t(&tk);
						if ((status = PutAsciiHex (tk,"Rendo_Geometry_Options", m_geometry_options)) != TK_Normal)
							return status;
						m_progress++;
					}

					case 1: {
						PutTab t(&tk);
						if ((m_geometry_options & TKO_Geometry_Options_Dihedral) != 0) {
							
							if ((status = PutAsciiData (tk,"Geometry_Options_Dihedral", m_dihedral)) != TK_Normal)
								return status;
						}
						m_progress = 0;
					}
				}
			}

            m_stage++;
        }   nobreak;

		// image tint
		case 28: {
               PutTab t1(&tk);
            if ((emask & m_value[1] & TKO_Rendo_Image_Tint) != 0 &&
				tk.GetTargetVersion() >= 1210) {
                if ((status = PutAsciiData (tk,"Image_Tint_RGB", m_simple_shadow_color, 3)) != TK_Normal)
                    return status;
				_W3DTK_REQUIRE_VERSION( 1210 );
            }
            m_stage++;

		} nobreak;

		// general displacement
		case 29: {
			PutTab t1(&tk);
			if ((emask & m_value[1] & TKO_Rendo_General_Displacement) != 0 &&
				tk.GetTargetVersion() >= 1335) {

				if ((status = PutAsciiData (tk,"General_Displacement", m_general_displacement)) != TK_Normal)
					return status;
                _W3DTK_REQUIRE_VERSION( 1335 );
			}
			m_stage++;

		} nobreak;

		// join cutoff angle
		case 30: {
			PutTab t1(&tk);
			if ((emask & m_value[1] & TKO_Rendo_Join_Cutoff_Angle) != 0 &&
				tk.GetTargetVersion() >= 1405) {

				if ((status = PutAsciiData (tk,"Join Cutoff Angle", m_join_cutoff_angle)) != TK_Normal)
					return status;
                _W3DTK_REQUIRE_VERSION( 1405 );
			}
			m_stage++;

				 } nobreak;

        case 31: {
			// always last or not
#if 0
            if (m_rrt != null) {
                if ((status = m_rrt->Write (tk)) != TK_Normal)
                    return status;
            }
#endif
            m_stage++;
        }   nobreak;

		case 32: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;
        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Rendering_Options::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
            m_progress = 0; // make sure
            m_stage++;
        }   nobreak;


        case 1: {
			if ((status = GetAsciiHex (tk,"Mask", m_mask[0])) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
            if ((m_mask[0] & TKO_Rendo_Extended) != 0) {
				if ((status = GetAsciiHex (tk,"Extended_Mask", m_mask[1])) != TK_Normal)
                    return status;
            }
            else
                m_mask[1] = 0;
            m_stage++;
        }   nobreak;

        case 3: {
			if ((status = GetAsciiData (tk,"Value", m_value[0])) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
            if ((m_mask[0] & TKO_Rendo_Extended) != 0) {
				if ((status = GetAsciiData (tk,"Extended_Value", m_value[1])) != TK_Normal)
                    return status;
            }
            else
                m_value[1] = 0;
            m_stage++;
        }   nobreak;


        case 5: {
            if ((m_mask[0] & TKO_Rendo_Any_HSR) != 0) {
				if ((status = GetAsciiData (tk,"HSR",m_int )) != TK_Normal)
                    return status;
				m_hsr = (unsigned char)m_int; 
            }
            m_stage++;
        }   nobreak;

        case 6: {
            if ((m_mask[0] & TKO_Rendo_TQ) != 0) {
				if ((status = GetAsciiData (tk,"TQ", m_int )) != TK_Normal)
                    return status;
				m_tq = (unsigned char) m_int;
            }
            m_stage++;
        }   nobreak;

        case 7: {
            if ((m_mask[0] & m_value[0] & TKO_Rendo_Face_Displacement) != 0) {
				if ((status = GetAsciiData (tk,"Face_Displacement", m_face_displacement)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;


        // attribute locks
        case 8: {
            if ((m_mask[0] & m_value[0] & TKO_Rendo_Attribute_Lock) != 0) {
				if ((status = GetAsciiData (tk,"Lock_Mask", m_lock_mask)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 9: {
			if ((m_mask[0] & m_value[0] & TKO_Rendo_Attribute_Lock) != 0) {
				if ((status = GetAsciiData (tk,"Lock_Value", m_lock_value)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 10: {
            if ((m_mask[0] & m_value[0] & TKO_Rendo_Attribute_Lock) != 0 &&
                (m_lock_mask & m_lock_value & TKO_Lock_Color) != 0) {
					if ((status = GetAsciiData (tk,"Lock_Color_Mask", m_lock_color_mask)) != TK_Normal)
                    return status;
            }
            m_progress = 0; // make sure
            m_stage++;
        }   nobreak;

        case 11: {
            if ((m_mask[0] & m_value[0] & TKO_Rendo_Attribute_Lock) != 0 &&
                (m_lock_mask & m_lock_value & TKO_Lock_Color) != 0) {
                if (tk.GetVersion() <= 1105) {
					if ((status = GetAsciiData (tk,"Lock_Color_Value", m_lock_color_value)) != TK_Normal)
                        return status;

                    if ((m_lock_color_mask & (TKO_Geo_Face|TKO_Geo_Front)) != 0) {
                        m_lock_color_face_mask = ~0;
                        if ((m_lock_color_value & (TKO_Geo_Face|TKO_Geo_Front)) != 0)
                            m_lock_color_face_value = ~0;
                        else
                            m_lock_color_face_value = 0;
                    }
                    else
                        m_lock_color_face_mask = m_lock_color_face_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Back) != 0) {
                        m_lock_color_back_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Back) != 0)
                            m_lock_color_back_value = ~0;
                        else
                            m_lock_color_back_value = 0;
                    }
                    else
                        m_lock_color_back_mask = m_lock_color_back_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Edge) != 0) {
                        m_lock_color_edge_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Edge) != 0)
                            m_lock_color_edge_value = ~0;
                        else
                            m_lock_color_edge_value = 0;
                    }
                    else
                        m_lock_color_edge_mask = m_lock_color_edge_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Line) != 0) {
                        m_lock_color_line_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Line) != 0)
                            m_lock_color_line_value = ~0;
                        else
                            m_lock_color_line_value = 0;
                    }
                    else
                        m_lock_color_line_mask = m_lock_color_line_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Text) != 0) {
                        m_lock_color_text_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Text) != 0)
                            m_lock_color_text_value = ~0;
                        else
                            m_lock_color_text_value = 0;
                    }
                    else
                        m_lock_color_text_mask = m_lock_color_text_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Marker) != 0) {
                        m_lock_color_marker_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Marker) != 0)
                            m_lock_color_marker_value = ~0;
                        else
                            m_lock_color_marker_value = 0;
                    }
                    else
                        m_lock_color_marker_mask = m_lock_color_marker_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Vertex) != 0) {
                        m_lock_color_vertex_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Vertex) != 0)
                            m_lock_color_vertex_value = ~0;
                        else
                            m_lock_color_vertex_value = 0;
                    }
                    else
                        m_lock_color_vertex_mask = m_lock_color_vertex_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Window) != 0) {
                        m_lock_color_window_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Window) != 0)
                            m_lock_color_window_value = ~0;
                        else
                            m_lock_color_window_value = 0;
                    }
                    else
                        m_lock_color_window_mask = m_lock_color_window_value = 0;

                    //contrast
                    if ((m_lock_color_mask & TKO_Geo_Face_Contrast) != 0) {
                        m_lock_color_face_contrast_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Face_Contrast) != 0)
                            m_lock_color_face_contrast_value = ~0;
                        else
                            m_lock_color_face_contrast_value = 0;
                    }
                    else
                        m_lock_color_face_contrast_mask = m_lock_color_face_contrast_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Edge_Contrast) != 0) {
                        m_lock_color_edge_contrast_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Edge_Contrast) != 0)
                            m_lock_color_edge_contrast_value = ~0;
                        else
                            m_lock_color_edge_contrast_value = 0;
                    }
                    else
                        m_lock_color_edge_contrast_mask = m_lock_color_edge_contrast_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Line_Contrast) != 0) {
                        m_lock_color_line_contrast_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Line_Contrast) != 0)
                            m_lock_color_line_contrast_value = ~0;
                        else
                            m_lock_color_line_contrast_value = 0;
                    }
                    else
                        m_lock_color_line_contrast_mask = m_lock_color_line_contrast_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Text_Contrast) != 0) {
                        m_lock_color_text_contrast_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Text_Contrast) != 0)
                            m_lock_color_text_contrast_value = ~0;
                        else
                            m_lock_color_text_contrast_value = 0;
                    }
                    else
                        m_lock_color_text_contrast_mask = m_lock_color_text_contrast_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Marker_Contrast) != 0) {
                        m_lock_color_marker_contrast_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Marker_Contrast) != 0)
                            m_lock_color_marker_contrast_value = ~0;
                        else
                            m_lock_color_marker_contrast_value = 0;
                    }
                    else
                        m_lock_color_marker_contrast_mask = m_lock_color_marker_contrast_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Vertex_Contrast) != 0) {
                        m_lock_color_vertex_contrast_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Vertex_Contrast) != 0)
                            m_lock_color_vertex_contrast_value = ~0;
                        else
                            m_lock_color_vertex_contrast_value = 0;
                    }
                    else
                        m_lock_color_vertex_contrast_mask = m_lock_color_vertex_contrast_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Window_Contrast) != 0) {
                        m_lock_color_window_contrast_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Window_Contrast) != 0)
                            m_lock_color_window_contrast_value = ~0;
                        else
                            m_lock_color_window_contrast_value = 0;
                    }
                    else
                        m_lock_color_window_contrast_mask = m_lock_color_window_contrast_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Simple_Reflection) != 0) {
                        m_lock_color_simple_reflection_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Simple_Reflection) != 0)
                            m_lock_color_simple_reflection_value = ~0;
                        else
                            m_lock_color_simple_reflection_value = 0;
                    }
                    else
                        m_lock_color_simple_reflection_mask = m_lock_color_simple_reflection_value = 0;
                    if ((m_lock_color_mask & TKO_Geo_Cut_Face) != 0) {
                        m_lock_color_cut_face_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Cut_Face) != 0)
                            m_lock_color_cut_face_value = ~0;
                        else
                            m_lock_color_cut_face_value = 0;
                    }
                    else
                        m_lock_color_cut_face_mask = m_lock_color_cut_face_value = 0;

                    if ((m_lock_color_mask & TKO_Geo_Cut_Edge) != 0) {
                        m_lock_color_cut_edge_mask = ~0;
                        if ((m_lock_color_value & TKO_Geo_Cut_Edge) != 0)
                            m_lock_color_cut_edge_value = ~0;
                        else
                            m_lock_color_cut_edge_value = 0;
                    }
                    else
                        m_lock_color_cut_edge_mask = m_lock_color_cut_edge_value = 0;
                }
                else switch (m_progress) {
                    case 0:
						if ((status = GetAsciiData (tk,"Lock_Color_Value", m_lock_color_value)) != TK_Normal)
                            return status;
                        m_progress++;

                    case 1:
                        if ((m_lock_color_value & (TKO_Geo_Face|TKO_Geo_Front)) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Face_Mask", m_lock_color_face_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 2:
                        if ((m_lock_color_value & (TKO_Geo_Face|TKO_Geo_Front)) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Face_Value", m_lock_color_face_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 3:
                        if ((m_lock_color_value & TKO_Geo_Back) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Back_Mask", m_lock_color_back_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 4:
                        if ((m_lock_color_value & TKO_Geo_Back) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Back_Value", m_lock_color_back_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 5:
                        if ((m_lock_color_value & TKO_Geo_Edge) != 0)
							if ((status = GetAsciiData (tk, "Lock_Color_Edge_Mask",m_lock_color_edge_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 6:
                        if ((m_lock_color_value & TKO_Geo_Edge) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Edge_Value", m_lock_color_edge_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 7:
                        if ((m_lock_color_value & TKO_Geo_Line) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Line_Mask", m_lock_color_line_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 8:
                        if ((m_lock_color_value & TKO_Geo_Line) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Line_Value", m_lock_color_line_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 9:
                        if ((m_lock_color_value & TKO_Geo_Text) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Text_Mask", m_lock_color_text_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 10:
                        if ((m_lock_color_value & TKO_Geo_Text) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Text_Value", m_lock_color_text_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 11:
                        if ((m_lock_color_value & TKO_Geo_Marker) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Marker_Mask", m_lock_color_marker_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 12:
                        if ((m_lock_color_value & TKO_Geo_Marker) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Marker_Value", m_lock_color_marker_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 13:
                        if ((m_lock_color_value & TKO_Geo_Vertex) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Vertex_Mask", m_lock_color_vertex_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 14:
                        if ((m_lock_color_value & TKO_Geo_Vertex) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Vertex_Value", m_lock_color_vertex_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 15:
                        if ((m_lock_color_value & TKO_Geo_Window) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Window_Mask", m_lock_color_window_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 16:
                        if ((m_lock_color_value & TKO_Geo_Window) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Window_Value", m_lock_color_window_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 17:
                        if ((m_lock_color_value & TKO_Geo_Face_Contrast) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Face_Contrast_Mask", m_lock_color_face_contrast_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 18:
                        if ((m_lock_color_value & TKO_Geo_Face_Contrast) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Face_Contrast_Value", m_lock_color_face_contrast_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 19:
                        if ((m_lock_color_value & TKO_Geo_Edge_Contrast) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Edge_Contrast_Mask", m_lock_color_edge_contrast_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 20:
                        if ((m_lock_color_value & TKO_Geo_Edge_Contrast) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Edge_Contrast_Value", m_lock_color_edge_contrast_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 21:
                        if ((m_lock_color_value & TKO_Geo_Line_Contrast) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Line_Contrast_Mask", m_lock_color_line_contrast_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 22:
                        if ((m_lock_color_value & TKO_Geo_Line_Contrast) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Line_Contrast_Value", m_lock_color_line_contrast_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 23:
                        if ((m_lock_color_value & TKO_Geo_Text_Contrast) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Text_Contrast_Mask", m_lock_color_text_contrast_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 24:
                        if ((m_lock_color_value & TKO_Geo_Text_Contrast) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Text_Contrast_Value", m_lock_color_text_contrast_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 25:
                        if ((m_lock_color_value & TKO_Geo_Marker_Contrast) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Marker_Contrast_Mask", m_lock_color_marker_contrast_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 26:
                        if ((m_lock_color_value & TKO_Geo_Marker_Contrast) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Marker_Contrast_Value", m_lock_color_marker_contrast_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 27:
                        if ((m_lock_color_value & TKO_Geo_Vertex_Contrast) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Vertex_Contrast_Mask", m_lock_color_vertex_contrast_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 28:
                        if ((m_lock_color_value & TKO_Geo_Vertex_Contrast) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Vertex_Contrast_Value", m_lock_color_vertex_contrast_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 29:
                        if ((m_lock_color_value & TKO_Geo_Window_Contrast) != 0)
							if ((status = GetAsciiData (tk,"Lock_Color_Window_Contrast_Mask", m_lock_color_window_contrast_mask)) != TK_Normal)
                                return status;
                        m_progress++;
                    case 30:
                        if ((m_lock_color_value & TKO_Geo_Window_Contrast) != 0)
							if ((status = GetAsciiData (tk, "Lock_Color_Window_Contrast_Value", m_lock_color_window_contrast_value)) != TK_Normal)
                                return status;
                        m_progress++;

                    case 31:
                        if ((m_lock_color_value & TKO_Geo_Simple_Reflection) != 0) {
							if (tk.GetVersion() >= 1550) {
								if ((status = GetAsciiData (tk,"Lock_Color_Simple_Reflection_Mask", m_lock_color_simple_reflection_mask)) != TK_Normal)
									return status;
							}
							else
								m_lock_color_simple_reflection_mask = TKO_Lock_Channel_ALL;
						}
                        m_progress++;
                    case 32:
                        if ((m_lock_color_value & TKO_Geo_Simple_Reflection) != 0) {
							if (tk.GetVersion() >= 1550) {
								if ((status = GetAsciiData (tk,"Lock_Color_Simple_Reflection_Value", m_lock_color_simple_reflection_value)) != TK_Normal)
									return status;
							}
							else
								m_lock_color_simple_reflection_value = TKO_Lock_Channel_ALL;
						}
                        m_progress++;
                    case 33:
                        if ((m_lock_color_value & TKO_Geo_Cut_Face) != 0) {
							if (tk.GetVersion() >= 1220) {
								if ((status = GetAsciiData (tk,"Lock_Color_Cut_Face_Mask", m_lock_color_cut_face_mask)) != TK_Normal)
									return status;
							}
							else
								m_lock_color_cut_face_mask = TKO_Lock_Channel_ALL;
						}
                        m_progress++;
                    case 34:
                        if ((m_lock_color_value & TKO_Geo_Cut_Face) != 0) {
							if (tk.GetVersion() >= 1220) {
								if ((status = GetAsciiData (tk,"Lock_Color_Cut_Face_Value", m_lock_color_cut_face_value)) != TK_Normal)
									return status;
							}
							else
								m_lock_color_cut_face_value = TKO_Lock_Channel_ALL;
						}
                        m_progress++;

                    case 35:
                        if ((m_lock_color_value & TKO_Geo_Cut_Edge) != 0) {
							if (tk.GetVersion() >= 1220) {
								if ((status = GetAsciiData (tk,"Lock_Color_Cut_Edge_Mask", m_lock_color_cut_edge_mask)) != TK_Normal)
									return status;
							}
							else
								m_lock_color_cut_edge_mask = TKO_Lock_Channel_ALL;
						}
                        m_progress++;
                    case 36:
                        if ((m_lock_color_value & TKO_Geo_Cut_Edge) != 0) {
							if (tk.GetVersion() >= 1220) {
								if ((status = GetAsciiData (tk,"Lock_Color_Cut_Edge_Value", m_lock_color_cut_edge_value)) != TK_Normal)
									return status;
							}
							else
								m_lock_color_cut_edge_value = TKO_Lock_Channel_ALL;
						}
                        m_progress = 0;
                        break;

                    default:
                        return tk.Error ("lost track processing color locks");
                }
            }
            m_stage++;
        }   nobreak;

        case 12: {
            if ((m_mask[0] & m_value[0] & TKO_Rendo_Attribute_Lock) != 0 &&
                (m_lock_mask & m_lock_value & TKO_Lock_Visibility) != 0) {
                switch (m_progress) {
                    case 0:
						if ((status = GetAsciiData (tk,"Lock_Visibility_Mask", m_lock_visibility_mask)) != TK_Normal)
                            return status;
                        m_progress++;
                    case 1:
						if ((status = GetAsciiData (tk,"Lock_Visibility_Value", m_lock_visibility_value)) != TK_Normal)
                            return status;
                        m_progress = 0;
                        break;

                    default:
                        return tk.Error ("lost track processing visibility locks");
                }
            }
            m_stage++;
        }   nobreak;


        case 13: {
            if ((m_mask[0] & m_value[0] & TKO_Rendo_Fog) != 0) {
				if ((status = GetAsciiData (tk,"Fog_Limits", m_fog_limits, 2)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 14: {
            if ((m_mask[0] & m_value[0] & TKO_Rendo_Debug) != 0) {
				if ((status = GetAsciiData (tk,"Rendering_Options_Debug", m_debug)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        // new with 650
        case 15: {
            if ((m_mask[0] & m_value[0] & TKO_Rendo_Stereo_Separation) != 0) {
				if ((status = GetAsciiData (tk,"Stereo_Seperation", m_stereo_separation)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;


        // sfb options
        case 16: {
            if ((m_mask[0] & m_value[0] & TKO_Rendo_Buffer_Options) != 0) {
                switch (m_progress) {
                    case 0:
						if ((status = GetAsciiData (tk,"Buffer_Options_Mask", m_buffer_options_mask)) != TK_Normal)
                            return status;
                        m_progress++;
                    case 1:
						if ((status = GetAsciiData (tk,"Buffer_Options_Value", m_buffer_options_value)) != TK_Normal)
                            return status;
                        m_progress++;
                    case 2:
                        if ((m_buffer_options_mask & m_buffer_options_value & TKO_Buffer_Size_Limit) != 0) {
							if ((status = GetAsciiData (tk,"Buffer_Size_Limit", m_buffer_size_limit)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                }
            }
            m_stage++;
        }   nobreak;

        // hlr options
        case 17: {
            if ((m_mask[0] & m_value[0] & TKO_Rendo_Hidden_Line_Options) != 0) {
                switch (m_progress) {
                    case 0: {
						if ((status = GetAsciiHex (tk, "HLR_Mask", m_hlr_options)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 1: {
                        if ((m_hlr_options & TKO_Hidden_Line_Extended) != 0) {
							if ((status = GetAsciiHex (tk, "HLR_Mask", m_byte)) != TK_Normal)
                                return status;
                            m_hlr_options |= m_byte << 8;
                        }
                        m_progress++;
                    }

                    case 2: {
                        if ((m_hlr_options & TKO_Hidden_Line_Extended2) != 0) {
							if ((status = GetAsciiData (tk,"HLR_Mask", m_unsigned_short)) != TK_Normal)
                                return status;
                            m_hlr_options |= m_unsigned_short << 16;
                        }
                        m_progress++;
                    }

                    case 3: {
                        if ((m_hlr_options & TKO_Hidden_Line_Pattern) != 0) {
							if ((status = GetAsciiData (tk,"HLR_Line_Pattern", m_hlr_line_pattern)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 4: {
                        if ((m_hlr_options & TKO_Hidden_Line_Dim_Factor) != 0) {
							if ((status = GetAsciiData (tk,"HLR_Dim_Factor", m_hlr_dim_factor)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 5: {
                        if ((m_hlr_options & TKO_Hidden_Line_Face_Displacement) != 0) {
							if ((status = GetAsciiData (tk,"HLR_Face_Displacement", m_hlr_face_displacement)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 6: {
                        if ((m_hlr_options & TKO_Hidden_Line_Color) != 0) {
							if ((status = GetAsciiData (tk,"Hidden_Line_Color", m_hlr_color, 3)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 7: {
                        if ((m_hlr_options & TKO_Hidden_Line_Weight) != 0) {
							if ((status = GetAsciiData (tk,"HLR_Weight", m_hlr_weight)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 8: {
                        if ((m_hlr_options & TKO_Hidden_Line_Weight) != 0 &&
                            m_hlr_weight != -1.0f) {
								if ((status = GetAsciiData (tk,"HLR_Weight_Units", m_hlr_weight_units)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 9: {
                        if ((m_hlr_options & TKO_Hidden_Line_HSR_Algorithm) != 0) {
							if ((status = GetAsciiData (tk,"HLR_HSR_Algorithm", m_hlr_hsr_algorithm)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                    }
                }
            }
            m_stage++;
        }   nobreak;


        // NURBS curve and surface options
        case 18: {
            if (m_mask[0] & TKO_Rendo_NURBS_Options) {
                switch (m_progress) {
                    case 0: {
						if ((status = GetAsciiHex (tk,"Nurbs_Mask", m_nurbs_options_mask)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 1: {
                        if ((m_nurbs_options_mask & TKO_NURBS_Extended)) {
							if ((status = GetAsciiHex (tk,"Nurbs_Options_Mask", m_byte)) != TK_Normal)
                                return status;
                            m_nurbs_options_mask |= (m_byte << 8);
                        }
                        m_progress++;
                    }

                    case 2: {
                       if ((status = GetAsciiData (tk,"Nurbs_Options_Value", m_nurbs_options_value)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 3: {
                        if ((m_nurbs_options_mask & TKO_NURBS_Extended)) {
							if ((status = GetAsciiData (tk,"Nurbs_Options_Value", m_byte)) != TK_Normal)
                                return status;
                            m_nurbs_options_value |= (m_byte << 8);
                        }
                        m_progress++;
                    }

                    case 4: {
                        if ((m_mask[0] & m_value[0] & TKO_Rendo_NURBS_Curve_Options) != 0 &&
                            (m_nurbs_options_mask & m_nurbs_options_value & TKO_NURBS_Curve_Budget) != 0) {
								if ((status = GetAsciiData (tk,"Curve_Budget", m_curve_budget)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 5: {
                        if ((m_mask[0] & m_value[0] & TKO_Rendo_NURBS_Curve_Options) != 0 &&
                            (m_nurbs_options_mask & m_nurbs_options_value & TKO_NURBS_Curve_Continued_Budget) != 0) {
								if ((status = GetAsciiData (tk,"Curve_Continued_Budget", m_curve_continued_budget)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 6: {
                        if ((m_mask[0] & m_value[0] & TKO_Rendo_NURBS_Surface_Options) != 0 &&
                            (m_nurbs_options_mask & m_nurbs_options_value & TKO_NURBS_Surface_Budget) != 0) {
								if ((status = GetAsciiData (tk,"Surface_Budget", m_surface_budget)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 7: {
                        if ((m_mask[0] & m_value[0] & TKO_Rendo_NURBS_Surface_Options) != 0 &&
                            (m_nurbs_options_mask & m_nurbs_options_value & TKO_NURBS_Surface_Trim_Budget) != 0) {
								if ((status = GetAsciiData (tk, "Surface_Trim_Budget",m_surface_trim_budget)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 8: {
                        if ((m_mask[0] & m_value[0] & TKO_Rendo_NURBS_Surface_Options) != 0 &&
                            (m_nurbs_options_mask & m_nurbs_options_value & TKO_NURBS_Surface_Max_Trim_Curve_Deviation) != 0) {
								if ((status = GetAsciiData (tk,"Surface_Max_Trim_Curve_Deviation", m_surface_max_trim_curve_deviation)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 9: {
                        if ((m_mask[0] & m_value[0] & TKO_Rendo_NURBS_Surface_Options) != 0 &&
                            (m_nurbs_options_mask & m_nurbs_options_value & TKO_NURBS_Surface_Max_Facet_Angle) != 0) {
								if ((status = GetAsciiData (tk,"Surface_Max_Facet_Angle", m_surface_max_facet_angle)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 10: {
                        if ((m_mask[0] & m_value[0] & TKO_Rendo_NURBS_Surface_Options) != 0 &&
                            (m_nurbs_options_mask & m_nurbs_options_value & TKO_NURBS_Surface_Max_Facet_Deviation) != 0) {
								if ((status = GetAsciiData (tk, "Surface_Max_Facet_Deviation",m_surface_max_facet_deviation)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 11: {
                        if ((m_mask[0] & m_value[0] & TKO_Rendo_NURBS_Surface_Options) != 0 &&
                            (m_nurbs_options_mask & m_nurbs_options_value & TKO_NURBS_Surface_Max_Facet_Width) != 0) {
								if ((status = GetAsciiData (tk,"Surface_Max_Facet_Width", m_surface_max_facet_width)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 12: {
                        if ((m_mask[0] & m_value[0] & TKO_Rendo_NURBS_Curve_Options) != 0 &&
                            (m_nurbs_options_mask & m_nurbs_options_value & TKO_NURBS_Curve_Max_Angle) != 0) {
								if ((status = GetAsciiData (tk,"Curve_Max_Angle", m_curve_max_angle)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 13: {
                        if ((m_mask[0] & m_value[0] & TKO_Rendo_NURBS_Curve_Options) != 0 &&
                            (m_nurbs_options_mask & m_nurbs_options_value & TKO_NURBS_Curve_Max_Deviation) != 0) {
								if ((status = GetAsciiData (tk,"Curve_Max_Deviation", m_curve_max_deviation)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 14: {
                        if ((m_mask[0] & m_value[0] & TKO_Rendo_NURBS_Curve_Options) != 0 &&
                            (m_nurbs_options_mask & m_nurbs_options_value & TKO_NURBS_Curve_Max_Length) != 0) {
								if ((status = GetAsciiData (tk,"Curve_Max_Length", m_curve_max_length)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                    }
                }
            }
            m_stage++;
        }   nobreak;


        // lod options
        case 19: {
            if ((m_mask[0] & m_value[0] & TKO_Rendo_LOD_Options) != 0) {
                switch (m_progress) {
                    case 0: {
						if ((status = GetAsciiHex (tk,"LOD_Options_Mask", m_lod_options_mask)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 1: {
						if ((status = GetAsciiHex (tk,"LOD_Options_Value", m_lod_options_value)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 2: {
                        if ((m_lod_options_mask & TKO_LOD_Algorithm) != 0) {
							if ((status = GetAsciiData (tk,"LOD_Algorithm", m_lod_algorithm)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 3: {
                        if ((m_lod_options_mask & m_lod_options_value & TKO_LOD_Bounding_Explicit) != 0) {
							if ((status = GetAsciiData (tk,"LOD_Bounding", m_bounding, 6)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 4: {
                        if ((m_lod_options_mask & m_lod_options_value & TKO_LOD_Calculation_Cutoff) != 0) {
							if ((status = GetAsciiData (tk,"LOD_Calculation_Cutoff", m_num_cutoffs)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 5: {
                        if ((m_lod_options_mask & m_lod_options_value & TKO_LOD_Calculation_Cutoff) != 0) {
							if ((status = GetAsciiData (tk,"LOD_Calculation_Cutoff", m_cutoff, m_num_cutoffs)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 6: {
                        if ((m_lod_options_mask & m_lod_options_value & TKO_LOD_Clamp) != 0) {
							if ((status = GetAsciiData (tk,"LOD_Clamp", m_clamp)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 7: {
                        if ((m_lod_options_mask & m_lod_options_value & TKO_LOD_Fallback) != 0) {
							if ((status = GetAsciiData (tk,"LOD_FallBack", m_int)) != TK_Normal)
                                return status;
							m_fallback = (unsigned char)m_int;
                        }
                        m_progress++;
                    }

                    case 8: {
                        if ((m_lod_options_mask & m_lod_options_value & TKO_LOD_Max_Degree) != 0) {
							if ((status = GetAsciiData (tk,"LOD_Max_Degree", m_max_degree)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 9: {
                        if ((m_lod_options_mask & TKO_LOD_Min_Triangle_Count) != 0) {
							if ((status = GetAsciiData (tk,"LOD_Min_Triangle_Count", m_min_triangle_count)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 10: {
                        if ((m_lod_options_mask & TKO_LOD_Num_Levels) != 0) {
							if ((status = GetAsciiData (tk,"LOD_Num_Levels", m_int)) != TK_Normal)
                                return status;
							m_num_levels = (unsigned char)m_int;
                        }
                        m_progress++;
                    }

                    case 11: {
                        if ((m_lod_options_mask & TKO_LOD_Ratio) != 0) {
							if ((status = GetAsciiData (tk,"LOD_Ratio", m_int )) != TK_Normal)
                                return status;
							m_num_ratios = (char)m_int;
                        }
                        m_progress++;
                    }

                    case 12: {
                        if ((m_lod_options_mask & TKO_LOD_Ratio) != 0) {
							if ((status = GetAsciiData (tk, "LOD_Ratio",m_ratio, m_num_ratios)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 13: {
                        if ((m_lod_options_mask & TKO_LOD_Threshold) != 0) {
							if ((status = GetAsciiData (tk,"LOD_Threshold", m_num_thresholds)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 14: {
                        if ((m_lod_options_mask & TKO_LOD_Threshold) != 0) {
							if ((status = GetAsciiData (tk,"LOD_Threshold", m_threshold, m_num_thresholds)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 15: {
                        if ((m_lod_options_mask & TKO_LOD_Threshold) != 0 &&
                            tk.GetVersion() >= 908) {
								if ((status = GetAsciiData (tk,"Threshold_Type", m_threshold_type)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 16: {
                        if ((m_lod_options_mask & m_lod_options_value & TKO_LOD_Usefulness_Heuristic) != 0) {
							if ((status = GetAsciiData (tk,"LOD_Usefullness_Heuristic", m_heuristic)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                    }
                }
            }
            m_stage++;
        }   nobreak;


        // tessellation
        case 20: {
            if ((m_mask[1] & m_value[1] & TKO_Rendo_Tessellation) != 0) {
                switch (m_progress) {
                    case 0: {
						if ((status = GetAsciiData (tk,"Rendo_Tessellation", m_tessellations)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 1: {
                        if ((m_tessellations & TKO_Tessellation_Cylinder) != 0) {
							if ((status = GetAsciiData (tk,"Tessellation_Cylinder", m_num_cylinder)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 2: {
                        if ((m_tessellations & TKO_Tessellation_Cylinder) != 0) {
							if ((status = GetAsciiData (tk,"Tessellation_Cylinder", m_cylinder, m_num_cylinder)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 3: {
                        if ((m_tessellations & TKO_Tessellation_Sphere) != 0) {
							if ((status = GetAsciiData (tk,"Tessellation_Sphere", m_num_sphere)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 4: {
                        if ((m_tessellations & TKO_Tessellation_Sphere) != 0) {
							if ((status = GetAsciiData (tk,"Tessellation_Sphere", m_sphere, m_num_sphere)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                    }
                }
            }
            m_stage++;
        }   nobreak;

        // transparency style
        case 21: {
            if ((m_mask[1] & m_value[1] & (TKO_Rendo_Transparency_Style|TKO_Rendo_Transparency_Options)) != 0) {
                switch (m_progress) {
                    case 0: {
						if ((status = GetAsciiHex (tk,"Rendo_Transparency_Options", m_transparency_options)) != TK_Normal)
							return status;
                        m_progress++;
                    }

                    case 1: {
                        if ((m_transparency_options & TKO_Transparency_Peeling_Layers) != 0) {
							if ((status = GetAsciiHex (tk,"Transparency_Peeling_Layers", m_depth_peeling_layers)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 2: {
                        if ((m_transparency_options & TKO_Transparency_Peeling_Min_Area) != 0) {
							if ((status = GetAsciiData (tk,"Transparency_Peeling_Min_Area", m_depth_peeling_min_area)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                    }
                }
            }
            m_stage++;
        }   nobreak;


        // cut geometry
        case 22: {
            if ((m_mask[1] & m_value[1] & TKO_Rendo_Cut_Geometry) != 0) {
                switch (m_progress) {
                    case 0: {
						if ((status = GetAsciiData (tk,"Rendo_Cut_Geometry", m_cut_geometry)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 1: {
                        if ((m_cut_geometry & TKO_Cut_Geometry_Level) != 0) {
							if ((status = GetAsciiData (tk,"Cut_Geometry_Level", m_cut_geometry_level)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 2: {
                        if ((m_cut_geometry & TKO_Cut_Geometry_Tolerance) != 0) {
							if ((status = GetAsciiData (tk,"Cut_Geometry_Tolerance", m_cut_geometry_tolerance)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 3: {
                        if ((m_cut_geometry & TKO_Cut_Geometry_Match_Color) != 0) {
							if ((status = GetAsciiData (tk,"Cut_Geometry_Match_Color", m_cut_geometry_match)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                    }
                }
            }
            m_stage++;
        }   nobreak;


        // depth range
        case 23: {
            if ((m_mask[1] & m_value[1] & TKO_Rendo_Depth_Range) != 0) {
				if ((status = GetAsciiData (tk,"Rendo_Depth_Range", m_depth_range, 2)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        // mask transform
        case 24: {
            if ((m_mask[1] & m_value[1] & TKO_Rendo_Mask_Transform) != 0) {
				if ((status = GetAsciiHex (tk,"Rendo_Mask_Transform", m_mask_transform)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        // image scale
        case 25: {
            if ((m_mask[1] & m_value[1] & TKO_Rendo_Image_Scale) != 0) {
				if ((status = GetAsciiData (tk,"Rendo_Image_Scale", m_image_scale, 2)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

         // simple shadow
        case 26: {
            if ((m_mask[1] & m_value[1] & TKO_Rendo_Simple_Shadow) != 0) {
                switch (m_progress) {
                    case 0: {
						if ((status = GetAsciiHex (tk,"Rendo_Simple_Shadow_Mask", m_simple_shadow)) != TK_Normal)
                            return status;
                        m_progress++;
                    }

                    case 1: {
                        if ((m_simple_shadow & TKO_Simple_Shadow_Extended) != 0) {
							if ((status = GetAsciiHex (tk,"Simple_Shadow_Extended", m_byte)) != TK_Normal)
                                return status;
                            m_simple_shadow |= m_byte << 8;
                        }
                        m_progress++;
                    }

                    case 2: {
                        if ((m_simple_shadow & TKO_Simple_Shadow_Plane) != 0) {
							if ((status = GetAsciiData (tk,"Simple_Shadow_Plane", m_simple_shadow_plane, 4)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 3: {
                        if ((m_simple_shadow & TKO_Simple_Shadow_Light_Direction) != 0) {
							if ((status = GetAsciiData (tk,"Simple_Shadow_Light_Direction", m_simple_shadow_light, 3)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 4: {
                        if ((m_simple_shadow & TKO_Simple_Shadow_Color) != 0) {

							if ((status = GetAsciiData (tk,"Simple_Shadow_RGB", m_simple_shadow_color, 3)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 5: {
                        if ((m_simple_shadow & TKO_Simple_Shadow_Blur) != 0) {
							if ((status = GetAsciiData (tk,"Simple_Shadow_Blur", m_simple_shadow_blur)) != TK_Normal)
                                return status;
                        }
                        m_progress++;
                    }

                    case 6: {
                        if ((m_simple_shadow & TKO_Simple_Shadow_Resolution) != 0) {
							if ((status = GetAsciiData (tk,"Simple_Shadow_Resolution", m_simple_shadow_resolution)) != TK_Normal)
                                return status;
                        }
                        m_progress = 0;
                    }
                }
            }
            m_stage++;
        }   nobreak;

		// Geometry options
        case 27: {
            if ((m_mask[1] & m_value[1] & TKO_Rendo_Geometry_Options) != 0) {
				switch (m_progress) {
					case 0: {
						if ((status = GetAsciiHex (tk,"Rendo_Geometry_Options", m_geometry_options)) != TK_Normal)
							return status;
						m_progress++;
					}

					case 1: {
						if ((m_geometry_options & TKO_Geometry_Options_Dihedral) != 0) {
							if ((status = GetAsciiData (tk,"Geometry_Options_Dihedral", m_dihedral)) != TK_Normal)
								return status;
						}
						m_progress = 0;
					}
				}
			}

            m_stage++;
        }   nobreak;


		// image tint
		case 28: {

            if ((m_mask[1] & m_value[1] & TKO_Rendo_Image_Tint) != 0) {
		
				if ((status = GetAsciiData (tk,"Image_Tint_RGB", m_image_tint_color, 3)) != TK_Normal)
					return status;
            }
            m_stage++;

		} nobreak;

		// general displacement
		case 29: {

			if ((m_mask[1] & m_value[1] & TKO_Rendo_General_Displacement) != 0 &&
				tk.GetVersion() >= 1335) {

				if ((status = GetAsciiData (tk,"General_Displacement", m_general_displacement)) != TK_Normal)
					return status;
			}
			else
				m_mask[1] &= ~TKO_Rendo_General_Displacement;

			m_stage++;

		} nobreak;

		// join cutoff angle
		case 30: {

			if ((m_mask[1] & m_value[1] & TKO_Rendo_Join_Cutoff_Angle) != 0) {

				if ((status = GetAsciiData (tk,"Join Cutoff Angle", m_join_cutoff_angle)) != TK_Normal)
					return status;
			}
			m_stage++;

				 } nobreak;
        case 31: {
			// always last or not
               m_stage++;
        }   nobreak;
		
		case 32: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Heuristics::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
    int				mask = m_mask;

	if (tk.GetTargetVersion() < 705)
        mask &= ~TKO_Heuristic_Extended;
    if (tk.GetTargetVersion() < 1201)
		mask &= ~TKO_Heuristic_Culling;
    if (tk.GetTargetVersion() < 1401)
		mask &= ~TKO_Heuristic_Ordered_Weights;
	if (tk.GetTargetVersion() < 1401)
        mask &= ~TKO_Heuristic_Internal_Polyline_Limit;

	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            unsigned short      word = (unsigned short)(mask & 0x0000FFFF);
				
            if ((status = PutAsciiHex (tk,"Mask_Extended",word)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((mask & TKO_Heuristic_Extended)) {
                unsigned short      word = (unsigned short)(m_mask >> 16);
				
                if ((status = PutAsciiHex (tk, "Mask_Extended", (int)word)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
            unsigned short      word = (unsigned short)(m_value & 0x0000FFFF);
			PutTab t(&tk);
            if ((status = PutAsciiHex (tk,"Value_Extended", (int)word)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((mask & TKO_Heuristic_Extended)) {
                unsigned short      word = (unsigned short)(m_value >> 16);
				
                if ((status = PutAsciiHex (tk,"Value_Extended", (int)word)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if ((m_mask & m_value & TKO_Heuristic_Related_Select_Limit) != 0) {
				
                if ((status = PutAsciiData (tk,"Related_Select_Limit", m_related)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t(&tk);
            if ((m_mask & m_value & TKO_Heuristic_Internal_Shell_Limit) != 0) {
				
                if ((status = PutAsciiData (tk,"Internal_Select_Limit", m_internal_shell)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 7: {
			PutTab t(&tk);
            if ((m_mask & m_value & TKO_Heuristic_Extras) != 0) {
				
                if ((status = PutAsciiHex (tk,"Extras", m_extras)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 8: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Heuristic_Culling)) {
				
                if ((status = PutAsciiHex (tk,"Culling", m_culling)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 9: {
			PutTab t(&tk);
            if ((mask & TKO_Heuristic_Culling) &&
                (m_culling & TKO_Heur_Obscuration_Culling)) {
					
                if ((status = PutAsciiData (tk, "Pixel_Threshold", m_pixel_threshold)) != TK_Normal)
                    return status;

                _W3DTK_REQUIRE_VERSION( 1200 );
            }
            m_stage++;
        }   nobreak;

        case 10: {
			PutTab t(&tk);
            if ((mask & TKO_Heuristic_Culling) &&
                (m_culling & TKO_Heur_Extent_Culling)) {
					
                if ((status = PutAsciiData (tk,"Extent_Culling", m_maximum_extent)) != TK_Normal)
                    return status;

                _W3DTK_REQUIRE_VERSION( 1200 );
            }
            m_stage++;
        }   nobreak;

        case 11: {
            if ((mask & TKO_Heuristic_Ordered_Weights) ) {
                if ((status = PutAsciiData (tk, "Ordered_Weight_Mask", m_ordered_weights_mask)) != TK_Normal)
                    return status;
				m_progress = 0;
            }
            m_stage++;
        }   nobreak;

        case 12: {
            if ((mask & TKO_Heuristic_Ordered_Weights) ) {
				while (m_progress < TKO_Heur_Order_Count) {
					if ((m_ordered_weights_mask & (1<<m_progress)) != 0) {
						char	tmp[64];

						sprintf (tmp, "Ordered_Weight_%d", m_progress);
						if ((status = PutAsciiData (tk, tmp, m_ordered_weights[m_progress])) != TK_Normal)
							return status;
					}
					m_progress++;
				}
				m_progress = 0;
            }
            m_stage++;
        }   nobreak;

        case 13: {
			PutTab t(&tk);
            if ((m_mask & m_value & TKO_Heuristic_Internal_Polyline_Limit) != 0) {
				
                if ((status = PutAsciiData (tk,"Internal_Polyline_Limit", m_internal_polyline)) != TK_Normal)
                    return status;
            }
			m_stage++;
        }   nobreak;

		case 14: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   nobreak;

		case 15: {
			PutTab t(&tk);
            if ((mask & TKO_Heuristic_Culling) &&
                (m_culling & TKO_Heur_Maximum_Extent_Mode)) {
					
                if ((status = PutAsciiData (tk,"Maximum_Extent_Mode", m_maximum_extent_mode)) != TK_Normal)
                    return status;
            }
            m_stage++;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Heuristics::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
	switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Mask_Extended", m_mask)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            if (m_mask & TKO_Heuristic_Extended) {
				if ((status = GetAsciiHex (tk,"Mask_Extended", m_unsigned_short)) != TK_Normal)
                    return status;
                m_mask |= m_unsigned_short << 16;
            }
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiHex (tk, "Value_Extended",m_value)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
            if (m_mask & TKO_Heuristic_Extended) {
				if ((status = GetAsciiHex (tk, "Value_Extended",m_unsigned_short)) != TK_Normal)
                    return status;
                m_value |= m_unsigned_short << 16;
            }
            m_stage++;
        }   nobreak;

        case 4: {
            if ((m_mask & m_value & TKO_Heuristic_Related_Select_Limit) != 0) {
				if ((status = GetAsciiData (tk,"Related_Select_Limit", m_related)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
            if ((m_mask & m_value & TKO_Heuristic_Internal_Shell_Limit) != 0) {
				if ((status = GetAsciiData (tk,"Internal_Select_Limit", m_internal_shell)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 6: {
            if ((m_mask & m_value & TKO_Heuristic_Extras) != 0) {
				if ((status = GetAsciiHex (tk,"Extras", m_extras)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 7: {
            if (tk.GetVersion() > 705 && (m_mask & TKO_Heuristic_Culling) &&
				(m_mask & m_value & TKO_Heuristic_Culling) != 0) {
					if ((status = GetAsciiHex (tk,"Culling", m_culling)) != TK_Normal)
                    return status;
            }
			else
				m_culling = 0;
            m_stage++;
        }   nobreak;

        case 8: {
            if (tk.GetVersion() > 1200 && (m_mask & TKO_Heuristic_Culling) &&
                (m_culling & TKO_Heur_Obscuration_Culling)) {
					if ((status = GetAsciiData (tk,"Pixel_Threshold", m_pixel_threshold)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 9: {
            if (tk.GetVersion() > 1200 && (m_mask & TKO_Heuristic_Culling) &&
                (m_culling & TKO_Heur_Extent_Culling)) {
					if ((status = GetAsciiData (tk,"Extent_Culling", m_maximum_extent)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 10: {
            if ((m_mask & TKO_Heuristic_Ordered_Weights) ) {
                if ((status = GetAsciiData (tk, "Ordered_Weight_Mask", m_ordered_weights_mask)) != TK_Normal)
                    return status;
				m_progress = 0;
            }
            m_stage++;
        }   nobreak;

        case 11: {
            if ((m_mask & TKO_Heuristic_Ordered_Weights) ) {
				while (m_progress < TKO_Heur_Order_Count) {
					if ((m_ordered_weights_mask & (1<<m_progress)) != 0) {
						char	tmp[64];

						sprintf (tmp, "Ordered_Weight_%d", m_progress);
						if ((status = GetAsciiData (tk, tmp, m_ordered_weights[m_progress])) != TK_Normal)
							return status;
					}
					m_progress++;
				}
				m_progress = 0;
            }
            m_stage++;
        }   nobreak;

        case 12: {
            if ((m_mask & m_value & TKO_Heuristic_Internal_Polyline_Limit) != 0) {
				if ((status = GetAsciiData (tk,"Internal_Polyline_Limit", m_internal_polyline)) != TK_Normal)
                    return status;
            }
               m_stage++;
        }   nobreak;
		
		case 13: {
			if ((status = ReadEndOpcode(tk)) != TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


////////////////////////////////////////

TK_Status TK_Visibility::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            unsigned char       byte = (unsigned char) (m_mask & 0x000000FF);
			PutTab t(&tk);
            if ((status = PutAsciiMask (tk,"Mask", byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
            unsigned char       byte = (unsigned char) (m_value & 0x000000FF);
			PutTab t(&tk);
            if ((status = PutAsciiHex (tk,"Value", byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended) != 0) {
                unsigned short      word = (unsigned short) ((m_mask>>8) & 0x0000FFFF);
				
                if ((status = PutAsciiHex(tk, "Geo_Extended_Mask",(int)word)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended) != 0) {
                unsigned short      word = (unsigned short) ((m_value>>8) & 0x0000FFFF);
					
                if ((status = PutAsciiHex (tk,"Geo_Extended_Value", (int)word)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended2) != 0) {
                int      byte = (unsigned short) ((m_mask>>24) & 0x000000FF);
					
                if ((status = PutAsciiHex(tk, "Geo_Extended2_Mask",byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended2) != 0) {
                int     byte = (unsigned short) ((m_value>>24) & 0x000000FF);
					
                if ((status = PutAsciiHex (tk,"Geo_Extended2_Value", byte)) != TK_Normal)
                    return status;
            }
              
			m_stage++;
        }   nobreak;

		case 7: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;

		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Visibility::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Mask", m_mask)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiHex (tk,"Value", m_value)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
            if ((m_mask & TKO_Geo_Extended) != 0) {
				if ((status = GetAsciiHex (tk,"Geo_Extended_Mask", m_unsigned_short)) != TK_Normal)
                    return status;
                m_mask |= m_unsigned_short << 8;
            }
            m_stage++;
        }   nobreak;

        case 3: {
            if ((m_mask & TKO_Geo_Extended) != 0) {
				if ((status = GetAsciiHex (tk,"Geo_Extended_Value", m_unsigned_short)) != TK_Normal)
                    return status;
                m_value |= m_unsigned_short << 8;
            }
            m_stage++;
        }   nobreak;

        case 4: {
            if ((m_mask & TKO_Geo_Extended2) != 0) {
				if ((status = GetAsciiHex (tk,"Geo_Extended2_Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 24;
            }
            m_stage++;
        }   nobreak;

        case 5: {
            if ((m_mask & TKO_Geo_Extended2) != 0) {
				if ((status = GetAsciiHex (tk,"Geo_Extended2_Value", m_byte)) != TK_Normal)
                    return status;
                m_value |= m_byte << 24;
            }
               m_stage++;
        }   nobreak;
		
		case 6: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

////////////////////////////////////////

TK_Status TK_Selectability::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if (tk.GetTargetVersion() < 650)
                return TK_Normal;

            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended) != 0) {
                unsigned short      word = (unsigned short)(m_mask & 0x0000FFFF);
				
                if ((status = PutAsciiHex (tk,"Mask", word)) != TK_Normal)
                    return status;
            }
            else {
                unsigned char       byte = (unsigned char)(m_mask & 0x000000FF);
                if ((status = PutAsciiMask (tk,"Mask", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended) != 0) {
                unsigned short      word = (unsigned short)(m_down & 0x0000FFFF);
                
                if ((status = PutAsciiHex (tk,"Mouse_Down", word)) != TK_Normal)
                    return status;
            }
            else {
                unsigned char       byte = (unsigned char)(m_down & 0x000000FF);
                  
                if ((status = PutAsciiHex (tk,"Mouse_Down", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended) != 0) {
                unsigned short      word = (unsigned short)(m_up & 0x0000FFFF);
                  
                if ((status = PutAsciiHex (tk,"Mouse_Up", (int)word)) != TK_Normal)
                    return status;
            }
            else {
                unsigned char       byte = (unsigned char)(m_up & 0x000000FF);
                   
                if ((status = PutAsciiHex (tk,"Mouse_Up", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended) != 0) {
                unsigned short      word = (unsigned short)(m_move_down & 0x0000FFFF);
                       
                if ((status = PutAsciiHex (tk,"Mouse_Move_Down", (int)word)) != TK_Normal)
                    return status;
            }
            else {
                unsigned char       byte = (unsigned char)(m_move_down & 0x000000FF);
                    
                if ((status = PutAsciiHex (tk,"Mouse_Move_Down", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended) != 0) {
                unsigned short      word = (unsigned short)(m_move_up & 0x0000FFFF);
                       
                if ((status = PutAsciiHex (tk,"Mouse_Move_Up" ,(int)word)) != TK_Normal)
                    return status;
            }
            else {
                unsigned char       byte = (unsigned char)(m_move_up & 0x000000FF);
                        
                if ((status = PutAsciiHex (tk,"Mouse_Move_Up", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t(&tk);
            if ((m_mask & TKO_Geo_Extended) != 0) {
                unsigned short      word = (unsigned short)(m_invisible & 0x0000FFFF);
                     
                if ((status = PutAsciiHex (tk,"Invisible", (int)word)) != TK_Normal)
                    return status;
            }
            else {
                unsigned char       byte = (unsigned char)(m_invisible & 0x000000FF);
                  
                if ((status = PutAsciiHex (tk,"Invisible", byte)) != TK_Normal)
                    return status;
            }
               
			m_stage++;
        }   nobreak;

		case 7: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Selectability::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Mask", m_mask)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            if ((m_mask & TKO_Geo_Extended) != 0) {
				if ((status = GetAsciiHex (tk,"Mouse_Down", m_down)) != TK_Normal)
                    return status;
            }
            else {
				if ((status = GetAsciiHex (tk,"Mouse_Down", m_down)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 2: {
            if ((m_mask & TKO_Geo_Extended) != 0) {
				if ((status = GetAsciiHex (tk,"Mouse_Up", m_up)) != TK_Normal)
                    return status;
            }
            else {
				if ((status = GetAsciiHex (tk, "Mouse_Up", m_up)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
            if ((m_mask & TKO_Geo_Extended) != 0) {
				if ((status = GetAsciiHex (tk,"Mouse_Move_Down", m_move_down)) != TK_Normal)
                    return status;
            }
            else {
				if ((status = GetAsciiHex (tk,"Mouse_Move_Down", m_move_down)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
            if ((m_mask & TKO_Geo_Extended) != 0) {
				if ((status = GetAsciiHex (tk,"Mouse_Move_Up", m_move_up)) != TK_Normal)
                    return status;
            }
            else {
				if ((status = GetAsciiHex (tk,"Mouse_Move_Up", m_move_up)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;
		
		case 5: {
			if ((m_mask & TKO_Geo_Extended) != 0) {
				if ((status = GetAsciiHex (tk,"Invisible", m_invisible)) != TK_Normal)
					return status;
			}
			else {
				if ((status = GetAsciiHex (tk,"Invisible", m_invisible)) != TK_Normal)
					return status;
			}
			m_stage++;
		}   nobreak;

		case 6: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

////////////////////////////////////////

TK_Status TK_Matrix::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Matrix_0-3", &m_matrix[0], 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk, "Matrix_4-8",&m_matrix[4], 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Matrix_9-12", &m_matrix[8], 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Matrix_13-16", &m_matrix[12], 3)) != TK_Normal)
                return status;

            m_stage++;
        }   nobreak;

		case 5: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Matrix::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
    
	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
            m_matrix[3] = m_matrix[7] = m_matrix[11] = 0.0f;
            m_matrix[15] = 1.0f;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk, "Matrix_0-3",&m_matrix[0], 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"Matrix_4-8", &m_matrix[4], 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			if ((status = GetAsciiData (tk,"Matrix_9-12", &m_matrix[8], 3)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			if ((status = GetAsciiData (tk,"Matrix_13-16", &m_matrix[12], 3)) != TK_Normal)
                return status;
               m_stage++;
        }   nobreak;
		
		case 5: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

////////////////////////////////////////

TK_Status TK_Size::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            float           value = m_value;

            if (m_units != TKO_Generic_Size_Unspecified)
                value = -value;
			
            if ((status = PutAsciiData (tk,"Value", value)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if (m_units != TKO_Generic_Size_Unspecified) {
                if ((status = PutAsciiData (tk,"Units", (int)m_units)) != TK_Normal)
                    return status;
            }
                
			m_stage++;
        }   nobreak;

		case 3: {

			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Size::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Value", m_value)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            if (m_value < 0.0f) {
				if ((status = GetAsciiData (tk,"Units", m_int)) != TK_Normal)
                    return status;
				m_units = (unsigned char)m_int;
                m_value = -m_value;
            }
            else
                m_units = TKO_Generic_Size_Unspecified;
               m_stage++;
        }   nobreak;
		
		case 2: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

////////////////////////////////////////

TK_Status TK_Enumerated::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk)) != TK_Normal)
                return status;
            m_stage = 1;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Index", (int)m_index)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

		case 2: {

			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Enumerated::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
   
	TK_Status       status = TK_Normal;

	switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Index", m_int)) != TK_Normal)
				 return status;
			m_index = (char)m_int;
            m_stage++;
        }   nobreak;
		
		case 1: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

////////////////////////////////////////

TK_Status TK_Linear_Pattern::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage = 1;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Pattern", m_pattern)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

		case 2: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Linear_Pattern::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    if (m_stage != 0)
        return tk.Error();

	if ((status = GetAsciiData (tk,"Pattern", m_pattern)) != TK_Normal)
        return status;
    m_stage = -1;

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Named::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    if (tk.GetTargetVersion() < 1160)
        return TK_Normal;

    _W3DTK_REQUIRE_VERSION( 1160 );

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if (m_name_length > 0) {
				
				if ((status = PutAsciiData (tk, "Pattern", 255)) != TK_Normal)
                    return status;
            }
            else {
				
                if ((status = PutAsciiData (tk, "Pattern", m_index)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if (m_name_length > 0) {
				
                if ((status = PutAsciiData (tk, "Name_Length", m_name_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if (m_name_length > 0) {
				
                if ((status = PutAsciiData (tk, "Name", m_name, m_name_length)) != TK_Normal)
                    return status;
            }
                m_stage++;
        }   nobreak;

		case 4: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Named::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
 
    TK_Status       status = TK_Normal;

    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk, "Pattern", m_int)) != TK_Normal)
                return status;
            SetIndex (m_int);
            m_stage++;
        }   nobreak;

        case 1: {
            if ((GetIndex() & 0x00FF) == 0x00FF) {
				if ((status = GetAsciiData (tk, "Name_Length", m_int)) != TK_Normal)
                    return status;
                SetName ((unsigned char)m_int);
            }

        }   nobreak;

        case 2: {
            if (m_name_length > 0) {
				if ((status = GetAsciiData (tk, "Name", m_name, m_name_length)) != TK_Normal)
                    return status;
            }
               m_stage++;
        }   nobreak;
		
		case 3: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Conditions::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    if (tk.GetTargetVersion() < 1150)   // skip, not supported
        return status;

    _W3DTK_REQUIRE_VERSION( 1150 );

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            unsigned char      byte;
            
            if (m_length > 65535)
                byte = 255;                 // flag length stored as int following
            else if (m_length > 253)
                byte = 254;                 // flag length stored as short following
            else
                byte = (unsigned char)m_length;
				
            if ((status = PutAsciiData (tk,"Length", (int) byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if (m_length > 65535) {
                if ((status = PutAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                    return status;
            }
            else if (m_length > 253) {
                if ((status = PutAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if (m_length > 0) {
				
                if ((status = PutAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
                    return status;
            }
                m_stage++;
        }   nobreak;

		case 4: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Conditions::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
 
	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Length", m_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            if (m_length == 255) {                  // real length is a following int
				if ((status = GetAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                    return status;
            }
            else if (m_length == 254) {             // real length is a following short
				if ((status = GetAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                    return status;
            }
            SetConditions (m_length);
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
                return status;
               m_stage++;
        }   nobreak;
		
		case 3: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_User_Options::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
           {
			   if (m_length > 0) {
                if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                    return status;
               }
            }
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);

            if (m_length > 0) {
	            if ((status = PutAsciiData (tk,"Length", m_length)) != TK_Normal)
                    return status;
            }

            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if (m_length > 0) {
                if ((status = PutAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			Outdent one_tab(&tk);
            if (m_indices != null) {
                if ((status = m_indices->Write (tk)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			Outdent one_tab(&tk);
            if (m_unicode != null) {
                if ((status = m_unicode->Write (tk)) != TK_Normal)
                    return status;
            }
                 m_stage++;
        }   nobreak;

		case 5: {
			if (m_length > 0) {
			   if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			}
			m_stage = -1;
		}   break;

		default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_User_Options::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
 
	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Length", m_length)) != TK_Normal)
                return status;
			set_options (m_length);
             m_stage++;
        }   nobreak;

        case 1: {
			if( m_length > 0 )
			{
				if ((status = GetAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;
        
		default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Unicode_Options::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    if (tk.GetTargetVersion() < 905)
        return status;

    switch (m_stage) {
        case 0: {
            if (m_length > 0) {
                if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if (m_length > 0) {
                unsigned short      word;
                
                if (m_length > 65534)
                    word = 65535;       // real length will be a following int
                else
                    word = (unsigned short)m_length;
					
                if ((status = PutAsciiData (tk,"Length", (int) word)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if (m_length > 65534) {
                if ((status = PutAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if (m_length > 0) {
				
                if ((status = PutAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
                    return status;
            }
                m_stage++;
        }   nobreak;

		case 4: {
			PutTab t(&tk);
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;


        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Unicode_Options::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk, "Length", m_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            if (m_length == 65535) {        // real length is a following int
				if ((status = GetAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                    return status;
            }
            SetOptions (m_length);
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"String", (short*) m_string, m_length)) != TK_Normal)
                return status;
               m_stage++;
        }   nobreak;
		
		case 3: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_User_Index::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Count", m_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Indices", m_indices, m_count)) != TK_Normal)
                return status;
            m_progress = 0;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            while (m_progress < m_count) {
                int     value = (int)m_values[m_progress];
				
                if ((status = PutAsciiData (tk,"Values", value)) != TK_Normal)
                    return status;
                ++m_progress;
            }
            m_progress = 0;
                 m_stage++;
        }   nobreak;

		case 4: {

			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_User_Index::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Count", m_count)) != TK_Normal)
                return status;
            if (!validate_count (m_count))
                return tk.Error("bad User Index count");
            m_indices = new int [m_count];
            m_values = new long [m_count];
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Indices", m_indices, m_count)) != TK_Normal)
                return status;
            m_current_value = 0;
            m_stage++;
        }   nobreak;

        case 2: {
            while (m_current_value < m_count) {
				if ((status = GetAsciiData (tk,"Values", m_int)) != TK_Normal)
                    return status;
                m_values[m_current_value++] = m_int;
            }
            m_current_value = 0;
               m_stage++;
        }   nobreak;
		
		case 3: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_User_Value::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            int         value = (int)m_value;
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Value", value)) != TK_Normal)
                return status;
                m_stage++;
        }   nobreak;

		case 2: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_User_Value::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Value", m_int)) != TK_Normal)
                return status;
            m_value = (long)m_int;
              m_stage++;
        }   nobreak;
		
		case 1: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_Camera::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    if (Opcode() == TKE_View && tk.GetTargetVersion() < 610)
        return status;      // view added in 6.10, so don't generate one for older version files

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiFlag (tk,"Projection", m_projection)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Setting", m_settings, 11)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((m_projection & TKO_Camera_Oblique_Y) != 0) {
				
                if ((status = PutAsciiData (tk,"Camera_Oblique_Y", m_settings[11])) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((m_projection & TKO_Camera_Oblique_X) != 0) {
				
                if ((status = PutAsciiData (tk,"Camera_Oblique_X", m_settings[12])) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if ((m_projection & TKO_Camera_Near_Limit) != 0 && tk.GetTargetVersion() > 1000) {
				
                if ((status = PutAsciiData (tk,"Camera_Near_Limit", m_settings[13])) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t(&tk);
            if (Opcode() == TKE_View) {
                //unsigned char       length = (unsigned char)m_length;
				
                if ((status = PutAsciiData (tk,"Length", m_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 7: {
			PutTab t(&tk);
            if (Opcode() == TKE_View && m_length > 0) {
				
                if ((status = PutAsciiData (tk, "Name", m_name, m_length)) != TK_Normal)
                    return status;
            }
			m_stage++;
		}   nobreak;
		
		case 8: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Camera::ReadAscii(BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
 
	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Projection", m_byte)) != TK_Normal)
                return status;
		    m_projection = (unsigned char)m_byte;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk, "Setting",m_settings, 11)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
            if ((m_projection & TKO_Camera_Oblique_Y) != 0) {
				if ((status = GetAsciiData (tk,"Camera_Oblique_Y", m_settings[11])) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
            if ((m_projection & TKO_Camera_Oblique_X) != 0) {
				if ((status = GetAsciiData (tk, "Camera_Oblique_X",m_settings[12])) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
            if ((m_projection & TKO_Camera_Near_Limit) != 0) {
				if ((status = GetAsciiData (tk,"Camera_Near_Limit", m_settings[13])) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
            if (Opcode() == TKE_View) {
				if ((status = GetAsciiData (tk,"Length", m_int)) != TK_Normal)
                    return status;
                SetView (m_int);
            }
            m_stage++;
        }   nobreak;

        case 6: {
            if (Opcode() == TKE_View != 0 && m_length > 0) {
				if ((status = GetAsciiData (tk,"Name", m_name, m_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;
		
		case 7: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Window::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Window", m_window, 4)) != TK_Normal)
                return status;
                 m_stage++;
        }   nobreak;

		case 2: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;


        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Window::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
  
	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Window", m_window, 4)) != TK_Normal)
                return status;
               m_stage++;
        }   nobreak;
		
		case 1: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Text_Font::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
    int	    mask = m_mask;

	PutTab t0(&tk);

    if( (mask & ~0x00007FFF) != 0)
    {
        _W3DTK_REQUIRE_VERSION( 1210 );
    }

    if( (mask & ~0x0003FFFF) != 0)
    {
        _W3DTK_REQUIRE_VERSION( 1220 );
    }

    if (tk.GetTargetVersion() < 1210)
        mask &= 0x00007FFF;
    else if (tk.GetTargetVersion() < 1220)
        mask &= 0x0003FFFF;

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            unsigned char       byte = (unsigned char)(mask & 0x000000FF);
			PutTab t(&tk);
            if ((status = PutAsciiMask (tk,"Mask", byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
            unsigned char       byte = (unsigned char)(m_value & 0x000000FF);
			PutTab t(&tk);
            if ((status = PutAsciiHex (tk,"Value", byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((mask & TKO_Font_Extended) != 0) {
                unsigned char       byte = (unsigned char)((mask>>8) & 0x000000FF);
				
                if ((status = PutAsciiMask(tk,"Extended_Mask", byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((mask & TKO_Font_Extended) != 0) {
                unsigned char       byte = (unsigned char)((m_value>>8) & 0x000000FF);
				
                if ((status = PutAsciiData (tk,"Extended_Value", (int)byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 5: {
			PutTab t(&tk);
            if ((mask & TKO_Font_Extended2) != 0) {
                unsigned short      word = (unsigned short)((mask>>16) & 0x0000FFFF);
				
                if ((status = PutAsciiData (tk,"Font_Extended2", (int)word)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t(&tk);
            if ((mask & TKO_Font_Extended2) != 0) {
                unsigned short      word = (unsigned short)((m_value>>16) & 0x0000FFFF);
				
                if ((status = PutAsciiData (tk,"Font_Extended2", (int)word)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 7: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Names) != 0) {
                // need code here to handle length > 255 ?
                if ((status = PutAsciiData (tk,"Name_Length", m_names_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 8: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Names) != 0) {
				
                if ((status = PutAsciiData (tk,"Font_Name", m_names, m_names_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 9: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Size) != 0) {
				
                if ((status = PutAsciiData (tk,"Font_Size", m_size)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 10: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Size) != 0) {
				
                if ((status = PutAsciiData (tk,"Size_Units", (int)m_size_units)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 11: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Size_Tolerance) != 0) {
			
                if ((status = PutAsciiData (tk,"Font_Size_Tolerence", m_tolerance)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 12: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Size_Tolerance) != 0) {
				
                if ((status = PutAsciiData (tk,"Font_Size_Tolerence", (int)m_tolerance_units)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 13: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Transforms) != 0) {
				
                if ((status = PutAsciiData (tk,"Font_Transforms", (int)m_transforms)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 14: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Rotation) != 0) {
				
                if ((status = PutAsciiData (tk,"Font_Rotation", m_rotation)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 15: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Slant) != 0) {
				
                if ((status = PutAsciiData (tk,"Font_Slant", m_slant)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 16: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Width_Scale) != 0) {
				
                if ((status = PutAsciiData (tk,"Font_Width_Scale", m_width_scale)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 17: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Extra_Space) != 0) {
				
                if ((status = PutAsciiData (tk,"Font_Extra_Space", m_extra_space)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 18: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Extra_Space) != 0) {
				
                if ((status = PutAsciiData (tk,"Font_Extra_Space_Units", (int)m_space_units)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 19: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Line_Spacing) != 0) {
				
                if ((status = PutAsciiData (tk,"Font_Line_Spacing", m_line_spacing)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 20: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Greeking_Limit) != 0) {
				
                if ((status = PutAsciiData (tk,"Greeking_Limit", m_greeking_limit)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 21: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Greeking_Limit) != 0) {
				
                if ((status = PutAsciiData (tk,"Greeking_Units", (int)m_greeking_units)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 22: {
			PutTab t(&tk);
            if ((mask & m_value & TKO_Font_Greeking_Mode) != 0) {
				
                if ((status = PutAsciiData (tk, "Greeking_Mode", (int)m_greeking_mode)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 23: {
			PutTab t(&tk);
            if ((mask & TKO_Font_Renderer) != 0) {
				if (tk.GetTargetVersion() < 1325) {
					
					if ((status = PutAsciiData (tk, "Font_Renderer", m_renderers[0])) != TK_Normal)
						return status;
				}
				else {
					
					if ((status = PutAsciiData (tk, "Font_Renderer", m_renderers[1])) != TK_Normal)
						return status;
				}
            }
            m_stage++;
        }   nobreak;

        case 24: {
			PutTab t(&tk);
			if ((mask & TKO_Font_Renderer) != 0 && tk.GetTargetVersion() >= 1325) {
				
                if ((status = PutAsciiData (tk, "Font_Renderer_Cutoff", m_renderer_cutoff)) != TK_Normal)
                    return status;
                
                _W3DTK_REQUIRE_VERSION( 1325 );

            }
            m_stage++;
        }   nobreak;

        case 25: {
			PutTab t(&tk);
			if ((mask & TKO_Font_Renderer) != 0 && tk.GetTargetVersion() >= 1325) {
				
                if ((status = PutAsciiData (tk, "Font_Renderer_Cutoff_Units", (int)m_renderer_cutoff_units)) != TK_Normal)
                    return status;

                _W3DTK_REQUIRE_VERSION( 1325 );

            }
            m_stage++;
        }   nobreak;

        case 26: {
			PutTab t(&tk);
            if ((mask & TKO_Font_Preference) != 0) {
				
                if ((status = PutAsciiData (tk, "Font_Preferences", m_preferences[0])) != TK_Normal)
                    return status;


                if ((status = PutAsciiData (tk, "Font_Preferences", m_preferences[1])) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 27: {
			PutTab t(&tk);
			if ((mask & TKO_Font_Preference) != 0) {
				    if ((status = PutAsciiData (tk, "Font_Preferences_Cutoff", m_preference_cutoff)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 28: {
			PutTab t(&tk);
			if ((mask & TKO_Font_Preference) != 0) {
				
                if ((status = PutAsciiData (tk, "Font_Preferences_Cutoff", (int)m_preference_cutoff_units)) != TK_Normal)
                    return status;
            }
               m_stage++;
        }   nobreak;

		case 29: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;


        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Text_Font::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
 
	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Mask", m_mask)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiHex (tk,"Value", m_value)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
            if ((m_mask & TKO_Font_Extended) != 0) {
				if ((status = GetAsciiHex (tk,"Extended_Mask", m_byte)) != TK_Normal)
                    return status;
                m_mask |= m_byte << 8;
            }
            m_stage++;
        }   nobreak;

        case 3: {
            if ((m_mask & TKO_Font_Extended) != 0) {
				if ((status = GetAsciiData (tk,"Extended_Value", m_byte)) != TK_Normal)
                    return status;
                m_value |= m_byte << 8;
            }
            m_stage++;
        }   nobreak;

        case 4: {
            if ((m_mask & TKO_Font_Extended2) != 0) {
				if ((status = GetAsciiData (tk,"Font_Extended2", m_unsigned_short)) != TK_Normal)
                    return status;
                m_mask |= m_unsigned_short << 16;
            }
            m_stage++;
        }   nobreak;

        case 5: {
            if ((m_mask & TKO_Font_Extended2) != 0) {
				if ((status = GetAsciiData (tk,"Font_Extended2", m_unsigned_short)) != TK_Normal)
                    return status;
                m_value |= m_unsigned_short << 16;
            }
            m_stage++;
        }   nobreak;

        case 6: {
            if ((m_mask & m_value & TKO_Font_Names) != 0) {
				if ((status = GetAsciiData (tk, "Name_Length", m_names_length)) != TK_Normal)
                    return status;
                // would add code here if value indicated real length > 255
                SetNames (m_names_length);
            }
            m_stage++;
        }   nobreak;

        case 7: {
            if ((m_mask & m_value & TKO_Font_Names) != 0) {
				if ((status = GetAsciiData (tk,"Font_Name", m_names, m_names_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 8: {
            if ((m_mask & m_value & TKO_Font_Size) != 0) {
				if ((status = GetAsciiData (tk,"Font_Size", m_size)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 9: {
            if ((m_mask & m_value & TKO_Font_Size) != 0) {
				if ((status = GetAsciiData (tk,"Size_Units", m_int)) != TK_Normal)
                    return status;
				m_size_units = (unsigned char)m_int;
            }
            m_stage++;
        }   nobreak;

        case 10: {
            if ((m_mask & m_value & TKO_Font_Size_Tolerance) != 0) {
				if ((status = GetAsciiData (tk,"Font_Size_Tolerence", m_tolerance)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 11: {
            if ((m_mask & m_value & TKO_Font_Size_Tolerance) != 0) {
				if ((status = GetAsciiData (tk, "Font_Size_Tolerence",m_int)) != TK_Normal)
                    return status;
				m_tolerance_units = (unsigned char)m_int;
            }
            m_stage++;
        }   nobreak;

        case 12: {
            if ((m_mask & m_value & TKO_Font_Transforms) != 0) {
				if ((status = GetAsciiData (tk,"Font_Transforms", m_int)) != TK_Normal)
                    return status;
				m_transforms = (unsigned char)m_int;
            }
            m_stage++;
        }   nobreak;

        case 13: {
            if ((m_mask & m_value & TKO_Font_Rotation) != 0) {
				if ((status = GetAsciiData (tk,"Font_Rotation", m_rotation)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 14: {
            if ((m_mask & m_value & TKO_Font_Slant) != 0) {
				if ((status = GetAsciiData (tk,"Font_Slant", m_slant)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 15: {
            if ((m_mask & m_value & TKO_Font_Width_Scale) != 0) {
				if ((status = GetAsciiData (tk,"Font_Width_Scale", m_width_scale)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 16: {
            if ((m_mask & m_value & TKO_Font_Extra_Space) != 0) {
				if ((status = GetAsciiData (tk,"Font_Extra_Space", m_extra_space)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 17: {
            if ((m_mask & m_value & TKO_Font_Extra_Space) != 0) {
				if ((status = GetAsciiData (tk,"Font_Extra_Space_Units", m_int)) != TK_Normal)
                    return status;
				m_space_units = (unsigned char) m_int;
            }
            m_stage++;
        }   nobreak;

        case 18: {
            if ((m_mask & m_value & TKO_Font_Line_Spacing) != 0) {
				if ((status = GetAsciiData (tk,"Font_Line_Spacing", m_line_spacing)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 19: {
            if ((m_mask & m_value & TKO_Font_Greeking_Limit) != 0) {
				if ((status = GetAsciiData (tk,"Greeking_Limit", m_greeking_limit)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 20: {
            if ((m_mask & m_value & TKO_Font_Greeking_Limit) != 0) {
				if ((status = GetAsciiData (tk,"Greeking_Units", m_int )) != TK_Normal)
                    return status;
				m_greeking_units = (unsigned char) m_int;
            }
            m_stage++;
        }   nobreak;

        case 21: {
            if ((m_mask & m_value & TKO_Font_Greeking_Mode) != 0) {
				if ((status = GetAsciiData (tk,"Greeking_Mode", m_int)) != TK_Normal)
                    return status;
			 m_greeking_mode = (unsigned char)m_int;
            }
            m_stage++;
        }   nobreak;

        case 22: {
            if ((m_mask & TKO_Font_Renderer) != 0) {
				if ((status = GetAsciiData (tk,"Font_Renderer", m_char)) != TK_Normal)
                    return status;

				if (tk.GetVersion() < 1325)
					m_renderers[0] = m_renderers[1] = (int)m_char;
				else {
					if ((m_renderers[0] = (int)m_char & 0x0F) == 0x0F)
						m_renderers[0] = TKO_Font_Renderer_Undefined;
					if ((m_renderers[1] = (int)(m_char >> 4) & 0x0F) == 0x0F)
						m_renderers[1] = TKO_Font_Renderer_Undefined;
				}
            }
            m_stage++;
        }   nobreak;

        case 23: {
			if ((m_mask & TKO_Font_Renderer) != 0 && tk.GetVersion() >= 1325) {
				if ((status = GetAsciiData (tk,"Font_Renderer_Cutoff", m_renderer_cutoff)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 24: {
			if ((m_mask & TKO_Font_Renderer) != 0 && tk.GetVersion() >= 1325) {
				if ((status = GetAsciiData (tk, "Font_Renderer_Cutoff_Units", m_int )) != TK_Normal)
                    return status;
				m_renderer_cutoff_units  = (unsigned char)m_int;
            }
            m_stage++;
        }   nobreak;

        case 25: {
            if ((m_mask & TKO_Font_Preference) != 0) {
				if ((status = GetAsciiData (tk,"Font_Preferences", m_char)) != TK_Normal)
                    return status;

				if ((m_preferences[0] = (int)m_char & 0x0F) == 0x0F)
					m_preferences[0] = TKO_Font_Preference_Undefined;
				if ((m_preferences[1] = (int)(m_char >> 4) & 0x0F) == 0x0F)
					m_preferences[1] = TKO_Font_Preference_Undefined;
            }
            m_stage++;
        }   nobreak;

        case 26: {
			if ((m_mask & TKO_Font_Preference) != 0) {
				if ((status = GetAsciiData (tk,"Font_Preferences_Cutoff", m_preference_cutoff)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 27: {
			if ((m_mask & TKO_Font_Preference) != 0) {
				if ((status = GetAsciiData (tk,"Font_Preferences_Cutoff", m_int )) != TK_Normal)
                    return status;
				m_preference_cutoff_units  = (unsigned char)m_int;
            }
                m_stage++;
        }   nobreak;
		
		case 28: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Texture::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
    int             flags = m_flags;
	
	PutTab t0(&tk);

    if ((flags & ~0x00007FFF) != 0)
    {
        _W3DTK_REQUIRE_VERSION( 1165 );
    }

    if (tk.GetTargetVersion() < 1165)
        flags &= 0x00007FFF;

    if (m_flags == 0 && streq (m_name, m_image)) {
        // implicit texture with no options can be ignored
        return status;
    }

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
			m_progress = 0;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
			if (m_name_length < 255) {
				
				if ((status = PutAsciiData (tk,"Name_Length", m_name_length)) != TK_Normal)
					return status;
			}
			else {

				if (m_progress == 0) {
					
					if ((status = PutAsciiData (tk,"Name_Length" ,255)) != TK_Normal)
						return status;
					m_progress++;
				}
				
				if ((status = PutAsciiData (tk,"Name_Length", m_name_length)) != TK_Normal)
					return status;
			}
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Name", m_name, m_name_length)) != TK_Normal)
                return status;
			m_progress = 0;
            m_stage++;
        }   nobreak;

        case 3: {
                  PutTab t(&tk);
        			if (m_image_length < 255) {
				
				if ((status = PutAsciiData (tk, "Image_Length",m_image_length)) != TK_Normal)
					return status;
			}
			else {
				if (m_progress == 0) {
					
					if ((status = PutAsciiData (tk,"Image_Length",255)) != TK_Normal)
						return status;
					m_progress++;
				}
				
				if ((status = PutAsciiData (tk,"Image_Length", m_image_length)) != TK_Normal)
					return status;
			}

            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Image", m_image, m_image_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 5: {
            unsigned short      word = (unsigned short)flags;

			PutTab t(&tk);
            if ((status = PutAsciiFlag (tk,"Flags", word)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 6: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Extended) != 0) {
				
                unsigned short      word = (unsigned short)(flags >> 16);
                if ((status = PutAsciiData (tk,"Texture_Extended", word)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;


        case 7: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Param_Source) != 0)
                if ((status = PutAsciiData (tk,"Texture_Param_Source", (int)m_param_source)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 8: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Tiling) != 0)
				
                if ((status = PutAsciiData (tk,"Texture_Tiling", m_tiling)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 9: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Interpolation) != 0)
				
                if ((status = PutAsciiData (tk,"Texture_Interpolation", (int)m_interpolation)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 10: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Decimation) != 0)
				
                if ((status = PutAsciiData (tk,"Texture_Decimation", m_decimation)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 11: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Red_Mapping) != 0)
				
                if ((status = PutAsciiData (tk,"Texture_Red_Mapping", m_red_mapping)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 12: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Green_Mapping) != 0)
				
                if ((status = PutAsciiData (tk,"Texture_Green_Mapping", m_green_mapping)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 13: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Blue_Mapping) != 0)
				
                if ((status = PutAsciiData (tk,"Texture_Blue_Mapping", m_blue_mapping)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 14: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Alpha_Mapping) != 0)
				
                if ((status = PutAsciiData (tk,"Texture_Alpha_Mapping", m_alpha_mapping)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 15: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Param_Function) != 0)
				
                if ((status = PutAsciiData (tk,"Texture_Parameter_Function", m_param_function)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 16: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Layout) != 0)
				
                if ((status = PutAsciiData (tk,"Texture_Layout", m_layout)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 17: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Value_Scale) != 0)
				
                if ((status = PutAsciiData (tk,"Texture_Value_Scale", m_value_scale, 2)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 18: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Transform) != 0) {
                unsigned char       byte = (unsigned char) strlen (m_transform);
                if ((status = PutAsciiData (tk,"Length", (int) byte)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 19: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Transform) != 0)
				
                if ((status = PutAsciiData (tk,"Texture_Transform", m_transform, (int) strlen (m_transform))) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 20: {
			PutTab t(&tk);
            if ((flags & (TKO_Texture_Modulate|TKO_Texture_Decal)) != 0)
				
				if ((status = PutAsciiData (tk,"Apply_Mode", m_apply_mode)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 21: {
			PutTab t(&tk);
            if ((flags & TKO_Texture_Param_Offset) != 0)
					
				if ((status = PutAsciiData (tk,"Texture_Param_Offset", m_param_offset)) != TK_Normal)
                    return status;
                m_stage++;
        }   nobreak;

		case 22: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_Texture::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
		case 0: {
			m_substage = -1;	
			m_stage++;
		}	nobreak;

        case 1: {
			if (m_substage == -1) {
				if ((status = GetAsciiData (tk,"Name_Length", m_int)) != TK_Normal)
					return status;
				m_substage = m_int;
			}
			if (m_substage == 255) {
				if ((status = GetAsciiData (tk, "Name_Length", m_name_length)) != TK_Normal)
					return status;
			}
			else
				m_name_length = m_substage;
			SetName (m_name_length);       // allocate space
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"Name", m_name, m_name_length)) != TK_Normal)
                return status;
			m_substage = -1;	
            m_stage++;
        }   nobreak;

        case 3: {
 			if (m_substage == -1) {
				if ((status = GetAsciiData (tk,"Image_Length", m_int)) != TK_Normal)
					return status;
				m_substage = m_int;
			}
			if (m_substage == 255) {
				if ((status = GetAsciiData (tk,"Image_Length", m_image_length)) != TK_Normal)
					return status;
			}
			else
				m_image_length = m_substage;
            SetImage (m_image_length);      // allocate space
            m_stage++;
        }   nobreak;

        case 4: {
			if ((status = GetAsciiData (tk,"Image", m_image, m_image_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 5: {
           
			if ((status = GetAsciiHex (tk,"Flags", m_flags)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 6: {
            if ((m_flags & TKO_Texture_Extended) != 0) {
				if ((status = GetAsciiData (tk,"Texture_Extended", m_int)) != TK_Normal)
                    return status;
                m_flags |= m_int << 16;
            }
            m_stage++;
        }   nobreak;


        case 7: {
            if ((m_flags & TKO_Texture_Param_Source) != 0)
			{ 
				if ((status = GetAsciiData (tk,"Texture_Param_Source", m_int)) != TK_Normal)
                    return status;
				m_param_source = (char)m_int;
			}
            m_stage++;
        }   nobreak;

        case 8: {
            if ((m_flags & TKO_Texture_Tiling) != 0)
				if ((status = GetAsciiData (tk,"Texture_Tiling", m_tiling)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 9: {
            if ((m_flags & TKO_Texture_Interpolation) != 0)
			{
				if ((status = GetAsciiData (tk,"Texture_Interpolation", m_int)) != TK_Normal)
                    return status;
				m_interpolation = (char)m_int;
			}
            m_stage++;
        }   nobreak;

        case 10: {
            if ((m_flags & TKO_Texture_Decimation) != 0)
				if ((status = GetAsciiData (tk,"Texture_Decimation", m_decimation)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 11: {
            if ((m_flags & TKO_Texture_Red_Mapping) != 0)
				if ((status = GetAsciiData (tk,"Texture_Red_Mapping", m_red_mapping)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 12: {
            if ((m_flags & TKO_Texture_Green_Mapping) != 0)
				if ((status = GetAsciiData (tk,"Texture_Green_Mapping", m_green_mapping)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 13: {
            if ((m_flags & TKO_Texture_Blue_Mapping) != 0)
				if ((status = GetAsciiData (tk, "Texture_Blue_Mapping", m_blue_mapping)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 14: {
            if ((m_flags & TKO_Texture_Alpha_Mapping) != 0)
				if ((status = GetAsciiData (tk, "Texture_Alpha_Mapping",m_alpha_mapping)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 15: {
            if ((m_flags & TKO_Texture_Param_Function) != 0)
				if ((status = GetAsciiData (tk, "Texture_Parameter_Function",m_param_function)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 16: {
            if ((m_flags & TKO_Texture_Layout) != 0)
				if ((status = GetAsciiData (tk,"Texture_Layout", m_layout)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 17: {
            if ((m_flags & TKO_Texture_Value_Scale) != 0)
				if ((status = GetAsciiData (tk,"Texture_Value_Scale", m_value_scale, 2)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 18: {
            if ((m_flags & TKO_Texture_Transform) != 0) {
				if ((status = GetAsciiData (tk,"Length", m_int)) != TK_Normal)
                    return status;
                SetTransform (m_substage = m_int);
            }
            m_stage++;
        }   nobreak;

        case 19: {
            if ((m_flags & TKO_Texture_Transform) != 0) {
				if ((status = GetAsciiData (tk,"Texture_Transform", m_transform, m_substage)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 20: {
            if ((m_flags & (TKO_Texture_Modulate|TKO_Texture_Decal)) != 0)
				if ((status = GetAsciiData (tk,"Apply_Mode", m_apply_mode)) != TK_Normal)
                    return status;
            m_stage++;
        }   nobreak;

        case 21: {
            if ((m_flags & TKO_Texture_Param_Offset) != 0)
				if ((status = GetAsciiData (tk,"Texture_Param_Offset", m_param_offset)) != TK_Normal)
                    return status;
               m_stage++;
        }   nobreak;
		
		case 22: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_Glyph_Definition::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    if (tk.GetTargetVersion() < 1160)
        return TK_Normal;

    _W3DTK_REQUIRE_VERSION( 1160 );

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            // need code here to handle length > 255 ?
            unsigned char       byte = (unsigned char)m_name_length;
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Name_Length", (int) byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Name", m_name, m_name_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
            // need code here to handle length > 65535 ?
            unsigned short      word = (unsigned short)m_size;
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Size", word)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Data", m_data, m_size)) != TK_Normal)
                return status;
              m_stage++;
        }   nobreak;

		case 5: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;


        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Glyph_Definition::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Name_Length", m_int)) != TK_Normal)
                return status;
            SetName (m_int);       // allocate space
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Name", m_name, m_name_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"Size", m_int)) != TK_Normal)
                return status;
            SetDefinition (m_int);      // allocate space
            m_stage++;
        }   nobreak;

        case 3: {
			if ((status = GetAsciiData (tk,"Data", m_data, m_size)) != TK_Normal)
                return status;
               m_stage++;
        }   nobreak;
		
		case 4: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Line_Style::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    if (tk.GetTargetVersion() < 1160)
        return TK_Normal;

    _W3DTK_REQUIRE_VERSION( 1160 );

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            // need code here to handle length > 255 ?
				PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Name_Length", m_name_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
			if ((status = PutAsciiData (tk, "Name",m_name, m_name_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
            // need code here to handle length > 65535 ?
//            unsigned short      word = (unsigned short)m_definition_length;
				PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Definition_Length", m_definition_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 4: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Definition", m_definition, m_definition_length)) != TK_Normal)
                return status;
               m_stage++;
        }   nobreak;

		case 5: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Line_Style::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
           	if ((status = GetAsciiData (tk,"Name_Length", m_name_length)) != TK_Normal)
                return status;
            SetName (m_name_length);       // allocate space
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Name", m_name, m_name_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"Definition_Length", m_int)) != TK_Normal)
                return status;
            SetDefinition (m_int);      // allocate space
            m_stage++;
        }   nobreak;

        case 3: {
			if ((status = GetAsciiData (tk,"Definition", m_definition, m_definition_length)) != TK_Normal)
                return status;
                m_stage++;
        }   nobreak;
		
		case 4: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Clip_Rectangle::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiHex (tk,"Options", m_options)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Rect", m_rect, 4)) != TK_Normal)
                return status;
                 m_stage++;
        }   nobreak;

		case 3: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;


        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Clip_Rectangle::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Options", m_options)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Rect", m_rect, 4)) != TK_Normal)
                return status;
             m_stage++;
        }   nobreak;
		
		case 2: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Clip_Region::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiHex (tk,"Options",  m_options)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Count", m_count)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Points", m_points, 3*m_count)) != TK_Normal)
                return status;
                m_stage++;
        }   nobreak;

		case 4: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;


        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_Clip_Region::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiHex (tk,"Options", m_options)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Count", m_int)) != TK_Normal)
                return status;
            if (!validate_count (m_int))
                return tk.Error("bad Clip Region count");
            SetPoints (m_int);   // allocated space
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"Points", m_points, 3*m_count)) != TK_Normal)
                return status;
              m_stage++;
        }   nobreak;
		
		case 3: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_User_Data::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

     switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk, 0)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Size", m_size)) != TK_Normal)
                return status;
            m_progress = 0;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Data", m_data, m_size)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk, "Stop_User_Data",(unsigned char)TKE_Stop_User_Data)) != TK_Normal)
                return status;
 
                 m_stage++;
        }   nobreak;

		case 4: {
			if ((status = PutAsciiOpcode (tk,0,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;


        default:
            return tk.Error();
    }

	 
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_User_Data::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Size", m_size)) != TK_Normal)
                return status;
            set_data (m_size);      // allocate space
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk, "Data", m_data, m_size)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"Stop_User_Data", m_byte)) != TK_Normal)
                return status;

            if (m_byte != TKE_Stop_User_Data)    // sanity check
                return tk.Error();

              m_stage++;
        }   nobreak;
		
		case 3: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_XML::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

     if (tk.GetTargetVersion() < 705)  // XML opcode added in 7.05
        return status;

    switch (m_stage) {
        case 0: {
            if ((status = PutAsciiOpcode (tk, 0)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk, "Size", m_size)) != TK_Normal)
                return status;
            m_progress = 0;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"Data", m_data, m_size)) != TK_Normal)
                return status;
                m_stage++;
        }   nobreak;

		case 3: {
			if ((status = PutAsciiOpcode (tk,0,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;


        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 

}

TK_Status TK_XML::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

	TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Size", m_size)) != TK_Normal)
                return status;
            SetXML (m_size);      // allocate space
            m_stage++;
        }   nobreak;

        case 1: {
			if ((status = GetAsciiData (tk,"Data", m_data, m_size )) != TK_Normal)
                return status;
                m_stage++;
        }   nobreak;
		
		case 2: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;
        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_External_Reference::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

	switch (m_stage) {
        case 0: {
            if (tk.GetTargetVersion() < 805)    // first added in 8.05, so just skip it
                return TK_Normal;
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            unsigned char       byte;
            if (m_length > 65535)
                byte = 255;                     // flag length stored as int following
            else if (m_length > 253)
                byte = 254;                     // flag length stored as short following
            else
                byte = (unsigned char)m_length;

            if ((status = PutAsciiData (tk,"Length", (int) byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if (m_length > 65535) {
                if ((status = PutAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                    return status;
            }
            else if (m_length > 253) {
				if ((status = PutAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
                return status;

                 m_stage++;
        }   nobreak;

		case 4: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;


        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_External_Reference::ReadAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII
 
	TK_Status       status = TK_Normal;

    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk, "Length" , m_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            if (m_length == 255) {                      // real length is a following int
				if ((status = GetAsciiData (tk,"Length", m_length)) != TK_Normal)
                    return status;
            }
            else if (m_length == 254) {                 // real length is a following short
				if ((status = GetAsciiData (tk,"Length", m_length)) != TK_Normal)
                    return status;
            }
        
            SetString (m_length);      // allocate space & (re)set length
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
                return status;
               m_stage++;
        }   nobreak;
		
		case 3: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status TK_URL::WriteAscii (BStreamFileToolkit & tk) alter {
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
        case 0: {
            if (tk.GetTargetVersion() < 805)    // first added in 8.05, so just skip it
                return TK_Normal;
            if ((status = PutAsciiOpcode (tk,1)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
			PutTab t(&tk);
            unsigned char       byte;

            if (m_length > 65535)
                byte = 255;                     // flag length stored as int following
            else if (m_length > 253)
                byte = 254;                     // flag length stored as short following
            else
                byte = (unsigned char)m_length;

			if ((status = PutAsciiData (tk,"Length", (int) byte)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 2: {
			PutTab t(&tk);
            if (m_length > 65535) {
                if ((status = PutAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                    return status;
            }
            else if (m_length > 253) {
                if ((status = PutAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3: {
			PutTab t(&tk);
            if ((status = PutAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
                return status;

			m_stage++;
        }   nobreak;

		case 4: {
			if ((status = PutAsciiOpcode (tk,1,true)) != TK_Normal)
				return status;
			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

	
    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status TK_URL::ReadAscii (BStreamFileToolkit & tk) alter {

#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
    switch (m_stage) {
        case 0: {
			if ((status = GetAsciiData (tk,"Length", m_length)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {
            if (m_length == 255) {                      // real length is a following int
				if ((status = GetAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                    return status;
            }
            else if (m_length == 254) {                 // real length is a following short
				if ((status = GetAsciiData (tk,"Real_Length", m_length)) != TK_Normal)
                    return status;
            }
        
            SetString (m_length);      // allocate space & (re)set length
            m_stage++;
        }   nobreak;

        case 2: {
			if ((status = GetAsciiData (tk,"String", m_string, m_length)) != TK_Normal)
                return status;
                m_stage++;
        }   nobreak;
		
		case 3: {
			if ((status = ReadEndOpcode(tk))!= TK_Normal)
					return status;

			m_stage = -1;
		}   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}

TK_Status W3D_Image::WriteAscii (BStreamFileToolkit & tk) alter{
#ifndef BSTREAM_DISABLE_ASCII

    TK_Status       status = TK_Normal;
	
	PutTab t0(&tk);

    switch (m_stage) {
          case 0: {
            if ((status = PutAsciiOpcode (tk)) != TK_Normal)
                return status;
            m_stage++;
        }   nobreak;

        case 1: {

            if (m_name_length > 0) {
                unsigned char       byte = (unsigned char)m_name_length;
				byte;
                // need handling for string > 255 ?
                if ((status = PutAsciiData (tk,"Name_Length", m_name_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 2: {
            if (m_name_length > 0) {
                if ((status = PutAsciiData (tk,"Name", m_name, m_name_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 3:
        {
            if ((status = PutAsciiData(tk,"Width", m_size[0])) != TK_Normal)
                return status;
            if ((status = PutAsciiData(tk,"Height", m_size[1])) != TK_Normal)
                return status;
            if ((status = PutAsciiData(tk,"Bits_Per_Pixel", (int)m_bpp)) != TK_Normal)
                return status;
            m_stage++;
            nobreak;
        }

        case 4: {
            if (Tagging (tk))
                status = Tag (tk);

            m_stage = -1;
        }   break;

        default:
            return tk.Error();
    }

    return status;
#else
	return tk.Error(stream_disable_ascii);
#endif 
}


TK_Status W3D_Image::ReadAscii (BStreamFileToolkit & tk) alter{
	#ifndef BSTREAM_DISABLE_ASCII
	TK_Status       status = TK_Normal;

	    switch (m_stage) {

        case 0: {
                unsigned char       byte;
                if ((status = GetAsciiData (tk,"name length", byte)) != TK_Normal)
                    return status;
                set_name ((int)byte);

            m_stage++;
        }   nobreak;

        case 1: {

            if (m_name_length > 0) {
                if ((status = GetAsciiData (tk, m_name, m_name_length)) != TK_Normal)
                    return status;
            }
            m_stage++;
        }   nobreak;

        case 2:
        {
            if ((status = GetData(tk, m_size[0])) != TK_Normal)
            {
                return status;
            }
            m_stage++;
        }   nobreak;


        case 3:
        {
            if ((status = GetData(tk, m_size[1])) != TK_Normal)
            {
                return status;
            }
            m_stage++;
        }   nobreak;

        case 4:
        {
            if ((status = GetData(tk, m_bpp)) != TK_Normal)
            {
                return status;
            }
            m_stage = -1;
            break;
        }


        default:
            return tk.Error();
    }

    return status;
	#else
	return tk.Error(stream_disable_ascii);
	#endif 

}

////////////////////////////////////////








