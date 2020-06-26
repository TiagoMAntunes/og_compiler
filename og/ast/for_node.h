#ifndef __OG_AST_for_node_H__
#define __OG_AST_for_node_H__

#include <cdk/ast/sequence_node.h>
#include <cdk/ast/basic_node.h>

namespace og {

  /**
   * Class for describing for-cycle nodes.
   */
  class for_node: public cdk::basic_node {
    cdk::sequence_node *_assignment;
    cdk::sequence_node *_condition;
    cdk::sequence_node *_increment;
    cdk::basic_node *_block;

  public:
    inline for_node(int lineno, cdk::sequence_node *assignment, cdk::sequence_node *condition, cdk::sequence_node *increment, cdk::basic_node *block) :
        basic_node(lineno), _assignment(assignment), _condition(condition), _increment(increment), _block(block) {
    }

  public:
    inline cdk::sequence_node *assignment() {
      return _assignment;
    }

    inline cdk::sequence_node *condition() {
      return _condition;
    }

    inline cdk::sequence_node *increment() {
      return _increment;
    }

    inline cdk::basic_node *block() {
      return _block;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_for_node(this, level);
    }

  };

} // og

#endif
