
#include "vlogfile.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

#ifdef _WIN32_WCE
#include <winbase.h>
#endif

/*THESE ARE GLOBALS FOR DEBUG LOGGING*/
FILE * debug_log_file_handle;
char * debug_log_current_time;


VLOG_API vlogfile_t* new_vlogfile(
	const char * log_file_name,
	void *(*vlogfile_malloc) (size_t),
	void (*vlogfile_free) (void *))
{
	vlogfile_t *v = 0;

	v = (vlogfile_t *) vlogfile_malloc(sizeof(vlogfile_t));

	v->name = (const char *) vlogfile_malloc(sizeof(char)*strlen(log_file_name)+1);
	strcpy((char*)v->name, log_file_name);

	v->n_lines = 0;
	v->n_chars = 0;
	v->malloc = vlogfile_malloc;
	v->free = vlogfile_free;

	{
		char tmp_name[4096];

		sprintf(tmp_name,"%s.log",v->name);
		v->lines = fopen(tmp_name, "w+b");
		if (v->lines) {
			sprintf(tmp_name,"%s.dir",v->name);
			v->dir = fopen(tmp_name, "w+b");
			if(v->dir) goto DONE;
		}

		if (v->lines) fclose(v->lines);
		vlogfile_free((char*)v->name);
		vlogfile_free(v);
		v = 0;
	}

DONE:

	return v;
}


VLOG_API void delete_vlogfile(
	vlogfile_t* v)
{
	char tmp_name[4096];
#ifdef _WIN32_WCE
	wchar_t tmp_name_width[4096];

	fclose(v->dir);
	fclose(v->lines);

	sprintf(tmp_name,"%s.log",v->name);
	mbstowcs(tmp_name_width, tmp_name,4096);

	DeleteFile(tmp_name_width);
		
	sprintf(tmp_name,"%s.dir",v->name);
	mbstowcs(tmp_name_width, tmp_name,4096);
	DeleteFile(tmp_name_width);

#else
	fclose(v->dir);
	fclose(v->lines);
	sprintf(tmp_name,"%s.log",v->name);
	remove(tmp_name);
		
	sprintf(tmp_name,"%s.dir",v->name);
	remove(tmp_name);
#endif
	v->free((char*)v->name);
	v->free(v);

}

VLOG_API void vlogfile_add_line(
	vlogfile_t* v,
	const char * line,
	size_t line_length)
{
	fwrite(line, line_length, 1, v->lines);

	fwrite(&v->n_chars, 1, sizeof(unsigned long), v->dir);
	fwrite(&line_length, 1, sizeof(unsigned long), v->dir);

	/*TODO no sync here*/
	fflush(v->lines);
	fflush(v->dir);

	v->n_lines++;
	v->n_chars+=(unsigned long)line_length;
}

VLOG_API const char * vlogfile_get_line(
	vlogfile_t* v,
	unsigned long line_number,
	unsigned long * line_length)
{
	unsigned long offset;
	unsigned long length;
	unsigned long dir_offset;
	char * line;

	if (line_number > v->n_lines || line_number == 0) {
		*line_length = 0;
		return 0;
	}

	dir_offset = (line_number-1) * 2 * sizeof(unsigned long);
	fseek(v->dir, dir_offset, SEEK_SET);		
	fread(&offset, 1, sizeof(unsigned long), v->dir);
	fread(&length, 1, sizeof(unsigned long), v->dir);
	fseek(v->dir, 0, SEEK_END);		

	*line_length = length;
	line = (char *) v->malloc(length);
	fseek(v->lines, offset, SEEK_SET);		
	fread(line, length, 1, v->lines);
	fseek(v->lines, 0, SEEK_END);		

	return (const char *) line;
}

VLOG_API void vlogfile_free_line(
	vlogfile_t* v,
	const char * line)
{
	if (line)
		v->free((void*) line);
}

VLOG_API void vlogfile_clear_log(
	vlogfile_t* v)
{
	char tmp_name[4096];

	fclose(v->lines);
	sprintf(tmp_name,"%s.log",v->name);
	v->lines = fopen(tmp_name, "w+b");
		
	fclose(v->dir);
	sprintf(tmp_name,"%s.dir",v->name);
	v->dir = fopen(tmp_name, "w+b");

	v->n_lines = 0;
	v->n_chars = 0;
}


VLOG_API FILE * vlogfile_open_snapshot(
	vlogfile_t* v,
	const char * filename)
{
	FILE * snap_file = 0;

	(void)v;	/* unreferenced */
	snap_file = fopen(filename, "w+b");

	return snap_file;
}

VLOG_API void vlogfile_dump_snapshot(
	vlogfile_t* v,
	FILE * snap_file)
{
	unsigned long line_number = 1;
	unsigned long offset;
	unsigned long length;
	unsigned long dir_offset;
	char * line;
	unsigned long n;

	/*TODO this faster*/
	while (line_number <= v->n_lines) {
		dir_offset = (line_number-1) * 2 * sizeof(unsigned long);
		fseek(v->dir, dir_offset, SEEK_SET);		
		fread(&offset, 1, sizeof(unsigned long), v->dir);
		fread(&length, 1, sizeof(unsigned long), v->dir);

		line = (char *) v->malloc(length);
		fseek(v->lines, offset, SEEK_SET);		
		fread(line, length, 1, v->lines);

		/*this is really not the right way to do this*/
		/*TODO pass in a list of messages to skip*/
		if (strncmp(line,"H_DEV_NULL_DATA ", length) != 0) {
			fprintf(snap_file, "%lu ", length);

			for (n=0; n<length; n++) {
				fprintf(snap_file, "%c", line[n]);
			}

			fprintf(snap_file, "\x0D\x0A");
		}

		free(line);

		line_number++;
	}

	/*put the file pointer back at the end*/
	fseek(v->dir, 0, SEEK_END);		
	fseek(v->lines, 0, SEEK_END);		
}


VLOG_API void vlogfile_close_snapshot(
	vlogfile_t* v,
	FILE * snap_file)
{
	(void)v;	/* unreferenced */

	fclose(snap_file);
}






