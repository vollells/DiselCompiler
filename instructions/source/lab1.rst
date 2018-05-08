.. _lab1:

Lab 1: Lexical Analysis
=======================

In this lab, you are to do three things:

#. Describe DIESEL identifiers, integers, reals, comments and string
   constants with regular expressions (:ref:`theory-assignment-1`).

#. Choose some of the tokens and draw a DFA showing how the scanner is
   going to identify them (:ref:`theory-assignment-2`).

#. Complete an input file to the scanner generator program ``flex``,
   called ``scanner.l`` (Implementation).

Some years ago, most scanners were written by hand. Writing a scanner is
one of the less complex parts of constructing a compiler, but
nevertheless one that can be very tedious and time-consuming. Soon the
need for automated tools became evident, and several have been developed
over time. The one we will use is called ``flex``, and is freely
available under a BSD-style license. It is a free version of the unix
tool ``lex``, which simply stands for ``"lexical <something>"``.
However, before we go into details about how it works, we will look at
some general scanner concepts.

Lexical scanners
----------------

.. digraph :: lab1_lexical_scanner
  :alt: Lexical scanner
  :caption: A lexical scanner forms the interface between the source code and the syntax analysis part of the parser.

  ordering=out ;
  st [label="Symbol table", shape="box", style="dashed", color="red"];
  chars [label="", style="invis"];
  Scanner;
  Parser [shape="box", style="dashed", color="red"];

  st -> Scanner [dir="both"];
  st -> Parser [dir="both"];

  chars -> Scanner [label="Sequence of\ncharacters"];
  Scanner -> Parser [label="request token"];
  Parser -> Scanner [label="token"];

  { rank = same; chars; Scanner; Parser; }

Input
~~~~~

Sequence of characters, e.g.:

.. code-block :: pascal

  IF X < 10 THEN Y := 15

Output
~~~~~~

Tokens (so-called basic symbols, groups of successive characters which
belong together). Other information is attached to tokens, e.g. the
gathered characters and the value of constants. We will show, later on,
exactly what this additional information consists of and how it is
represented in our program.

Scanner functionality
---------------------

These are the general steps to follow if you wish to write your own
scanner:

#. Recognize tokens: variable names, reserved words, operators and
   special symbols.

#. Save identifiers in a string table and return references to them
   which are then used in other phases of the compiler.

#. Remove ``comments``, ``blanks``, ``TAB`` etc.

#. Convert strings to values, for instance ``'10'`` to ``10``.

#. Discover lexical errors, wrongly formatted floating point numbers,
   illegal characters such as ``%`` and ``$``, etc.

#. Count rows, managing ``#include-commands`` etc., if this is not done
   by a preprocessor. The position information is used in later compiler
   phases to help generate useful error messages.

Constructing scanners
---------------------

When constructing scanners, one would normally start by describing the
tokens using regular expressions. Then, the aim is to generate a DFA
(Deterministic Finite Automaton) – a graphic description of legal
sequences of characters which can easily be implemented in a program. If
the language is not too complicated, you can manually generate a DFA
directly using the regular expressions. If you are a little more
interested, or if you have a large language, you can first generate an
NFA (Non-deterministic Finite Automaton), and then use the algorithm for
converting an NFA to a DFA, that you find in the course textbook.
However, this will not be necessary.

For this assignment we will use a program that automatically generates a
scanner for regular expressions (e.g. ``lex``, ``flex``).

For those who are interested, some summary information on how to do it
"the hard way" is also provided.

#. A DFA has no edges with the empty string :math:`\epsilon`.

#. In any arbitrary state in a DFA there may not be several edges with
   the same symbol.

.. digraph :: lab1_small_dfa
   :name: lab1_small_dfa
   :caption: Example of a small DFA

   rankdir=LR;
   node [shape="circle"];
   start [label="",style="invis"];

   start -> 1 [label="start"];
   1 -> 1 [label="blank"];
   1 -> 2 [label="digit"];
   1 -> 3 [label="\".\""];
   3 -> 4 [label="\".\""];
   1 -> 5 [label="D"];
   5 -> 6 [label="O"];

   2 [shape="doublecircle"];
   3 [shape="doublecircle"];
   4 [shape="doublecircle"];
   6 [shape="doublecircle"];

   key [shape="none", label=< <table border="0">
     <tr><td align="left">2 → integer</td></tr>
     <tr><td align="left">3 → dot</td></tr>
     <tr><td align="left">4 → interval</td></tr>
     <tr><td align="left">6 → DO</td></tr>
     </table> >]

   4 -> key [style="invis"];

In this way a computer program which simulates a DFA can always decide
what the next stage is after reading a character. In principle there are
two ways of realising a DFA in a program:

#. Encode the DFA as a table. The scanner will thus be data-driven and
   easier to modify:

   .. table:: :ref:`lab1_small_dfa` in table representation

      +---+-------+-------+-----+-----+-----+
      |   | Blank | Digit | “D” | “O” | “.” |
      +---+-------+-------+-----+-----+-----+
      | 1 | 1     | 2     | 5   | \-  | 3   |
      +---+-------+-------+-----+-----+-----+
      | 2 | \-    | 2     | \-  | \-  | \-  |
      +---+-------+-------+-----+-----+-----+
      | 3 | \-    | \-    | \-  | \-  | 4   |
      +---+-------+-------+-----+-----+-----+
      | 4 | \-    | \-    | \-  | \-  | \-  |
      +---+-------+-------+-----+-----+-----+
      | 5 | \-    | \-    | \-  | 6   | \-  |
      +---+-------+-------+-----+-----+-----+
      | 6 | \-    | \-    | \-  | \-  | \-  |
      +---+-------+-------+-----+-----+-----+

   We suggest that the analysis progresses in this way: a character is
   read and the table is indexed with the current state and the
   character. This is repeated until the content in the table is not
   defined (-). If this is in an accepting state, the string is accepted
   and a token is returned; otherwise a lexical error is reported.

#. Encode the DFA as a program. This can be done using ``switch``-``case``
   constructions (or the infamous ``goto`` statement), where for each
   state, there is some code which reads in a character and decides
   which state to enter next. What you gain in flexibility (easy to add
   extra code), you lose in overview and maintenance.

As you can see, it is normal to divide the incoming characters into
classes such as *letter*, *digit*, etc. so that the tables/programs
don’t grow too large. There is, thus, a sort of scanner in the
``scanner.1``  [1]_.

Tokens in DIESEL
----------------

The tokens are what makes up a DIESEL program. In this section, we will
go through all the various tokens in DIESEL, and show some examples of
valid/invalid ones. You will have to write up regular expressions for
them (see below), and later on add them (most of which will be trivial)
to the ``scanner.l`` file. However, some of them are more complicated,
e.g., integers, reals, identifiers, strings and comments, and you should
make sure you get them right as well.

Numerical constants
~~~~~~~~~~~~~~~~~~~

A numerical constant is either an *integer* or a *floating-point*
number. Exponents are allowed in floating-point numbers. No character
may precede the constant as unary operators are managed by the parser.

Integers (non-negative)
^^^^^^^^^^^^^^^^^^^^^^^

Integers are only made from a sequence of digits as characters are dealt
with by the parser. For example, ``-39`` will result in two tokens. The
parser decides afterwards whether the minus character is a unary
operator (e.g. ``A := -39``) or a binary (e.g. ``A := 17 - 39``).

.. table:: Examples of integer numbers.

   +------------+----------------------------------------------+
   | ``10``     | OK                                           |
   +------------+----------------------------------------------+
   | ``0``      | OK                                           |
   +------------+----------------------------------------------+
   | ``862107`` | OK                                           |
   +------------+----------------------------------------------+
   | ``-7``     | Non-positive integer.                        |
   +------------+----------------------------------------------+
   | ``3E5``    | Not an integer but an floating-point number. |
   +------------+----------------------------------------------+

Floating-point numbers
^^^^^^^^^^^^^^^^^^^^^^

Floating-point numbers are integers or decimal numbers followed by an
exponent. A decimal number is a sequence of numbers where a decimal
point is placed at an arbitrary place in the sequence. The exponent
consists of an ``E`` or an ``e`` followed by an integer, with or
without a sign.

.. table:: Examples of floating-point numbers.

   +------------+---------------------------------------+
   | ``3.3``    | OK                                    |
   +------------+---------------------------------------+
   | ``.3``     | OK                                    |
   +------------+---------------------------------------+
   | ``5E6``    | OK                                    |
   +------------+---------------------------------------+
   | ``.2e-14`` | OK                                    |
   +------------+---------------------------------------+
   | ``7E+3``   | OK                                    |
   +------------+---------------------------------------+
   | ``4.``     | OK                                    |
   +------------+---------------------------------------+
   | ``5.e2``   | OK                                    |
   +------------+---------------------------------------+
   | ``3``      | Not floating-point, but integer.      |
   +------------+---------------------------------------+
   | ``E9``     | Not floating-point, but identifier.   |
   +------------+---------------------------------------+
   | ``.e3``    | Not floating-point, lacks numbers.    |
   +------------+---------------------------------------+
   | ``7E``     | Not floating-point, lacks exponent.   |
   +------------+---------------------------------------+
   | ``-3.2``   | Not floating-point, because negative. |
   +------------+---------------------------------------+

String constants
----------------

A string constant starts and finishes with an apostrophe. Between these
there can be zero or several characters. They can not contain line
breaks. If you want an apostrophe in the string, you write it twice,
e.g. ``'don''t care'``. The internal representation in the compiler will
then be ``don't care``.

Reserved words
--------------

All the reserved words in DIESEL have their own token, and you will need
to enter regular expressions describing them into the ``scanner.l``
file. Most of these regular expressions will be trivial, since each
reserved word is a token of its own, e.g.: ``BEGIN``.

.. table:: Examples of reserved words.

   +-----------------------------------+-----------------------------------+
   | ``end``                           | Reserved word                     |
   +-----------------------------------+-----------------------------------+
   | ``while``                         | Reserved word                     |
   +-----------------------------------+-----------------------------------+
   | ``IntEger``                       | Reserved word                     |
   +-----------------------------------+-----------------------------------+
   | ``x3b``                           | Not a reserved word, but          |
   |                                   | identifier                        |
   +-----------------------------------+-----------------------------------+
   | ``9b``                            | Not a reserved word, but illegal  |
   |                                   | token                             |
   +-----------------------------------+-----------------------------------+
   | ``foo%fie``                       | Not a reserved word, but illegal  |
   |                                   | token                             |
   +-----------------------------------+-----------------------------------+

Identifiers
-----------

An identifier always starts with a letter which can be followed by
letters/digits. Letters include ``A - Z,a - z`` and also ``_``
(underscore). We thus allow an identifier to begin with ``_``. Note that
*reserved words* and *identifiers* are case insensitive.

.. table:: Examples of identifiers.

   +--------------+---------------------------------------+
   | ``x``        | Identifier                            |
   +--------------+---------------------------------------+
   | ``a7``       | Identifier                            |
   +--------------+---------------------------------------+
   | ``odd_num``  | Identifier                            |
   +--------------+---------------------------------------+
   | ``_integer`` | Identifier                            |
   +--------------+---------------------------------------+
   | ``x4b``      | Identifier                            |
   +--------------+---------------------------------------+
   | ``9b``       | Not an identifier, but illegal token. |
   +--------------+---------------------------------------+

Special characters
------------------

The following are the allowed special characters in DIESEL:

.. table:: Special characters

   +--------+-------+-------+--------+
   | ``+``  | ``-`` | ``*`` | ``/``  |
   +--------+-------+-------+--------+
   | ``<``  | ``>`` | ``=`` | ``<>`` |
   +--------+-------+-------+--------+
   | ``:=`` |       |       |        |
   +--------+-------+-------+--------+
   | ``(``  | ``)`` | ``[`` | ``]``  |
   +--------+-------+-------+--------+
   | ``,``  | ``.`` | ``:`` | ``;``  |
   +--------+-------+-------+--------+

Comments
~~~~~~~~

Comments start with ``{`` and finish with ``}`` and can not be nested.
Comments can contain a line-break. This means that ``{`` in a comment
can either result in an error or be ignored.

To honor the move in implementation language made 1998-1999, from Pascal
to C++, DIESEL now also allows C++ style, one-line comments starting
with ``//``. Furthermore, C/C++ style comments starting with ``/*`` and
ending with ``*/`` (can span several lines) are supported. Both of these
have already been implemented in the ``scanner.l`` file, and might be
helpful to look at when you provide lexical actions for your own regular
expressions later on (hint: especially the old-style comments mentioned
above, and strings).

Separators
~~~~~~~~~~

The separators are ``blank``, ``TAB``, and ``return``.

.. _theory-assignment:

Theory assignment
-----------------

These assignments are *mandatory*, and must be handed in to your lab
assistant. Once you have finished these assignments the implementation
part of the lab will be much easier to do.

Do not spend too much time doing this assignments though, any sketch on
a paper will do. Save your time for the implementation of the future
labs, you will need it.

.. _theory-assignment-1:

Assignment 1: Regular expressions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Describe identifiers, constants (integers, reals and strings) and
special characters using regular expressions. You can write them
directly in the ``scanner.l`` file.

.. _theory-assignment-2:

Assignment 2: Deterministic finite state automation for DIESEL
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Design a deterministic finite state automaton for a subset of DIESEL(but
make sure to include floating-point numbers). Describe the automaton
graphically. Show clearly which are start and final states. Mark the
edges with the character causing the transition. You will need an error
state; edges leading to the error state do not have to be drawn. It is
implicitly assumed that all input which does not match one of your edges
causes the DFA to go into an error state.

Implementation
--------------

Complete the input file to ``flex``, ``scanner.l``, using the
information given above and below. You will have to use the regular
expressions you defined earlier. You must add token-specific information
into the :var:`yylval` union where appropriate (integers, reals, string
constants and identifiers), and position information in the :var:`yylloc`
struct for all tokens.

More specifically, your scanner should do the following:

-  *Keep track of position information*. This is done using the special variable :var:`yylloc`, which is really a struct:

   .. doxygenstruct :: YYLTYPE
     :no-link:
     :undoc-members:
     :members:

   .. note ::

     ``flex`` helps you keep track of the line number by use of the `yylineno <_static/flex/Options-Affecting-Scanner-Behavior.html#option_002dyylineno>`_ option, but you will have to *implement your own column counter* and update it as necessary when reading tokens.

-  When you find an *identifier, you have to install in the string
   pool*, which resides in the :class:`symbol_table` class. To this end, the
   methods :func:`~symbol_table::pool_install()`, :func:`~symbol_table::pool_forget()`, :func:`~symbol_table::lookup_symbol()`
   and :func:`~symbol_table::get_symbol_id()` are all (at least some, see below) useful.
   They are described in more detail below. Note that the symbol table
   is a global variable, called :var:`sym_tab`. See the file ``symtab.hh``
   for more information.

-  *Enter required information* for integers, reals, identifiers and
   string constants into the :var:`yylval` union.

-  Make sure that DIESEL is **case-insensitive**, i.e., the identifiers
   ``foo`` and ``Foo`` should both internally be represented as
   ``FOO``.

The :var:`yylval` variable is represented by a union:

.. doxygenunion :: YYSTYPE

.. code-block:: pascal

  IF FOO < 10 THEN BAR := 15.0

.. table:: Tokens returned after the parsing of a piece of code

   +-------------------------+-----------+------+------+-----+
   | token                   | pool_p    | ival | rval | str |
   +=========================+===========+======+======+=====+
   | :c:macro:`T_IF`         |           |      |      |     |
   +-------------------------+-----------+------+------+-----+
   | :c:macro:`T_IDENT`      | ``FOO``   |      |      |     |
   +-------------------------+-----------+------+------+-----+
   | :c:macro:`T_LESS`       |           |      |      |     |
   +-------------------------+-----------+------+------+-----+
   | :c:macro:`T_INTNUM`     |           | 10   |      |     |
   +-------------------------+-----------+------+------+-----+
   | :c:macro:`T_THEN`       |           |      |      |     |
   +-------------------------+-----------+------+------+-----+
   | :c:macro:`T_IDENT`      | ``BAR``   |      |      |     |
   +-------------------------+-----------+------+------+-----+
   | :c:macro:`T_ASSIGNMENT` |           |      |      |     |
   +-------------------------+-----------+------+------+-----+
   | :c:macro:`T_REALNUM`    |           |      | 15   |     |
   +-------------------------+-----------+------+------+-----+

.. note ::

  A ``union`` in C simply shares the memory location of the members.
  It is memory-efficient, but if you are not careful you may introduce
  weird bugs.

  .. code-block :: c

    yylval.ival = 13;
    yylval.rval = 1.0;
    // Gives "garbage" - the binary representation of 1.0
    x = yylval.ival;

Flex manual
~~~~~~~~~~~

For flex-specific information, see the `manual <_static/flex/index.html>`_.

You will have to study this manual in order to be able to complete this
lab successfully!
After having read this, the information in the skeleton ``scanner.l`` file
coupled with the information below should be enough to get you started.

The files that you need to change
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

scanner.l
   is the ``flex`` input file, which you’re going to complete. This is
   the only file you will need to edit in this lab.

Other files of interest
~~~~~~~~~~~~~~~~~~~~~~~

These files you might have to read through, but they should not require
editing at this point.

scanner.hh
   is a temporary include file used for scanner testing.

scantest.cc
   is an interactive test program for your scanner.

symtab.hh
   contains symbol table information, including string pool methods.

symbol.cc
   contains symbol implementations (will be edited in lab 2).

symtab.cc
   contains the symbol table implementation.

error.hh and error.cc
   contain debug and error message routines.

Declarations and routines available in *scanner.hh*, *error.hh*, and *symtab.hh*.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Constant declarations
^^^^^^^^^^^^^^^^^^^^^

The ``#defines`` in the ``scanner.hh`` file denote tokens. Using the
actual numbers is a very bad idea! They are not chosen randomly, but
rather by the ``bison`` program, which we will look closer at later on
in this lab course. The declarations of the :var:`yylval` and :var:`yylloc`
variables can also be found in this file. In total there are ``NR_SYMS``
symbols(terminals) in the grammar.

The file ``error.hh`` contains some handy error routines which are
globally available. Use ``yyerror()`` to give error messages from the
scanner. Most of the other routines in this file will not need to be
used until later in the lab course. The file ``symtab.hh`` is big, and
contains complete information on all symbol classes, as well as the
symbol table itself. You will not need to use most of them in this lab.
The ones of interest are described below.

Type declarations
^^^^^^^^^^^^^^^^^

In ``symtab.hh``, some types are defined to make the code more readable.
The only one you will need to use in this lab is the first one (the
second as well, if you want the symbol table to use shared strings):

.. code-block:: c++

  typedef long pool_index; /* indices into the string pool */
  typedef long hash_index; /* indices into the symbol table */
  typedef long sym_index;  /* indices into the hash table */
  typedef int block_level; /* indices into the block table */

If this doesn’t make sense to you right now, don’t worry: You won’t need
to understand what the three last ones are used for until the next lab.

Variable declarations
^^^^^^^^^^^^^^^^^^^^^

.. doxygenvariable :: yylval

.. doxygenvariable :: yylloc

.. doxygenvariable :: sym_tab

``flex`` provides the variable ``yylineno``, which is guaranteed to
contain the current line number in the source code file. This
information should be entered in :member:`yylloc.first_line <YYLTYPE::first_line>` for all tokens.

``flex`` also provides the variables ``yytext``, which contains the
string of the currently parsed token (``"16"``, ``"foo"``, ``"BEGIN"``
etc), and ``yyleng``, which is set to the length of ``yytext``.

The file ``error.hh`` declares a variable called :var:`error_count`.
Ideally compilation should continue as long as possible, but if one or
more errors are found, no assembler code will be generated. Increase
this variable by one every time you find an error. If the error is a
fatal one, you can call the :func:`fatal()` method, which will abort
compilation with an error message.

Methods
^^^^^^^

The following methods might come in handy:

.. doxygenclass :: symbol_table
  :members: pool_install, pool_lookup, pool_forget, fix_string, capitalize
  :no-link:

.. doxygenfunction :: yyerror
  :no-link:

Program specification
~~~~~~~~~~~~~~~~~~~~~

#. Lower- and upper-case characters should be treated in the same way,
   except for string constants.

#. When a numerical constant (this is not the same as a declared
   constant! we refer here to ``"2"``, ``"4.0"``, etc.) is found, its
   value should be stored in the :var:`yylval` union.

#. When an identifier is found, its index into the string table should
   be stored in the :var:`yylval` union.

#. When a string constant is found, an index into the string table,
   where its internalized form is stored, should be stored in the
   :var:`yylval` union.

#. Position information, both line and column, is to be kept for each
   token, using the :var:`yylloc` variable.

#. The scanner should only give up on truly fatal errors, such as a full
   string pool, un-terminated comments and the like. Otherwise, an error
   should be printed, but the compiler should continue reading tokens.
   The more errors printed in one compiler iteration, the better.

How do I know it’s correct?
~~~~~~~~~~~~~~~~~~~~~~~~~~~

When you compile your scanner using ``make``, a binary executable file
named ``scanner`` will be created. When run, it will wait for keyboard
input and return the token found, its position, and any other
information associated with it. Type in all valid tokens and make sure
they are identified correctly. A list of all valid tokens can be found
in the ``scanner.hh`` file. You can type in more than one token at a
time; the input will only be scanned after you hit ``<enter>``. End the
program by typing ``<enter>`` on a new line, or by hitting ``Ctrl-C``. A
sample execution is provided at the end of this chapter.

Also note that the test program will get jammed pretty quickly if you
try to compile it using the bare-bones skeleton file ``scanner.l``.
Complete the scanner, and then run the test program.

Reporting your work
-------------------

* Hand in all the parts of the :ref:`theory-assignment`.
* For the demonstration, show that your program works as expected on some
  example programs (also show that ``scannertest1.d`` produces the result shown
  in ``scannertest1.trace`` by running ``make lab1``).
* For the implementation part, hand in your code as described in :ref:`handing-in-results`.

Example execution
-----------------

Running ``make lab1`` will take the sample input and compare it to the
sample output. You should test your code on more examples than this.

.. code-block:: bash

  ./scanner ../testpgm/scannertest1.d

.. literalinclude:: ../../code/trace/scannertest1.trace
  :language: none
  :caption: trace/scannertest1.trace

.. [1]
   Actually one ought to distinguish between a *scanner*, which works on
   characters, and a *lexical scanner* which groups output from the
   scanner to tokens.
