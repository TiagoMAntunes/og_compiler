#ifndef __OG_AST_IDENTITY_H__
#define __OG_AST_IDENTITY_H__

#include <cdk/ast/unary_operation_node.h>
#include <cdk/ast/expression_node.h>
namespace og {

  /**
   * Class for describing the negation operator
   */
  class identity_node: public cdk::unary_operation_node {
  public:
    identity_node(int lineno, cdk::expression_node *arg) :
        unary_operation_node(lineno, arg) {
    }


    void accept(basic_ast_visitor *av, int level) {
      av->do_identity_node(this, level);
    }

  };

} // og

#endif