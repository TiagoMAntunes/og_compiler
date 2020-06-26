%{
//-- don't change *any* of these: if you do, you'll break the compiler.
#include <cdk/compiler.h>
#include "ast/all.h"
#define LINE               compiler->scanner()->lineno()
#define yylex()            compiler->scanner()->scan()
#define yyerror(s)         compiler->scanner()->error(s)
#define YYPARSE_PARAM_TYPE std::shared_ptr<cdk::compiler>
#define YYPARSE_PARAM      compiler
//-- don't change *any* of these --- END!

#include <cdk/types/types.h>
#include <algorithm>
#include <vector>
%}

%union {
  int                   i;	/* integer value */
  double                d;
  std::string          *s;	/* symbol name or string literal */
  cdk::integer_node    *integer;
  cdk::double_node     *real;
  cdk::basic_node      *node;	/* node pointer */
  cdk::sequence_node   *sequence;
  cdk::expression_node *expression; /* expression nodes */
  cdk::lvalue_node     *lvalue;
  og::variable_declaration_node   *variable_decl;
  og::function_declaration_node   *function_decl;
  og::function_definition_node    *function_def;
  og::function_call_node          *call;
  og::block_node      *block;
  og::index_node      *index;
  og::nullptr_node    *null;
  og::tuple_node *tuple;
  std::vector<std::string*> *vector;
  cdk::reference_type *type;
};

%token <i> tINTEGER
%token <d> tREALNUM
%token <s> tIDENTIFIER tSTRING

%token tFOR tDO tIF tELSE tELIF tTHEN
%token tWRITE tWRITELN tINPUT
%token tAUTO tINT tREAL tSTRING tPTR
%token tPUBLIC
%token tREQUIRE
%token tSIZEOF
%token tNULLPTR tPROCEDURE tBREAK tCONTINUE tRETURN

%nonassoc tIFX tFOR
%nonassoc tTHEN tDO
%nonassoc tELIF tELSE

%right '='  
%nonassoc ',' 
%right '~'
%left tOR
%left tAND
%left tGE tLE tEQ tNE
%left '>' '<'
%left '+' '-' 
%left '*' '/' '%' 
%nonassoc '[' '@' 
%nonassoc tUNARY 

%type <integer> integer
%type <real> real
%type <s> string
%type <node> declaration, instruction, conditional, else
%type <sequence> file, exprs, args, var_decls, variables, instructions, program, for_exprs
%type <expression> expr, literal
%type <lvalue> lval
%type <variable_decl> var_decl
%type <function_decl> function_decl
%type <function_def> function_def
%type <call> function_call
%type <block> block
%type <null> nullptr
%type <tuple> tuple
%type <vector> identifiers
%type <type> ptr_type basic_type_ptr

%{
//-- The rules below will be included in yyparse, the main parsing function.
%}
%%

// SEQUENCES

program : file { compiler->ast($1); }
        ;
file : declaration	   { $$ = new cdk::sequence_node(LINE, $1); }
     | file declaration    { $$ = new cdk::sequence_node(LINE, $2, $1); }
     ;

declaration : var_decl ';'     { $$ = $1; }
            | function_decl    { $$ = $1; }
            | function_def     { $$ = $1; }
            ;

args : var_decl                   { $$ = new cdk::sequence_node(LINE, $1); }
     | args ',' var_decl          { $$ = new cdk::sequence_node(LINE, $3, $1); }
     | /* Empty arguments list */ { $$ = new cdk::sequence_node(LINE); }
     ;

var_decls : var_decl ';'             { $$ = new cdk::sequence_node(LINE, $1); }
          | var_decls var_decl ';'   { $$ = new cdk::sequence_node(LINE, $2, $1); }
          ;

variables : var_decl                  { $$ = new cdk::sequence_node(LINE, $1); }
          | variables ',' var_decl    { $$ = new cdk::sequence_node(LINE, $3, $1); }
          ;

instructions : instruction                 { $$ = new cdk::sequence_node(LINE, $1); }
             | instructions  instruction   { $$ = new cdk::sequence_node(LINE, $2, $1); }
             ;
identifiers  : tIDENTIFIER                  { std::vector<std::string*>*a_ptr = new std::vector<std::string*>(); a_ptr->push_back($1); $$ = a_ptr; }
             | identifiers ',' tIDENTIFIER  { $1->push_back($3); $$ = $1; }
             ;

tuple : expr            { $$ = new og::tuple_node(LINE, new cdk::sequence_node(LINE, $1)); }    
      | tuple ',' expr  { $$ = new og::tuple_node(LINE, new cdk::sequence_node(LINE, $3, $1->values())); }
      ; 

exprs : expr                  { $$ = new cdk::sequence_node(LINE, $1); }
      | exprs ',' expr        { $$ = new cdk::sequence_node(LINE, $3, $1); }
      |                       { $$ = new cdk::sequence_node(LINE); }
      ;  

for_exprs : expr                  { $$ = new cdk::sequence_node(LINE, new og::evaluation_node(LINE, $1)); }
          | exprs ',' expr        { $$ = new cdk::sequence_node(LINE, new og::evaluation_node(LINE, $3), $1); }
          |                       { $$ = new cdk::sequence_node(LINE); }
          ;  
      
             

// VARIABLES

var_decl  :           tINT      tIDENTIFIER                  { $$ = new og::variable_declaration_node(LINE, 0, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_INT), $2); } 
          |           tINT      tIDENTIFIER '='     expr     { $$ = new og::variable_declaration_node(LINE, 0, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_INT), $2, $4); }
          |           tREAL     tIDENTIFIER                  { $$ = new og::variable_declaration_node(LINE, 0, std::make_shared<cdk::primitive_type>(8, cdk::TYPE_DOUBLE), $2); } 
          |           tREAL     tIDENTIFIER '='     expr     { $$ = new og::variable_declaration_node(LINE, 0, std::make_shared<cdk::primitive_type>(8, cdk::TYPE_DOUBLE), $2, $4); } 
          |           tSTRING   tIDENTIFIER                  { $$ = new og::variable_declaration_node(LINE, 0, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_STRING), $2); } 
          |           tSTRING   tIDENTIFIER '='     expr     { $$ = new og::variable_declaration_node(LINE, 0, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_STRING), $2, $4); }
          |           ptr_type  tIDENTIFIER                  { $$ = new og::variable_declaration_node(LINE, 0, std::make_shared<cdk::reference_type>($1->size(), $1->referenced()), $2); }
          |           ptr_type  tIDENTIFIER '='     expr     { $$ = new og::variable_declaration_node(LINE, 0, std::make_shared<cdk::reference_type>($1->size(), $1->referenced()), $2, $4); }
          |           tAUTO     identifiers '='     exprs    { $$ = new og::variable_declaration_node(LINE, 0, *$2, $4); }                                                                                         
          | tPUBLIC   tINT      tIDENTIFIER                  { $$ = new og::variable_declaration_node(LINE, 1, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_INT), $3); }
          | tPUBLIC   tINT      tIDENTIFIER '='     expr     { $$ = new og::variable_declaration_node(LINE, 1, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_INT), $3, $5); } 
          | tPUBLIC   tREAL     tIDENTIFIER                  { $$ = new og::variable_declaration_node(LINE, 1, std::make_shared<cdk::primitive_type>(8, cdk::TYPE_DOUBLE), $3); } 
          | tPUBLIC   tREAL     tIDENTIFIER '='     expr     { $$ = new og::variable_declaration_node(LINE, 1, std::make_shared<cdk::primitive_type>(8, cdk::TYPE_DOUBLE), $3, $5); }
          | tPUBLIC   tSTRING   tIDENTIFIER                  { $$ = new og::variable_declaration_node(LINE, 1, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_STRING), $3); } 
          | tPUBLIC   tSTRING   tIDENTIFIER '='     expr     { $$ = new og::variable_declaration_node(LINE, 1, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_STRING), $3, $5); }
          | tPUBLIC   ptr_type  tIDENTIFIER                  { $$ = new og::variable_declaration_node(LINE, 1, std::make_shared<cdk::reference_type>($2->size(), $2->referenced()), $3); }
          | tPUBLIC   ptr_type  tIDENTIFIER '='     expr     { $$ = new og::variable_declaration_node(LINE, 1, std::make_shared<cdk::reference_type>($2->size(), $2->referenced()), $3, $5); }
          | tPUBLIC   tAUTO     identifiers '='     exprs    { $$ = new og::variable_declaration_node(LINE, 1, *$3, $5); }                                                                                        
          | tREQUIRE  tREAL     tIDENTIFIER                  { $$ = new og::variable_declaration_node(LINE, 2, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_INT), $3); }
          | tREQUIRE  tINT      tIDENTIFIER                  { $$ = new og::variable_declaration_node(LINE, 2, std::make_shared<cdk::primitive_type>(8, cdk::TYPE_DOUBLE), $3); }
          | tREQUIRE  tSTRING   tIDENTIFIER                  { $$ = new og::variable_declaration_node(LINE, 2, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_STRING), $3); }
          | tREQUIRE  ptr_type  tIDENTIFIER                  { $$ = new og::variable_declaration_node(LINE, 2, std::make_shared<cdk::reference_type>($2->size(), $2->referenced()), $3); }
          ;



// FUNCTIONS & PROCEDURES

function_def  :         tINT       tIDENTIFIER '(' args ')' '{' block '}'   { $$ = new og::function_definition_node(LINE, 0, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_INT), $2, $4, $7); }
              |         tREAL      tIDENTIFIER '(' args ')' '{' block '}'   { $$ = new og::function_definition_node(LINE, 0, std::make_shared<cdk::primitive_type>(8, cdk::TYPE_DOUBLE), $2, $4, $7); }
              |         tSTRING    tIDENTIFIER '(' args ')' '{' block '}'   { $$ = new og::function_definition_node(LINE, 0, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_STRING), $2, $4, $7); }
              |         ptr_type   tIDENTIFIER '(' args ')' '{' block '}'   { $$ = new og::function_definition_node(LINE, 0, std::make_shared<cdk::reference_type>($1->size(), $1->referenced()), $2, $4, $7); }
              |         tAUTO      tIDENTIFIER '(' args ')' '{' block '}'   { $$ = new og::function_definition_node(LINE, 0, std::make_shared<cdk::primitive_type>(), $2, $4, $7); }
              | tPUBLIC tINT       tIDENTIFIER '(' args ')' '{' block '}'   { $$ = new og::function_definition_node(LINE, 1, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_INT), $3, $5, $8); }
              | tPUBLIC tREAL      tIDENTIFIER '(' args ')' '{' block '}'   { $$ = new og::function_definition_node(LINE, 1, std::make_shared<cdk::primitive_type>(8, cdk::TYPE_DOUBLE), $3, $5, $8); }
              | tPUBLIC tSTRING    tIDENTIFIER '(' args ')' '{' block '}'   { $$ = new og::function_definition_node(LINE, 1, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_STRING), $3, $5, $8); }
              | tPUBLIC ptr_type   tIDENTIFIER '(' args ')' '{' block '}'   { $$ = new og::function_definition_node(LINE, 1, std::make_shared<cdk::reference_type>($2->size(), $2->referenced()), $3, $5, $8); }
              | tPUBLIC tAUTO      tIDENTIFIER '(' args ')' '{' block '}'   { $$ = new og::function_definition_node(LINE, 1, std::make_shared<cdk::primitive_type>(), $3, $5, $8); }
              |         tPROCEDURE tIDENTIFIER '(' args ')' '{' block '}'   { $$ = new og::function_definition_node(LINE, 0, $2, $4, $7); }
              | tPUBLIC tPROCEDURE tIDENTIFIER '(' args ')' '{' block '}'   { $$ = new og::function_definition_node(LINE, 1, $3, $5, $8); }
              ;

function_decl :          tINT        tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 0, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_INT), $2, $4); }
              |          tREAL       tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 0, std::make_shared<cdk::primitive_type>(8, cdk::TYPE_DOUBLE), $2, $4); }
              |          tSTRING     tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 0, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_STRING), $2, $4); }
              |          ptr_type    tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 0, std::make_shared<cdk::reference_type>($1->size(), $1->referenced()), $2, $4); }
              |          tAUTO       tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 0, std::make_shared<cdk::primitive_type>(), $2, $4); }
              | tPUBLIC  tINT        tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 1, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_INT), $3, $5); }
              | tPUBLIC  tREAL       tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 1, std::make_shared<cdk::primitive_type>(8, cdk::TYPE_DOUBLE), $3, $5); }
              | tPUBLIC  tSTRING     tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 1, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_STRING), $3, $5); }
              | tPUBLIC  ptr_type    tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 1, std::make_shared<cdk::reference_type>($2->size(), $2->referenced()), $3, $5); }
              | tPUBLIC  tAUTO       tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 1, std::make_shared<cdk::primitive_type>(), $3, $5); }
              |          tPROCEDURE  tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 0, $2, $4); }
              | tPUBLIC  tPROCEDURE  tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 1, $3, $5); }
              | tREQUIRE tINT        tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 2, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_INT), $3, $5); }
              | tREQUIRE tREAL       tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 2, std::make_shared<cdk::primitive_type>(8, cdk::TYPE_DOUBLE), $3, $5); }
              | tREQUIRE tSTRING     tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 2, std::make_shared<cdk::primitive_type>(4, cdk::TYPE_STRING), $3, $5); }
              | tREQUIRE ptr_type    tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 2, std::make_shared<cdk::reference_type>($2->size(), $2->referenced()), $3, $5); }
              | tREQUIRE tAUTO       tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 2, std::make_shared<cdk::primitive_type>(), $3, $5); }
              | tREQUIRE tPROCEDURE  tIDENTIFIER '(' args ')'  { $$ = new og::function_declaration_node(LINE, 2, $3, $5); }
              ;

function_call : tIDENTIFIER '(' exprs ')'     { $$ = new og::function_call_node(LINE, $1, $3); }
              ;

// BLOCK

block :  var_decls instructions  { $$ = new og::block_node(LINE, $1, $2); }
      |  var_decls               { $$ = new og::block_node(LINE, $1, NULL); }
      |  instructions            { $$ = new og::block_node(LINE, NULL, $1); }
      | /* empty */              { $$ = new og::block_node(LINE, NULL, NULL); }
      ;


instruction :          expr ';'                                    { $$ = new og::evaluation_node(LINE, $1); }
            | tWRITE   tuple ';'                                  { $$ = new og::write_node(LINE, $2); } 
            | tWRITELN tuple ';'                                   { $$ = new og::write_node(LINE, $2, true); }
            | tBREAK                                               { $$ = new og::break_node(LINE); }
            | tCONTINUE                                            { $$ = new og::continue_node(LINE); }
            | tRETURN  tuple ';'                                   { $$ = new og::return_node(LINE, $2); }   
            | conditional                                          { $$ = $1; }
            | tFOR for_exprs ';' exprs ';' for_exprs tDO instruction   { $$ = new og::for_node(LINE, $2, $4, $6, $8); }
            | tFOR variables ';' exprs ';' for_exprs tDO instruction   { $$ = new og::for_node(LINE, $2, $4, $6, $8); }
            | '{' block '}'                                        { $$ = $2; }
            ;

conditional : tIF expr tTHEN instruction             { $$ = new og::if_node(LINE, $2, $4); }
            | tIF expr tTHEN instruction else        { $$ = new og::if_else_node(LINE, $2, $4, $5); } 
            ;

else : tELIF expr tTHEN instruction             { $$ = new og::if_node(LINE, $2, $4); }   
     | tELIF expr tTHEN instruction else        { $$ = new og::if_else_node(LINE, $2, $4, $5); }
     | tELSE instruction                        { $$ = new cdk::sequence_node(LINE, $2); } 
     ;
        

// LEFT VALUE

lval : tIDENTIFIER           { $$ = new cdk::variable_node(LINE, $1); } 
     | expr '[' expr ']'     { $$ = new og::index_node(LINE, $1, $3); }
     | expr '@' integer      { $$ = new og::tuple_index_node(LINE, $1, $3); }
     ;

// EXPRESSIONS

expr : literal                  { $$ = $1; }
     | function_call            { $$ = $1; }
     // UNARY EXPRESSIONS
     | '-' expr                 { $$ = new cdk::neg_node(LINE, $2); }
     | '+' expr                 { $$ = new og::identity_node(LINE, $2); }
     | '~' expr                 { $$ = new cdk::not_node(LINE, $2); }
     | lval '?'                 { $$ = new og::address_node(LINE, $1); }
     // ARITHMETIC EXPRESSIONS
     | expr '+' expr	        { $$ = new cdk::add_node(LINE, $1, $3); }      
     | expr '-' expr	        { $$ = new cdk::sub_node(LINE, $1, $3); }
     | expr '*' expr	        { $$ = new cdk::mul_node(LINE, $1, $3); }
     | expr '/' expr	        { $$ = new cdk::div_node(LINE, $1, $3); }
     | expr '%' expr	        { $$ = new cdk::mod_node(LINE, $1, $3); }
     // LOGICAL EXPRESSIONS
     | expr '<' expr	        { $$ = new cdk::lt_node(LINE, $1, $3); }
     | expr tLE expr            { $$ = new cdk::le_node(LINE, $1, $3); }
     | expr tEQ expr	        { $$ = new cdk::eq_node(LINE, $1, $3); }
     | expr '>' expr	        { $$ = new cdk::gt_node(LINE, $1, $3); }
     | expr tGE expr	        { $$ = new cdk::ge_node(LINE, $1, $3); }
     | expr tNE expr	        { $$ = new cdk::ne_node(LINE, $1, $3); }
     | expr tAND expr           { $$ = new cdk::and_node(LINE, $1, $3); }
     | expr tOR  expr           { $$ = new cdk::or_node(LINE, $1, $3); }
     // OTHER EXPRESSIONS
     | '(' expr ')'             { $$ = $2; }
     // LEFT VALUES
     | lval                     { $$ = new cdk::rvalue_node(LINE, $1); }
     | lval '=' expr            { $$ = new cdk::assignment_node(LINE, $1, $3); }
     // OTHER EXPRESSIONS
     | tINPUT                   { $$ = new og::input_node(LINE); }
     | tSIZEOF '(' tuple ')'    { $$ = new og::sizeof_node(LINE, $3); }     // can be a simple tuple AKA 1 expression
     | '[' expr ']'             { $$ = new og::stack_alloc_node(LINE, $2); }
     ;


// POINTERS

ptr_type : basic_type_ptr                               { $$ = $1; }
         | tPTR '<' tAUTO '>'                           { $$ = new cdk::reference_type(4, cdk::make_primitive_type(0, cdk::TYPE_UNSPEC)); }
         ;

basic_type_ptr : tPTR '<' tINT '>'                      { $$ = new cdk::reference_type(4, cdk::make_primitive_type(4, cdk::TYPE_INT)); }
               | tPTR '<' tREAL '>'                     { $$ = new cdk::reference_type(4, cdk::make_primitive_type(8, cdk::TYPE_DOUBLE)); }
               | tPTR '<' tSTRING '>'                   { $$ = new cdk::reference_type(4, cdk::make_primitive_type(4, cdk::TYPE_STRING)); }
               | tPTR '<' basic_type_ptr '>'            { $$ = new cdk::reference_type(4, cdk::make_reference_type($3->size(),$3->referenced())); delete $3; }
               ;

// LITERALS

literal : integer   { $$ = $1; }
        | real      { $$ = $1; }
        | string    { $$ = new cdk::string_node(LINE, $1); }
        | nullptr   { $$ = $1; }
        ;

integer : tINTEGER   { $$ = new cdk::integer_node(LINE, $1); }         
        ;

real    : tREALNUM   { $$ = new cdk::double_node(LINE, $1); }        
        ;

string  : tSTRING             { $$ = $1; }
        | string tSTRING      { $$ = new std::string(*$1 + *$2); delete $1; delete $2; }
        ;

nullptr : tNULLPTR  { $$ = new og::nullptr_node(LINE); }
        ;


%%
