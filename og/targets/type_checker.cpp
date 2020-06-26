#include <string>
#include <vector>
#include "targets/type_checker.h"
#include "ast/all.h"  // automatically generated
#include <cdk/types/basic_type.h>
#include <cdk/types/primitive_type.h>

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

#define REQUIRE  2
//---------------------------------------------------------------------------

void og::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++)
    node->node(i)->accept(this, lvl);
}

//-------------------------------------------------------------------
// EMPTY NODES
void og::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY - keep it
}
void og::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}
void og::type_checker::do_block_node(og::block_node *const node, int lvl) {
  _symtab.push();
  node->declarations()->accept(this, lvl);
  node->instructions()->accept(this, lvl);
  _symtab.pop();
}
void og::type_checker::do_break_node(og::break_node *const node, int lvl) {
  // EMPTY
}
void og::type_checker::do_continue_node(og::continue_node *const node, int lvl) {
  // EMPTY
}
void og::type_checker::do_input_node(og::input_node *const node, int lvl) {
  // EMPTY - already type unspec
}
//---------------------------------------------------------------------------

void og::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}

void og::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
}

void og::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC; 
  node->type(cdk::make_primitive_type(4, cdk::TYPE_STRING));
}

void og::type_checker::do_nullptr_node(og::nullptr_node *const node, int lvl) {
  ASSERT_UNSPEC
  node->type(cdk::make_reference_type(4, cdk::make_primitive_type(4,cdk::TYPE_VOID))); 
}

void og::type_checker::do_tuple_node(og::tuple_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->values()->accept(this, lvl);

  size_t size = node->values()->size();
  std::vector<std::shared_ptr<cdk::basic_type>> types(size);
 
  for (size_t i = 0; i < size; i++) {
    auto el = dynamic_cast<cdk::expression_node*>(node->values()->node(i));
    if (el->is_typed(cdk::TYPE_UNSPEC)) el->type(cdk::make_primitive_type(4, cdk::TYPE_INT)); // inputs become ints
    types[i] = el->type();
  }

  node->type(cdk::make_structured_type(types));
}


//---------------------------------------------------------------------------

void og::type_checker::processUnaryArithmeticExpression(cdk::unary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (!node->argument()->is_typed(cdk::TYPE_INT) && !node->argument()->is_typed(cdk::TYPE_DOUBLE)) 
    throw std::string("Unary arithmetic expression requires integer or real type.");

  node->type(node->argument()->type());
}
void og::type_checker::do_neg_node(cdk::neg_node *const node, int lvl) {
  processUnaryArithmeticExpression(node, lvl);
}
void og::type_checker::do_identity_node(og::identity_node *const node, int lvl) {
  processUnaryArithmeticExpression(node, lvl);
}

void og::type_checker::processAditiveExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  // operations with pointers
  if (node->left()->is_typed(cdk::TYPE_POINTER)) {
    if (!node->right()->is_typed(cdk::TYPE_POINTER) && !node->right()->is_typed(cdk::TYPE_INT)) {
      throw std::string("Wrong types in right argument of operation with pointers.");
    }
    node->type(node->left()->type());
  }
  else if (node->right()->is_typed(cdk::TYPE_POINTER)) {
    if (!node->left()->is_typed(cdk::TYPE_POINTER) && !node->left()->is_typed(cdk::TYPE_INT)) {
      throw std::string("Wrong types in left argument of operation with pointers.");
    }
    node->type(cdk::make_primitive_type(4, cdk::TYPE_POINTER));
  }
  // operations with ints and reals
  else if ((node->left()->is_typed(cdk::TYPE_INT) || node->left()->is_typed(cdk::TYPE_DOUBLE)) &&
          (node->right()->is_typed(cdk::TYPE_INT) || node->right()->is_typed(cdk::TYPE_DOUBLE))) {
    // both int -> so is the node
    if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    }
    // both real -> so is the node
    else if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
    }
    // different types -> the node is real
    else {
      node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
    }
  }
  // og only takes int, real or pointer
  else {
    throw std::string("Wrong types in aditive binary expression.");
  }
}
void og::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  processAditiveExpression(node, lvl);
}
void og::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  processAditiveExpression(node, lvl);
}

void og::type_checker::processMultiplicativeExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);

  if ((node->left()->is_typed(cdk::TYPE_INT) || node->left()->is_typed(cdk::TYPE_DOUBLE)) &&
      (node->right()->is_typed(cdk::TYPE_INT) || node->right()->is_typed(cdk::TYPE_DOUBLE))) {
    // at least one is real -> node is real        
    if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
    }
    // both are int
    else {
      node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    }
  }
  else {
    throw std::string("Wrong types in multiplicative binary operation.");
  }
}
void og::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  processMultiplicativeExpression(node, lvl);
}
void og::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  processMultiplicativeExpression(node, lvl);
}

void og::type_checker::processIntOnlyExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->left()->accept(this, lvl);
  if (node->left()->type()->name() != cdk::TYPE_INT) {
    throw std::string("Integer expression expected in binary operator (left)");
  }

  node->right()->accept(this, lvl);
  if (node->right()->type()->name() != cdk::TYPE_INT) {
    throw std::string("Integer expression expected in binary operator (right)");
  }

  node->type(node->left()->type());
}
void og::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  processIntOnlyExpression(node, lvl);
}
void og::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  processIntOnlyExpression(node, lvl);
}
void og::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  processIntOnlyExpression(node, lvl);
}

void og::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl);

  if (!node->argument()->is_typed(cdk::TYPE_INT)) {
    throw std::string("'not' operator can only be applied to integers.");
  }
  // gr8 verifies if type is unspec -> why?

  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}

void og::type_checker::processRelationalExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;

  /* Can only compare integers and reals */

  node->left()->accept(this, lvl);
  if (!node->left()->is_typed(cdk::TYPE_INT) && !node->left()->is_typed(cdk::TYPE_DOUBLE))
    throw std::string("Comparative expression expected either integer or real (left)");

  node->right()->accept(this, lvl);
  if (!node->right()->is_typed(cdk::TYPE_INT) && !node->right()->is_typed(cdk::TYPE_DOUBLE))
    throw std::string("Comparative expression expected either integer or real (right)");
  
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT)); // all comparations result in an integer type
}
void og::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  processRelationalExpression(node, lvl);
}
void og::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  processRelationalExpression(node, lvl);
}
void og::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  processRelationalExpression(node, lvl);
}
void og::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  processRelationalExpression(node, lvl);
}

void og::type_checker::processEqualityExpression(cdk::binary_operation_node *const node, int lvl) {
  ASSERT_UNSPEC;

  /* Can compare integers, reals and pointers */
  
  node->left()->accept(this, lvl);
  if (!node->left()->is_typed(cdk::TYPE_INT) && !node->left()->is_typed(cdk::TYPE_DOUBLE) && !node->left()->is_typed(cdk::TYPE_POINTER))
    throw std::string("Equality expression expected either integer, real or pointer (left)");

  node->right()->accept(this, lvl);
  if (!node->right()->is_typed(cdk::TYPE_INT) && !node->right()->is_typed(cdk::TYPE_DOUBLE) && !node->right()->is_typed(cdk::TYPE_POINTER))
    throw std::string("Equality expression expected either integer, real or pointer (right)");

  if ((node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_DOUBLE)) || (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_POINTER)))
    throw std::string("Cannot compare pointer with double.");

  if ((!node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_VOID)) || (node->left()->is_typed(cdk::TYPE_VOID) && !node->right()->is_typed(cdk::TYPE_POINTER)))
    throw std::string("Null pointer can only be compared to pointer.");

  if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER)) { 
    auto type1 = dynamic_cast<cdk::reference_type *>(node->left()->type().get())->referenced()->name();
    auto type2 = dynamic_cast<cdk::reference_type *>(node->right()->type().get())->referenced()->name();
    if (type1 != type2 && type1 != cdk::TYPE_UNSPEC && type2 != cdk::TYPE_UNSPEC)
      throw std::string("Comparison between different type pointers.");
  }
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT)); // all equalities result in an integer type, analogous to comparative
}
void og::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  processEqualityExpression(node, lvl);
}
void og::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  processEqualityExpression(node, lvl);
}

//---------------------------------------------------------------------------
void og::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<og::symbol> symbol = _symtab.find(id);

  if (symbol == nullptr) {
    throw "Undeclared variable '" + id + "'."; 
  }
  
  if (symbol->is_function()) {
    throw "'" + id + "' is not a variable.";
  }
  node->type(symbol->type());
}

void og::type_checker::do_variable_declaration_node(og::variable_declaration_node *const node, int lvl) {
  // WIP
  ASSERT_UNSPEC;

  // type id [= value] 
  if (node->declType()->name() != cdk::TYPE_STRUCT) {
    if (node->initialized()) {
      node->expression()->accept(this, lvl);

      if (node->names().size() == 1) {
        // type name = f(1)
        if (node->expression()->is_typed(cdk::TYPE_STRUCT)) {
          og::tuple_node *initializer = dynamic_cast<og::tuple_node*>(node->expression());
          if (initializer->values()->size() != 1) {
            throw std::string("Invalid declaration of variable '" + *node->names()[0] + "'.");
          }

          cdk::expression_node *value = dynamic_cast<cdk::expression_node*>(initializer->values()->node(0));
          if (!value->is_typed(node->declType()->name()) && !(node->is_typed(cdk::TYPE_DOUBLE) && value->is_typed(cdk::TYPE_INT)) /* Implicit conversion */ ) {
            throw std::string("Invalid declaration of variable '" + *node->names()[0] + "'.");
          }
        }

        // type name = val
        else if (node->expression()->type()->name() == cdk::TYPE_INT && node->declType()->name() == cdk::TYPE_DOUBLE) {
          node->expression()->type(node->declType());
        }
        // type name = input
        else if (node->expression()->type()->name() == cdk::TYPE_UNSPEC) {
          if (node->declType()->name() == cdk::TYPE_INT || node->declType()->name() == cdk::TYPE_DOUBLE)
            node->expression()->type(node->declType());
          else
            throw std::string("Input only works in real or integer types.");
        }

        else if (!node->expression()->is_typed(node->declType()->name())) {
          throw std::string("Invalid declaration of variable '" + *node->names()[0] + "'.");
        }
        // ptr<type> name = [int]
        else if (node->expression()->type()->name() == cdk::TYPE_POINTER && node->declType()->name() == cdk::TYPE_POINTER) {
          auto expr_type = dynamic_cast<cdk::reference_type *>(node->expression()->type().get());
          if (expr_type->referenced() == NULL) 
            // stack memory allocation
            node->expression()->type(node->declType()); 
        }
      }
    }
    
    auto variable = std::make_shared<og::symbol>(node->declType(), *node->names()[0], node->access(), false, node->initialized());
    if (!_symtab.insert(variable->name(), variable)) {
      throw std::string("Redeclaring variable '" + variable->name() +"'.");
    }
    _parent->set_new_symbol(variable); 

    node->type(node->declType());
  }
  // auto ... = ...
  else {
    if (!node->initialized()) {
      throw std::string("Auto variables must be initialized.");
    }

    // generates types information
    node->expression()->accept(this, lvl); 
    // auto a = 1
    if (!node->expression()->is_typed(cdk::TYPE_STRUCT)) { 
      // cant do auto a = [2]
      if (node->expression()->is_typed(cdk::TYPE_POINTER) && dynamic_cast<cdk::reference_type *>(node->expression()->type().get())->referenced() == NULL) {
        throw std::string("Cannot use auto with stack allocation. Requires an explicit type.");
      } 
      else if (node->expression()->is_typed(cdk::TYPE_UNSPEC)) 
        node->expression()->type(cdk::make_primitive_type(4, cdk::TYPE_INT)); // input node
      
      auto variable = std::make_shared<og::symbol>(node->expression()->type(), *node->names()[0], node->access(), false, true);
      if (!_symtab.insert(variable->name(), variable)) {
        throw std::string("Redeclaring variable '" + variable->name() +"'.");
      }
      _parent->set_new_symbol(variable);
      node->type(node->expression()->type());
    }
    // auto a = 1, 2, 3 || auto a,b,c = 1, 2,3 
    else {
      if (node->names().size() == 1) { // 1 - n
        // auto name = tuple
        auto variable = std::make_shared<og::symbol>(node->expression()->type(), *node->names()[0], node->access(), false, true);
        if (!_symtab.insert(variable->name(), variable)) {
          throw std::string("Redeclaring variable '" + variable->name() +"'.");
        }
        _parent->set_new_symbol(variable);
      } 
      else {
        // auto names = tuple, must have the same size
        auto types = dynamic_cast<cdk::structured_type *>(node->expression()->type().get())->components();
        
        if (node->names().size() != types.size())
          throw std::string("Conflicting declaration. Multiple auto variables, must have an equally sized tuple.");
        
        for (unsigned int i = 0; i < node->names().size(); i++) { // n-n 
          std::shared_ptr<cdk::basic_type> type = types.at(i); // get nth type of tuple

          if (type->name() == cdk::TYPE_POINTER && dynamic_cast<cdk::reference_type *>(type.get())->referenced() == NULL) {
            throw std::string("Cannot use auto with stack allocation. Requires an explicit type.");
          }
          else if (type->name() == cdk::TYPE_UNSPEC)
            types[i] = cdk::make_primitive_type(4, cdk::TYPE_INT);
            

          auto variable = std::make_shared<og::symbol>(type, *node->names()[i], node->access(), false, true);
          if (!_symtab.insert(variable->name(), variable)) {
            throw std::string("Redeclaring variable '" + variable->name() +"'.");
          }
          _parent->set_new_symbol(variable);

        }
      }
      node->type(cdk::make_structured_type(dynamic_cast<cdk::structured_type*>(node->expression()->type().get())->components()));
    }
  }
}

void og::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl);
  node->type(node->lvalue()->type());
}

void og::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl);
  node->rvalue()->accept(this, lvl);

  if (node->lvalue()->is_typed(cdk::TYPE_INT)) {
    if (node->rvalue()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    }
    // can assign void* to int
    else if (node->rvalue()->is_typed(cdk::TYPE_POINTER) && 
          dynamic_cast<cdk::reference_type*>(node->type().get())->referenced()->name() == cdk::TYPE_UNSPEC) {
      node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
      node->rvalue()->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
    }
    else if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {
      //input node
      node->rvalue()->type(node->lvalue()->type());
    }
    else {
      throw std::string("Invalid assignment to int.");
    }
  }
  else if (node->lvalue()->is_typed(cdk::TYPE_DOUBLE)) {
    if (node->rvalue()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
    }
    else if (node->rvalue()->is_typed(cdk::TYPE_INT)) {
      node->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE));
      //node->rvalue()->type(cdk::make_primitive_type(8, cdk::TYPE_DOUBLE)); //automatic conversion
    }
    else if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {
      //input node
      node->rvalue()->type(node->lvalue()->type());
    }
    else {
      throw std::string("Invalid assignment to real.");
    }
  }
  else if (node->lvalue()->is_typed(cdk::TYPE_STRING)) {
    if (!node->rvalue()->is_typed(cdk::TYPE_STRING)) {
      throw std::string("Invalid assignment to string.");
    }
    node->type(cdk::make_primitive_type(4, cdk::TYPE_STRING));
  }
  else if (node->lvalue()->is_typed(cdk::TYPE_POINTER)) {
    if (!node->rvalue()->is_typed(cdk::TYPE_POINTER)) {
      throw std::string("Invalid assignment to pointer.");
    }

    cdk::reference_type * right_type = dynamic_cast<cdk::reference_type *>(node->rvalue()->type().get());

    if (right_type->referenced() == NULL)
      //Stack allocation
      node->rvalue()->type(node->lvalue()->type());
    else {
      // go down the tree checking if their types match
      cdk::reference_type * ltype = dynamic_cast<cdk::reference_type *>(node->lvalue()->type().get());
      cdk::reference_type * rtype = dynamic_cast<cdk::reference_type *>(node->rvalue()->type().get());

      while (ltype->referenced()->name() == cdk::TYPE_POINTER && rtype->referenced()->name() == cdk::TYPE_POINTER) {
        ltype = dynamic_cast<cdk::reference_type *>(ltype->referenced().get());
        rtype = dynamic_cast<cdk::reference_type *>(rtype->referenced().get());
      }

      if (ltype->referenced()->name() == cdk::TYPE_UNSPEC) {
        node->lvalue()->type(node->rvalue()->type()); // ptr<auto> cases
        // in this case, it should also update its symbol type
        auto var = dynamic_cast<cdk::variable_node *>(node->lvalue());
        if (var) {
          auto sym = _symtab.find(var->name());
          auto new_sym = std::make_shared<og::symbol>(node->lvalue()->type(), sym->name(), sym->access(), sym->is_function(), sym->initialized());
          _symtab.replace(new_sym->name(), new_sym);
        }

      }
      else if (ltype->referenced()->name() != rtype->referenced()->name())
        throw std::string("Different pointer types assignment");
    }

    node->type(node->lvalue()->type()); // so it references the same time as the lvalue
  }
  else {
    throw std::string("Invalid types in assignment.");
  }
}

//---------------------------------------------------------------------------

void og::type_checker::do_evaluation_node(og::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl);
}

void og::type_checker::do_write_node(og::write_node *const node, int lvl) {
  node->argument()->accept(this, lvl);

  if (node->argument()->is_typed(cdk::TYPE_STRUCT)) {
    //multiple elements to print
    auto values = dynamic_cast<tuple_node *>(node->argument())->values();
    for (size_t i = 0; i < values->size(); i++) {
      if (dynamic_cast<cdk::expression_node *>(values->node(i))->is_typed(cdk::TYPE_POINTER))
        throw std::string("Cannot print pointer types.");
    }
  }
  else {
    throw std::string("Invalid type in 'write' expressi");
  }
}

//---------------------------------------------------------------------------


void og::type_checker::do_for_node(og::for_node *const node, int lvl) {
  // reminder: for(;;) is possible
  // more than one assingment -> auto can't be used
  _symtab.push();

  if (node->assignment()) {
    size_t size = node->assignment()->size();
    if (size > 1) {
      for (unsigned int i = 0; i < size; i++) 
        if (dynamic_cast<cdk::expression_node*>(node->assignment()->node(i))->is_typed(cdk::TYPE_STRUCT)) 
          throw std::string("If auto is used in 'for' assignment, only one assigment is possible."); 
    } 
    node->assignment()->accept(this, lvl);
  }

  if (node->condition()) {
    node->condition()->accept(this, lvl);
    //C specification: condition must have arithmetic or pointer type
    for (unsigned int i = 0; i < node->condition()->size(); i++) {
      auto n = dynamic_cast<cdk::typed_node *>(node->condition()->node(i));
      if (!n->is_typed(cdk::TYPE_INT) && !n->is_typed(cdk::TYPE_DOUBLE) && !n->is_typed(cdk::TYPE_POINTER))
          throw std::string("For condition must have arithmetic or pointer type.");
    }
  }
  
  if (node->increment())
    node->increment()->accept(this, lvl);

  node->block()->accept(this, lvl + 2);
  _symtab.pop();
}

//---------------------------------------------------------------------------

void og::type_checker::do_if_node(og::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl);
  if (!node->condition()->is_typed(cdk::TYPE_INT) && !node->condition()->is_typed(cdk::TYPE_DOUBLE) && !node->condition()->is_typed(cdk::TYPE_POINTER))
    throw std::string("'if' statement requires an arithmetic or pointer type condition.");
  node->block()->accept(this, lvl + 2);
}

void og::type_checker::do_if_else_node(og::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl);
  if (!node->condition()->is_typed(cdk::TYPE_INT) && !node->condition()->is_typed(cdk::TYPE_DOUBLE) && !node->condition()->is_typed(cdk::TYPE_POINTER))
    throw std::string("'if else' statement requires an arithmetic or pointer type condition.");
  node->thenblock()->accept(this, lvl + 2);
  node->elseblock()->accept(this, lvl + 2);
}


//---------------------------------------------------------------------------

void og::type_checker::do_sizeof_node(og::sizeof_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->argument()->accept(this, lvl);
  node->type(cdk::make_primitive_type(4, cdk::TYPE_INT));
}

void og::type_checker::do_stack_alloc_node(og::stack_alloc_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl);

  if (!node->argument()->is_typed(cdk::TYPE_INT))
    throw std::string("Memory allocation requires an integer value.");

  node->type(cdk::make_reference_type(4, NULL)); 
}

void og::type_checker::do_address_node(og::address_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->lvalue()->accept(this, lvl); 
  node->type(cdk::make_reference_type(4, node->lvalue()->type())); // the result is always a pointer
}

void og::type_checker::do_index_node(og::index_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->base()->accept(this, lvl);      
  node->index()->accept(this, lvl); 

  if (!node->base()->is_typed(cdk::TYPE_POINTER))
    throw std::string("Cannot index non-pointer expression.");

  if (!node->index()->is_typed(cdk::TYPE_INT))
    throw std::string("Index value must be an integer.");

  //Node type is the type referenced by the pointer                         
  auto referenced_type = dynamic_cast<cdk::reference_type*>(node->base()->type().get())->referenced();
  node->type(referenced_type);
}

void og::type_checker::do_tuple_index_node(og::tuple_index_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->base()->accept(this, lvl);
  node->index()->accept(this, lvl);

  if (!node->base()->is_typed(cdk::TYPE_STRUCT)) 
    throw std::string("@ operator requires a tuple");

  if (!node->index()->is_typed(cdk::TYPE_INT)) 
    throw std::string("Tuple indexation requires an integer node");
  
  // Get the type from the given index and assign it
  // auto tuple_val = dynamic_cast<tuple_node *>(node->base())->values()->node(node->index()->value());
  auto type = dynamic_cast<cdk::structured_type *>(node->base()->type().get())->component(node->index()->value() - 1);
  node->type(cdk::make_primitive_type(type->size(), type->name()));

  // f()@1 - where f returns a tuple
  if (!dynamic_cast<cdk::rvalue_node *>(node->base())) // tuple is volatile
    node->type(cdk::make_primitive_type(node->type()->size() + node->base()->type()->size(), node->type()->name()));
  
}

// FUNCTIONS

std::vector<std::shared_ptr<cdk::basic_type>> og::type_checker::build_args(cdk::sequence_node *arguments) {
  std::vector<std::shared_ptr<cdk::basic_type>> args;

  for (size_t i = 0; i < arguments->size(); i++) {
    cdk::typed_node *arg = dynamic_cast<cdk::typed_node*>(arguments->node(i));
    args.push_back(arg->type());
  }

  return args;
}

bool og::type_checker::same_args(std::shared_ptr<og::symbol> function, std::shared_ptr<og::symbol> prev) {
  size_t size = function->args().size();
  if (size != prev->args().size()) {
    return false;
  }

  for (int i = 0; i < (int) size; i++) {
    if (prev->args()[i]->name() != function->args()[i]->name()) {
      return false;
    }
  }

  return true;
}

void og::type_checker::do_function_declaration_node(og::function_declaration_node *const node, int lvl) {
  ASSERT_UNSPEC;
  std::string id;
  
  // _main is a reserved identifier
  if (*node->name() == "og")
    id = "_main";
  else if (*node->name() == "_main")
    id = "._main";
  else
    id = *node->name();

  _symtab.push();
  std::vector<std::shared_ptr<cdk::basic_type>> args;
  if (node->arguments()) {
    node->arguments()->accept(this, lvl);
    args = og::type_checker::build_args(node->arguments());
  }
  _symtab.pop();

  auto function = std::make_shared<og::symbol>(node->declType(), *node->name(), node->access(), true, true, args, false);

  std::shared_ptr<og::symbol> prev = _symtab.find(function->name());
  if (prev) {
    // a redeclaration must be equal to the original declaration
    if (function->access() != prev->access() || !prev->is_typed(function->type()->name()) || !og::type_checker::same_args(function, prev)) {
      throw std::string("Conflicting declaration of function '" + function->name() + "'.");
    }
  }
  else {
    _symtab.insert(function->name(), function);
    _parent->set_new_symbol(function);
    node->type(function->type());
  }
  
}

void og::type_checker::do_function_definition_node(og::function_definition_node *const node, int lvl) {
  ASSERT_UNSPEC;
  std::string id;

  if (*node->name() == "og")
    id = "_main";
  else if (*node->name() == "_main")
    id = "._main";
  else
    id = *node->name();

  //create symbol without args being verified to avoid arguments with function name
  auto function = std::make_shared<og::symbol>(node->declType(), id, node->access(), true, true);
  function->set_offset(-node->declType()->size());

  //insert symbol into context
  std::shared_ptr<og::symbol> prev = _symtab.find(function->name());
  if (prev != nullptr) {
    // the function has already been declared but not defined
    if (!prev->defined() && function->access() != REQUIRE && function->access() == prev->access() && 
          prev->is_typed(function->type()->name())) {
      _symtab.replace(function->name(), function);
    }
    else {
      throw std::string("Conflicting definition for function '" + function->name() + "'.");
    }
  }
  // we're defining a whole new function
  else {
    _symtab.insert(function->name(), function);
  }

  //Create function context
  _symtab.push();

  //Check args
  std::vector<std::shared_ptr<cdk::basic_type>> args;
  if (node->arguments()->size() > 0) {
    node->arguments()->accept(this, lvl);
    args = og::type_checker::build_args(node->arguments());
  }

  // update function args
  function->set_args(args);
  function->defined(true);
  if (prev && !og::type_checker::same_args(function, prev)) {
     throw std::string("Conflicting definition for function '" + function->name() + "'.");
  }
  // Replace symbol for recursive
  _symtab.replace(function->name(), function);
  _parent->set_new_symbol(function);

  // now block knows the function exists
  _function = function;
  node->block()->accept(this, lvl);
  _symtab.pop();
  
  node->type(function->type());
}

void og::type_checker::do_function_call_node(og::function_call_node *const node, int lvl) {
  // WIP
  ASSERT_UNSPEC;
  std::shared_ptr<og::symbol> sym = _symtab.find(*node->name());

  if (!sym) {
    throw std::string("Symbol '" + *node->name() + "' undeclared.");
  }
  if (!sym->is_function()) {
    throw std::string("'" + sym->name() + "' is not a function.");
  } 
  if (node->arguments()) {
    if (node->arguments()->size() == sym->args().size()) {  
      node->arguments()->accept(this, lvl);
      
      for (size_t i = 0; i < node->arguments()->size(); i++) {
        auto arg = dynamic_cast<cdk::typed_node *>(node->arguments()->node(i));
        // handles input
        if (arg->type()->name() == cdk::TYPE_UNSPEC)
          arg->type(sym->args()[i]);
        
        if (arg->type()->name() != sym->args()[i]->name() && !(arg->type()->name() == cdk::TYPE_INT && sym->args()[i]->name() == cdk::TYPE_DOUBLE))
          throw std::string("Incorrect type of argument in function call to " + sym->name() + ".");
      }
    }
    else {
      throw std::string("Wrong number of arguments provided to function '" + sym->name() + "'.");
    }
  }

  node->type(sym->type());

}

void og::type_checker::do_return_node(og::return_node *const node, int lvl) {
  node->argument()->accept(this, lvl);
  auto tuple = dynamic_cast<og::tuple_node*>(node->argument());
  
  if (_function && _function->is_typed(cdk::TYPE_VOID) && tuple->values()->size() != 0)
    throw std::string("Procedure cannot return any values.");

  if (tuple->values()->size() == 0)
    throw std::string("Function cannot have empty return");

  auto value = dynamic_cast<cdk::typed_node*>(tuple->values()->node(0)); //first element

  if (_function && _function->is_typed(cdk::TYPE_UNSPEC)) {
    //function unknown type (auto)
    if (tuple->values()->size() > 1)
      //tuple return
      _function->type(node->argument()->type());
    else
      //non-tuple
      _function->type(value->type()); // set type as the first position of tuple
  } 
  else if (_function) {
    //validate compatible types
    if (_function->is_typed(cdk::TYPE_STRUCT)) {
      //same size and types 
      auto function_types = dynamic_cast<cdk::structured_type *>(_function->type().get())->components();
      if (function_types.size() != tuple->values()->size())
        throw std::string("Returning different sized tuples in function '" + _function->name() + "'.");
        
      for (size_t i = 0; i < function_types.size(); i++) {
        auto el = dynamic_cast<cdk::typed_node *>(tuple->values()->node(i));

        if (function_types.at(i)->name() == cdk::TYPE_INT && el->is_typed(cdk::TYPE_DOUBLE))
          function_types[i] = el->type();
        else if (!el->is_typed(function_types.at(i)->name()) && !(function_types.at(i)->name() == cdk::TYPE_DOUBLE && el->is_typed(cdk::TYPE_INT)))
          //different types or type order
          throw std::string("Returning different typed tuple in function '" + _function->name() + "'.");
      }
    }
    else {
      //same type or int to real (implicit)
      if (!_function->is_typed(value->type()->name()) && !(_function->is_typed(cdk::TYPE_DOUBLE) && value->is_typed(cdk::TYPE_INT)))
        throw std::string("Incompatible function and return types in function '" + _function->name() + "'.");
    }
  }
}
