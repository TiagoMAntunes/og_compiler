#ifndef __OG_AST_return_node_H__
#define __OG_AST_return_node_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/basic_node.h>

namespace og {

  /**
   * Class for describing print nodes.
   */
  class return_node: public cdk::basic_node {
    cdk::expression_node *_argument;

  public:
    inline return_node(int lineno, cdk::expression_node *argument) :
        cdk::basic_node(lineno), _argument(argument) {
    }

  public:
    inline cdk::expression_node *argument() {
      return _argument;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_return_node(this, level);
    }

  };

} // og

#endif
