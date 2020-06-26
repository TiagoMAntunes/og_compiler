#ifndef __OG_AST_FUNCTION_DEFINITION_H__
#define __OG_AST_FUNCTION_DEFINITION_H__

#include <cdk/ast/typed_node.h>
#include <cdk/ast/sequence_node.h>
#include <cdk/types/typename_type.h>
#include <cdk/types/basic_type.h>
#include "block_node.h"

namespace og {

  class function_definition_node: public cdk::typed_node {
    int _access;
    std::string *_name;
    cdk::sequence_node *_arguments;
    og::block_node *_block;
    std::shared_ptr<cdk::basic_type> _declType;

    

  public:
    // procedure -> returns void
    function_definition_node(int lineno, int access, std::string *name, cdk::sequence_node *args, og::block_node *block) :
      cdk::typed_node(lineno), _access(access), _name(name), _arguments(args), _block(block) {
        _declType = cdk::make_primitive_type(0, cdk::typename_type::TYPE_VOID);
    }

    // function -> returns non void
    function_definition_node(int lineno, int access, std::shared_ptr<cdk::basic_type> type, std::string *name, cdk::sequence_node *args, og::block_node *block) :
      cdk::typed_node(lineno), _access(access), _name(name), _arguments(args), _block(block) {
      //  cdk::typed_node::type(type);
      _declType = type;
      }

  public:
    inline int access() {
      return _access;
    } 

    inline std::string *name() {
      return _name;
    } 

    inline cdk::sequence_node *arguments() {
      return _arguments;
    }

    inline og::block_node *block() {
      return _block;
    }

    inline std::shared_ptr<cdk::basic_type> declType() {
      return _declType;
    }

    void accept(basic_ast_visitor *sp, int level) {
      sp->do_function_definition_node(this, level);
    }
  };

} // og

#endif
