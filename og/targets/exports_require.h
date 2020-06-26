#ifndef __OG_TARGETS_EXPORTS_REQUIRE_H__
#define __OG_TARGETS_EXPORTS_REQUIRE_H__

#include "targets/basic_ast_visitor.h"

#include <sstream>
#include <set>

namespace og
{
    /* This visitor will find all non-defined functions that need to be imported from RTS */
    class exports_require : public basic_ast_visitor
    {
        int _size;
        std::set<std::string> _functions;

    public:
        exports_require(std::shared_ptr<cdk::compiler> compiler) : basic_ast_visitor(compiler), _size(0), _functions() {}

    public:
        ~exports_require()
        {
            os().flush();
        }

        int size() { return _size; }
        void size(int j) { _size += j; }

        std::set<std::string> functions() {
            return _functions;
        }

    public:
        // do not edit these lines
#define __IN_VISITOR_HEADER__
#include "ast/visitor_decls.h" // automatically generated
#undef __IN_VISITOR_HEADER__
        // do not edit these lines: end
    };

} // namespace og

#endif