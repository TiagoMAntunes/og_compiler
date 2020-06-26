#ifndef __OG_TARGETS_POSTFIX_WRITER_H__
#define __OG_TARGETS_POSTFIX_WRITER_H__

#include "targets/basic_ast_visitor.h"

#include <sstream>
#include <cdk/emitters/basic_postfix_emitter.h>
#include "targets/exports_require.h"

namespace og {

  //!
  //! Traverse syntax tree and generate the corresponding assembly code.
  //!
  class postfix_writer: public basic_ast_visitor {
    cdk::symbol_table<og::symbol> &_symtab;
    cdk::basic_postfix_emitter &_pf;
    int _lbl;
    std::string _function;
    int _offset;
    int _frame_offset;
    std::string _startFor, _endFor;

  public:
    postfix_writer(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<og::symbol> &symtab,
                   cdk::basic_postfix_emitter &pf) :
        basic_ast_visitor(compiler), _symtab(symtab), _pf(pf), _lbl(0), _function(""), _offset(0), _frame_offset(0), _startFor(""), _endFor("") {
    }

  public:
    ~postfix_writer() {
      os().flush();
    }

  private:
    /** Method used to generate sequential labels. */
    inline std::string mklbl(int lbl) {
      std::ostringstream oss;
      if (lbl < 0)
        oss << ".L" << -lbl;
      else
        oss << "_L" << lbl;
      return oss.str();
    }

    void processAditiveExpression(cdk::binary_operation_node *const node, int lvl);
    void processMultiplicativeExpression(cdk::binary_operation_node *const node, int lvl);
    void processRelationalExpression(cdk::binary_operation_node *const node, int lvl);
    void processEqualityExpression(cdk::binary_operation_node *const node, int lvl);
    inline bool isInFunction() { return _function != ""; }
    inline void isInFunction(std::string s) { _function = s; }
    inline int offset() { return _offset; }
    inline void offset(int o) { _offset = o; }

  public:
  // do not edit these lines
#define __IN_VISITOR_HEADER__
#include "ast/visitor_decls.h"       // automatically generated
#undef __IN_VISITOR_HEADER__
  // do not edit these lines: end

  };

} // og

#endif
