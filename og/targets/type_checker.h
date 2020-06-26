#ifndef __OG_TARGETS_TYPE_CHECKER_H__
#define __OG_TARGETS_TYPE_CHECKER_H__

#include "targets/basic_ast_visitor.h"
#include <vector>

namespace og {

  /**
   * Print nodes as XML elements to the output stream.
   */
  class type_checker: public basic_ast_visitor {
    cdk::symbol_table<og::symbol> &_symtab;
    basic_ast_visitor *_parent;
    std::shared_ptr<og::symbol> _function;

  public:
    type_checker(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<og::symbol> &symtab, basic_ast_visitor *parent) :
        basic_ast_visitor(compiler), _symtab(symtab), _parent(parent) {
    }

  public:
    ~type_checker() {
      os().flush();
    }

  protected:
    void processUnaryArithmeticExpression(cdk::unary_operation_node *const node, int lvl);
    void processBinaryExpression(cdk::binary_operation_node *const node, int lvl);
    template<typename T>
    void process_literal(cdk::literal_node<T> *const node, int lvl) {
    }
    std::vector<std::shared_ptr<cdk::basic_type>> build_args(cdk::sequence_node *arguments);
    bool same_args(std::shared_ptr<og::symbol> function, std::shared_ptr<og::symbol> prev);
    void processAditiveExpression(cdk::binary_operation_node *const node, int lvl);
    void processMultiplicativeExpression(cdk::binary_operation_node *const node, int lvl);
    void processIntOnlyExpression(cdk::binary_operation_node *const node, int lvl);
    void processRelationalExpression(cdk::binary_operation_node *const node, int lvl);
    void processEqualityExpression(cdk::binary_operation_node *const node, int lvl);
  public:
    // do not edit these lines
#define __IN_VISITOR_HEADER__
#include "ast/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
    // do not edit these lines: end

  };

} // og

//---------------------------------------------------------------------------
//     HELPER MACRO FOR TYPE CHECKING
//---------------------------------------------------------------------------

#define CHECK_TYPES(compiler, symtab, node) { \
  try { \
    og::type_checker checker(compiler, symtab, this); \
    (node)->accept(&checker, 0); \
  } \
  catch (const std::string &problem) { \
    std::cerr << (node)->lineno() << ": " << problem << std::endl; \
    return; \
  } \
}

#define ASSERT_SAFE_EXPRESSIONS CHECK_TYPES(_compiler, _symtab, node)

#endif
