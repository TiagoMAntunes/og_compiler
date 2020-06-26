#ifndef __OG_AST_VARIABLE_DECLARATION_H__
#define __OG_AST_VARIABLE_DECLARATION_H__

#include <cdk/ast/typed_node.h>
#include <cdk/types/basic_type.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>
#include <vector>

namespace og {

  class variable_declaration_node: public cdk::typed_node {
    int _access;
    std::vector<std::string*> _names;
    cdk::expression_node *_expression; 
    std::shared_ptr<cdk::basic_type> _declType;
    int _offset; 


  public:
    // type name [= expression];
    variable_declaration_node(int lineno, int access, std::shared_ptr<cdk::basic_type> type, 
                              std::string *name, cdk::expression_node *expr = NULL) :
      cdk::typed_node(lineno), _access(access), _expression(expr) {
        _names.push_back(name);
   //     cdk::typed_node::type(type);
        _declType = type;
      }

    // auto names = tuple;
    variable_declaration_node(int lineno, int access, std::vector<std::string*> names, cdk::sequence_node *exprs) :
    cdk::typed_node(lineno), _access(access), _names(names) {
      _declType = std::make_shared<cdk::structured_type>(std::vector<std::shared_ptr<cdk::basic_type>>());
      if (exprs->size() == 1) 
        _expression = dynamic_cast<cdk::expression_node*>(exprs->node(0));
      
      else 
        _expression = new og::tuple_node(lineno, exprs);
  
    }

  public:
    inline int access() {
      return _access;
    } 

    inline std::vector<std::string*> names() {
      return _names;
    } 

    inline cdk::expression_node* expression() {
      return _expression;
    }

    inline bool initialized() {
      return _expression != NULL;
    }

    inline std::shared_ptr<cdk::basic_type> declType() {
      return _declType;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_variable_declaration_node(this, level);
    }
  };

} // og

#endif
