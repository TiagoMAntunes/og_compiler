#ifndef __OG_TARGETS_FRAME_SIZE_CALCULATOR_H__
#define __OG_TARGETS_FRAME_SIZE_CALCULATOR_H__

#include "targets/basic_ast_visitor.h"

#include <sstream>

namespace og
{

    class frame_size_calculator : public basic_ast_visitor
    {
        int _size;

    public:
        frame_size_calculator(std::shared_ptr<cdk::compiler> compiler) : basic_ast_visitor(compiler), _size(0) {}

    public:
        ~frame_size_calculator()
        {
            os().flush();
        }

        int size() { return _size; }
        void size(int j) { _size += j; }

    public:
        // do not edit these lines
#define __IN_VISITOR_HEADER__
#include "ast/visitor_decls.h" // automatically generated
#undef __IN_VISITOR_HEADER__
        // do not edit these lines: end
    };

} // namespace og

#endif