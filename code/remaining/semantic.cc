#include "semantic.hh"

semantic *type_checker = new semantic();

/* Used to check that all functions contain return statements.
   Static means that it is only visible inside this file.
   It is set to false in do_typecheck() (ie, every time we start type checking
   a new block) and set to true if we find an ast_return node. See below. */
static bool has_return = false;

/* Interface for type checking a block of code represented as an AST node. */
void semantic::do_typecheck(symbol *env, ast_stmt_list *body) {
    // Reset the variable, since we're checking a new block of code.
    has_return = false;
    if (body) {
        body->type_check();
    }

    // This is the only case we need this variable for - a function lacking
    // a return statement. All other cases are already handled in
    // ast_return::type_check(); see below.
    if (env->tag == SYM_FUNC && !has_return) {
        // Note: We could do this by overloading the do_typecheck() method -
        // one for ast_procedurehead and one for ast_functionhead, but this
        // will do... Hopefully people won't write empty functions often,
        // since in that case we won't have position information available.
        if (body != NULL) {
            type_error(body->pos) << "A function must return a value.\n";
        } else {
            type_error() << "A function must return a value.\n";
        }
    }
}

/* Compare formal vs. actual parameters recursively. */
bool semantic::chk_param(ast_id *env,
                         parameter_symbol *formals,
                         ast_expr_list *actuals) {
    if (formals && actuals) {
        if (formals->type == actuals->last_expr->type_check()) {
            chk_param(env, formals->preceding, actuals->preceding);
        } else {
            type_error(actuals->pos) << "Types not matching" << endl;
        }
    } else if (formals || actuals) {
        type_error(env->pos) << "Mismatched arity" << endl;
    }
    return false; // :/
}

/* Check formal vs. actual parameters at procedure/function calls. */
void semantic::check_parameters(ast_id *call_id,
                                ast_expr_list *param_list) {
    symbol *new_symbol = sym_tab->get_symbol(call_id->sym_p);

    if (new_symbol->tag == SYM_FUNC) {
        chk_param(call_id, new_symbol->get_function_symbol()->last_parameter, param_list);
    } else if (new_symbol->tag == SYM_PROC) {
        chk_param(call_id, new_symbol->get_procedure_symbol()->last_parameter, param_list);
    } else {
        type_error(call_id->pos) << "Can only call func or proc" << endl;
    };
}

/* We overload this method for the various ast_node subclasses that can
   appear in the AST. By use of virtual (dynamic) methods, we ensure that
   the correct method is invoked even if the pointers in the AST refer to
   one of the abstract classes such as ast_expression or ast_statement. */
sym_index ast_node::type_check() {
    fatal("Trying to type check abstract class ast_node.");
    return void_type;
}

sym_index ast_statement::type_check() {
    fatal("Trying to type check abstract class ast_statement.");
    return void_type;
}

sym_index ast_expression::type_check() {
    fatal("Trying to type check abstract class ast_expression.");
    return void_type;
}

sym_index ast_lvalue::type_check() {
    fatal("Trying to type check abstract class ast_lvalue.");
    return void_type;
}

sym_index ast_binaryoperation::type_check() {
    fatal("Trying to type check abstract class ast_binaryoperation.");
    return void_type;
}

sym_index ast_binaryrelation::type_check() {
    fatal("Trying to type check abstract class ast_binaryrelation.");
    return void_type;
}

/* Type check a list of statements. */
sym_index ast_stmt_list::type_check() {
    if (preceding != NULL) {
        preceding->type_check();
    }
    if (last_stmt != NULL) {
        last_stmt->type_check();
    }
    return void_type;
}

/* Type check a list of expressions. */
sym_index ast_expr_list::type_check() {
    if (preceding) {
        preceding->type_check();
    }
    last_expr->type_check();
    return void_type;
}

/* Type check an elsif list. */
sym_index ast_elsif_list::type_check() {
    if (preceding != NULL && preceding->type_check() != void_type) {
        type_error(preceding->pos) << "Not an elsif node or null in preceding" << endl;
    }
    last_elsif->type_check();
    return void_type;
}

/* "type check" an indentifier. We need to separate nametypes from other types
   here, since all nametypes are of type void, but should return an index to
   itself in the symbol table as far as typechecking is concerned. */
sym_index ast_id::type_check() {
    if (sym_tab->get_symbol(sym_p)->tag != SYM_NAMETYPE) {
        return type;
    }
    return sym_p;
}

sym_index ast_indexed::type_check() {
    auto index_type = index->type_check();
    if (index_type != integer_type) {
        type_error(index->pos) << "Index has to be of type integer" << endl;
    }
    if (sym_tab->get_symbol(id->sym_p)->tag != SYM_ARRAY) {
        type_error(id->pos) << "Can only index into arrays" << endl;
    }
    return id->type_check();
}

/* This convenience function is used to type check all binary operations
   in which implicit casting of integer to real is done: plus, minus,
   multiplication. We synthesize type information as well. */
sym_index check_binop_with_cast(ast_binaryoperation *node, string op) {
    // TODO(ed): Do some casting?
    auto left = node->left->type_check();
    auto right = node->right->type_check();
    if (left != integer_type && right != real_type) {
        type_error(node->left->pos) << "Left operand is not a number-type (" << op << ")" << endl;
    }
    if (right != integer_type && right != real_type) {
        type_error(node->right->pos) << "Right operand is not a number-type (" << op << ")" << endl;
    }
    auto ret = void_type;
    if (left == right) {
        ret = left;
    } else if (left == integer_type) {
        node->left = new ast_cast(node->left->pos, node->left);
        node->left->type = real_type;
        ret = real_type;
    } else if (right == integer_type) {
        node->right = new ast_cast(node->right->pos, node->right);
        node->right->type = real_type;
        ret = real_type;
    } else {
        type_error(node->pos) << "Edvard's logic is fallable" << endl;
    }
    node->type = ret;
    return ret;
}

sym_index ast_add::type_check() {
    return check_binop_with_cast(this, "+");
}

sym_index ast_sub::type_check() {
    return check_binop_with_cast(this, "-");
}

sym_index ast_mult::type_check() {
    return check_binop_with_cast(this, "*");
}

/* Divide is a special case, since it always returns real. We make sure the
   operands are cast to real too as needed. */
sym_index ast_divide::type_check() {
    auto node = this;
    auto op = "/";
    auto left = node->left->type_check();
    auto right = node->right->type_check();
    if (left != integer_type && right != real_type) {
        type_error(node->left->pos) << "Left operand is not a number-type (" << op << ")" << endl;
    }
    if (right != integer_type && right != real_type) {
        type_error(node->right->pos) << "Right operand is not a number-type (" << op << ")" << endl;
    }
    if (left == integer_type) {
        node->left = new ast_cast(node->left->pos, node->left);
    }
    if (right == integer_type) {
        node->right = new ast_cast(node->right->pos, node->right);
    }
    type = real_type;
    return type;
}

/* This convenience method is used to type check all binary operations
   which only accept integer operands: AND, OR, MOD, DIV.
   The second argument is the name of the operator, so we can generate a
   good error message.
   All of these return integers, so we synthesize that.
   */
sym_index check_binop_int(ast_binaryoperation *node, string op) {
    auto left = node->left->type_check();
    auto right = node->right->type_check();
    if (left != integer_type) {
        type_error(node->left->pos) << "Left operand is not a number-type (" << op << ")" << endl;
    }
    if (right != integer_type) {
        type_error(node->left->pos) << "Left operand is not a number-type (" << op << ")" << endl;
    }
    node->type = integer_type;
    return node->type;
}

sym_index ast_or::type_check() {
    return check_binop_int(this, "OR");
}

sym_index ast_and::type_check() {
    return check_binop_int(this, "AND");
}

sym_index ast_idiv::type_check() {
    return check_binop_int(this, "IDIV");
}

sym_index ast_mod::type_check() {
    return check_binop_int(this, "MOD");
}

/* Convienience method for all binary relations, since they're all typechecked
   the same way. They all return integer types, 1 = true, 0 = false. */
sym_index check_binrel(ast_binaryrelation *node, string op) {
    // Maybe question your inheritance here...
    auto dummy_node = new ast_add(node->pos, node->left, node->right);
    check_binop_with_cast(dummy_node, op);
    node->left = dummy_node->left;
    node->right = dummy_node->right;
    node->type = integer_type;
    return node->type;
}

sym_index ast_equal::type_check() {
    return check_binrel(this, "=");
}

sym_index ast_notequal::type_check() {
    return check_binrel(this, "!=");
}

sym_index ast_lessthan::type_check() {
    return check_binrel(this, "<");
}

sym_index ast_greaterthan::type_check() {
    return check_binrel(this, ">");
}

/*** The various classes derived from ast_statement. ***/

sym_index ast_procedurecall::type_check() {
    type_checker->check_parameters(id, parameter_list);
    return void_type;
}

sym_index ast_assign::type_check() {
    auto lhs_type = lhs->type_check();
    auto rhs_type = rhs->type_check();
    if (lhs_type == real_type && rhs_type == integer_type) {
        rhs = new ast_cast(pos, rhs);
        rhs->type = real_type;
    } else if (lhs_type != rhs_type) {
        type_error(pos) << "Cannot assign to different type (except - int := real)" << endl;
    }
    return void_type;
}

sym_index ast_while::type_check() {
    if (condition->type_check() != integer_type) {
        type_error(condition->pos) << "while predicate must be of integer type" << endl;
    }

    if (body != NULL) {
        body->type_check();
    }
    return void_type;
}

sym_index ast_if::type_check() {
    if (condition->type_check() != integer_type) {
        type_error(condition->pos) << "Not an integer vaule in if" << endl;
    }
    body->last_stmt->type_check();
    if (elsif_list != NULL) {
        elsif_list->type_check();
    }
    if (else_body != NULL) {
        else_body->type_check();
    }

    return void_type;
}

sym_index ast_return::type_check() {
    // This static global (meaning it is global for all methods in this file,
    // but not visible outside this file) variable is reset to 0 every time
    // we start type checking a new block of code. If we find a return
    // statement, we set it to 1. It is checked in the do_typecheck() method
    // of semantic.cc.
    has_return = true;

    // Get the current environment. We don't yet know if it's a procedure or
    // a function.
    symbol *tmp = sym_tab->get_symbol(sym_tab->current_environment());
    if (value == NULL) {
        // If the return value is NULL,
        if (tmp->tag != SYM_PROC)
        // ...and we're not inside a procedure, something is wrong.
        {
            type_error(pos) << "Must return a value from a function.\n";
        }
        return void_type;
    }

    sym_index value_type = value->type_check();

    // The return value is not NULL,
    if (tmp->tag != SYM_FUNC) {
        // ...so if we're not inside a function, something is wrong too.
        type_error(pos) << "Procedures may not return a value.\n";
        return void_type;
    }

    // Now we know it's a function and can safely downcast.
    function_symbol *func = tmp->get_function_symbol();

    // Must make sure that the return type matches the function's
    // declared return type.
    if (func->type != value_type) {
        type_error(value->pos) << "Bad return type from function.\n";
    }

    return void_type;
}

sym_index ast_functioncall::type_check() {
    type_checker->check_parameters(id, parameter_list);
    return void_type;
}

sym_index ast_uminus::type_check() {
    type = expr->type_check();
    if (type != integer_type && type != real_type) {
        type_error(expr->pos) << "Not an integer or real in uminus" << endl;
        return void_type;
    }
    return type;
}

sym_index ast_not::type_check() {
    if (expr->type_check() != integer_type) {
        type_error(expr->pos) << "Not an integer in a boolean not" << endl;
    }
    type = integer_type;
    return type;
}

sym_index ast_elsif::type_check() {
    if (condition->type_check() != integer_type) {
        type_error(condition->pos) << "Not an integer vaule in if" << endl;
    }
    body->type_check();
    return void_type;
}

sym_index ast_integer::type_check() {
    type = integer_type;
    return integer_type;
}

sym_index ast_real::type_check() {
    type = real_type;
    return real_type;
}
