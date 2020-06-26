#ifndef __OG_AST_BLOCK_H__
#define __OG_AST_BLOCK_H__

#include <cdk/ast/basic_node.h>
#include <cdk/ast/sequence_node.h>

namespace og {

  class block_node: public cdk::basic_node {
    cdk::sequence_node *_declarations;
    cdk::sequence_node *_instructions;
    

  public:
    block_node(int lineno, cdk::sequence_node *decls, cdk::sequence_node *insts) :
        cdk::basic_node(lineno), _declarations(decls), _instructions(insts) {
          if (_declarations == NULL) _declarations = new cdk::sequence_node(lineno);
          if (_instructions == NULL) _instructions = new cdk::sequence_node(lineno);
    }

  public:
    inline cdk::sequence_node *declarations() {
      return _declarations;
    } 

    inline cdk::sequence_node *instructions() {
      return _instructions;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_block_node(this, level);
    }
  };

} // og

#endif
