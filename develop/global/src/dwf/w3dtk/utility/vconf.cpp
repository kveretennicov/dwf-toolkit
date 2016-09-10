
#include "vconf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#pragma warning(disable: 4127 4996)
#endif

VCONF_API vconf_t* new_vconf(
	void *(VHASH_CDECL *vconf_malloc) (size_t),
	void (VHASH_CDECL *vconf_free) (void *))
{
	vconf_t *v = 0;

	v = (vconf_t *) vconf_malloc(sizeof(vconf_t));

	v->malloc = vconf_malloc;
	v->free = vconf_free;

	v->delimiter = '=';
	v->style = VCONF_FILE_STYLE_FLAT;
	v->chunk = 0;

	v->filename = 0;
 	v->options_hash = new_vhash(1, v->malloc, v->free);
	v->options_list = 0;

 	return v;
}

static void delete_options_helper(
	void * option, 
	const char * option_name, 
	void * user_data)
{
	vconf_t *v = (vconf_t*)user_data;

#if 0
	printf("%s %s %p\n", (char*)option_name, (char*)option, option);
#endif

	vhash_remove_string_key(v->options_hash, option_name);
	v->free(option);
}

VCONF_API void delete_vconf(
	vconf_t* v)
{
    if (v->filename)
		v->free((void*)v->filename);

    if (v->options_list) {
		/*chunked*/
		while ((v->options_hash = (vhash_t*)vlist_remove_first(v->options_list)) != 0) {
			vhash_string_key_map_function(v->options_hash, delete_options_helper, v);

			delete_vhash(v->options_hash) ;
		}

		delete_vlist(v->options_list);

    }
	else {
		/*flat*/
		vhash_string_key_map_function(v->options_hash, delete_options_helper, v);

		delete_vhash(v->options_hash);
    }

    v->free(v);
}


VCONF_API void vconf_clear_options(
	vconf_t* v)
{
	/*clear old options if any*/
	vhash_string_key_map_function(v->options_hash, delete_options_helper, v);
}

VCONF_API void vconf_set_delimiter(
	vconf_t* vconf,
	char delimiter) 
{
	vconf->delimiter = delimiter;
}

VCONF_API int vconf_set_file_style(
	vconf_t* v,
	int style) 
{
	if (v->style != VCONF_FILE_STYLE_CHUNKED &&
		v->style != VCONF_FILE_STYLE_FLAT) return 0;

	v->style = style;
	return 1;
}

VCONF_API unsigned int vconf_chunk_count(
	vconf_t* v)
{
	if (!v->options_list) return 0;
	return vlist_count(v->options_list);
}

VCONF_API int vconf_set_chunk(
	vconf_t* v, 
	unsigned int chunk)
{
	if (v->style != VCONF_FILE_STYLE_CHUNKED) return 0;
	
	/*XXX this should extend the list instead*/
	if (chunk > vlist_count(v->options_list))
		chunk = vlist_count(v->options_list);

	v->chunk = chunk;
	v->options_hash = (vhash_t*)vlist_nth_item(v->options_list, v->chunk);
	return 1;
}

static void write_options_helper(
	void * option, 
	const char * option_name, 
	void * user_data)
{
	FILE *f = (FILE*)user_data;

	/*printf("%s \"%s\"\n", (char*)option_name, (char*)option);*/
	fprintf(f, "%s \"%s\"\n", (char*)option_name, (char*)option);
}
		
VCONF_API int vconf_write_file(
	vconf_t* v, 
	const char * filename)
{
	FILE * f = fopen(filename, "w");

	if (f == NULL) return 0;

	if (v->options_list) {
		/*chunked*/
		vhash_t * hash;
		unsigned int i;

		for (i=0; i<vlist_count(v->options_list); i++) {
			hash = (vhash_t*)vlist_nth_item(v->options_list, i);

			vhash_string_key_map_function(
				hash, 
				write_options_helper, (void*)f);
		}

    }
	else {
		/*flat*/
		vhash_string_key_map_function(
			v->options_hash, 
			write_options_helper, (void*)f);
	}

	fclose(f);

	return 1;
}

VCONF_API int vconf_read_file(
	vconf_t* v, 
	const char * filename)
{
	int filename_length = (int) strlen(filename);

	if (v->filename)
		 v->free((void*)v->filename);

	v->filename = (const char*)v->malloc(filename_length +1);

	strcpy((char*)v->filename, filename);

	if (v->filename[0] != '\0'){
		FILE * file = 0;

		int read_again = 1;

		char * line_buffer = 0;
		unsigned long line_buffer_size = 0;

		int a_char = 0;
		int use_a_char = 0;

		unsigned long n;
		unsigned long i;

		char * option_type;
		char * option;

		file = fopen(v->filename, "rb");

		if (!file) return 0;

		fseek(file, 0, SEEK_SET);

		/*TODO make bigger later*/
		line_buffer_size = 1;
		line_buffer = (char*) v->malloc(line_buffer_size);

READ_A_LINE:
		n = 0;
		while (1) {
			/*resize buffer as needed*/
			if (n == line_buffer_size) {
				char * new_line_buffer;

				new_line_buffer = (char*) v->malloc(line_buffer_size*2);
				memcpy(new_line_buffer, line_buffer, line_buffer_size);

				v->free(line_buffer);

				line_buffer = new_line_buffer;
				line_buffer_size = line_buffer_size*2;
			}
			
			if (use_a_char)
				use_a_char = 0;
			else
				a_char = fgetc(file);
			
			if (a_char == EOF) {
				read_again=0;
				line_buffer[n] = '\0';
				break;
			}

			if (a_char == 0x0D || a_char == 0x0A){
				int last_char = a_char;
				a_char = fgetc(file);
				if (a_char == 0x0D || a_char == 0x0A) {
				    if (a_char == last_char) use_a_char = 1;
				}
				else
				    use_a_char = 1;
				line_buffer[n] = '\0';
				break;
			}

			if (a_char == '\t' || a_char == '\0') a_char = ' ';

			line_buffer[n] = (char) a_char;

			n++;
		}/*while*/

		if (n == 0) {
			if (v->style == VCONF_FILE_STYLE_CHUNKED &&
				vhash_count(v->options_hash) > 0) {

				if (!v->options_list) 
					v->options_list = new_vlist(v->malloc, v->free);

				vlist_add_last(v->options_list, (void*)v->options_hash);

				v->options_hash = new_vhash(1, v->malloc, v->free);
			}

			goto PARSE_DONE;
		}
		
		i = 0;
#if 0
		printf("%s\n",&line_buffer[i]);
#endif

		while (i<n && line_buffer[i] == ' ') i++;

		if (line_buffer[i] == '\0') goto PARSE_DONE;

		if (line_buffer[i] == ';' || line_buffer[i] == '#') goto PARSE_DONE;

#if 0
		printf("%s\n",&line_buffer[i]);
#endif

		option_type = &line_buffer[i];

		while (i<n && (line_buffer[i] != ' ' && line_buffer[i] != v->delimiter)) i++;

		line_buffer[i] = '\0';
		/*default empty option*/
		option = &line_buffer[i];
		i++;

		/*once for space and once for = once more for space*/
		while (i<n && (line_buffer[i] == ' ' || line_buffer[i] == v->delimiter)) i++;
		while (i<n && (line_buffer[i] == ' ' || line_buffer[i] == v->delimiter)) i++;
		while (i<n && (line_buffer[i] == ' ' || line_buffer[i] == v->delimiter)) i++;

		if (line_buffer[i] == ';' || line_buffer[i] == '#') goto USE_PARSE;


		if (line_buffer[i] == '"') {
			/*quoted option*/
			i++;
			/*real option start*/
			option = &line_buffer[i];
			while(i<n && line_buffer[i] != '"') i++;
			line_buffer[i] = '\0';
			goto USE_PARSE;
		}

		if (line_buffer[i] == '\'') {
			/*quoted option*/
			i++;
			/*real option start*/
			option = &line_buffer[i];
			while(i<n && line_buffer[i] != '\'') i++;
			line_buffer[i] = '\0';
			goto USE_PARSE;
		}

		/*unquoted option*/
		/*real option start*/
		option = &line_buffer[i];
		while (i<n && (line_buffer[i] != '#' && line_buffer[i] != ';')) i++;
		/* backup over any blanks */
		while (line_buffer[i] == ' ') i--;
		line_buffer[i] = '\0';

USE_PARSE:
		{
			char * new_option = 0;
			char * old_option = 0;

			new_option = (char*) v->malloc(strlen(option)+1);

			strcpy(new_option, option);

			old_option = (char*) vhash_remove_string_key(v->options_hash, option_type);

			vhash_insert_string_key(v->options_hash, option_type, new_option);

#if 0
			printf("%s %s %s %p %p\n", option_type, new_option, (char*)old_option ,new_option, old_option);
#endif

			if (old_option) 
				v->free(old_option);
		}


PARSE_DONE:
		if (read_again)
			goto READ_A_LINE;

		fclose(file);

		if (line_buffer) free(line_buffer);

	}

	if (v->style == VCONF_FILE_STYLE_CHUNKED) {
		if (!v->options_list) 
			v->options_list = new_vlist(v->malloc, v->free);

		if (vhash_count(v->options_hash) > 0 || vlist_count(v->options_list) == 0) 
			vlist_add_last(v->options_list, (void*)v->options_hash);
		else 
			delete_vhash(v->options_hash);
			
		v->chunk = 0;
		v->options_hash = (vhash_t*)vlist_nth_item(v->options_list, v->chunk);
	}

	return 1;
}

VCONF_API const char * vconf_get_filename(
	vconf_t* v)
{
	return v->filename;
}

VCONF_API vhash_t * vconf_get_option_hash(
	vconf_t* v)
{
	return v->options_hash;
}


VCONF_API const char * vconf_get_option(
	vconf_t* v, 
	const char * option)
{
	return (const char *) vhash_lookup_string_key(v->options_hash, option);
}

VCONF_API void vconf_set_option(
	vconf_t* v, 
	const char * option_type,
	const char * option)
{
	char * old_option = 0;
	char * new_option = 0;

	new_option = (char*) v->malloc(strlen(option)+1);

	strcpy(new_option, option);

	old_option = (char*) vhash_remove_string_key(v->options_hash, option_type);

	vhash_insert_string_key(v->options_hash, option_type, new_option);

	/*
	printf("%s %s %s %p %p\n", option_type, new_option, (char*)old_option ,new_option, old_option);
	*/

	if(old_option) 
		v->free(old_option);
}




