#ifndef __OG_TARGETS_SYMBOL_H__
#define __OG_TARGETS_SYMBOL_H__

#include <string>
#include <memory>
#include <vector>
#include <cdk/types/basic_type.h>

namespace og {

  class symbol {
    std::shared_ptr<cdk::basic_type> _type;
    std::string _name;
    int _access;
    bool _function;     // is the symbol a function?
    bool _defined;      // is the function defined?
    bool _initialized;  // is the variable initialized?
    int _offset = 0; 
    std::vector<std::shared_ptr<cdk::basic_type>> _args;

  public:
    symbol(std::shared_ptr<cdk::basic_type> type, const std::string &name,
      int access, bool func, bool init,
      std::vector<std::shared_ptr<cdk::basic_type>> args = std::vector<std::shared_ptr<cdk::basic_type>>(), 
      bool defined = false) :
        _type(type), _name(name), _access(access), 
        _function(func), _defined(defined), _initialized(init), _offset(0), _args(args)
      { }

    virtual ~symbol() {
      // EMPTY
    }

    std::shared_ptr<cdk::basic_type> type() const {
      return _type;
    }
    void type(std::shared_ptr<cdk::basic_type> new_type) {
      _type = new_type;
    } 
    bool is_typed(cdk::typename_type name) const {
      return _type->name() == name;
    }
    const std::string &name() const {
      return _name;
    }
    int access() const {
      return _access;
    }
    bool is_function() const {
      return _function;
    }

    bool initialized() const {
      return _initialized;
    }
    std::vector<std::shared_ptr<cdk::basic_type>> args() const {
      return _args;
    }
    bool has_args() const {
      return !_args.empty();
    }

    void set_args(std::vector<std::shared_ptr<cdk::basic_type>> args) {
      _args = args;
    }

    bool defined() const {
      return _defined;
    }

    void defined(bool d) {
      _defined = d;
    }

    void set_offset(int offset) {
      _offset = offset;
    }

    int offset() {
      return _offset;
    }
  };

} // og

#endif
