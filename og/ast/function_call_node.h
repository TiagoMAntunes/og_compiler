#ifndef __OG_AST_FUNCTION_CALL_H__
#define __OG_AST_FUNCTION_CALL_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>
#include <cdk/types/typename_type.h>
#include <cdk/types/basic_type.h>

namespace og {

  class function_call_node: public cdk::expression_node {
    std::string *_name;
    cdk::sequence_node *_arguments;

  public:
    
    /*
      No arguments given
     */
    function_call_node(int lineno, std::string *name) :
      cdk::expression_node(lineno), _name(name), _arguments(new cdk::sequence_node(lineno)) {
    }   

    /*
      Arguments given
     */
    function_call_node(int lineno, std::string *name, cdk::sequence_node *args) :
      cdk::expression_node(lineno), _name(name), _arguments(args) {
    }

  public:
    inline std::string *name() {
      return _name;
    } 

    inline cdk::sequence_node *arguments() {
      return _arguments;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_call_node(this, level);
    }
  };

} // og

#endif
