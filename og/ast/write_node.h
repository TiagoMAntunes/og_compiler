#ifndef __OG_AST_write_node_H__
#define __OG_AST_write_node_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/basic_node.h>

namespace og {

  /**
   * Class for describing print nodes.
   */
  class write_node: public cdk::basic_node {
    cdk::expression_node *_argument;
    bool _newline;

  public:
    inline write_node(int lineno, cdk::expression_node *argument, bool newline = false) :
        cdk::basic_node(lineno), _argument(argument), _newline(newline) {
    }

  public:
    inline cdk::expression_node *argument() {
      return _argument;
    }

    inline bool newline() {
      return _newline;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_write_node(this, level);
    }

  };

} // og

#endif
