#include "optimize.hh"

/*** This file contains all code pertaining to AST optimisation. It currently
     implements a simple optimisation called "constant folding". Most of the
     methods in this file are empty, or just relay optimize calls downward
     in the AST. If a more powerful AST optimization scheme were to be
     implemented, only methods in this file should need to be changed. ***/

ast_optimizer *optimizer = new ast_optimizer();

/* The optimizer's interface method. Starts a recursive optimize call down
   the AST nodes, searching for binary operators with constant children. */
void ast_optimizer::do_optimize(ast_stmt_list *body) {
    if (body != NULL) {
        body->optimize();
    }
}

/* Returns 1 if an AST expression is a subclass of ast_binaryoperation,
   ie, eligible for constant folding. */
bool ast_optimizer::is_binop(ast_expression *node) {
    switch (node->tag) {
    case AST_ADD:
    case AST_SUB:
    case AST_OR:
    case AST_AND:
    case AST_MULT:
    case AST_DIVIDE:
    case AST_IDIV:
    case AST_MOD:
        return true;
    default:
        return false;
    }
}

bool ast_optimizer::is_condop(ast_expression *node) {
    switch (node->tag) {
    case AST_LESSTHAN:
    case AST_GREATERTHAN:
    case AST_EQUAL:
    case AST_NOTEQUAL:
        return true;
    default:
        return false;
    }
}

/* We overload this method for the various ast_node subclasses that can
   appear in the AST. By use of virtual (dynamic) methods, we ensure that
   the correct method is invoked even if the pointers in the AST refer to
   one of the abstract classes such as ast_expression or ast_statement. */
void ast_node::optimize() {
    fatal("Trying to optimize abstract class ast_node.");
}

void ast_statement::optimize() {
    fatal("Trying to optimize abstract class ast_statement.");
}

void ast_expression::optimize() {
    fatal("Trying to optimize abstract class ast_expression.");
}

void ast_lvalue::optimize() {
    fatal("Trying to optimize abstract class ast_lvalue.");
}

void ast_binaryoperation::optimize() {
    // fatal("Trying to optimize abstract class ast_binaryoperation.");
}

/*** The optimize methods for the concrete AST classes. ***/

/* Optimize a statement list. */
void ast_stmt_list::optimize() {
    if (preceding != NULL) {
        preceding->optimize();
    }
    if (last_stmt != NULL) {
        last_stmt->optimize();
    }
}

/* Optimize a list of expressions. */
void ast_expr_list::optimize() {
    if (preceding != NULL) {
        preceding->optimize();
    }
    if (last_expr != NULL) {
        last_expr = optimizer->fold_constants(last_expr);
    }
}

/* Optimize an elsif list. */
void ast_elsif_list::optimize() {
    if (preceding != NULL) {
        preceding->optimize();
    }
    if (last_elsif != NULL) {
        last_elsif->optimize();
    }
}

/* An identifier's value can change at run-time, so we can't perform
   constant folding optimization on it unless it is a constant.
   Thus we just do nothing here. It can be treated in the fold_constants()
   method, however. */
void ast_id::optimize() {
}

void ast_indexed::optimize() {
    index->optimize();
}

template <typename T>
T do_binop(T left, T right, ast_node_type tag) {
    switch (tag) {
    case AST_ADD:
        return left + right;
    case AST_SUB:
        return left - right;
    case AST_OR:
        return (T)((bool)left) || ((bool)right);
    case AST_AND:
        return (T)((bool)left) && ((bool)right);
    case AST_MULT:
        return left * right;
    case AST_IDIV:
        return left / right;
    case AST_MOD:
        return ((long)left) % ((long)right);
    case AST_LESSTHAN:
        return left < right;
    case AST_GREATERTHAN:
        return left > right;
    case AST_EQUAL:
        return left == right;
    case AST_NOTEQUAL:
        return left != right;
    default:
        fatal("Huh?");
        return left;
    }
}

template <typename T>
ast_expression *fold_constants_binaryop(ast_expression *node) {
    T *binop_node = dynamic_cast<T *>(node);
    if (!binop_node) {
        fatal("Failed to cast - check the optimization function");
    }
    ast_expression *left_node = optimizer->fold_constants(binop_node->left);
    ast_expression *right_node = optimizer->fold_constants(binop_node->right);
    binop_node->left = left_node;
    binop_node->right = right_node;

    if (left_node->get_ast_integer() && right_node->get_ast_integer()) {
        int left = left_node->get_ast_integer()->value;
        int right = right_node->get_ast_integer()->value;
        int ret = do_binop(left, right, node->tag);
        return new ast_integer(node->pos, ret);
    }
    if (left_node->get_ast_real() && right_node->get_ast_real()) {
        double left = left_node->get_ast_real()->value;
        double right = right_node->get_ast_real()->value;
        double ret = do_binop(left, right, node->tag);
        return new ast_real(node->pos, ret);
    }
    // TODO[et]: Do cast?
    return node;
}

/* This convenience method is used to apply constant folding to all
   binary operations. It returns either the resulting optimized node or the
   original node if no optimization could be performed. */
ast_expression *ast_optimizer::fold_constants(ast_expression *node) {
    if (is_binop(node)) {
        return fold_constants_binaryop<ast_binaryoperation>(node);
    } else if (is_condop(node)) {
        return fold_constants_binaryop<ast_binaryrelation>(node);
    } else if (node->tag == AST_ID) {
        auto *id = node->get_ast_id();
        auto *symbol = sym_tab->get_symbol(id->sym_p);
        if (symbol->tag == SYM_CONST) {
            auto *const_symbol = symbol->get_constant_symbol();
            if (const_symbol->type == integer_type) {
                return new ast_integer(node->pos, const_symbol->const_value.ival);
            }
            if (const_symbol->type == real_type) {
                return new ast_real(node->pos, const_symbol->const_value.rval);
            }
        }
    } else if (node->tag == AST_UMINUS) {
        auto *new_node = dynamic_cast<ast_uminus *>(node);
        new_node->expr = optimizer->fold_constants(new_node->expr);
        if (new_node->get_ast_integer()) {
            return new ast_integer(new_node->pos, -new_node->get_ast_integer()->value);
        }
        if (new_node->get_ast_real()) {
            return new ast_real(new_node->pos, -new_node->get_ast_real()->value);
        }
    } else if (node->tag == AST_NOT) {
        auto *new_node = dynamic_cast<ast_not *>(node);
        new_node->expr = optimizer->fold_constants(new_node->expr);
        if (new_node->get_ast_integer()) {
            return new ast_integer(new_node->pos, !new_node->get_ast_integer()->value);
        }
    } else if (node->tag == AST_FUNCTIONCALL) {
        node->optimize();
    }
    return node;
}

void ast_binaryrelation::optimize() {
    left = optimizer->fold_constants(left);
    right = optimizer->fold_constants(right);
}

/*** The various classes derived from ast_statement. ***/

void ast_procedurecall::optimize() {
    parameter_list->optimize();
}

void ast_assign::optimize() {
    rhs = optimizer->fold_constants(rhs);
}

void ast_while::optimize() {
    condition->optimize();
    body->optimize();
}

void ast_if::optimize() {
    condition = optimizer->fold_constants(condition);
    body->optimize();
    if (elsif_list) {
        elsif_list->optimize();
    }
    if (else_body) {
        else_body->optimize();
    }
}

void ast_return::optimize() {
    value = optimizer->fold_constants(value);
}

void ast_functioncall::optimize() {
    parameter_list->optimize();
}

void ast_uminus::optimize() {
}

void ast_not::optimize() {
}

void ast_elsif::optimize() {
    condition = optimizer->fold_constants(condition);
    if (body) {
        body->optimize();
    }
}

void ast_integer::optimize() {}

void ast_real::optimize() {}

/* Note: See the comment in fold_constants() about casts and folding. */
void ast_cast::optimize() {
    expr = optimizer->fold_constants(expr);
}

void ast_procedurehead::optimize() {
    fatal("Trying to call ast_procedurehead::optimize()");
}

void ast_functionhead::optimize() {
    fatal("Trying to call ast_functionhead::optimize()");
}
