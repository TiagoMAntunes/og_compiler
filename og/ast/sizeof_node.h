#ifndef __OG_AST_SIZEOF_NODE_H__
#define __OG_AST_SIZEOF_NODE_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/unary_operation_node.h>

namespace og {

  /**
   * Class for describing evaluation nodes.
   */
  class sizeof_node: public cdk::unary_operation_node {

  public:
    inline sizeof_node(int lineno, cdk::expression_node *argument) :
        cdk::unary_operation_node(lineno, argument) {
    }

  public:

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_sizeof_node(this, level);
    }

  };

} // og

#endif
