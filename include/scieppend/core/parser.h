#ifndef SCIEPPEND_CORE_PARSER_H
#define SCIEPPEND_CORE_PARSER_H

/* File parser for custom data file type.
 * Each parser field must be registered explicitly via parser_register_field().
 *
 * The data file format consists of empty-line separated blocks of data. Each line of a data block
 * is a parseable comma separated key value pair. Keys must be unique WITHIN a data block. The
 * value may be a list of semicolon separated values. The data type of the field's values is
 * specified by the user when registering the field.
 *
 * e.g.
 * id:1
 * name:exempli
 * colour:32;32;32
 *
 * id:2
 * name:gratis
 * colour:9;100;27
 */

#include "scieppend/core/list.h"

#include <stdbool.h>

enum ParserCode
{
    PARSER_OK,
    PARSER_NO_OP,
    PARSER_MALFORMED_FORMAT,
    PARSER_NO_FORMAT_FOUND,
    PARSER_MALFORMED_KEY_VALUE_PAIR,
    PARSER_MALFORMED_DATA,
    PARSER_UNKNOWN_DATA_TYPE,
    PARSER_FILE_OPEN_FAIL,
    PARSER_PARSE_CALLBACK_ERROR
};

enum ParseCallbackCode
{
    PARSE_CALLBACK_OK,
    PARSE_CALLBACK_ERROR
};

struct ParseState
{
    const char* filename;
    int line_no;
    char line[256];
    bool active;
};

struct Parser;

typedef enum ParseCallbackCode(*parse_method)(struct Parser*);

/* Create a new parser.
 */
struct Parser* parser_new(void);

/* Free a parser
 */
void parser_free(struct Parser* parser);

/* Get the internal state of the given parser
 */
void parser_get_state(struct Parser* parser, struct ParseState* state);

/* Register a data field with a given parser.
 * The field name corresponds with the field's unique key. The field data format is a string
 * describing the field's values in the format "name type".
 *
 * e.g. parser_register_field(p, "colour", "red int green int blue int");
 *      This could parse a data field such as:
 *      colour:32;32;32
 */
enum ParserCode parser_register_field(struct Parser* parser, char* field_name, char* field_data_format, parse_method pmeth);

/* Parse a given line.
 * Returns the parse status code for given line.
 */
enum ParserCode parser_parse(struct Parser* parser, char* line);

/* Parse a given file.
 * Return true if no parse errors encountered during parsing.
 */
bool parser_parse_file(struct Parser* parser, const char* filename);

/* Set the active data being parsed into.
 * This should be called by the user from the initial parsing callback.
 */
void parser_set_userdata(struct Parser* parser, void* userdata);

/* Gets all the parsed userdata.
 * This should be called by the user at the end to deal with the loaded data.
 */
struct List* parser_get_userdata(struct Parser* parser);

/* Get the active data being parsed into.
 * This should be called by the user from every parsing callback so they can set the data onto it.
 */
void* parser_get_userdata_active(struct Parser* parser);

/* Get the last parser code to check status.
 */
enum ParserCode parser_get_last_code(struct Parser* parser);

/* Return an int from a named data field's named data value.
 */
int parser_field_get_int(struct Parser* parser, const char* field_name, const char* field_data_name);

/* Return a char from a named data field's named data value.
 */
char parser_field_get_char(struct Parser* parser, const char* field_name, const char* field_data_name);

/* Return a string from a named data field's named data value.
 */
char* parser_field_get_string(struct Parser* parser, const char* field_name, const char* field_data_name);

/* Return a bool from a named data field's named data value.
 */
bool parser_field_get_bool(struct Parser* parser, const char* field_name, const char* field_data_name);

/* DEBUG_CORE_PARSER ONLY
 * Dumps the parser's parse formats to stdout.
 */
void parser_print_formats(struct Parser* parser);

#endif
