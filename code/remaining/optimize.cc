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
    fatal("Trying to optimize abstract class ast_binaryoperation.");
}

void ast_binaryrelation::optimize() {
    fatal("Trying to optimize abstract class ast_binaryrelation.");
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
        last_expr->optimize();
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

/* This convenience method is used to apply constant folding to all
   binary operations. It returns either the resulting optimized node or the
   original node if no optimization could be performed. */
ast_expression *ast_optimizer::fold_constants(ast_expression *node) {
    if (is_binop(node)){
        ast_binaryoperation *binop_node = node->get_ast_binaryoperation();
        ast_expression *left_node = fold_constants(binop_node->left);
        ast_expression *right_node= fold_constants(binop_node->right);
        int new_int = 0;

        if (left_node->get_ast_integer() && right_node->get_ast_integer()){
            int left_int  = left_node->get_ast_integer()->value;
            int right_int = right_node->get_ast_integer()->value;
            switch (node->tag) {
            case AST_ADD:
                new_int = left_int + right_int;
                break;
            case AST_SUB:
                new_int = left_int - right_int;
                break;
            case AST_OR:
                new_int = left_int || right_int;
                break;
            case AST_AND:
                new_int = left_int && right_int;
                break;
            case AST_MULT:
                new_int = left_int * right_int;
                break;
            case AST_IDIV:
                new_int = left_int / right_int;
                break;
            case AST_MOD:
                new_int = left_int % right_int;
                break;
            default:
                fatal("Huh?");
            }
        }
        return new ast_integer(node->pos, new_int);
    } else if (node->tag == AST_ID) {
        auto *id = node->get_ast_id();
        auto *symbol = sym_tab->get_symbol(id->sym_p);
        if (symbol->tag == SYM_CONST) {
            auto *const_symbol = symbol->get_constant_symbol();
            if (const_symbol->type == integer_type) {
                return new ast_integer(node->pos, const_symbol->const_value.ival);
            }
        }
    }
    return node;
}

/* All the binary operations should already have been detected in their parent
   nodes, so we don't need to do anything at all here. */
void ast_add::optimize() {
}

void ast_sub::optimize() {
}

void ast_mult::optimize() {
}

void ast_divide::optimize() {
}

void ast_or::optimize() {
    /* Your code here */
}

void ast_and::optimize() {
    /* Your code here */
}

void ast_idiv::optimize() {
    /* Your code here */
}

void ast_mod::optimize() {
    /* Your code here */
}

/* We can apply constant folding to binary relations as well. */
void ast_equal::optimize() {
    /* Your code here */
}

void ast_notequal::optimize() {
    /* Your code here */
}

void ast_lessthan::optimize() {
    /* Your code here */
}

void ast_greaterthan::optimize() {
    /* Your code here */
}

/*** The various classes derived from ast_statement. ***/

void ast_procedurecall::optimize() {
    /* Your code here */
}

void ast_assign::optimize() {
    rhs = optimizer->fold_constants(rhs);
}

void ast_while::optimize() {
}

void ast_if::optimize() {
}

void ast_return::optimize() {
}

void ast_functioncall::optimize() {
    /* Your code here */
}

void ast_uminus::optimize() {
    /* Your code here */
}

void ast_not::optimize() {
    /* Your code here */
}

void ast_elsif::optimize() {
    /* Your code here */
}

void ast_integer::optimize() {
}

void ast_real::optimize() {
}

/* Note: See the comment in fold_constants() about casts and folding. */
void ast_cast::optimize() {
    /* Your code here */
}

void ast_procedurehead::optimize() {
    fatal("Trying to call ast_procedurehead::optimize()");
}

void ast_functionhead::optimize() {
    fatal("Trying to call ast_functionhead::optimize()");
}
