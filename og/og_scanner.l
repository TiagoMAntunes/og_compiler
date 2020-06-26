%option c++ prefix="og_scanner_" outfile="og_scanner.cpp"
%option stack noyywrap yylineno 8bit
%option debug
%{ 
// make relevant includes before including the parser's tab file
#include <string>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "og_parser.tab.h"

// don't change this
#define yyerror LexerError
%}
%x X_STRING X_COMMENT X_STRING_IGNORE
%%
    
"//".*                 ; /* ignore one-line comments */

<INITIAL,X_COMMENT>"/*" yy_push_state(X_COMMENT);
<X_COMMENT>"*/"         yy_pop_state();
<X_COMMENT>.|\n         ;


">="                   return tGE; 
"<="                   return tLE; 
"=="                   return tEQ; 
"!="                   return tNE; 

"&&"                   return tAND;
"||"                   return tOR;

"for"                  return tFOR; 
"do"                   return tDO;
"if"                   return tIF; 
"else"                 return tELSE; 
"elif"                 return tELIF;
"then"                 return tTHEN;

"write"                return tWRITE;
"writeln"              return tWRITELN;
"input"                return tINPUT; 

"auto"                 return tAUTO;
"int"                  return tINT;
"real"                 return tREAL;
"string"               return tSTRING;
"ptr"                  return tPTR;

"public"               return tPUBLIC;

"require"              return tREQUIRE;
"sizeof"               return tSIZEOF;

"nullptr"              return tNULLPTR;
"procedure"            return tPROCEDURE;
"break"                return tBREAK;
"continue"             return tCONTINUE;
"return"               return tRETURN;

[A-Za-z][A-Za-z0-9_]*  yylval.s = new std::string(yytext); return tIDENTIFIER;

\"                     yy_push_state(X_STRING); yylval.s = new std::string("");
<X_STRING>\"           yy_pop_state(); return tSTRING;
 
 /* \\ or \" */
<X_STRING>\\[\\\"]              *yylval.s += yytext + 1;
<X_STRING>\\n                   *yylval.s += "\n";
<X_STRING>\\r                   *yylval.s += "\r";
<X_STRING>\\t                   *yylval.s += "\t";

<X_STRING>\\0                   yy_push_state(X_STRING_IGNORE);
 /* Ignore the remaining of the string */
<X_STRING_IGNORE>\"             yy_pop_state(); yy_pop_state(); return tSTRING;
<X_STRING_IGNORE>.              ; // ignore

<X_STRING>\\[0-9a-fA-F]{1,2}    { std::string c(1,strtol(yytext+1, nullptr, 16)); *yylval.s += c; }
<X_STRING>.                     *yylval.s += yytext;


[0-9]+                 yylval.i = strtol(yytext, nullptr, 10); if (errno == ERANGE) yyerror("Number of base 10 too big (overflowed)."); else return tINTEGER;
0x[0-9a-fA-F]+         yylval.i = strtol(yytext, nullptr, 16); if (errno == ERANGE) yyerror("Number of base 16 too small (underflowed)."); else return tINTEGER;

 /*   .x              x.        x */
([0-9]*\.[0-9]+|[0-9]+\.[0-9]*|[0-9]+)([Ee]([-+])?[0-9]+)?   yylval.d = strtod(yytext, NULL); if (errno == ERANGE) yyerror("Floating point number too small to be represented."); else return tREALNUM; 


[-()<>=+*/%;{}?.,@\[\]]       return *yytext;

[ \t\n\r]+               ; /* ignore whitespace */

.                      yyerror("Unknown character");

%%
