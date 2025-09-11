/**
 * Made by Arseniy Kuskov
 * This file has no copyright assigned and is placed in the Public Domain.
 * No warranty is given.
 */

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

#include <ctype.h>

#define CYRYLLIC_ENCODING 1251
#define UTF_8_ENCODING 65001

/* =============== DEBUGGER SETUP ================ */
#define LOGLEVEL_CRITICAL 0
#define LOGLEVEL_WARNING  1
#define LOGLEVEL_INFO     2
#define LOGLEVEL_DEBUG    3
#define LOGLEVEL_NONE     4

#ifndef LOG_LEVEL
#define LOG_LEVEL LOGLEVEL_INFO // CHANGE THIS TO LOGLEVEL_NONE IF YOU WANT FOR THIS LIB TO SHUT UP
#endif

static const char* loglevels[] =
{
    [LOGLEVEL_CRITICAL] = "CRIT",
    [LOGLEVEL_WARNING] = "WARN",
    [LOGLEVEL_INFO] = "INFO",
    [LOGLEVEL_DEBUG] = "DEBUG",
    [LOGLEVEL_NONE] = "NONE"
};

#if LOG_LEVEL > LOGLEVEL_DEBUG
#define __parser_log(level, format, ...) ((void)0)
#else
#define __parser_log(level, format, ...) \
    do { \
            if (level <= LOG_LEVEL)\
                fprintf(stderr, "[%s]: %s:%s:%zu: " format "\n", loglevels[level], __func__, __FILE__, __LINE__, ##__VA_ARGS__); \
        } while (0)
#endif

#define PARSER_LOG_CRITICAL(format, ...) __parser_log(LOGLEVEL_CRITICAL, format, ##__VA_ARGS__)
#define PARSER_LOG_WARNING(format, ...) __parser_log(LOGLEVEL_WARNING, format, ##__VA_ARGS__)
#define PARSER_LOG_INFO(format, ...) __parser_log(LOGLEVEL_INFO, format, ##__VA_ARGS__)
#define PARSER_LOG_DEBUG(format, ...) __parser_log(LOGLEVEL_DEBUG, format, ##__VA_ARGS__)

/* =============== PUBLIC TYPES ================ */
typedef unsigned long long ull;
typedef long double bigfloat;

typedef struct __parser_settings
{
    char splitter;
    int ignore_first_line;
    int ignore_errors;
    int first_line_as_header;
    int save_memory; // makes parsing slower but saving a lot of memory
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
    PARSER_CONTAINER container;
    PARSER_SORT_SETTINGS sort_settings;
    PARSER_SETTINGS settings;
} PARSER;

typedef PARSER* P_PARSER;

P_PARSER create_parser();
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