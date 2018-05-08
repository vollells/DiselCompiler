Lab 4 : Type Checking and Semantic Analysis
===========================================

The aim of this lab is to perform a semantic analysis of the internal
form we built in the previous lab, especially type checking.

Checking static semantics
-------------------------

Static semantics concern things that a context-free grammar can’t
handle. Examples of this include checking that:

* Variables are declared this was actually already done for you in
  ``parser.y``. Ideally it should have been done in this phase, since that
  is where it conceptually belongs. Why isn’t it?

* Operands have compatible types.

* The number of arguments and their types at sub-routine calls are
  consistent with declarations.

* All functions return something.

In this way we try to follow the semantic rules that have been
stipulated for the language, for instance:

  *"If both operands of arithmetic operations such as addition, subtraction or multiplication are integers, the result should also be an integer."*

Type information
~~~~~~~~~~~~~~~~

Information for type checking comes from:

* the symbol table, where we store, for example, the type of a variable, a
  list of formal parameters for a procedure, what type a function returns,
  etc.

* synthesised attributes, e.g. for arithmetic expressions. As operands of
  arithmetic operations may be arbitrary arithmetic expressions, we must
  synthesise information on their types. Where possible, this is done in
  the AST class constructors, but there are places where the type can’t be
  known at node creation time, and in these cases it has to be done during
  this phase.

In DIESEL we have to store, for example, the type of arithmetic
expressions so that type checking can be performed on expressions of
the form
``(1 + 2) div (4 / 2)``\  (which actually results in a type conflict).
You can, of course, synthesise the
type information and throw it away when you have checked the
semantics, but then you may be forced to traverse the tree each time
you need to determine the type of an expression.

Type conversion
~~~~~~~~~~~~~~~

It may also be useful to allow the type checking to extend the AST. The
semantics in DIESEL allow us, for example, to add floating-point numbers
to integers. Since we did not concern ourselves with type checking while
constructing the AST, we are very likely to have partial trees like the
one shown on the left of the figure below, for example. The value in the
nodes’ type field has been written at each node. Even when doing type
checking we can see that we will have to change the integer in x to a
floating-point number before performing the addition. If we let the tree
stay as it is, we will have to take the addition’s type into
consideration when deciding how to generate quadruples for the
right-hand side child. Alternatively we have to know the types of the
*operands* when we generate quadruples for addition. To make quadruple
generation as simple as possible, we add a type translation node
(:class:`ast_cast`) which has a child of integer type but which is itself of
floating-point type. This is known as casting the integer number to real
type and is an example of type conversion. Since DIESEL is a rather
simple language with few data types, this is the only kind of type
conversion we will need to worry about. In a language which allows
pointers to objects (like C++), type casting and conversion can be a
whole science in itself. Anyway, after semantic analysis which includes
adding this node we get the tree on the right:

.. digraph :: diesel_lab4_casting
  :alt: Example of type casting.

  subgraph pre_cast {
    "pre+" [label="+\ntype=?"];
    "pre3.1" [label="3.1\ntype=real"];
    "prex" [label="x\ntype=integer"];
    "pre+" -> "pre3.1";
    "pre+" -> "prex";
  }
  subgraph post_cast {
    "post+" [label="+\ntype=real"];
    "post3.1" [label="3.1\ntype=real"];
    "postx" [label="x\ntype=integer"];
    ast_cast [label="ast_cast\ntype=real"];
    "post+" -> "post3.1";
    "post+" -> ast_cast;
    ast_cast -> "postx";
  }

Quadruple generation for the modified tree will thus be simpler. All
addition- and variable- nodes can now be translated to the same sequence
of quadruples regardless of the context.

Type specifications for DIESEL
------------------------------

Here is the grammar (in reverse order) for DIESEL, with a description of
which type restrictions apply for each production (Note that the extra
error productions you were supposed to add in the previous lab are not
included here). We perform type checking one block at a time, by use of
a recursive call :func:`~ast_node::type_check()` which is passed from the root
downwards in the AST representing the block. You will need to keep in
mind which AST structures these productions correspond to (you should
have gained a good understanding of that in the previous lab), and
implement type checking and synthesizing where appropriate. Keep a copy
of ``parser.y`` handy while reading this. The tokens generally appear as
their DIESEL symbols rather than their token names, e.g. ``(`` instead of
:c:macro:`T_LEFTPAR`, due to space considerations.

The numbers are just for ease of reference. ``bison`` notation actually
groups several right-hand sides together under one left-hand side.

#. ``<id>`` → :c:macro:`T_IDENT`

   If the identifier is not declared, an error message is issued. The
   semantic action for this production can be found in ``parser.y``, and
   has already been written. Ideally this check should take place at the
   same time as the rest of the type checking instead of at AST creation
   time. Why doesn’t it? The same comment applies for the following seven
   productions.

#. ``<array_id>`` → ``<id>``

   If the identifier is not declared as an *array*, an error message is
   issued.

#. ``<func_id>`` → ``<id>``

   If the identifier is not declared as a *function*, an error message is
   issued.

#. ``<proc_id>`` → ``<id>``

   If the identifier is not declared as a *procedure*, an error message
   is issued.

#. ``<rvar_id>`` → ``<id>``

   If the identifier is not declared as a *variable*, *parameter*, or
   *constant*, an error message is issued. This production signifies any
   variable-like identifier that can appear on the right-hand side of an
   assignment statement.

#. ``<lvar_id>`` → ``<id>``

   If the identifier is not declared as a *variable* or *parameter*, an
   error message is issued. This production signifies any variable-like
   identifier that can appear on the left-hand side of an assignment
   statement. Comparing it to the previous production we see that a
   constant can’t be assigned to, which makes sense. This duality is also
   reflected below in the productions for *lvariable* and *rvariable*,
   respectively.

#. ``<const_id>`` → ``<id>``

   If the identifier is not declared as a *constant*, an error message is
   issued.

#. ``<type_id>`` → ``<id>``

   If the identifier is not declared as a *name* type, an error message
   is issued.

#. ``<integer>`` → :c:macro:`T_INTNUM`

#. ``<real>`` → :c:macro:`T_REALNUM`

#. ``<func_call>`` → ``<func_id>`` ``(`` ``<opt_expr_list>`` ``)``

   The formal parameters (in the symbol table) must agree with the actual
   ones (in the ``opt_expr_list``), both in type and number. The result
   must be the same type as the function’s return type.

#. ``<factor>`` → ``(`` ``<expr>`` ``)``

#. ``<factor>`` → ``NOT`` ``<factor>``

   If the operand is not of integer type, an error message is issued. The
   result is of integer type.

#. ``<factor>`` → ``<real>``

#. ``<factor>`` → ``<integer>``

#. ``<factor>`` → ``<func_call>``

#. ``<factor>`` → ``<rvariable>``

#. ``<term>`` → ``<factor>``

#. ``<term>`` → ``<term>`` ``MOD`` ``<factor>``

   Both operands must be of integer type. The result is of integer type.

#. ``<term>`` → ``<term>`` ``DIV`` ``<factor>``

   Both operands must be of integer type. The result is of integer type.

#. ``<term>`` → ``<term>`` ``/`` ``<factor>``

   The result is of real type. Both operands must be of real type or be
   transformed to it.

#. ``<term>`` → ``<term>`` ``*`` ``<factor>``

   The result has the same type as both the operands, possibly after an
   operand has been transformed.

#. ``<term>`` → ``<term>`` ``AND`` ``<factor>``

   Both operands must be of integer type. The result is of integer type.

#. ``<simple_expr>`` → ``<term>``

#. ``<simple_expr>`` ``-`` ``<term>``

   This is unary minus, as opposed to binary minus below.

#. ``<simple_expr>`` → ``+`` ``<term>``

#. ``<simple_expr>`` → ``<simple_expr>`` ``-`` ``<term>``

   The result has the same type as both the operands, possibly after an
   operand has been transformed.

#. ``<simple_expr>`` → ``<simple_expr>`` ``+`` ``<term>``

   The result has the same type as both the operands, possibly after an
   operand has been transformed.

#. ``<simple_expr>`` → ``<simple_expr>`` ``OR`` ``<term>``

   Both operands must be of integer type. The result is of integer type.

#. ``<expr>`` → ``<simple_expr>``

#. ``<expr>`` → ``<expr>`` ``>`` ``<simple_expr>``

   The result is of integer type. Both operands have the same type,
   possibly after type transformation.

#. ``<expr>`` → ``<expr>`` ``<`` ``<simple_expr>``

   The result is of integer type. Both operands have the same type,
   possibly after type transformation.

#. ``<expr>`` → ``<expr>`` ``<>`` ``<simple_expr>``

   The result is of integer type. Both operands have the same type,
   possibly after type transformation.

#. ``<expr>`` → ``<expr>`` ``=`` ``<simple_expr>``

   The result is of integer type. Both operands have the same type,
   possibly after type transformation.

#. ``<expr_list>`` → ``<expr>``

#. ``<expr_list>`` → ``<expr_list>`` ``,`` ``<expr>``

#. ``<opt_expr_list>`` →

#. ``<opt_expr_list>`` → ``<expr_list>``

#. ``<else_part>`` →

#. ``<else_part>`` → ``ELSE`` ``<stmt_list>``

#. ``<elsif>`` → ``ELSIF`` ``<expr>`` ``THEN`` ``<stmt_list>``

   The predicate must be of integer type.

#. ``<elsif_list>`` →

#. ``<elsif_list>`` → ``<elsif_list>`` ``<elsif>``

#. ``<rvariable>`` → ``<rvar_id>``

#. ``<rvariable>`` → ``<array_id>  ``[`` ``<expr>`` ``]``

   If the type of ``<expr>`` is different from the index type of
   ``<array_id>``, an error message is issued.

#. ``<lvariable>`` → ``<lvar_id>``

#. ``<lvariable>`` → ``<array_id>`` ``[`` ``<expr>`` ``]``

   If the type of ``<expr>`` is different from the index type of
   ``<array_id>``, an error message is issued.

#. ``<stmt>`` →

#. ``<stmt>`` → ``RETURN``

   Can only be executed in procedures.

#. ``<stmt>`` → ``RETURN`` ``<expr>``

   Can only be executed in functions. The type of ``<expr>`` must agree
   with the type of the function.

#. ``<stmt>`` → ``<lvariable>`` ``:=`` ``<expr>``

   It is not allowed to assign a real expression to an integer variable.
   Real variables can, however, be assigned integer expressions after
   type transformation. Note the use of ``lvariable`` here.

#. ``<stmt>`` → ``<proc_id>`` ``(`` ``<opt_expr_list>`` ``)``

   All actual parameters must agree in type and number with the formal
   parameters.

#. ``<stmt>`` → ``WHILE`` ``<expr>`` ``DO`` ``<stmt_list>`` ``END``

   The predicate must be of integer type.

#. ``<stmt>`` → ``IF`` ``<expr>`` ``THEN`` ``<stmt_list>`` ``<elsif_list>`` ``<else_part>`` ``END``

   The predicate must be of integer type.

#. ``<stmt_list>`` → ``<stmt>``

#. ``<stmt_list>`` → ``<stmt_list>`` ``;`` ``<stmt>``

#. ``<comp_stmt>`` → ``BEGIN`` ``<stmt_list>`` ``END``

#. ``<param>`` → :c:macro:`T_IDENT` ``:`` ``<type_id>``

#. ``<parm_list>`` → ``<param>``

#. ``<parm_list>`` → ``<param_list>`` ``;`` ``<param>``

#. ``<opt_param_list>`` →

#. ``<opt_param_list>`` → ``(`` ``<param_list>`` ``)``

#. ``<func_head>`` → ``FUNCTION`` :c:macro:`T_IDENT`

#. ``<proc_head>`` → ``PROCEDURE`` :c:macro:`T_IDENT`

#. ``<func_decl>`` → ``<func_head>`` ``<opt_param_list>`` ``:`` ``<type_id>`` ``;`` ``<const_part>`` ``<variable_part>``

#. ``<proc_decl>`` → ``<proc_head>`` ``<opt_param_list>`` ``;`` ``<const_part>`` ``<variable_part>``

#. ``<subprog_decl>`` → ``<func_decl>`` ``<subprog_part>`` ``<comp_stmt>`` ``;``

   A function must have at least one RETURN statement. This particular
   check has already been written in the skeleton program. Take a look at
   it.

#. ``<subprog_decl>`` → ``<proc_decl>`` ``<subprog_part>`` ``<comp_stmt>`` ``;``

   When this production is reduced, we have created an AST for one block,
   i.e., it is time to start type checking. This is already written, but
   currently commented out, in ``parser.y``. Take a look.

#. ``<subprog_decls>`` → ``<subprog_decl>``

#. ``<subprog_decls>`` → ``<subprog_decls>`` ``<subprog_decl>``

#. ``<subprog_part>`` →

#. ``<subprog_part>`` → ``<subprog_decls>``

#. ``<var_decl>`` → :c:macro:`T_IDENT` ``:`` ``<type_id>`` ``;``

#. ``<var_decl>`` → :c:macro:`T_IDENT` ``:`` ``ARRAY`` ``[`` ``<integer>`` ``]`` ``OF`` ``<type_id>`` ``;``

#. ``<var_decl>`` → :c:macro:`T_IDENT` ``:`` ``ARRAY`` ``[`` ``<const_id>`` ``]`` ``OF`` ``<type_id>`` ``;``

   The index must be of integer type. This is already written for you.

#. ``<var_decls>`` → ``<var_decl>``

#. ``<var_decls>`` → ``<var_decls>`` ``<var_decl>``

#. ``<variable_part>`` →

#. ``<variable_part>`` → ``VAR`` ``<var_decls>``

#. ``<const_decl>`` → :c:macro:`T_IDENT` ``=`` ``<integer>`` ``;``

#. ``<const_decl>`` → :c:macro:`T_IDENT` ``=`` ``<real>`` ``;``

#. ``<const_decl>`` → :c:macro:`T_IDENT` ``=`` :c:macro:`T_STRINGCONST` ``;``

#. ``<const_decl>`` → :c:macro:`T_IDENT` ``=`` ``<const_id>`` ``;``

#. ``<const_decls>`` → ``<const_decl>``

#. ``<const_decls>`` → ``<const_decls>`` ``<const_decl>``

#. ``<const_part>`` →

#. ``<const_part>`` → ``CONST`` ``<const_decls>``

#. ``<proghead>`` → ``PROGRAM`` :c:macro:`T_IDENT`

#. ``<prog_decl>`` → ``<proghead>`` ``;`` ``<const_part>`` ``<variable_part>``

#. ``<program>`` → ``<prog_decl>`` ``<subprog_part>`` ``<comp_stmt>`` ``.``

Implementation
--------------

Your job is to implement type checking and semantic analysis by editing
the file ``semantic.cc`` (and, in the quite likely case of your adding
internal methods, the file ``semantic.hh`` as well).

In our implementation, type checking is done for one block at a time,
and is started by a call from ``parser.y`` in the appropriate production
to :func:`~semantic::do_typecheck()` in the semantic class. This call will perform some
block-specific initialisation, and then call the method :func:`~ast_node::type_check()`
in the root node of the AST we are checking. This call will propagate
down through the AST and as nodes are passed, their types are checked
and their attributes are synthesized. The order of traversal is
important, since you obviously can’t synthesize information about a node
until you have information about its children.

The semantic class is a standalone class which is really only used as a
wrapper for type checking. It has a global instantiation, called
:var:`type_checker`, which is declared in ``semantic.cc``. The actual
:func:`~ast_node::type_check()` methods are part of the AST nodes, but their
implementations lie in the ``semantic.cc`` file. This makes it easy to
modify the type checking implementation (by replacing this file with
another) without having to rewrite the entire AST class hierarchy.

Some of the methods have already been implemented; look at them to get
an idea of what you are supposed to do. Most of them you are to complete
yourself (look for ``Your code here`` comments). In some cases it might be
convenient to add new methods to treat AST nodes which are very similar.
If you do so, you will also need to add your methods to the
``semantic.hh`` file.

Files that you have to change
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

semantic.hh and semantic.cc
   contain type-checking code implementation for
   the AST nodes as well as the declaration and implementation of the
   semantic class. These are the files you’re going to edit in this lab.
   They deal with type checking, type synthesizing, and parameter
   checking.

Other files of interest
~~~~~~~~~~~~~~~~~~~~~~~

All these files are the same as in lab 3:

parser.y
   is the input file to ``bison``. This is the file you edited in the last
   lab; it contains the call to :func:`~semantic::do_typecheck()`.

ast.hh
   contains the definitions for the AST nodes.

ast.cc
   contains (part of) the implementations of the AST nodes.

optimize.hh and optimize.cc
   contain optimizing code. You won’t need to edit
   these files in this lab.

quads.hh and quads.cc
   contain quad generation code. You won’t need to
   edit these files in this lab.

codegen.hh and codegen.cc
   contain assembler generation code. You won’t need
   to edit these files until the last lab.

error.hh, error.cc, symtab.hh, symbol.cc, symtab.cc, and scanner.l
   use your versions from the earlier labs.

main.cc
   this is the compiler wrapper, parsing flags and the like.

Makefile
   and **diesel** use the same files as in the last lab.

Declarations and routines available in *ast.hh*, *semantic.hh*, and *semantic.cc*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The recursive :func:`~ast_node::type_check()` method common to all the AST classes takes
no arguments, and should return the type of the node it is called in.
The declarations can be found in ``ast.hh``. You will have to implement most
of them yourself.

The semantic class (currently) only contains two methods:

.. doxygenclass :: semantic
  :members: do_typecheck, check_parameters
  :no-link:

Program specification
~~~~~~~~~~~~~~~~~~~~~

#. Your program must synthesize type information correctly.

#. Your program must give error messages at semantic conflicts.

#. Your program must ensure that formal and actual parameters match.

#. Your program must make sure that functions return a value and that
   procedures do not.

#. Your program should extend the AST with type transformation nodes
   where appropriate.

#. Type checking should be done one block at a time. Apart from checking
   for undeclared or wrongly declared identifiers and the call to
   :func:`~semantic::do_typecheck()`,
   no type checking code is to reside in ``parser.y``.

Debugging help
~~~~~~~~~~~~~~

As before, you can send all AST nodes to ``cout``. The types of
expressions are written out in the printout. Look at it to make sure
your nodes are given the correct type and that you add cast nodes in the
right places.

How do I know it’s correct?
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Compare the AST before and after type checking has been performed. Study
the node types and the cast nodes that were added.

Reporting your work
-------------------

* For the demonstration, run the test program
  for the two test cases ``semtest1.d`` and  ``semtest2.d`` and discuss your code.
* Hand in your code as described in :ref:`handing-in-results`.

Example execution
-----------------

Running ``make lab4`` will take the sample input and compare it to the
sample output. You should test your code on more examples than this.

.. code-block:: bash

  ./diesel -b -p -f -y -a ../testpgm/semtest1.d

.. literalinclude:: ../../code/trace/semtest1.trace
  :language: none
  :caption: trace/semtest1.trace

.. code-block:: bash

  ./diesel -b -p -f -y -a ../testpgm/semtest2.d

.. literalinclude:: ../../code/trace/semtest2.trace
  :language: none
  :caption: trace/semtest2.trace

.. note ::

  Your ``col`` value might differ slightly from the ones shown above
  depending on how you implemented your column counter in your scanner.
