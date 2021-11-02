%{

#include <stdlib.h>
#include <string.h>

#if defined(LAB1)

#include "scanner.hh"

#elif defined(LAB2)

#include "scanner.hh"
#include "symtab.hh"

#else

#include "ast.hh"
#include "parser.hh"

#endif

int column = 0;

extern YYLTYPE yylloc; // Used for position information, see below.

// We don't have tokens over multiple lines
#define SPAN() \
{                               \
    yylloc.first_line = yylineno; \
    yylloc.first_column = column; \
    column += yyleng;             \
    yylloc.last_line = yylineno;  \
    yylloc.last_column = column;  \
}                               \

%}

%option yylineno
%option 8bit
%option noyywrap
%option caseless
%option outfile = "scanner.cc"

/* If you want to include any flex declarations, this is where to do it. */

/*
 TODO
  - seperators
  - strings
  - parse reals and ints to actual numbers 
 */

%x c_comment disel_comment
DIGIT [0-9]
/* {DIGIT}+"."{DIGIT}*
 ID [a-z][a-z0-9]* */

/* Position information is entered into the yylloc variable, so we will be
   able to generate good error messages later in later phases. */
/* Your code should be entered below the %%. Expressions to handle the
   following: Diesel comments, Diesel string constants, Diesel
   identifiers, integers, reals, and whitespace. */

%%

\n       { column = 0; }
[ \t\n]+ {}

{DIGIT}+ {
    SPAN();
    yylval.ival = stoi(yytext);
    return T_INTNUM;
}


({DIGIT}+\.{DIGIT}*|\.{DIGIT}+)([eE][-+]?{DIGIT}+)?  {
    SPAN();
    yylval.rval = stof(yytext);
    return T_REALNUM;
}

'([^']*('')?)*' {
    SPAN();
    char* fixed_string = sym_tab->fix_string(yytext);
    yylval.str = sym_tab->pool_install(fixed_string);
    return T_STRINGCONST;
}


\.    { SPAN(); return T_DOT; }
;     { SPAN(); return T_SEMICOLON; }
=     { SPAN(); return T_EQ; }
\:    { SPAN(); return T_COLON; }
\(    { SPAN(); return T_LEFTPAR; }
\)    { SPAN(); return T_RIGHTPAR; }
\[    { SPAN(); return T_LEFTBRACKET; }
\]    { SPAN(); return T_RIGHTBRACKET; }
,     { SPAN(); return T_COMMA; }
\<    { SPAN(); return T_LESSTHAN; }
\>    { SPAN(); return T_GREATERTHAN; }
\+    { SPAN(); return T_ADD; }
\-    { SPAN(); return T_SUB; }
\*    { SPAN(); return T_MUL; }
\/    { SPAN(); return T_RDIV; }
":="  { SPAN(); return T_ASSIGN; }
"<>"  { SPAN(); return T_NOTEQ; }


of          { SPAN(); return T_OF; }
if          { SPAN(); return T_IF; }
do          { SPAN(); return T_DO; }
or          { SPAN(); return T_OR; }
var         { SPAN(); return T_VAR; }
end         { SPAN(); return T_END; }
and         { SPAN(); return T_AND; }
div         { SPAN(); return T_IDIV; }
mod         { SPAN(); return T_MOD; }
not         { SPAN(); return T_NOT; }
then        { SPAN(); return T_THEN; }
else        { SPAN(); return T_ELSE; }
const       { SPAN(); return T_CONST; }
array       { SPAN(); return T_ARRAY; }
begin       { SPAN(); return T_BEGIN; }
while       { SPAN(); return T_WHILE; }
elsif       { SPAN(); return T_ELSIF; }
return      { SPAN(); return T_RETURN; }
program     { SPAN(); return T_PROGRAM; }
function    { SPAN(); return T_FUNCTION; }
procedure   { SPAN(); return T_PROCEDURE; }

[A-Z_]+ {
    SPAN();
    auto captialized = sym_tab->capitalize(yytext);
    yylval.pool_p = sym_tab->pool_install(captialized);
    return T_IDENT;
}

\{ {
    column += yyleng;
    BEGIN(disel_comment);
}
<disel_comment>
{
    "\}" {
        column += 2;
        BEGIN(INITIAL);
    }
    "/{" {
        column += 2;
        yyerror("Suspicious comment");
    }
    [^\n] column++; /* Skip stuff in comments */
    \n column = 0;
    <<EOF>> {
        yyerror("Unterminated comment");
        yyterminate();
    }
}

\/\/.*$                  column = 0; /* Skip single-line comment */
"/\*"                    {
                            column += yyleng;
                            BEGIN(c_comment);
                         }

<c_comment>
{
    "\*/"                {
                            column += 2;
                            BEGIN(INITIAL);
                         }
    "/\*"                {
                            column += 2;
                            yyerror("Suspicious comment");
                         }
    [^\n]                column++; /* Skip stuff in comments */
    \n                   column = 0;
    <<EOF>>              {
                            yyerror("Unterminated comment");
                            yyterminate();
                         }
}

<<EOF>>                  yyterminate();
.                        yyerror("Illegal character");
