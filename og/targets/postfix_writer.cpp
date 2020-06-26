#include <string>
#include <sstream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include "targets/frame_size_calculator.h"
#include "ast/all.h" // all.h is automatically generated

//---------------------------------------------------------------------------

void og::postfix_writer::do_nil_node(cdk::nil_node *const node, int lvl)
{
  // EMPTY
}
void og::postfix_writer::do_data_node(cdk::data_node *const node, int lvl)
{
  // EMPTY
}
void og::postfix_writer::do_double_node(cdk::double_node *const node, int lvl)
{
  if (isInFunction()) {
    _pf.DOUBLE(node->value());
  }
  else {
    _pf.SDOUBLE(node->value());
  }
}
void og::postfix_writer::do_not_node(cdk::not_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  _pf.INT(0); // 0 => 1
  _pf.EQ();   // !0 => 0
}
void og::postfix_writer::do_and_node(cdk::and_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  std::string false_lbl = mklbl(++_lbl), end_lbl = mklbl(++_lbl);

  node->left()->accept(this, lvl);
  _pf.INT(0);
  _pf.NE(); // 1 if not zero
  _pf.JZ(false_lbl);

  node->right()->accept(this, lvl);
  _pf.INT(0);
  _pf.NE();
  _pf.JZ(false_lbl);

  _pf.INT(1); // true
  _pf.JMP(end_lbl);

  _pf.LABEL(false_lbl);
  _pf.INT(0); // false
  
  _pf.LABEL(end_lbl);
}
void og::postfix_writer::do_or_node(cdk::or_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  std::string true_lbl = mklbl(++_lbl), end_lbl = mklbl(++_lbl);
  
  node->left()->accept(this, lvl);
  _pf.INT(0);
  _pf.EQ(); // 1 if zero
  _pf.JZ(true_lbl);

  node->right()->accept(this, lvl);
  _pf.INT(0);
  _pf.EQ();
  _pf.JZ(true_lbl);

  _pf.INT(0); // false
  _pf.JMP(end_lbl);

  _pf.LABEL(true_lbl);
  _pf.INT(1); // true
  
  _pf.LABEL(end_lbl);
}

void og::postfix_writer::do_sequence_node(cdk::sequence_node *const node, int lvl)
{
  if (lvl == 0) {
    // HACK !!! We are the the top of the ast
    og::exports_require functions_validator(_compiler);
    node->accept(&functions_validator, 0);
    for (auto name : functions_validator.functions()) {
      _pf.EXTERN(name);
    }      
  }

  for (size_t i = 0; i < node->size(); i++) {
    try {
      node->node(i)->accept(this, lvl + 2);
    } catch (const std::string &problem) {
      std::cerr << node->lineno() << ": " << problem << std::endl;
    }
  }
}

void og::postfix_writer::do_integer_node(cdk::integer_node *const node, int lvl)
{
  // NOTE: This node might actually need to save a double
  if (isInFunction()) {
    node->type()->name() == cdk::TYPE_INT ? _pf.INT(node->value()) : _pf.DOUBLE(node->value()); 
  }
  else {
    node->type()->name() == cdk::TYPE_INT ? _pf.SINT(node->value()) : _pf.SDOUBLE(node->value());
  }
}

void og::postfix_writer::do_string_node(cdk::string_node *const node, int lvl)
{
  int lbl1;

  /* generate the string */
  _pf.RODATA();                    // strings are DATA readonly
  _pf.ALIGN();                     // make sure we are aligned
  _pf.LABEL(mklbl(lbl1 = ++_lbl)); // give the string a name
  _pf.SSTRING(node->value());      // output string characters

  /* leave the address on the stack */
  if (isInFunction()) {
    _pf.TEXT();            // return to the TEXT segment
    _pf.ADDR(mklbl(lbl1)); // the string to be printed
  }
  else {
    _pf.DATA();
    _pf.SADDR(mklbl(lbl1));
  }
}

void og::postfix_writer::do_neg_node(cdk::neg_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl); // determine the value
  if (node->argument()->is_typed(cdk::TYPE_INT))
    _pf.NEG(); // 2-complement
  else
    _pf.DNEG(); //negate floating point
}

void og::postfix_writer::do_identity_node(og::identity_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
}

void og::postfix_writer::processAditiveExpression(cdk::binary_operation_node *const node, int lvl)
{
  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();
  else if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_POINTER))
  {
    _pf.INT(dynamic_cast<cdk::reference_type *>(node->right()->type().get())->referenced()->size()); //size of pointed type
    _pf.MUL();
  }

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    _pf.I2D();
  else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_INT))
  {
    _pf.INT(dynamic_cast<cdk::reference_type *>(node->left()->type().get())->referenced()->size()); //size of pointed type
    _pf.MUL();
  }
}

void og::postfix_writer::do_add_node(cdk::add_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;

  processAditiveExpression(node, lvl);

  //Stack now has same type operands
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.DADD();
  else if (!node->left()->is_typed(cdk::TYPE_POINTER) && !node->right()->is_typed(cdk::TYPE_POINTER))
    _pf.ADD();
  else //pointer arithmetic
    _pf.ADD();
}

void og::postfix_writer::do_sub_node(cdk::sub_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;

  processAditiveExpression(node, lvl);

  //Stack now has same type operands
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.DSUB();
  else if (!node->left()->is_typed(cdk::TYPE_POINTER) && !node->right()->is_typed(cdk::TYPE_POINTER))
    _pf.SUB();
  else if (node->left()->is_typed(cdk::TYPE_POINTER) && node->right()->is_typed(cdk::TYPE_POINTER))
  { //pointer difference
    _pf.SUB();
    _pf.INT(node->left()->type()->size());
    _pf.DIV(); // left and right have same type, get number of position in between
  }
  else
  {
    _pf.SUB(); // offset
  }
}

void og::postfix_writer::processMultiplicativeExpression(cdk::binary_operation_node *const node, int lvl)
{
  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    _pf.I2D();
}

void og::postfix_writer::do_mul_node(cdk::mul_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  processMultiplicativeExpression(node, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.DMUL();
  else
    _pf.MUL();
}
void og::postfix_writer::do_div_node(cdk::div_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  processMultiplicativeExpression(node, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.DDIV();
  else
    _pf.DIV();
}

void og::postfix_writer::do_mod_node(cdk::mod_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}

void og::postfix_writer::processRelationalExpression(cdk::binary_operation_node *const node, int lvl)
{
  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    _pf.I2D();

  //floating point needs to compare to 0 to get the result
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DCMP();
    _pf.INT(0);
  }
}

void og::postfix_writer::do_lt_node(cdk::lt_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  processRelationalExpression(node, lvl);
  _pf.LT();
}
void og::postfix_writer::do_le_node(cdk::le_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  processRelationalExpression(node, lvl);
  _pf.LE();
}
void og::postfix_writer::do_ge_node(cdk::ge_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  processRelationalExpression(node, lvl);
  _pf.GE();
}
void og::postfix_writer::do_gt_node(cdk::gt_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  processRelationalExpression(node, lvl);
  _pf.GT();
}

void og::postfix_writer::processEqualityExpression(cdk::binary_operation_node *const node, int lvl)
{
  node->left()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_INT) && node->right()->is_typed(cdk::TYPE_DOUBLE))
    _pf.I2D();

  node->right()->accept(this, lvl);
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) && node->right()->is_typed(cdk::TYPE_INT))
    _pf.I2D();

  //floating point needs to compare to 0 to get the result
  if (node->left()->is_typed(cdk::TYPE_DOUBLE) || node->right()->is_typed(cdk::TYPE_DOUBLE))
  {
    _pf.DCMP();
    _pf.INT(0);
  }
}

void og::postfix_writer::do_ne_node(cdk::ne_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  processEqualityExpression(node, lvl);
  _pf.NE();
}
void og::postfix_writer::do_eq_node(cdk::eq_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  processEqualityExpression(node, lvl);
  _pf.EQ();
}

void og::postfix_writer::do_variable_node(cdk::variable_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  
  auto symbol = _symtab.find(node->name());
  if (symbol->offset() != 0) {
    _pf.LOCAL(symbol->offset());
  }
  else
    _pf.ADDR(node->name());
}

void og::postfix_writer::do_rvalue_node(cdk::rvalue_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl); // get variable address

  if (node->lvalue()->is_typed(cdk::TYPE_DOUBLE))
    _pf.LDDOUBLE();
    
  else if (node->lvalue()->is_typed(cdk::TYPE_STRUCT)) {
    // Load tuple in correct order
    auto types = dynamic_cast<cdk::structured_type *>(node->lvalue()->type().get())->components();
    int size = node->lvalue()->type()->size();
    for (int i = types.size() - 1; i >= 0; i--) {
      _pf.INT(size - types[i]->size());
      _pf.ADD();
      if (types[i]->name() ==cdk::TYPE_DOUBLE)
        _pf.LDDOUBLE();
      else
        _pf.LDINT();
      node->lvalue()->accept(this, lvl);
      size -= types[i]->size();
    }
    _pf.TRASH(4); // remove extra address
  }
  else _pf.LDINT();
}

void og::postfix_writer::do_assignment_node(cdk::assignment_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->rvalue()->accept(this, lvl); // determine the new value

  // Duplicate assignment value
  if (node->rvalue()->is_typed(cdk::TYPE_DOUBLE))
    _pf.DUP64();
  else if (node->rvalue()->is_typed(cdk::TYPE_INT) && node->lvalue()->is_typed(cdk::TYPE_DOUBLE))
  {
    //implicit conversion
    _pf.I2D();
    _pf.DUP64();
  }
  else
    _pf.DUP32();

  // Writes address to stack
  node->lvalue()->accept(this, lvl);

  // Store value
  if (node->lvalue()->is_typed(cdk::TYPE_DOUBLE))
    _pf.STDOUBLE();

  else
    _pf.STINT();
}

void og::postfix_writer::do_evaluation_node(og::evaluation_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);         // determine the value
  _pf.TRASH(node->argument()->type()->size()); // Delete duplicated value
}

void og::postfix_writer::do_write_node(og::write_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  
  // Write always receives a tuple
  auto types = dynamic_cast<cdk::structured_type *>(node->argument()->type().get())->components();
  
  if (types.size() == 1 && types[0]->name() == cdk::TYPE_STRUCT) {
    auto real_tuple = dynamic_cast<cdk::typed_node *>(dynamic_cast<tuple_node *>(node->argument())->values()->node(0));
    types = dynamic_cast<cdk::structured_type *>(real_tuple->type().get())->components();
  }

  for (auto type : types) {
    switch(type->name()) {
      case cdk::TYPE_INT:
        _pf.CALL("printi");
        break;
      case cdk::TYPE_DOUBLE:
        _pf.CALL("printd");
        break;
      case cdk::TYPE_STRING:
        _pf.CALL("prints");
        break;
      default:
        throw std::string("Invalid type to print.");
    }
    _pf.TRASH(type->size());
  }

  if (node->newline())
    _pf.CALL("println");
}


void og::postfix_writer::do_input_node(og::input_node *const node, int lvl)
{
  if (node->is_typed(cdk::TYPE_INT)) {
    _pf.CALL("readi");
    _pf.LDFVAL32();
  }
  else if(node->is_typed(cdk::TYPE_DOUBLE)) {
    _pf.CALL("readd");
    _pf.LDFVAL64();
  } else 
    throw std::string("Reading invalid type in input.");
  

}


void og::postfix_writer::do_for_node(og::for_node *const node, int lvl)
{
  _symtab.push();
  std::string condition = mklbl(++_lbl), increment = mklbl(++_lbl), exit = mklbl(++_lbl); // labels
  _startFor = increment;
  _endFor = exit;
  _pf.TEXT();
  _pf.ALIGN();
  node->assignment()->accept(this, lvl);
  
  _pf.LABEL(condition); //start for
  node->condition()->accept(this, lvl);
  _pf.JZ(exit);
  node->block()->accept(this, lvl);

  _pf.LABEL(increment);
  node->increment()->accept(this, lvl);
  _pf.JMP(condition);

  _pf.LABEL(exit);

  _symtab.pop();
}


void og::postfix_writer::do_if_node(og::if_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1));
}


void og::postfix_writer::do_if_else_node(og::if_else_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  int lbl1, lbl2;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1 = lbl2));
}


void og::postfix_writer::do_return_node(og::return_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;

  auto ret_tuple = dynamic_cast<tuple_node *>(node->argument()); // returned type is always a tuple
  // return 1,2,3;
  // return c; // c e' tuplo

  // return 1;
  // return c; // c != tuple

  if (ret_tuple->values()->size() != 1 || dynamic_cast<cdk::typed_node *>(ret_tuple->values()->node(0))->is_typed(cdk::TYPE_STRUCT)) {
    //Either a literal tuple or tuple typed function / variable   
    node->argument()->accept(this, lvl);


    std::vector<std::shared_ptr<cdk::basic_type>> types;
    if (ret_tuple->values()->size() == 1)  //type of returned inner tuple
      types = dynamic_cast<cdk::structured_type *>(dynamic_cast<cdk::typed_node *>(ret_tuple->values()->node(0))->type().get())->components(); 
    else 
      types = dynamic_cast<cdk::structured_type *>(ret_tuple->type().get())->components();

    int size = 0;
    for (size_t i = 0; i < types.size(); i++) {
      auto dest_type = dynamic_cast<cdk::structured_type *>(_symtab.find(_function)->type().get())->components()[i]->name();
      if (dest_type == cdk::TYPE_DOUBLE && types[i]->name() == cdk::TYPE_INT)
        //implicit cast
        _pf.I2D();
      
      
      _pf.LOCAL(8); // load base address
      _pf.LDINT(); // load write address

      _pf.INT(size);
      _pf.ADD();

      if (dest_type == cdk::TYPE_DOUBLE)
        _pf.STDOUBLE();
      else
        _pf.STINT();
        
      size += types[i]->size() + (dest_type == cdk::TYPE_DOUBLE && types[i]->name() == cdk::TYPE_INT ? 4 : 0);
    }
  }
  else {
    node->argument()->accept(this, lvl);
    auto symb = _symtab.find(_function);
    if (dynamic_cast<cdk::typed_node *>(ret_tuple->values()->node(0))->is_typed(cdk::TYPE_INT) && _symtab.find(_function)->type()->name() == cdk::TYPE_DOUBLE)
      _pf.I2D();

    if (symb->type()->name() == cdk::TYPE_DOUBLE)  //returned type is double
      _pf.STFVAL64();
    else if (symb->type()->name() != cdk::TYPE_VOID) // all other cases have 32 bit result, handle procedure case
      _pf.STFVAL32();
    //void type has no return value
  }

  _pf.LEAVE();
  _pf.RET();

}

void og::postfix_writer::do_break_node(og::break_node *const node, int lvl)
{
  _pf.JMP(_endFor);
}

void og::postfix_writer::do_continue_node(og::continue_node *const node, int lvl)
{
  _pf.JMP(_startFor);
}

void og::postfix_writer::do_nullptr_node(og::nullptr_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  
  if (isInFunction()) {
    _pf.INT(0); 
  }
  else {
    _pf.SINT(0);
  }
}

void og::postfix_writer::do_sizeof_node(og::sizeof_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  _pf.INT(node->argument()->type()->size());
}

void og::postfix_writer::do_block_node(og::block_node *const node, int lvl)
{
  _symtab.push();
  if (node->declarations())
    node->declarations()->accept(this, lvl);

  if (node->instructions())
    node->instructions()->accept(this, lvl);
  _symtab.pop();
}

void og::postfix_writer::do_function_declaration_node(og::function_declaration_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  _pf.TEXT(); 
  if (node->access() == 2)
    _pf.EXTERN(*node->name());
}

void og::postfix_writer::do_function_definition_node(og::function_definition_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;

  std::string id;
  if (*node->name() == "og")
    id = "_main";
  else if (*node->name() == "_main")
    id = "._main";
  else
    id = *node->name();

  isInFunction(id); //enter function
  _symtab.push();

  _pf.TEXT();
  _pf.ALIGN();
  if (node->access() == 1)
  {
    _pf.GLOBAL(id, _pf.FUNC());
  }
  _pf.LABEL(id);

  // give offset to arguments
  _offset = node->is_typed(cdk::TYPE_STRUCT) ? 12 : 8; // needs to verify if returns primitive type
  auto args = node->arguments()->nodes();
  for (auto arg : args) {
    auto arg_decl = dynamic_cast<variable_declaration_node *>(arg);
                                                            /* Arguments only have one name*/
    auto symbol = std::make_shared<og::symbol>(arg_decl->type(), *arg_decl->names()[0], arg_decl->access(), false, arg_decl->initialized());
    symbol->set_offset(_offset);
    _offset += arg_decl->type()->size();
    _symtab.insert(symbol->name(), symbol);
  }

  // calculate frame size
  og::frame_size_calculator calculator(_compiler);
  node->accept(&calculator, lvl);

  // local variables space
  _frame_offset = calculator.size();
  _pf.ENTER(_frame_offset);

  _offset = 0; // set offset for local variables
  //eval
  node->block()->accept(this, lvl + 2);

  if (node->is_typed(cdk::TYPE_VOID)) { // procedures may not have return node
    _pf.LEAVE();
    _pf.RET();
  }

  _symtab.pop();
  isInFunction(""); //exit function
}

void og::postfix_writer::do_variable_declaration_node(og::variable_declaration_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  auto names = node->names();

  if (node->initialized()) {
    if (isInFunction())
      _pf.TEXT();
    else
      _pf.DATA();

    _pf.ALIGN();

    if (names.size() == 1) { // type a = ...
      if (node->access() == 1)
        _pf.GLOBAL(*names[0], _pf.OBJ());
      
      if (!isInFunction()) {
        _pf.LABEL(*names[0]);
        node->expression()->accept(this, lvl);
      }
      else {  //local assignment
        _offset -= node->type()->size();
        auto symbol = std::make_shared<og::symbol>(node->type(), *names[0], node->access(), false, node->initialized());
        symbol->set_offset(_offset);
        _symtab.insert(symbol->name(), symbol);
        
        node->expression()->accept(this, lvl);

        if (node->is_typed(cdk::TYPE_DOUBLE) && node->expression()->is_typed(cdk::TYPE_INT)) {
          _pf.I2D();
        }
        
        if (node->expression()->is_typed(cdk::TYPE_DOUBLE)) { 
          _pf.LOCAL(_offset); // writes to variable position
          _pf.STDOUBLE();
        }
        else if (node->expression()->is_typed(cdk::TYPE_INT) || node->expression()->is_typed(cdk::TYPE_POINTER) || node->expression()->is_typed(cdk::TYPE_STRING)) {
          _pf.LOCAL(_offset); // writes to variable position
          _pf.STINT(); // stores 32 bit value (string, pointer, int)
        }
        // auto a = ...
        else if (node->expression()->is_typed(cdk::TYPE_STRUCT)) { 
          int sum = 0;
          
          for (auto type : dynamic_cast<cdk::structured_type *>(node->expression()->type().get())->components()) {
            _pf.LOCAL(_offset); // writes to variable position
            
            _pf.INT(sum); // write to specific position
            _pf.ADD();
          
            if (type->name() == cdk::TYPE_DOUBLE) { // handle write
              _pf.STDOUBLE();
              sum += 8;
            } else {
              _pf.STINT();
              sum += 4;
            }
          } // end for
        } // end struct
      } // end local assignment
    }
    else {  // more than one name, must be a tuple
      if (isInFunction()) {
        _offset -= node->expression()->type()->size();
        node->expression()->accept(this, lvl); //writes all results in stack 

        std::vector<std::shared_ptr<cdk::basic_type>> types = dynamic_cast<cdk::structured_type *>(node->expression()->type().get())->components();
        int sum = 0;
        for (size_t i = 0; i < types.size(); i++) {
          _pf.ALIGN();
          if (node->access() == 1)
            _pf.GLOBAL(*names[i], _pf.OBJ());

          _pf.LOCAL(_offset);
          auto symbol = std::make_shared<og::symbol>(node->type(), *names[i], node->access(), false, node->initialized());
          symbol->set_offset(_offset);
          _symtab.insert(symbol->name(), symbol);

          _offset += types[i]->size(); 

          if (types[i]->name() == cdk::TYPE_DOUBLE) {
            _pf.STDOUBLE();
            sum += 8;
          } else {
            _pf.STINT();
            sum += 4;
          }
        } //end for
        _offset -= node->expression()->type()->size(); // compensate for
      }
      else { // global
        // global assignments require literals
        auto tup_node = dynamic_cast<tuple_node *>(node->expression());
        for (size_t i = 0; i < names.size(); i++) {
          _pf.ALIGN();
          if (node->access() == 1)
            _pf.GLOBAL(*names[i], _pf.OBJ());
          
          _pf.LABEL(*names[i]);
          tup_node->values()->node(i)->accept(this, lvl);
        }
      }
      
    }// end else
  } 
  else if (!isInFunction()) {  // global not initialized
    //bss
    _pf.BSS();
    _pf.ALIGN();
    for (auto name : names) {
      if (node->access() == 1)
        _pf.GLOBAL(*name, _pf.OBJ());
      _pf.LABEL(*name);
      _pf.SALLOC(node->type()->size());
    }
  } else {
    //local uninitialized variables, give correct offset
    _offset -= node->type()->size();
    auto symbol = std::make_shared<og::symbol>(node->type(), *names[0], node->access(), false, node->initialized());
    symbol->set_offset(_offset);
    _symtab.insert(symbol->name(), symbol);
  }
}

void og::postfix_writer::do_stack_alloc_node(og::stack_alloc_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->argument()->accept(this, lvl);
  auto ref_type = dynamic_cast<cdk::reference_type *>(node->type().get());
  _pf.INT(ref_type->referenced()->size()); 
  _pf.MUL(); // #bytes of n objects

  _pf.ALLOC();
  _pf.SP();
}

void og::postfix_writer::do_address_node(og::address_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->lvalue()->accept(this, lvl);
}

void og::postfix_writer::do_function_call_node(og::function_call_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;

  // if a tuple is returned we need to allocate the space for it in the stack
  if (node->is_typed(cdk::TYPE_STRUCT)) {
    _pf.INT(node->type()->size());
    _pf.ALLOC();
  }

  auto values = node->arguments()->nodes();
  size_t total_size = 0;
  for (int i = values.size() - 1; i >= 0; i--) {// put arguments in reversed order
    values[i]->accept(this, lvl);
    if (dynamic_cast<cdk::typed_node *>(values[i])->is_typed(cdk::TYPE_INT) && _symtab.find(*node->name())->args()[i]->name() == cdk::TYPE_DOUBLE) {
      //implicit conversion
      _pf.I2D();
      total_size += 8;
    } else
      total_size += dynamic_cast<cdk::typed_node *>(values[i])->type()->size();
  }
  

  if (node->is_typed(cdk::TYPE_STRUCT))
    _pf.LOCAL(-_frame_offset - node->type()->size()); // return pointer to write non primitive type -> writes to end of all variable declarations + allocated size

  _pf.CALL(*node->name());
  _pf.TRASH(total_size + (node->is_typed(cdk::TYPE_STRUCT) ? 4 : 0)); // delete arguments and reference to tuple
  if (node->is_typed(cdk::TYPE_DOUBLE))
    _pf.LDFVAL64();
  else if (!node->is_typed(cdk::TYPE_VOID) && !node->is_typed(cdk::TYPE_STRUCT)) // avoid tuple
    _pf.LDFVAL32();
}

void og::postfix_writer::do_index_node(og::index_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  node->index()->accept(this, lvl); // offset to count
  _pf.INT(dynamic_cast<cdk::reference_type *>(node->base()->type().get())->referenced()->size()); 
  _pf.MUL(); // real offset according to type
  node->base()->accept(this, lvl); // base address
  _pf.ADD();
}

void og::postfix_writer::do_tuple_node(og::tuple_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  auto values = node->values()->nodes();
  if (!isInFunction()) {
    for (size_t i = 0; i < values.size(); i++)
      values[i]->accept(this, lvl);
  } 
  else {
    for (int i = values.size() - 1; i >= 0; i--) // puts values in reversed order so stack can pop ordered
      values[i]->accept(this, lvl);
  }
  
}

void og::postfix_writer::do_tuple_index_node(og::tuple_index_node *const node, int lvl)
{
  ASSERT_SAFE_EXPRESSIONS;
  auto types = dynamic_cast<cdk::structured_type *>(node->base()->type().get())->components();
  int j = node->index()->value(); // index to check (1-based)
  int total_size = 0;
  for (int i = 0; i < j - 1; i++) total_size += types[i]->size();

  if (dynamic_cast<cdk::rvalue_node *>(node->base())) {
    // if tuple exists in memory already
    auto lval = dynamic_cast<cdk::rvalue_node *>(node->base())->lvalue();
    lval->accept(this, lvl); // tuple base address
  } else {
    // load tuple to stack
    node->base()->accept(this, lvl);
    _pf.SP();
  }
  
  _pf.INT(total_size);     // tuples cant be indexed because it can have different types
  _pf.ADD();               // address is now the address of the position
}
