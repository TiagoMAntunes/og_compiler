#include "targets/exports_require.h"
#include "ast/all.h"  // all.h is automatically generated

void og::exports_require::do_function_definition_node(og::function_definition_node *const node, int lvl) {
  node->block()->accept(this, lvl);
}

void og::exports_require::do_function_declaration_node(og::function_declaration_node * const node, int lvl) {
  /* Get the used RTS functions */
  auto name = *node->name();
  
  if (name == "strlen" || name == "atoi" || name == "atod" || name == "itoa" || name == "dtoa" || name == "argc" || name == "argv" || name == "envp") 
    _functions.insert(*node->name());
}

void og::exports_require::do_write_node(og::write_node * const node, int lvl) {
  node->argument()->accept(this, lvl);
  
  _functions.insert("printd");
  _functions.insert("printi");
  _functions.insert("prints");
  _functions.insert("println");
}

void og::exports_require::do_input_node(og::input_node * const node, int lvl) {
  _functions.insert("readln");
  _functions.insert("readb");
  _functions.insert("readi");
  _functions.insert("readd");
}

void og::exports_require::do_block_node(og::block_node *const node, int lvl) {
  if (node->declarations())
    node->declarations()->accept(this, lvl);
  if (node->instructions())
    node->instructions()->accept(this, lvl);
}

void og::exports_require::do_for_node(og::for_node *const node, int lvl) {
  if (node->assignment())
    node->assignment()->accept(this, lvl);

  node->block()->accept(this, lvl);
}

void og::exports_require::do_if_else_node(og::if_else_node *const node, int lvl) {
  node->thenblock()->accept(this, lvl);
  if (node->elseblock())
    node->elseblock()->accept(this, lvl);
}

void og::exports_require::do_if_node(og::if_node *const node, int lvl) {
  node->block()->accept(this, lvl);
}

void og::exports_require::do_sequence_node(cdk::sequence_node *const node, int lvl) {
 for (size_t i = 0; i < node->size(); i++) {
      node->node(i)->accept(this, lvl);
  }
}

void og::exports_require::do_assignment_node(cdk::assignment_node * const node, int lvl) {
  node->lvalue()->accept(this, lvl);
  node->rvalue()->accept(this, lvl);
}

void og::exports_require::do_variable_declaration_node(og::variable_declaration_node *const node, int lvl) {
  if (node->expression())
    node->expression()->accept(this, lvl);
}

void og::exports_require::do_add_node(cdk::add_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::exports_require::do_and_node(cdk::and_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl); 
}

void og::exports_require::do_div_node(cdk::div_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}

void og::exports_require::do_data_node(cdk::data_node * const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_double_node(cdk::double_node * const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_eq_node(cdk::eq_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::exports_require::do_ge_node(cdk::ge_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::exports_require::do_gt_node(cdk::gt_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::exports_require::do_variable_node(cdk::variable_node * const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_integer_node(cdk::integer_node * const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_le_node(cdk::le_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::exports_require::do_lt_node(cdk::lt_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::exports_require::do_mod_node(cdk::mod_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::exports_require::do_mul_node(cdk::mul_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::exports_require::do_ne_node(cdk::ne_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::exports_require::do_neg_node(cdk::neg_node * const node, int lvl) {
  node->argument()->accept(this, lvl);
  
}
void og::exports_require::do_nil_node(cdk::nil_node * const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_not_node(cdk::not_node * const node, int lvl) {
  node->argument()->accept(this, lvl);
}
void og::exports_require::do_or_node(cdk::or_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::exports_require::do_rvalue_node(cdk::rvalue_node * const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_string_node(cdk::string_node * const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_sub_node(cdk::sub_node * const node, int lvl) {
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
}
void og::exports_require::do_evaluation_node(og::evaluation_node * const node, int lvl) {
  node->argument()->accept(this, lvl);
}
void og::exports_require::do_address_node(og::address_node * const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_function_call_node(og::function_call_node * const node, int lvl) {
  if (node->arguments())
    node->arguments()->accept(this, lvl);
}
void og::exports_require::do_index_node(og::index_node * const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_continue_node(og::continue_node * const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_nullptr_node(og::nullptr_node * const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_return_node(og::return_node * const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_stack_alloc_node(og::stack_alloc_node * const node, int lvl) {
  node->argument()->accept(this, lvl);
}
void og::exports_require::do_break_node(og::break_node * const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_identity_node(og::identity_node *const node, int lvl) {
  node->argument()->accept(this, lvl);
}
void og::exports_require::do_sizeof_node(og::sizeof_node *const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_tuple_index_node(og::tuple_index_node *const node, int lvl) {
  // EMPTY
}
void og::exports_require::do_tuple_node(og::tuple_node *const node, int lvl) {
  node->values()->accept(this, lvl);
}