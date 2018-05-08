Overview of the Compiler
========================

Introduction
------------

The compiler takes a DIESEL program as input and produces an Intel
assembler file. This is done in six phases:

#. Scanner – manages lexical analysis.

#. Parser – manages syntactic analysis, builds internal form.

#. Semantics – checks static semantics.

#. Optimization – optimizes the internal form.

#. Quads – generates quadruples from the internal form

#. Codegen – expands quadruples to assembler.

All these phases use a symbol manager, the *symbol table*, where
information is gathered on *variable names* and *memory requirements*,
*function parameters*, etc.

While reading the description of the phases below, it is worthwhile to
study the appropriate source code file. Each step above, plus the symbol
table, is made up of two files - one header file (with a ``.hh`` suffix)
containing class declarations, and a code file (with a ``.cc`` suffix)
containing the method bodies. The exceptions are the scanner, which is
contained in a file named ``scanner.l``, and the parser, which is
contained in a file named ``parser.y``. These files are input files to
the compiler-generating tools ``flex`` and ``bison``, respectively. In
some cases the implementation for a header file is spread out over
several ``.cc`` files, most notably in the case of abstract syntax
trees. This will be pointed out in the text where needed.

The scanner
-----------

While it is an interesting exercise to hand-write a scanner, it is not
very complicated, and rather tedious. There exist several useful tools
to help automate this process. In this lab, you will be using ``flex``,
which takes as input a file (``scanner.l``) containing regular
expressions, and which as output generates a C file which implements a
scanner recognizing strings described by the regular expressions you
declared.

To be able to complete :ref:`lab1`, you will have to learn how to use the
``flex`` tool. A complete `manual <_static/flex/index.html>`_ is included
in the instructions; here we will only give a very brief overview of the tool does.

Every time it is called, the scanner reads characters from the terminal
until it can return a token to the parser. A token is simply an integer
representing a certain terminal symbol. The next token [1]_ is read by
calling the scanner’s ``yylex()`` procedure. Some tokens require
additional attributes to be stored (for later use by the parser), such
as the value of an integer or real number, or the string table index of
an identifier. The variable :var:`yylval` is used for this. To be able to
generate good error messages, the line and column number for each token
read is stored in the :var:`yylloc` variable. The :var:`yylval` variable is of
type :type:`YYSTYPE`, a union of all allowed token attributes, while
:var:`yylloc` is of type :type:`YYLTYPE`, a struct of position data. A list of
all tokens in DIESEL, as well as definitions of the above-mentioned
types, are contained in the file ``scanner.hh`` which you will use in
lab 1 and 2.

Example: Assume we have the following DIESEL fragment:

::

      program main;
      const version = 'version 1.1';
      var x : integer;
      begin
         x := 0;

shows the token values returned by ``yylex()`` as this code fragment is
fed to the scanner, and also what the :var:`yylval` variable is set to for
each call to ``yylex()``.

.. table:: Obtained tokens.

   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_PROGRAM`                          |         |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_IDENT`                            | MAIN    |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_SEMICOLON`                        |         |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_CONST`                            |         |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_IDENT`                            | VERSION |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_EQ`                               |         |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_STRINGCONST`                      |         |   | version 1.1 |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_SEMICOLON`                        |         |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_VAR`                              |         |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_IDENT`                            | X       |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_COLON`                            |         |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_IDENT`                            | INTEGER |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_SEMICOLON`                        |         |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_BEGIN`                            |         |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_IDENT`                            | X       |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_ASSIGN`                           |         |   |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_INTNUM`                           |         | 0 |             |
   +-------------------------------------------------+---------+---+-------------+
   |   :c:macro:`T_SEMICOLON`                        |         |   |             |
   +-------------------------------------------------+---------+---+-------------+

The parser
----------

The tokens that the ``scanner`` produces form the input to the
``parser``.

The ``parser``, which is of the `LR` type, checks whether the
program is syntactically correct. The syntax is described using a
context-free grammar which for DIESEL consists of a number of
productions of which the four first are:

.. code-block :: bnf

      <program>    ::= <prog_decl> <subprog_part> <comp_stmt> ’.’ ;
      <prog_decl>  ::= <proghead> ’;’ <const_part> <variable_part> ;
      <proghead>   ::= ’PROGRAM’ ’IDENT’ ;
      <const_part> ::= ’CONST’ <const_decls>
                    |                   ;

To complete this lab, you will have to learn how to use the parser
generator tool ``bison``. A complete manual can be found on the course
home page. We will only give a brief overview of how it works here.

In this lab, the grammar is fed into ``bison``, a so-called *parser
generator* which generates C code from the grammar input file,
``parser.y``. The parser is, essentially, a function which loops through
the stream of tokens it gets as input, from the scanner, until it
encounters fatal errors or an ``end-of-file`` token is read. The aim is
to reduce the flow of tokens to the start symbol ``<program>``. If this
is achieved, the program is syntactically correct. In this lab, you will
be given the complete grammar. However, you will have to add *semantic
actions* to the input file (more about this in the semantic lab). It is
also your job to try to improve the parser’s error handling, which is
rather non-existing in the bare skeleton grammar file.

Reducing tokens to the start symbol is, of course, not accomplished in
one step. Example: for row ::

  x := 0

and the following reductions:

.. code-block :: bnf

      1. <stmt_list>   ::= <stmt>
      2. <stmt>        ::= <variable> ’:=’ <expr>
      3. <variable>    ::= <var_id>
      4. <var_id>      ::= <id>
      5. <id>          ::= T_IDENT
      6. <expr>        ::= <simple_expr>
      7. <simple_expr> ::= <term>
      8. <term>        ::= <factor>
      9. <factor>      ::= T_INTNUM

we get the following sequence of reductions:

.. code-block :: none

      x := 0 <=                       (production 5)
             rm
      <id> := 0 <=                    (production 4)
                rm
      <var_id> := 0 <=                (production 3)
                    rm
      <variable> := 0 <=              (production 9)
                       rm
      <variable> := <factor> <=       (production 8)
                              rm
      <variable> := <term> <=         (production 7)
                           rm
      <variable> := <simple_expr> <=  (production 6)
                                   rm
      <variable> := <expr> <=         (production 2)
                           rm
      <stmt> <=                       (production 1)
             rm
      <stmt_list>

Before a reduction can take place, the whole of the right-hand side must
be completed, e.g. for:

.. code-block :: bnf

      2. <stmt> ::= <variable> ’:=’ <expr>

``<variable>`` must first have been reduced, ``:=`` scanned and
``<expr>`` reduced before you can reduce to ``<stmt>``. The right-hand
side is stored in a parse stack, which bison will keep track of for you,
until a reduction can be performed.

Each time a production is reduced, the right-hand side is replaced on
the parse stack by the left-hand side and a semantic action is
performed. See the file ``parser.y`` for how this is done.

Semantics
---------

The majority of productions have a semantic action belonging to them. A
semantic action is nothing more than a few lines of code that are
executed when the parser reduces with the associated production. Note
that a compiler can work in one or several phases. The one we are
writing here will only have one phase, which means that we will not have
to write any intermediary forms to file. However, certain things are not
done every time a production is reduced, but rather every time a block
(e.g., a function, procedure or program) has been reduced successfully.
These things include type checking, optimization, parameter checking,
quad generation and code generation.

The semantic actions
~~~~~~~~~~~~~~~~~~~~

-  update the symbol table. This occurs, of course, only when we parse
   declarations and in the lab will be realised as calls to a family of
   routines ( :func:`~symbol_table::enter_variable()`, :func:`~symbol_table::enter_array()`,
   :func:`~symbol_table::enter_function()`, :func:`~symbol_table::enter_procedure()` etc.) with suitable
   parameters. This is an important distinction: we first parse
   declarations and collect information about the current code block,
   and then we start building an internal form using that information.

-  check static semantics. for example checks that the number of
   parameters at a procedure call is correct or that the operands of an
   integer division really are integers. This is done every time we are
   done parsing a whole block of code, by recursive calls propagating
   down through the abstract syntax tree representing the current
   program block. We will be consulting the symbol table heavily during
   this step, using the information we entered while parsing the block.

-  construct an internal form of the program. We have chosen *abstract
   syntax trees (AST)* as the internal form where the leaves are
   identifiers or numbers and the nodes operators, e.g. arithmetic
   operations or control structures. Each block is represented as an
   AST. Note that the tree only is made up of the procedure or function
   *body*, not the declarations. For example,

   ::

         procedure goofy;
         var
            x : integer;
         begin
            while (x > 0) do
               x := x - 1;
            end;
         end;


   has the abstract syntax tree representation from :

   .. digraph :: diesel_2_ast_while
     :alt: AST for *while* instruction
     :align: center

     node [fontname="Courier New"];
     x1 [label="x"];
     x2 [label="x"];
     x3 [label="x"];

     WHILE -> ">";
     ">" -> x1;
     ">" -> 0;

     WHILE -> ":=";
     ":=" -> x2;
     ":=" -> "-";
     "-" -> x3;
     "-" -> 1;

   Each terminal and non-terminal on the parse stack has an *attribute*
   which the semantic actions can utilise. The attributes that each tree
   node has vary with the node type. In this lab course, each node type
   is a class of its own, i.e, we are using *abstract syntax classes*.
   The complete declarations for these classes can be found in the file
   ``ast.hh``. For a terminal symbol, the attributes are the contents of
   the variable :var:`yylval` (see the Scanner section). For non-terminals,
   the attributes are pointers to other abstract syntax trees which have
   been built during previous reductions. A semantic action for the
   production:

   .. code-block :: bnf

         <stmt> ::= T_WHILE <expr> T_DO <stmt_list> T_END


   is to create the tree shown in the diagram above.
   This is a rather trivial operation as the children of the ``WHILE``-node are available as attributes of ``<expr>`` and ``<stmt_list>``.
   The beauty of using abstract syntax classes is that when we reduce and want to create a new node, we simply pass on the various attributes on the right-hand side as parameters to the new node’s constructor.

In conclusion we can say that when we parse procedure/function:

-  *declarations* – we update the symbol table

-  *statements* – we construct an abstract syntax tree., and when the
   whole procedure/function is reduced, we check static semantics.

Optimization
------------

After we have built an abstract syntax tree for a program block, we have
an excellent opportunity to perform some optimization on it.
Optimization is a whole field in itself, and we will only implement a
very simple type in this lab, called *constant folding*. This means that
we try to identify expressions on the form *constant operator constant*
(e.g., ``4 + 5``), and evaluate them at compile time, replacing the
operator node in the AST with the result (the integer ``9``, in our
example). This takes place after type checking has been done on the
block, and will be implemented pretty much the same way – through
recursive calls propagating downwards in the AST. A more graphic example
is shown in :numref:`overview_constant_folding`.

.. digraph :: overview_constant_folding
  :alt: Simple example of constant folding.
  :caption: Simple example of constant folding ``x := 3 - 1`` to ``x := 2``.
  :align: center
  :name: overview_constant_folding

  node [fontname="Courier New"];

  "x1" [label="x"];
  "x2" [label="x"];
  ":=1" [label=":="];
  ":=2" [label=":="];

  subgraph cluster_before {
    label=< <i>Before constant folding</i> >;
    penwidth = 0;

    ":=1" -> "x1";
    ":=1" -> "-";
    "-" -> 3;
    "-" -> 1;
  }

  subgraph cluster_after {
    label=< <i>After constant folding</i> >;
    penwidth = 0;

    ":=2" -> "x2";
    ":=2" -> 2;
  }

Quadruples
----------

As we’ve seen above, for each function- or procedure block an abstract syntax tree is constructed.
After it has been type checked and optimized, it is traversed once more, this time in order to generate a list of quadruples.
All in all there are about forty different quadruples – see the file ``quads.hh``.
As we have to break down structured programming (``if``, ``while`` etc.) to linear code, we must introduce jumps and labels.
Thus there will be quadruples which serve only as labels.
Later when we expand quadruples to assembler code, they will be translated to assembler labels.

In the symbol table manager there is a function :func:`~symbol_table::get_next_label()`
which returns unique labels. The label number for a function/procedure
block is stored in the symbol table so we can make a call using the
:enumerator:`q_call` quadruple. It also makes sense to have a common exit from
each procedure/function, as in assembler you have to clear the runtime
stack. This can be performed in the last instructions before the return
jump. The quad list’s :member:`~quad_list::last_label` variable is assumed to contain the
number of the current procedure/function exit label. Example:

::

      function foo(x : integer) : integer;
      begin
         if (x < 0) then
            return -2*x;
         else
            return 2*x;
         end;
      end;

generates the following quadruples:

.. code-block :: none

      q_iload   0          $1    ($1 = 0)
      q_ilt     X    $1    $2    ($2 = X < $1)
      q_jmpf    6    $2          (if not $2 then jump to 8)
      q_iload   2          $3    ($3 = 2)
      q_imult   $3   X     $4    ($4 = X * $3)
      q_iuminus $4         $5    ($5 = - $4)
                                 (return $5)
      q_ireturn 5    $5
                                 (jump to label 7)
      q_jmp     7
                                 (can be removed by optimization)
      q_labl    6
      q_iload   2          $6
      q_imult   $6   X     $7
                                 (return $7)
      q_ireturn 5    $7
                                 (end of if-statement)
      q_labl    7
                                 (foo’s exit)
      q_labl    5

(``$n`` are temporary variables which are used to break down arithmetic
expressions).

Code generation
---------------

Code generation in this compiler is naive. Each quadruple is expanded to
one or more assembler instructions. Before the code for each block you
have to include some assembler instructions which reserve space on the
stack for local and temporary variables (*activation records*). The
amount of space needed is stored in the symbol table for the current
procedure/function. After each block we include code for restoring the
stack. This will be more explained in the code generation lab.

.. literalinclude:: overview-assembly.txt
  :language: nasm
  :linenos:
  :caption: The quadruples above are expanded (with ``;`` being comments)

The symbol table
----------------

The symbol table stores information about the identifiers that appear in
the program. The :type:`sym_type` type contains the objects that can occur
in DIESEL. Each symbol type is represented by a class. Each class
contains a number of attributes. Certain attributes are common to all
symbol types, and are therefore placed in the abstract symbol
superclass, which all the concrete symbols classes inherit:

.. doxygenclass :: symbol
  :members: id, tag, type, hash_link, back_link, level, offset
  :outline:
  :no-link:

Apart from the shared attributes listed above, some of the symbol classes have additional individual attributes which are summarized below:

.. doxygenclass :: constant_symbol
  :members: const_value
  :no-link:
  :outline:

.. doxygenclass :: array_symbol
  :members: array_cardinality, index_type
  :no-link:
  :outline:

.. doxygenclass :: parameter_symbol
  :members: size, preceding
  :no-link:
  :outline:

.. doxygenclass :: procedure_symbol
  :members: ar_size, label_nr, last_parameter
  :no-link:
  :outline:

.. doxygenclass :: function_symbol
  :members: ar_size, label_nr, last_parameter
  :no-link:
  :outline:

For example, for :class:`constants <constant_symbol>` we need only to remember which block they were declared in (another variable with the same name can “hide” the constant), what its type is (string, integer or floating-point) and, of course, its value.
For a :class:`variable <variable_symbol>` one must know *where* in the activation record the variable will be stored.
The :member:`~symbol::offset` attribute contains this information.
For procedures and functions all the formal parameters have to be linked together.
When we then parse a call, we can check the type of the actual parameters with the formal parameters by looking up the procedure or function in the symbol table and following the exiting parameter link.

Some elements in the symbol table are predefined. For example the name
types ``VOID``, ``INTEGER`` and ``REAL`` are already installed in the
symbol table [2]_. The same goes for the standard functions ``READ`` and
``TRUNC``, and also the standard ``WRITE`` procedure.

.. [1]
   A token is also often called a *terminal symbol*.

.. [2]
   At present there is no way to define new name types, but it is more
   general not to let *INTEGER* and *REAL* be reserved words.
