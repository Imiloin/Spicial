/*******************************************************

Tokenizer for SPICE

flex -o src/scanner.cpp --header-file=src/scanner.hpp src/scanner.lex

*******************************************************/

%option case-insensitive
%option yylineno

%option noyywrap
%{
void yyrestart(FILE *);
%}

/*%option nounistd*/
/*%option never-interactive*/

/* Defining output file name */
/* %option outfile="src/scanner.cpp" */
/* %option header-file="src/scanner.hpp" */

%{
#include "parser.hpp"
#include "linetype.h"
#include <ctype.h>
#include <algorithm>
#include <cctype>

#define YY_DECL int yylex(yy::Parser::semantic_type *yylval, yy::Parser::location_type *yylloc)

namespace yy { int column = 1; }  // yy::column is used to record the current column number

// run each time a token is matched
#define YY_USER_ACTION {if(yylineno != yylloc->begin.line) yy::column = 0; \
                                                yylloc->begin.line = yylineno; \
                                                yylloc->begin.column = yy::column; \
                                                yy::column = yy::column+yyleng; \
                                                yylloc->end.column = yy::column; \
                                                yylloc->end.line = yylineno;}

typedef yy::Parser::token token;

char *copyStr(const char *str);
char *copyStrTolower(const char *str);
char *copyStrToupper(const char *str);
double parseValue(const char *str);

char* first_token_of_current_line = NULL;
int current_line_type = LINE_TYPE_COMMENT;
int current_token_needed = 0;
bool optional_token = false;
bool uppercasing = false;
%}

%x NODES VALUES VARIABLES FUNCTIONS
%x NODES_BRANCHES
%x MODELNAMES
%x KEYWORD_PARAM
%x ANALYSIS_TYPE
%x DC_SOURCE
%x AC_TYPES
%x FUNCTION_VALUES
%x OPTIONS
%x FILEEND


/** Defining regular expressions of the patterns */
ALPHA     [A-Za-z_]
DIGIT     [0-9]
ALPHANUM  [A-Za-z_0-9]
STRING    {ALPHA}{ALPHANUM}+
INTEGER   {DIGIT}+
/* FLOAT     [\-]?{DIGIT}+"."{DIGIT}+([Ee][\+\-]?{DIGIT}+)? */
/* FLOAT     [\-]?({DIGIT}?"."{DIGIT}*|{DIGIT}*)([Ee][\+\-]?{DIGIT}+)? */
FLOAT     [\-]?(({DIGIT}+"."{DIGIT}*)|("."{DIGIT}+))([Ee][\+\-]?{DIGIT}+)?

/* components */
RESISTOR         [Rr]{ALPHANUM}+
CAPACITOR        [Cc]{ALPHANUM}+
INDUCTOR         [Ll]{ALPHANUM}+
VCVS             [Ee]{ALPHANUM}+
CCCS             [Ff]{ALPHANUM}+
VCCS             [Gg]{ALPHANUM}+
CCVS             [Hh]{ALPHANUM}+
VOLTAGE_SOURCE   [Vv]{ALPHANUM}+
CURRENT_SOURCE   [Ii]{ALPHANUM}+
DIODE            [Dd]{ALPHANUM}+

/* analysis */
OP        ^[\.][Oo][Pp]
DC        ^[\.][Dd][Cc]
AC        ^[\.][Aa][Cc]
TRAN      ^[\.][Tt][Rr][Aa][Nn]
PRINT     ^[\.][Pp][Rr][Ii][Nn][Tt]
PLOT      ^[\.][Pp][Ll][Oo][Tt]
OPTION    ^[\.][Oo][Pp][Tt][Ii][Oo][Nn][Ss]*

TYPE_OP   [Oo][Pp]
TYPE_DC   [Dd][Cc]
TYPE_AC   [Aa][Cc]
TYPE_TRAN [Tt][Rr][Aa][Nn]

TYPE_DEC  [Dd][Ee][Cc]
TYPE_OCT  [Oo][Cc][Tt]
TYPE_LIN  [Ll][Ii][Nn]

OPTION_NODE    [Nn][Oo][Dd][Ee]
OPTION_LIST    [Ll][Ii][Ss][Tt]

EOL       [\n]
DELIMITER [ \t]+
UNIT      [Ff]|[Pp]|[Nn]|[Uu]|[Mm]|[Kk]|[Mm][Ee][Gg]|[Gg]|[Tt]
VALUE     ({FLOAT}|[\-]?{INTEGER}){UNIT}?

VALUE_VOLTAGE       {VALUE}[Vv]
VALUE_CURRENT       {VALUE}[Aa]
VALUE_RESISTANCE    {VALUE}[Oo][Hh][Mm]
VALUE_CAPACITANCE   ({FLOAT}|[\-]?{INTEGER}){UNIT}[Ff]
VALUE_INDUCTANCE    {VALUE}[Hh]
VALUE_TIME          {VALUE}[Ss]
VALUE_LENGTH        ({FLOAT}|[\-]?{INTEGER}){UNIT}[Mm]
VALUE_FREQUENCY     {VALUE}[Hh][Zz]
VALUE_ANGLE         {VALUE}[Dd][Ee][Gg]

VAR_TYPE_VOLTAGE_REAL    [Vv][Rr]
VAR_TYPE_VOLTAGE_IMAG    [Vv][Ii]
VAR_TYPE_VOLTAGE_MAG     [Vv][Mm]*
VAR_TYPE_VOLTAGE_PHASE   [Vv][Pp]
VAR_TYPE_VOLTAGE_DB      [Vv][Dd][Bb]
VAR_TYPE_CURRENT_REAL    [Ii][Rr]
VAR_TYPE_CURRENT_IMAG    [Ii][Ii]
VAR_TYPE_CURRENT_MAG     [Ii][Mm]*
VAR_TYPE_CURRENT_PHASE   [Ii][Pp]
VAR_TYPE_CURRENT_DB      [Ii][Dd][Bb]

FUNC_TYPE_SIN    [Ss][Ii][Nn]
FUNC_TYPE_PULSE  [Pp][Uu][Ll][Ss][Ee]

/* xx=yy , xx is the keyword, yy is the parameter */
IC_EQUAL  [Ii][Cc]{DELIMITER}*={DELIMITER}*

COMMA     {DELIMITER}*","{DELIMITER}*
LPAREN    [\(]{DELIMITER}*
RPAREN    {DELIMITER}*[\)]

COMMENT          ^[\*][^\n]+
INLINE_COMMENT   [\$][^\n]+
END              [\.][Ee][Nn][Dd]

%%

%{
    // start where previous token ended
    yylloc->step();
%}

<INITIAL>{
%{
/* components */
%}
{RESISTOR} {
    BEGIN(NODES); 
    first_token_of_current_line = strdup(yytext); 
    yylval->s = copyStrToupper(yytext); 
    current_line_type = COMPONENT_RESISTOR;
    current_token_needed = 2;
    return token::RESISTOR;
}
{CAPACITOR} {
    BEGIN(NODES); 
    first_token_of_current_line = strdup(yytext); 
    yylval->s = copyStrToupper(yytext); 
    current_line_type = COMPONENT_CAPACITOR;
    current_token_needed = 2;
    return token::CAPACITOR;
}
{INDUCTOR} {
    BEGIN(NODES); 
    first_token_of_current_line = strdup(yytext); 
    yylval->s = copyStrToupper(yytext);
    current_line_type = COMPONENT_INDUCTOR;
    current_token_needed = 2; 
    return token::INDUCTOR;
}
{VCVS} {
    BEGIN(NODES); 
    first_token_of_current_line = strdup(yytext); 
    yylval->s = copyStrToupper(yytext); 
    current_line_type = COMPONENT_VCVS;
    current_token_needed = 4;
    return token::VCVS;
}
{CCCS} {
    BEGIN(NODES); 
    first_token_of_current_line = strdup(yytext); 
    yylval->s = copyStrToupper(yytext); 
    current_line_type = COMPONENT_CCCS;
    current_token_needed = 2;
    return token::CCCS;
}
{VCCS} {
    BEGIN(NODES); 
    first_token_of_current_line = strdup(yytext); 
    yylval->s = copyStrToupper(yytext); 
    current_line_type = COMPONENT_VCCS;
    current_token_needed = 4;
    return token::VCCS;
}
{CCVS} {
    BEGIN(NODES); 
    first_token_of_current_line = strdup(yytext); 
    yylval->s = copyStrToupper(yytext); 
    current_line_type = COMPONENT_CCVS;
    current_token_needed = 2;
    return token::CCVS;
}
{VOLTAGE_SOURCE} {
    BEGIN(NODES); 
    first_token_of_current_line = strdup(yytext); 
    yylval->s = copyStrToupper(yytext); 
    current_line_type = COMPONENT_VOLTAGE_SOURCE;
    current_token_needed = 2;
    return token::VOLTAGE_SOURCE;
}
{CURRENT_SOURCE} {
    BEGIN(NODES); 
    first_token_of_current_line = strdup(yytext); 
    yylval->s = copyStrToupper(yytext); 
    current_line_type = COMPONENT_CURRENT_SOURCE;
    current_token_needed = 2;
    return token::CURRENT_SOURCE;
}
{DIODE} {
    BEGIN(NODES); 
    first_token_of_current_line = strdup(yytext); 
    yylval->s = copyStrToupper(yytext); 
    current_line_type = COMPONENT_DIODE;
    current_token_needed = 2;
    return token::DIODE;
}

%{
/* analysis */
%}
{OP} {
    BEGIN(INITIAL); 
    first_token_of_current_line = strdup(yytext); 
    current_line_type = ANALYSIS_OP;
    return token::OP;
}
{DC} {
    BEGIN(DC_SOURCE); 
    first_token_of_current_line = strdup(yytext); 
    current_line_type = ANALYSIS_DC;
    current_token_needed = 1;
    return token::DC;
}
{AC} {
    BEGIN(AC_TYPES);
    first_token_of_current_line = strdup(yytext); 
    current_line_type = ANALYSIS_AC;
    current_token_needed = 1;
    return token::AC;
}
{TRAN} {
    BEGIN(VALUES);
    first_token_of_current_line = strdup(yytext); 
    current_line_type = ANALYSIS_TRAN;
    current_token_needed = 2;
    return token::TRAN;
}
{PRINT} {
    BEGIN(ANALYSIS_TYPE); 
    first_token_of_current_line = strdup(yytext); 
    current_line_type = ANALYSIS_PRINT;
    current_token_needed = 1;
    return token::PRINT;
}
{PLOT} {
    BEGIN(ANALYSIS_TYPE); 
    first_token_of_current_line = strdup(yytext); 
    current_line_type = ANALYSIS_PLOT;
    current_token_needed = 1;
    return token::PLOT;
}
{OPTION} {
    BEGIN(OPTIONS); 
    first_token_of_current_line = strdup(yytext); 
    current_line_type = ANALYSIS_OPTIONS;
    optional_token = true;
    return token::OPTION;
}
{END} {
    BEGIN(FILEEND); 
    current_line_type = ANALYSIS_END;
    current_token_needed = 0;
    return token::END;
}
}

<NODES>{
%{
/* node */
%}
{STRING} {
    yylval->s = copyStrTolower(yytext); 
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case COMPONENT_RESISTOR:
            case COMPONENT_CAPACITOR:
            case COMPONENT_INDUCTOR:
            case COMPONENT_VCVS:
            case COMPONENT_VCCS:
                BEGIN(VALUES); current_token_needed = 1; break;
            case COMPONENT_CCCS:
            case COMPONENT_CCVS:
                BEGIN(NODES_BRANCHES); current_token_needed = 1; uppercasing = true; break;
            case COMPONENT_VOLTAGE_SOURCE:
                BEGIN(ANALYSIS_TYPE); optional_token = true; break;
            case COMPONENT_CURRENT_SOURCE:
                BEGIN(VALUES); current_token_needed = 1; break;
            case COMPONENT_DIODE:
                BEGIN(MODELNAMES); current_token_needed = 1; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing NODES\n");
        }
    }
    return token::NODE;
}
{INTEGER} {
    yylval->s = copyStrTolower(yytext); 
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case COMPONENT_RESISTOR:
            case COMPONENT_CAPACITOR:
            case COMPONENT_INDUCTOR:
            case COMPONENT_VCVS:
            case COMPONENT_VCCS:
                BEGIN(VALUES); current_token_needed = 1; break;
            case COMPONENT_CCCS:
            case COMPONENT_CCVS:
                BEGIN(NODES_BRANCHES); current_token_needed = 1; uppercasing = true; break;
            case COMPONENT_VOLTAGE_SOURCE:
                BEGIN(ANALYSIS_TYPE); optional_token = true; break;
            case COMPONENT_CURRENT_SOURCE:
                BEGIN(VALUES); current_token_needed = 1; break;
            case COMPONENT_DIODE:
                BEGIN(MODELNAMES); current_token_needed = 1; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing NODES\n");
        }
    }
    return token::NODE;
}
}

<MODELNAMES>{
%{
/* modelnames */
%}
{STRING} {
    yylval->s = copyStrTolower(yytext); 
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case COMPONENT_RESISTOR:
            case COMPONENT_CAPACITOR:
            case COMPONENT_INDUCTOR:
            case COMPONENT_VCVS:
            case COMPONENT_CCCS:
            case COMPONENT_VCCS:
            case COMPONENT_CCVS:
            case COMPONENT_VOLTAGE_SOURCE:
            case COMPONENT_CURRENT_SOURCE:
            case COMPONENT_DIODE:
                BEGIN(KEYWORD_PARAM); optional_token = true; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing MODELNAMES\n");
        }
    }
    return token::MODELNAME;
}
{INTEGER} {
    yylval->s = copyStrTolower(yytext); 
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case COMPONENT_RESISTOR:
            case COMPONENT_CAPACITOR:
            case COMPONENT_INDUCTOR:
            case COMPONENT_VCVS:
            case COMPONENT_CCCS:
            case COMPONENT_VCCS:
            case COMPONENT_CCVS:
            case COMPONENT_VOLTAGE_SOURCE:
            case COMPONENT_CURRENT_SOURCE:
            case COMPONENT_DIODE:
                BEGIN(KEYWORD_PARAM); optional_token = true; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing MODELNAMES\n");
        }
    }
    return token::MODELNAME;
}
}

<KEYWORD_PARAM>{
%{
/* keyword parameters or nothing */
%}
{IC_EQUAL} {
    BEGIN(VALUES);
    optional_token = false;
    current_token_needed = 1;
    return token::IC_EQUAL; 
}
}

<ANALYSIS_TYPE>{
{TYPE_OP} {
    return token::TYPE_OP;
}
{TYPE_DC} {
    if (optional_token) {
        switch(current_line_type) {
            case COMPONENT_VOLTAGE_SOURCE:
                BEGIN(VALUES); current_token_needed = 1; optional_token = false; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing ANALYSIS_TYPE\n");
        }
    } else {
        switch(current_line_type) {
            case ANALYSIS_PRINT:
            case ANALYSIS_PLOT:
                BEGIN(VARIABLES); current_token_needed = 1; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing ANALYSIS_TYPE\n");
        }
    }
    return token::TYPE_DC;
}
{TYPE_AC} {
    if (optional_token) {
        switch(current_line_type) {
            case COMPONENT_VOLTAGE_SOURCE:
                BEGIN(VALUES); current_token_needed = 1; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing ANALYSIS_TYPE\n");
        }
    } else {
        switch(current_line_type) {
            case COMPONENT_VOLTAGE_SOURCE:
                BEGIN(VALUES); current_token_needed = 1; optional_token = true; break;
            case ANALYSIS_PRINT:
            case ANALYSIS_PLOT:
                BEGIN(VARIABLES); current_token_needed = 1; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing ANALYSIS_TYPE\n");
        }
    }
    return token::TYPE_AC;
}
{TYPE_TRAN} {
    if (!optional_token) {
        switch(current_line_type) {
            case ANALYSIS_PRINT:
            case ANALYSIS_PLOT:
                BEGIN(VARIABLES); current_token_needed = 1; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing ANALYSIS_TYPE\n");
        }
    }
    return token::TYPE_TRAN;
}
{VALUE} {
    yylval->f = parseValue(yytext); 
    if (optional_token) {
        switch(current_line_type) {
            case COMPONENT_VOLTAGE_SOURCE:
                optional_token = true; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing ANALYSIS_TYPE\n");
        }
    }
    return token::VALUE;
}
{VALUE_VOLTAGE} {
    char* temp = strndup(yytext, yyleng - 1);
    yylval->f = parseValue(temp);
    free(temp);
    if (optional_token) {
        switch(current_line_type) {
            case COMPONENT_VOLTAGE_SOURCE:
                optional_token = true; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing ANALYSIS_TYPE\n");
        }
    }
    return token::VALUE_VOLTAGE;
}
{FUNC_TYPE_SIN} {
    if (optional_token) {
        switch(current_line_type) {
            case COMPONENT_VOLTAGE_SOURCE:
            case COMPONENT_CURRENT_SOURCE:
                BEGIN(FUNCTION_VALUES); optional_token = false; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing ANALYSIS_TYPE\n");
        }
    }
    return token::FUNC_TYPE_SIN;
}
{FUNC_TYPE_PULSE} {
    if (optional_token) {
        switch(current_line_type) {
            case COMPONENT_VOLTAGE_SOURCE:
            case COMPONENT_CURRENT_SOURCE:
                BEGIN(FUNCTION_VALUES); optional_token = false; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing ANALYSIS_TYPE\n");
        }
    }
    return token::FUNC_TYPE_PULSE;
}
}

<DC_SOURCE>{
{VOLTAGE_SOURCE} {
    BEGIN(VALUES); 
    first_token_of_current_line = strdup(yytext); 
    yylval->s = copyStrToupper(yytext); 
    current_line_type = COMPONENT_VOLTAGE_SOURCE;
    current_token_needed = 3;
    return token::VOLTAGE_SOURCE;
}
{CURRENT_SOURCE} {
    BEGIN(VALUES); 
    first_token_of_current_line = strdup(yytext); 
    yylval->s = copyStrToupper(yytext); 
    current_line_type = COMPONENT_CURRENT_SOURCE;
    current_token_needed = 3;
    return token::CURRENT_SOURCE;
}
}

<AC_TYPES>{
{TYPE_DEC} {
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case ANALYSIS_AC:
                BEGIN(VALUES); current_token_needed = 3; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing AC_TYPES\n");
        }
    }
    return token::TYPE_DEC;
}
{TYPE_OCT} {
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case ANALYSIS_AC:
                BEGIN(VALUES); current_token_needed = 3; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing AC_TYPES\n");
        }
    }
    return token::TYPE_OCT;
}
{TYPE_LIN} {
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case ANALYSIS_AC:
                BEGIN(VALUES); current_token_needed = 3; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing AC_TYPES\n");
        }
    }
    return token::TYPE_LIN;
}
}

<VARIABLES>{
%{
/* variables */
%}
{VAR_TYPE_VOLTAGE_REAL}   {uppercasing = false; return token::VAR_TYPE_VOLTAGE_REAL;}
{VAR_TYPE_VOLTAGE_IMAG}   {uppercasing = false; return token::VAR_TYPE_VOLTAGE_IMAG;}
{VAR_TYPE_VOLTAGE_MAG}    {uppercasing = false; return token::VAR_TYPE_VOLTAGE_MAG;}
{VAR_TYPE_VOLTAGE_PHASE}  {uppercasing = false; return token::VAR_TYPE_VOLTAGE_PHASE;}
{VAR_TYPE_VOLTAGE_DB}     {uppercasing = false; return token::VAR_TYPE_VOLTAGE_DB;}
{VAR_TYPE_CURRENT_REAL}   {uppercasing = true; return token::VAR_TYPE_CURRENT_REAL;}
{VAR_TYPE_CURRENT_IMAG}   {uppercasing = true; return token::VAR_TYPE_CURRENT_IMAG;}
{VAR_TYPE_CURRENT_MAG}    {uppercasing = true; return token::VAR_TYPE_CURRENT_MAG;}
{VAR_TYPE_CURRENT_PHASE}  {uppercasing = true; return token::VAR_TYPE_CURRENT_PHASE;}
{VAR_TYPE_CURRENT_DB}     {uppercasing = true; return token::VAR_TYPE_CURRENT_DB;}

{COMMA} {
    BEGIN(NODES_BRANCHES);
    current_token_needed = 1;
    optional_token = true;
    return token::COMMA;
}
{LPAREN} {
    BEGIN(NODES_BRANCHES);
    current_token_needed = 1;
    return token::LPAREN;
}
{RPAREN} {
    optional_token = true; 
    return token::RPAREN;
}
}

<NODES_BRANCHES>{
%{
/* node or branch, lowercase or uppercase */
%}
{STRING} {
    if (uppercasing) {
        yylval->s = copyStrToupper(yytext);
    } else {
        yylval->s = copyStrTolower(yytext); 
    }
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case COMPONENT_CCCS:
            case COMPONENT_CCVS:
                BEGIN(VALUES); current_token_needed = 1; break;
            case ANALYSIS_PRINT:
            case ANALYSIS_PLOT:
                BEGIN(VARIABLES); current_token_needed = 1; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing NODES_BRANCHES\n");
        }
    }
    return token::NODE;
}
{INTEGER} {
    yylval->s = copyStrTolower(yytext); 
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case COMPONENT_CCCS:
            case COMPONENT_CCVS:
                BEGIN(VALUES); current_token_needed = 1; break;
            case ANALYSIS_PRINT:
            case ANALYSIS_PLOT:
                BEGIN(VARIABLES); current_token_needed = 1; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing NODES_BRANCHES\n");
        }
    }
    return token::NODE;
}
{RPAREN} {
    if (optional_token) {
        switch(current_line_type) {
            case ANALYSIS_PRINT:
            case ANALYSIS_PLOT:
                BEGIN(VARIABLES); current_token_needed = 1; optional_token = true; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing ANALYSIS_TYPE\n");
        }
    }
    return token::RPAREN;
}
}

<FUNCTIONS>{
%{
/* functions */
%}
{FUNC_TYPE_SIN} {
    BEGIN(FUNCTION_VALUES);
    optional_token = true;
    return token::FUNC_TYPE_SIN;
}
{FUNC_TYPE_PULSE} {
    BEGIN(FUNCTION_VALUES);
    optional_token = true;
    return token::FUNC_TYPE_PULSE;
}
}

<FUNCTION_VALUES>{
%{
/* function values */
%}
{VALUE} {
    yylval->f = parseValue(yytext); 
    return token::VALUE;
}
{VALUE_VOLTAGE} {
    char* temp = strndup(yytext, yyleng - 1);
    yylval->f = parseValue(temp);
    free(temp);
    return token::VALUE_VOLTAGE;
}
{VALUE_CURRENT} {
    char* temp = strndup(yytext, yyleng - 1);
    yylval->f = parseValue(temp);
    free(temp);
    return token::VALUE_CURRENT;
}
{VALUE_TIME} {
    char* temp = strndup(yytext, yyleng - 1);
    yylval->f = parseValue(temp);
    free(temp);
    return token::VALUE_TIME;
}
{VALUE_ANGLE} {
    char* temp = strndup(yytext, yyleng - 3);
    yylval->f = parseValue(temp);
    free(temp);
    return token::VALUE_ANGLE;
}
{LPAREN} {
    return token::LPAREN;
}
{RPAREN} {
    return token::RPAREN;
}
}

<OPTIONS>{
%{
/* options */
%}
{OPTION_NODE} {
    return token::OPTION_TYPE_NODE;
}
{OPTION_LIST} {
    return token::OPTION_TYPE_LIST;
}
}

<VALUES>{
%{
/* values */
%}
{VALUE} {
    yylval->f = parseValue(yytext); 
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case COMPONENT_RESISTOR:
            case COMPONENT_CAPACITOR:
            case COMPONENT_INDUCTOR:
            case COMPONENT_VCVS:
            case COMPONENT_CCCS:
            case COMPONENT_VCCS:
            case COMPONENT_CCVS:
                BEGIN(KEYWORD_PARAM); current_token_needed = 0; break;
            case COMPONENT_VOLTAGE_SOURCE:
            case COMPONENT_CURRENT_SOURCE:
                BEGIN(ANALYSIS_TYPE); optional_token = true; break;
            case ANALYSIS_DC:
                BEGIN(DC_SOURCE); optional_token = true; break;
            case ANALYSIS_AC:
                current_token_needed = 0; break;
            case ANALYSIS_TRAN:
                optional_token = true; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing VALUES\n");
        }
    }
    return token::VALUE;
}
{VALUE_VOLTAGE} {
    char* temp = strndup(yytext, yyleng - 1);
    yylval->f = parseValue(temp);
    free(temp);
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case COMPONENT_VOLTAGE_SOURCE:
                BEGIN(ANALYSIS_TYPE); optional_token = true; break;
            case ANALYSIS_DC:
                BEGIN(DC_SOURCE); optional_token = true; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing VALUES\n");
        }
    }
    return token::VALUE_VOLTAGE;
}
{VALUE_CURRENT} {
    char* temp = strndup(yytext, yyleng - 1);
    yylval->f = parseValue(temp);
    free(temp);
    return token::VALUE_CURRENT;
}
{VALUE_RESISTANCE} {
    char* temp = strndup(yytext, yyleng - 3);
    yylval->f = parseValue(temp);
    free(temp);
    return token::VALUE_RESISTANCE;
}
{VALUE_CAPACITANCE} {
    char* temp = strndup(yytext, yyleng - 1);
    yylval->f = parseValue(temp);
    free(temp);
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case COMPONENT_CAPACITOR:
                BEGIN(KEYWORD_PARAM); optional_token = true; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing VALUES\n");
        }
    }
    return token::VALUE_CAPACITANCE;
}
{VALUE_INDUCTANCE} {
    char* temp = strndup(yytext, yyleng - 1);
    yylval->f = parseValue(temp);
    free(temp);
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case COMPONENT_INDUCTOR:
                BEGIN(KEYWORD_PARAM); optional_token = true; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing VALUES\n");
        }
    }
    return token::VALUE_INDUCTANCE;
}
{VALUE_TIME} {
    char* temp = strndup(yytext, yyleng - 1);
    yylval->f = parseValue(temp);
    free(temp);
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case ANALYSIS_TRAN:
                optional_token = true; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing VALUES\n");
        }
    }
    return token::VALUE_TIME;
}
{VALUE_LENGTH} {
    char* temp = strndup(yytext, yyleng - 1);
    yylval->f = parseValue(temp);
    free(temp);
    return token::VALUE_LENGTH;
}
{VALUE_FREQUENCY} {
    char* temp = strndup(yytext, yyleng - 2);
    yylval->f = parseValue(temp);
    free(temp);
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case ANALYSIS_AC:
                current_token_needed = 0; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing VALUES\n");
        }
    }
    return token::VALUE_FREQUENCY;
}
{VALUE_ANGLE} {
    char* temp = strndup(yytext, yyleng - 3);
    yylval->f = parseValue(temp);
    free(temp);
    if ((--current_token_needed) == 0) {
        switch(current_line_type) {
            case ANALYSIS_AC:
                current_token_needed = 0; break;
            default:
                printf("Current line type is %d\n", current_line_type);
                printf("ERROR_UNKOWN_LINE_TYPE when parsing VALUES\n");
        }
    }
    return token::VALUE_ANGLE;
}
}

<*>{
%{
/* others */
%}
{EOL} {
    BEGIN(INITIAL); 
    current_token_needed = 0; 
    optional_token = false; 
    return token::EOL;
}
{DELIMITER} {
    yylloc->step();
}
{COMMENT} {
    BEGIN(INITIAL); 
    yylloc->step();
    current_token_needed = 0; 
    optional_token = false; 
}
{INLINE_COMMENT} {
    yylloc->step();
}
}

<FILEEND><<EOF>>     {yyterminate();}




%%

char *copyStr(const char *str)
{
    char *newStr;
    newStr = (char *)malloc((strlen(str) + 1) * sizeof(char));
    strcpy(newStr, str);
    newStr[strlen(str)] = '\0';
    return newStr;
}

char *copyStrTolower(const char *str)
{
    char *newStr;
    newStr = (char *)malloc((strlen(str) + 1) * sizeof(char));
    strcpy(newStr, str);
    std::transform(newStr, newStr + strlen(str), newStr, ::tolower);
    newStr[strlen(str)] = '\0';
    return newStr;
}

char *copyStrToupper(const char *str)
{
    char *newStr;
    newStr = (char *)malloc((strlen(str) + 1) * sizeof(char));
    strcpy(newStr, str);
    std::transform(newStr, newStr + strlen(str), newStr, ::toupper);
    newStr[strlen(str)] = '\0';
    return newStr;
}

double parseValue(const char *str)
{
    double value = atof(str);
    int    len = strlen(str);

    char u = str[len - 1];
    if (u == 'F' || u == 'f')
        value *= 1e-15;
    else if (u == 'P' || u == 'p')
        value *= 1e-12;
    else if (u == 'N' || u == 'n')
        value *= 1e-9;
    else if (u == 'U' || u == 'u')
        value *= 1e-6;
    else if (u == 'M' || u == 'm')
        value *= 1e-3;
    else if (u == 'K' || u == 'k')
        value *= 1e3;
    else if (u == 'G' || u == 'g') {
        if (str[len - 2] == 'E' || str[len - 2] == 'e')
            value *= 1e6;
        else
            value *= 1e9;
    } else if (u == 'T' || u == 't')
        value *= 1e12;

    return value; 
}
