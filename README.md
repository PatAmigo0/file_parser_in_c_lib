# File Parser Library

This lightweight library provides a simple, efficient way to parse, manipulate, and save CSV (and other delimited) files in C. It features automatic type detection, sorting capabilities, and flexible data handling.

## Version
```0.1 BETA```

## Features

- **Automatic type detection**: Strings, integers, floats, and NULL values
- **Flexible parsing**: Customizable delimiters and parsing options
- **Sorting capabilities**: Sort by column index or name, ascending or descending
- **Memory efficient**: Smart memory management with automatic cleanup
- **Comprehensive logging**: Configurable logging levels for debugging
- **Header support**: Automatic header detection and handling
- **Cross-platform**: Works on any platform with a C99 compiler

## Installation

1. Add `fileparser.h` and `fileparser.c` to your project
2. Include the header in your code:
```c
#include "fileparser.h"
```

## Requirements

- C99 compatible compiler
- Standard C libraries (stdio.h, stdlib.h, string.h)

## Usage

### Basic Setup

```c
#include "fileparser.h"

int main()
{
    // create a parser with default settings
    PARSER parser = create_parser();
    
    // parse a CSV file
    if (parse_file(&parser, "data.csv"))
    {
        printf("Failed to parse file\n");
        return 1;
    }
    
    // print all data
    print_all_data(&parser);
    
    // sort by the second column (index 1)
    PARSER_SORT_SETTINGS sort_settings = create_parser_sort_settings();
    sort_settings.tag = COLUMN_INDEX;
    sort_settings.value.column_index = 1;
    sort_settings.direction = ASCENDING;
    
    if (sort_data(&parser, sort_settings))
    {
        printf("Failed to sort data\n");
    }
    
    // save the sorted data
    save_data(&parser, "sorted_data.csv");
    
    // always clean up
    free_parser(&parser);
    
    return 0;
}
```

### Advanced Usage with Custom Settings

```c
#include "fileparser.h"

int main()
{
    // create custom parser settings
    PARSER_SETTINGS settings = create_parser_settings();
    settings.splitter = ',';        // use comma as delimiter
    settings.ignore_first_line = 0; // don't ignore first line
    settings.first_line_as_header = 1; // treat first line as header
    
    // change default settings
    change_default_settings(settings);
    
    // create parser with custom settings
    PARSER parser = create_parser();
    
    // parse file
    if (parse_file(&parser, "data.csv"))
    {
        PARSER_LOG_CRITICAL("Failed to parse file");
        return 1;
    }
    
    // sort by column name
    PARSER_SORT_SETTINGS sort_settings = create_parser_sort_settings();
    sort_settings.tag = COLUMN_NAME;
    sort_settings.value.column_name = "Age"; // column name to sort by
    sort_settings.direction = DESCENDING;
    sort_settings.case_sensitive = 0; // case insensitive comparison
    
    if (sort_data(&parser, sort_settings))
    {
        PARSER_LOG_CRITICAL("Failed to sort data");
    }
    else
    {
        // save sorted data with pipe delimiter
        parser.settings.splitter = '|';
        save_data(&parser, "sorted_data.txt");
    }
    
    // always clean up
    free_parser(&parser);
    
    return 0;
}
```

## Core Functions Reference

### Parser Creation & Management
1. **`PARSER create_parser()`**  
   Creates and returns a new parser object with default settings.

2. **`int parse_file(PARSER* parser, const char* filename)`**  
   Parses a file and stores the data in the parser object. (You can do your custom logic with it)

3. **`void free_parser(PARSER* parser)`**  
   Frees all resources associated with the parser.

### Data Manipulation
4. **`int sort_data(PARSER* parser, PARSER_SORT_SETTINGS settings)`**  
   Sorts the parsed data by the specified column.

5. **`int save_data(PARSER* parser, const char* filename)`**  
   Saves the parsed data to a file.

### Data Display
6. **`int print_all_data(PARSER* parser)`**  
   Prints all parsed data to the console.

7. **`int print_data(PARSER* parser, size_t how_much_to_print)`**  
   Prints a specified amount of parsed data to the console.

### Settings Management
8. **`PARSER_SETTINGS create_parser_settings()`**  
   Creates a new settings object with default values.

9. **`void change_default_settings(PARSER_SETTINGS settings)`**  
   Changes the default parser settings.

10. **`PARSER_SORT_SETTINGS create_parser_sort_settings()`**  
    Creates a new sort settings object with default values.

11. **`void change_default_sort_settings(PARSER_SORT_SETTINGS settings)`**  
    Changes the default sort settings.

## Configuration

### Logging Levels
The library provides configurable logging levels:

```c
#define LOGLEVEL_CRITICAL 0  // only critical errors
#define LOGLEVEL_WARNING  1  // warnings and errors
#define LOGLEVEL_INFO     2  // informational messages (default)
#define LOGLEVEL_DEBUG    3  // debug information
#define LOGLEVEL_NONE     4  // no logging
```

To change the logging level, modify the `LOG_LEVEL` definition in `fileparser.h`:

```c
#define LOG_LEVEL LOGLEVEL_WARNING // change to the desired level
```

### Parser Settings
Customize parsing behavior with `PARSER_SETTINGS`:

- `splitter`: Character used to separate values (default: ';')
- `ignore_first_line`: Whether to ignore the first line (default: 0)
- `ignore_errors`: Whether to continue parsing on errors (default: 1)
- `first_line_as_header`: Whether to treat the first line as header (default: 1)

### Sort Settings
Customize sorting behavior with `PARSER_SORT_SETTINGS`:

- `tag`: Specify whether to sort by `COLUMN_NAME` or `COLUMN_INDEX`
- `value`: The column name or index to sort by
- `direction`: `ASCENDING` or `DESCENDING` order
- `case_sensitive`: Whether string comparisons should be case sensitive (default: 1)

## Building

Compile with your project:
```bash
gcc <your_app.c> fileparser.c -o your_app
```

## Data Types

The library automatically detects and handles these data types:

- **STRING_TYPE**: Text values (quoted or unquoted)
- **INTEGER_TYPE**: Whole numbers
- **FLOAT_TYPE**: Decimal numbers
- **NULL_TYPE**: Empty values or explicit NULL strings

**NOTE:** More to be added in the future.

## IMPORTANT NOTES

1. **Memory Management**  
   - Always use `free_parser()` to properly free parser resources

2. **File Format**  
   - Supports any delimiter (CHAR) (configurable via `splitter` setting)
   - Handles multi quoted values (like """hello""")
   - Automatically trims whitespace and newlines
   - Recognizes "NULL" (case-insensitive) as a null value

3. **Performance**  
   - Efficient parsing with minimal memory overhead
   - Sorting uses quicksort algorithm for performance

4. **Error Handling**  
   - Comprehensive error logging at multiple levels
   - All functions return error codes for programmatic handling

5. **Header Handling**  
   - Headers are automatically converted to strings if needed
   - Missing headers are given automatic names (\_\_parser_column\_\%d\_\_) where \%d stands for column index

## Contributing

Contributions are welcome! Please submit pull requests or open issues on GitHub.
