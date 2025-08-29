/**
 * Made by Arseniy Kuskov
 * This file has no copyright assigned and is placed in the Public Domain.
 * No warranty is given.
 */

#include "fileparser.h"

/* =============== MACROS ================ */
#define PARSER_COLUMN_CUSTOM_NAME "__parser_column_%zu__"
#define PRINTING_BOND 65535
#define BUFFER_CAPACITY 128
#define HEADER_MAX_WIDTH 32
#define INITIAL_TOKENS_CAPACITY 10

/* =============== TYPES ================ */
typedef FILE* P_PFILE;
typedef int (*QuickSortComp)(size_t, size_t, size_t, PARSER*);

/* =============== PRIVATE FUNCTIONS ================ */
void _init_parser();
PARSER_SETTINGS _create_default_parser_settings();
PARSER_SORT_SETTINGS _create_default_parser_sort_settings();

void _parse_file(PARSER* parser, P_PFILE file_to_parse);
CONTAINER_DATA* _parse_line(const char* line, char splitter, size_t* token_count);
CONTAINER_DATA _parse_token(char* token);

int _check_for_quotes(char* str, size_t len);
char* _trim_whitespace(char* str);
char* _trim_newlines(char* str);
char* _remove_quotes(char* str);
void* _fix_headers(CONTAINER_DATA** lines, size_t token_count);

char* _container_value_to_str(CONTAINER_DATA data);
int _compare_cells(
    const size_t a_idx,
    const size_t b_idx,
    const size_t sort_column,
    PARSER* parser);
void _quick_sort(PARSER* parser, size_t sort_column, QuickSortComp comp_func, size_t left, size_t right, size_t* indices);
size_t _partition(PARSER* parser, size_t sort_column, QuickSortComp comp_func, size_t left, size_t right, size_t* indices);
void _swap(size_t* a, size_t* b);

/* =============== STATIC VARS ================ */
PARSER_SETTINGS DEFAULT_PARSER_SETTINGS;
PARSER_SORT_SETTINGS DEFAULT_PARSER_SORT_SETTINGS;

static int system_initialized = 0;
static int parser_settings_initialized = 0;
static int parser_sort_settings_initialized = 0;

/* =============== PUBLIC ================ */
PARSER create_parser()
{
    if (system_initialized ^ 1) _init_parser();

    PARSER parser;
    parser.container = NULL;
    parser.settings = DEFAULT_PARSER_SETTINGS;
    return parser;
}

int parse_file(PARSER* parser, const char* filename)
{
    if (system_initialized ^ 1) _init_parser();

    P_PFILE target_file = fopen(filename, "r");
    if (target_file == NULL)
        return 1;

    _parse_file(parser, target_file);
    fclose(target_file);

    return 0;
}

int sort_data(PARSER* parser, PARSER_SORT_SETTINGS settings)
{
    if (!parser || parser->container->line_count == 0 || parser->container->column_count == 0) return 1;

    PARSER_CONTAINER* container = parser->container;
    size_t line_count = container->line_count;
    size_t target_column_idx = -1;

    // checking if everything is okay and getting column idx
    if (settings.tag == COLUMN_INDEX)
        {
            if (settings.value.column_index >= container->column_count) return 1;
            else target_column_idx = settings.value.column_index;
        }
    else if (settings.tag == COLUMN_NAME)
        {
            if (container->header_included ^ 1)
                return 1;
            else
                {
                    int found = 0;
                    size_t i;
                    CONTAINER_DATA* first_line = container->lines[0];
                    for (i = 0; i < container->info[0].token_count && found == 0; i++)
                        if (first_line->type == STRING_TYPE && strcasecmp(first_line->value.string, settings.value.column_name) == 0)
                            {
                                found = 1;
                                break;
                            }

                    if (found) target_column_idx = i;
                    else return 1;
                }
        }

    // sorting logic
    parser->sort_settings = settings;

    LINE_INFO* old_info = container->info;
    CONTAINER_DATA** old_lines = container->lines;

    size_t start_index = (container->header_included) ? 1 : 0;
    size_t data_count = line_count - start_index;

    size_t* indices = malloc(data_count * sizeof(size_t));
    LINE_INFO* sorted_info = malloc(line_count * sizeof(LINE_INFO));
    CONTAINER_DATA** sorted_lines = malloc(line_count * sizeof(CONTAINER_DATA*));

    for (size_t i = 0; i < data_count; i++) indices[i] = start_index + i;

    _quick_sort(parser, target_column_idx, _compare_cells, 0, data_count-1, indices);

    for (size_t i = 0; i < data_count; i++)
        {
            sorted_info[start_index + i] = old_info[indices[i]];
            sorted_lines[start_index + i] = old_lines[indices[i]];
        }
    if (container->header_included)
        {
            sorted_info[0] = old_info[0];
            sorted_lines[0] = old_lines[0];
        }

    free(container->lines);
    free(container->info);
    free(indices);

    container->lines = sorted_lines;
    container->info = sorted_info;

    return 0;
}

int save_data(PARSER* parser, const char* filename)
{
    P_PFILE target_file = fopen(filename, "w");

    if (target_file == NULL)
        return 1;

    size_t line_count = parser->container->line_count;
    char splitter = parser->settings.splitter;

    CONTAINER_DATA** lines = parser->container->lines;
    LINE_INFO* info = parser->container->info;

    for (size_t i = 0; i < line_count; i++)
        {
            CONTAINER_DATA* current_data = lines[i];

            for (size_t j = 0; j < info[i].token_count - 1; j++)
                {
                    CONTAINER_DATA data = lines[i][j];
                    switch (data.type)
                        {
                            case STRING_TYPE:
                                fprintf(target_file, "%s%c", data.value.string, splitter);
                                break;
                            case INTEGER_TYPE:
                                fprintf(target_file, "%llu%c", data.value.integer, splitter);
                                break;
                            case FLOAT_TYPE:
                                fprintf(target_file, "%Lf%c", data.value.floating, splitter);
                                break;
                            case NULL_TYPE:
                                fprintf(target_file, "NULL%c", splitter);
                                break;
                        }
                }

            // last value without the %c symbol
            CONTAINER_DATA data = lines[i][info[i].token_count - 1];
            switch (data.type)
                {
                    case STRING_TYPE:
                        fprintf(target_file, "\%s", data.value.string);
                        break;
                    case INTEGER_TYPE:
                        fprintf(target_file, "%llu", data.value.integer);
                        break;
                    case FLOAT_TYPE:
                        fprintf(target_file, "%Lf", data.value.floating);
                        break;
                    case NULL_TYPE:
                        fprintf(target_file, "NULL");
                        break;
                }

            fputc('\n', target_file);
        }

    return 0;
}

int print_all_data(PARSER* parser)
{
    return print_data(parser, PRINTING_BOND);
}

int print_data(PARSER* parser, size_t how_much_to_print)
{
    if (!parser || parser->container == NULL)
        return 1;

    PARSER_CONTAINER container = *parser->container;
    size_t line_count = container.line_count;

    if (how_much_to_print > line_count) how_much_to_print = line_count;

    printf("Printing %zu/%zu lines.\n", how_much_to_print, line_count);

    for (size_t i = 0; i < how_much_to_print; i++)
        {
            CONTAINER_DATA* current_line = container.lines[i];

            if (container.info[i].is_header)
                printf("Header: ");
            else
                printf("Line %zu: ", i);

            for (size_t j = 0; j < container.info[i].token_count; j++)
                {
                    CONTAINER_DATA data = current_line[j];
                    switch (data.type)
                        {
                            case STRING_TYPE:
                                printf("\"%s\" ", data.value.string);
                                break;
                            case INTEGER_TYPE:
                                printf("%llu ", data.value.integer);
                                break;
                            case FLOAT_TYPE:
                                printf("%Lf ", data.value.floating);
                                break;
                            case NULL_TYPE:
                                printf("NULL ");
                                break;
                            default:
                                printf("NO DATA ");
                        }
                }
            putchar('\n');
        }

    return 0;
}

void free_parser(PARSER* parser)
{
    if (parser->container == NULL) return;

    for (size_t i = 0; i < parser->container->line_count; i++)
        {
            for (size_t j = 0; j < parser->container->info[i].token_count; j++)
                if (parser->container->lines[i][j].type == STRING_TYPE)
                    free(parser->container->lines[i][j].value.string);
            free(parser->container->lines[i]);
        }

    free(parser->container->lines);
    free(parser->container->info);
    free(parser->container);
    parser->container = NULL;
}

PARSER_SETTINGS create_parser_settings()
{
    if (system_initialized ^ 1) _init_parser();
    return DEFAULT_PARSER_SETTINGS;
}

void change_default_settings(PARSER_SETTINGS settings)
{
    parser_settings_initialized = 1;
    DEFAULT_PARSER_SETTINGS = settings;
}

PARSER_SORT_SETTINGS create_parser_sort_settings()
{
    if (parser_sort_settings_initialized ^ 1) _init_parser();
    return DEFAULT_PARSER_SORT_SETTINGS;
}

void change_default_sort_settings(PARSER_SORT_SETTINGS settings)
{
    parser_sort_settings_initialized = 1;
    DEFAULT_PARSER_SORT_SETTINGS = settings;
}

/* =============== PRIVATE ================ */
// Initialization functions
void _init_parser()
{
    if (parser_settings_initialized ^ 1)
        {
            change_default_settings(
                _create_default_parser_settings());
        }
    if (parser_sort_settings_initialized ^ 1)
        {
            change_default_sort_settings(
                _create_default_parser_sort_settings());
        }
    system_initialized = 1;
}

PARSER_SETTINGS _create_default_parser_settings()
{
    PARSER_SETTINGS settings;
    settings.splitter = ';';
    settings.ignore_errors = 1;
    settings.ignore_first_line = 0;
    settings.first_line_as_header = 1;
    return settings;
}

PARSER_SORT_SETTINGS _create_default_parser_sort_settings()
{
    PARSER_SORT_SETTINGS settings;
    settings.tag = COLUMN_INDEX;
    settings.value.column_index = 0;
    settings.case_sensitive = 1;
    settings.direction = ASCENDING;
    return settings;
}

// Parser functions
void _parse_file(PARSER* parser, P_PFILE file_to_parse)
{
    char buffer[BUFFER_CAPACITY];
    size_t line_count = 0;
    size_t column_count = 0;
    size_t capacity = 10;

    CONTAINER_DATA** lines = malloc(capacity * sizeof(CONTAINER_DATA*));
    LINE_INFO* info = malloc(capacity * sizeof(LINE_INFO));

    const char splitter = parser->settings.splitter;
    const int ignore_first_line = parser->settings.ignore_first_line;
    const int first_line_as_header = (ignore_first_line) ? 0 : parser->settings.first_line_as_header;

    // handling the first line here ( outside the loop ) to avoid repeated checks
    if (fgets(buffer, sizeof(buffer), file_to_parse))
        {
            if (ignore_first_line ^ 1)
                {
                    if (first_line_as_header)
                        info[line_count].is_header = 1;
                    else
                        info[line_count].is_header = 0;
                    size_t token_count;
                    lines[line_count] = _parse_line(buffer, splitter, &token_count);
                    info[line_count].token_count = token_count;

                    // checking for 'bad' headers and making them str
                    if (info[line_count].is_header)
                        _fix_headers(lines, token_count);

                    line_count++;
                }
        }

    // process remaining lines
    while (fgets(buffer, sizeof(buffer), file_to_parse))
        {
            if (line_count >= capacity)
                {
                    capacity *= 2;
                    lines = realloc(lines, capacity * sizeof(CONTAINER_DATA*));
                    info = realloc(info, capacity * sizeof(LINE_INFO));
                }

            size_t token_count;
            lines[line_count] = _parse_line(buffer, splitter, &token_count);
            info[line_count].token_count = token_count;
            info[line_count].is_header = 0;

            if (info[line_count].token_count > column_count) column_count = info[line_count].token_count;

            line_count++;
        }

    parser->container = malloc(sizeof(PARSER_CONTAINER));
    parser->container->lines = lines;
    parser->container->info = info;
    parser->container->column_count = column_count;
    parser->container->line_count = line_count;
    parser->container->header_included = first_line_as_header;
}

CONTAINER_DATA* _parse_line(const char* line, char splitter, size_t* token_count)
{
    char* line_copy = strdup(line);
    size_t count = 0;
    size_t capacity = INITIAL_TOKENS_CAPACITY;
    CONTAINER_DATA* tokens = malloc(capacity * sizeof(CONTAINER_DATA));

    char* start = line_copy;
    char* end = line_copy;

    while (*end)
        {
            if (*end == splitter)
                {
                    *end = '\0';
                    if (count >= capacity)
                        {
                            capacity *= 2;
                            tokens = realloc(tokens, capacity * sizeof(CONTAINER_DATA));
                        }
                    tokens[count++] = _parse_token(start);
                    start = end + 1; // move to next token start
                }
            end++;
        }

    // process if any last token
    if (start < end)
        {
            if (count >= capacity)
                {
                    capacity += 1;
                    tokens = realloc(tokens, capacity * sizeof(CONTAINER_DATA));
                }
            tokens[count++] = _parse_token(start);
        }

    free(line_copy);
    *token_count = count;
    return tokens;
}

CONTAINER_DATA _parse_token(char* token)
{
    CONTAINER_DATA data;

    // remove new lines, then trim whitespace
    char* trimmed = _trim_whitespace(_trim_newlines(token));
    char* unquoted = _remove_quotes(trimmed);

    int search_result = _check_for_quotes(trimmed, strlen(trimmed));

    // check for NULL/empty values
    if (trimmed[0] == '\0' || search_result == 2 || strcasecmp(trimmed, "NULL") == 0)
        {
            data.type = NULL_TYPE;
            data.value.null = NULL;
            return data;
        }

    // if search result is 1 then we always treat it as string no matter what
    if (search_result)
        {
            data.type = STRING_TYPE;
            data.value.string = strdup(unquoted);
            return data;
        }

    // try parsing as integer
    char* endptr;
    ull integer_value = strtoull(trimmed, &endptr, 10);
    if (*endptr == '\0')
        {
            data.type = INTEGER_TYPE;
            data.value.integer = integer_value;
            return data;
        }

    // try parsing as float
    endptr = NULL;
    bigfloat float_value = strtold(trimmed, &endptr);
    if (endptr != NULL && *endptr == '\0')
        {
            data.type = FLOAT_TYPE;
            data.value.floating = float_value;
            return data;
        }

    // if neither worked, treat as string
    data.type = STRING_TYPE;
    data.value.string = strdup(unquoted);
    return data;
}

char* _trim_whitespace(char* str)
{
    char* end;

    // trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)  // all spaces?
        return str;

    // trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // write new null terminator
    *(end + 1) = '\0';

    return str;
}

char* _trim_newlines(char* str)
{
    size_t len = strlen(str);

    size_t search_index = strcspn(str, "\n\r");
    while (search_index < len)
        {
            memmove(&str[search_index], &str[search_index + 1], len - search_index);
            len--;
            search_index = strcspn(str, "\n\r");
        }

    return str;
}

int _check_for_quotes(char* str, size_t len)
{
    if (len >= 2 && str[0] == '"' && str[len-1] == '"')
        {
            if (len > 2)
                return 1;
            else
                return 2; // means that str is NULL
        }

    return 0;
}

char* _remove_quotes(char* str)
{
    size_t len = strlen(str);

    if (_check_for_quotes(str, len) == 1)
        {
            // remove surrounding quotes somehow
            memmove(str, str + 1, len - 2);
            str[len - 2] = '\0';
        }
    return str;
}

void* _fix_headers(CONTAINER_DATA** lines, size_t token_count)
{
    for (size_t i = 0; i < token_count; i++)
        {
            CONTAINER_DATA* current_data = &lines[0][i]; // thinking that lines[0] is header line
            if (current_data->type == INTEGER_TYPE
                    || current_data->type == FLOAT_TYPE)
                {
                    current_data->value.string = _container_value_to_str(*current_data);
                    current_data->type = STRING_TYPE;
                }
            else if(current_data->type == NULL_TYPE)
                {
                    char* new_char = malloc(32);
                    sprintf(new_char, PARSER_COLUMN_CUSTOM_NAME, i);
                    current_data->type = STRING_TYPE;
                    current_data->value.string = new_char;
                }
        }
}

// Sorting functions
int _compare_cells(
    const size_t a_idx,
    const size_t b_idx,
    const size_t sort_column,
    PARSER* parser)
{
    PARSER_CONTAINER* container = parser->container;
    PARSER_SORT_SETTINGS settings = parser->sort_settings;

    CONTAINER_DATA* data_a = container->lines[a_idx];
    CONTAINER_DATA* data_b = container->lines[b_idx];

    CONTAINER_DATA cell_a = data_a[sort_column];
    CONTAINER_DATA cell_b = data_b[sort_column];

    int result = 0;
    if (cell_a.type == cell_b.type)
        {
            switch (cell_a.type)
                {
                    case INTEGER_TYPE:
                        if (cell_a.value.integer < cell_b.value.integer) result = -1;
                        else if (cell_a.value.integer > cell_b.value.integer) result = 1;
                        else result = 0;
                        break;
                    case FLOAT_TYPE:
                        if (cell_a.value.floating < cell_b.value.floating) result = -1;
                        else if (cell_a.value.floating > cell_b.value.floating) result = 1;
                        else result =  0;
                        break;
                    case STRING_TYPE:
                        if (settings.case_sensitive)
                            result = strcmp(cell_a.value.string, cell_b.value.string);
                        else
                            result = strcasecmp(cell_a.value.string, cell_b.value.string);
                        break;
                    case NULL_TYPE:
                        result =  0; // equality muthafaka
                        break;
                }
        }
    else
        {
            char* str_a = _container_value_to_str(cell_a);
            char* str_b = _container_value_to_str(cell_b);

            if (settings.case_sensitive)
                result = strcmp(str_a, str_b);
            else
                result = strcasecmp(str_a, str_b);

            free(str_a);
            free(str_b);
        }

    if (settings.direction == DESCENDING)
        result = -result;

    return result;
}

char* _container_value_to_str(CONTAINER_DATA data)
{
    char* str_buffer = malloc(BUFFER_CAPACITY);

    switch (data.type)
        {
            case STRING_TYPE:
                strncpy(str_buffer, data.value.string, BUFFER_CAPACITY);
                break;
            case INTEGER_TYPE:
                snprintf(str_buffer, BUFFER_CAPACITY, "%llu", data.value.integer);
                break;
            case FLOAT_TYPE:
                snprintf(str_buffer, BUFFER_CAPACITY, "%Lf", data.value.floating);
                break;
            case NULL_TYPE:
                strncpy(str_buffer, "NULL ", BUFFER_CAPACITY);
                break;
        }

    return str_buffer;
}

void _quick_sort(PARSER* parser, size_t sort_column, QuickSortComp comp_func, size_t left, size_t right, size_t* indices)
{
    if (left < right)
        {
            size_t pr = _partition(parser, sort_column, comp_func, left, right, indices);
            _quick_sort(parser, sort_column, comp_func, left, pr - 1, indices);
            _quick_sort(parser, sort_column, comp_func, pr + 1, right, indices);
        }
}

size_t _partition(PARSER* parser, size_t sort_column, QuickSortComp comp_func, size_t left, size_t right, size_t* indices)
{
    size_t i = left - 1, j, pivot = right;
    for (j = left; j < right; j++)
        {
            if (comp_func(indices[j], indices[pivot], sort_column, parser) < 0)
                {
                    ++i;
                    _swap(&indices[i], &indices[j]);
                }
        }
    size_t pr = i + 1;
    _swap(&indices[pr], &indices[pivot]);
    return pr;
}

void _swap(size_t* a, size_t* b)
{
    size_t temp = *a;
    *a = *b;
    *b = temp;
}