#ifndef __OG_AST_TUPLE_H__
#define __OG_AST_TUPLE_H__

#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>

namespace og {

  class tuple_node: public cdk::expression_node {
    
   	cdk::sequence_node* _values;

  public:
    tuple_node(int lineno, cdk::sequence_node* values) :
      cdk::expression_node(lineno), _values(values) {
    }
    
  public:
  	inline cdk::sequence_node* values() {
  		return _values;
  	}

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_tuple_node(this, level);
    }
  };

} // og

#endif
