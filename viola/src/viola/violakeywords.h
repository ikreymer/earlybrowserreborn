struct lexrecord {
    char *name;
    unsigned long token;
};

enum { SPECIAL_TRUE = 1, SPECIAL_FALSE = 2, SPECIAL_COMMENT = 3 };

/*
 * None of the above enums can be equal to 0
 *
 */

/*
 * All of the keywords below should be entered in lowercase,
 * otherwise the trie insertion routine will not produce tables
 * conducive to lowercase keyword matches.
 *
 */

struct lexrecord primstuff[] = {
    "true",     SPECIAL_TRUE,
    "false",    SPECIAL_FALSE,
    "if",       IF,
    "else",     ELSE,
    "for",      FOR,
    "do",       DO,
    "while",    WHILE,
    "switch",   SWITCH,
    "eswitch",  ESWITCH,
    "case",     CASE,
    "default",  DEFAULT,
    "break",    BREAK,
    "return",   RETURN,
    "get",      GET,
    "set",      SET,
    "persistent", PERSISTENT,

    "/*",       SPECIAL_COMMENT,
    "+",        '+',
    "-",        '-',
    "%",        '%',
    "*",        '*',
    "/",        '/',
    "=",        '=',
    ".",        '.',
    ":",        ':',
    ",",        ',',
    "(",        '(',
    ")",        ')',
    "[",        '[',
    "]",        ']',
    "{",        '{',
    "}",        '}',
    ";",        ';',
    ">",        '>',
    "<",        '<',
    "!",        '!',
    "\\",       '\\',

    "==",        EQ,
    "!=",        NE,
    "<=",        LE,
    ">=",        GE,
    "++",        INC,
    "--",        DEC,
    "+=",        PLUS_ASSERT,
    "-=",        MINUS_ASSERT,
    "%=",        MOD_ASSERT,
    "*=",        MULT_ASSERT,
    "/=",        DIV_ASSERT,
    "&&",        AND,
    "||",        OR,
};
