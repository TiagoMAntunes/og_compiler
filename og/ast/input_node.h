#ifndef __OG_AST_input_node_H__
#define __OG_AST_input_node_H__

#include <cdk/ast/expression_node.h>

namespace og {

  /**
   * Class for describing read nodes.
   */
  class input_node: public cdk::expression_node {

  public:
    inline input_node(int lineno) :
        cdk::expression_node(lineno) {
          cdk::typed_node::type(cdk::make_primitive_type(0, cdk::TYPE_UNSPEC));
    }

  public:
    void accept(basic_ast_visitor *sp, int level) {
      sp->do_input_node(this, level);
    }

  };

} // og

#endif
