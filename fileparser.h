#pragma once

#ifndef _FILEPARSER_H_
#define _FILEPARSER_H_

#ifndef _STDIO_H_
#include <stdio.h>
#endif

#ifndef _STDLIB_H_
#include <stdlib.h>
#endif

#ifndef _STRING_H_
#include <string.h>
#endif

#ifndef _CTYPE_H_
#include <ctype.h>
#endif

typedef unsigned long long ull;
typedef long double bigfloat;

typedef struct __parser_settings
{
    char splitter;
    int ignore_first_line;
    int ignore_errors;
    int first_line_as_header;
} PARSER_SETTINGS;

typedef enum __container_data_type
{
    STRING_TYPE,
    INTEGER_TYPE,
    FLOAT_TYPE,
    NULL_TYPE
} DATA_TYPE;

typedef union __container_data_var
{
    char* string;
    ull integer;
    bigfloat floating;
    void* null;
} DATA_VAR;

typedef struct __container_data
{
    DATA_TYPE type;
    DATA_VAR value;
} CONTAINER_DATA;

typedef struct _line_info
{
    size_t token_count;
    int is_header;
} LINE_INFO;

typedef struct __parser_container
{
    CONTAINER_DATA** lines;
    LINE_INFO* info;
    size_t line_count;
    size_t column_count;
    int header_included;
} PARSER_CONTAINER;


/* ================= PARSER SORT SETTING ================*/
typedef enum __parser_sort_settings_tag
{
    COLUMN_NAME,
    COLUMN_INDEX
} SORT_SETTINGS_TAG;

typedef union __parser_sort_settings_value
{
    const char* column_name;
    size_t column_index;
} SORT_SETTINGS_VALUE;

typedef enum __sort_direction
{
    ASCENDING,
    DESCENDING
} SORT_DIRECTION;

typedef struct __parser_sort_settings
{
    SORT_SETTINGS_TAG tag;
    SORT_SETTINGS_VALUE value;
    SORT_DIRECTION direction;
    int case_sensitive;
} PARSER_SORT_SETTINGS;

typedef struct __parser_object
{
    PARSER_CONTAINER* container;
    PARSER_SORT_SETTINGS sort_settings;
    PARSER_SETTINGS settings;
} PARSER;

PARSER create_parser();
int parse_file(PARSER* parser, const char* filename);
int sort_data(PARSER* parser, PARSER_SORT_SETTINGS settings);
int save_data(PARSER* parser, const char* filename);
int print_all_data(PARSER* parser);
int print_data(PARSER* parser, size_t how_much_to_print);
void free_parser(PARSER* parser);

PARSER_SETTINGS create_parser_settings();
void change_default_settings(PARSER_SETTINGS settings);

PARSER_SORT_SETTINGS create_parser_sort_settings();
void change_default_sort_settings(PARSER_SORT_SETTINGS settings);

#endif