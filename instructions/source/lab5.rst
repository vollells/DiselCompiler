Lab 5: Optimization
===================

The aim of this lab is to perform optimization on the internal form
constructed and type checked in previous labs.

Optimization
------------

Optimization is a whole field of research in itself, and we will only
briefly discuss the various types here. Refer to the lectures and course
book (especially Chapters 9 and 10) for a more in-depth treatment.

When to optimize?
~~~~~~~~~~~~~~~~~

Optimization can be applied during several compiling phases. While the
program is represented as some form of tree, it is relatively easy to
transform the tree by use of tree traversals that analyze the trees and
nodes and replace or rearrange them if possible (indeed, that is what we
will do in this lab), in order to generate better code later on. Later
on, the program needs to be linearized into list form, since assembler
and ultimately processors are linear, for example into quadruples. These
quadruple lists are suited for other kinds of optimization, where
unnecessary statements can be eliminated, loop invariants moved outside
loops, and many other things. Finally the assembler code generated can
be analyzed and optimized, this time with special properties of the
target processor in mind.

Constant folding
~~~~~~~~~~~~~~~~

We will only implement a very simple algorithm for optimization known as
*constant folding*. The idea is to identify constant arithmetic
expressions at compile time and replace them with the result right away,
saving us some run-time evaluations. The optimizing is done on the AST
internal form, i.e., before we transform the program into quadruples,
and is destructive in the sense that it modifies the AST, replacing
subtrees with their evaluations.

As an example, the DIESEL code ::

  if (a > 2) then
    a := 2 + 3;
  end;

will be optimized to ::

  if (a > 2) then
    a := 5;
  end;

Obviously, most programmers are smart enough not to write code like this
themselves, but there are cases where it can come in useful. A more
intelligent version could analyze program blocks and identify variables
whose value don’t change during the block’s lifetime, and pre-calculate
expressions involving this variable once instead of several times, for
example. However, we will stick with the basic version outlined above in
this lab.

Expressed as AST nodes, the above optimization will look like this (node
types are represented by their DIESEL symbols out of space
considerations):

.. digraph :: diesel_lab5_constant_folding
  :alt: Example of constant folding.

  node [fontname="Courier New"];


  "a1" [label="a"];
  "a2" [label="a"];
  "a3" [label="a"];
  "a4" [label="a"];
  "2_1" [label="2"];
  "2_2" [label="2"];
  "2_3" [label="2"];

  if1 [label="if"];
  ">1" [label=">"];
  ":=1" [label=":="];
  "+1" [label="+"];

  if2 [label="if"];
  ">2" [label=">"];
  ":=2" [label=":="];

  subgraph cluster_before {
    label=< <i>Before constant folding</i> >;
    penwidth = 0;
    if1 -> ">1";
    if1 -> ":=1";

    ">1" -> "a1";
    ">1" -> "2_1";

    ":=1" -> "a2";
    ":=1" -> "+1";
    "+1" -> "2_2";
    "+1" -> 3;
  }

  subgraph cluster_after {
    label=< <i>After constant folding</i> >;
    penwidth = 0;
    if2 -> ">2";
    if2 -> ":=2";

    ">2" -> "a3";
    ">2" -> "2_3";

    ":=2" -> "a4";
    ":=2" -> 5;
  }

Implementation
--------------

In this lab, you are to implement the constant folding algorithm as
described above for DIESEL, by editing the file ``optimize.cc``.

The tree traversal will be done using recursive method calls, much like
the type checking in the previous lab. Optimization is performed one
program block at a time. It is started from parser.y by a call to
:func:`~ast_optimizer::do_optimize()` in a global optimizer object which is an instance of the
:class:`ast_optimizer` class, which will in turn start an :func:`~ast_node::optimize()` call
which will propagate down the AST and try to identify subtrees eligible
for optimization.

The :class:`ast_optimizer` class is a standalone class, again much like the
semantic class, used as a wrapper for optimization. Its instantiation is
a global variable, optimizer, which is declared in ``optimize.cc``. The
actual :func:`~ast_node::optimize()` methods are part of the AST nodes, but their
implementations lie in the ``optimize.cc`` file. The interface is kept
clean in order to make it easy to change the algorithm used without
having to fiddle with other parts of the compiler.

Some of the methods have already been implemented; look at them to get
an idea of what you are supposed to do. Most of them you are to complete
yourself (look for ``Your code here`` comments). Depending on your
solution, it might be convenient to add new methods to the
:class:`ast_optimizer` class. If you do so, you will also need to add the
declarations of these methods to the ``optimize.hh`` file. Remember to try
to keep the interface as clean as possible, though.

Files that you need to change
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

optimize.hh and optimize.cc
   contain optimizing code for the AST nodes as
   well as the declaration and implementation of the :class:`ast_optimizer`
   class. These are the files you will edit in this lab.

Other files of interest
~~~~~~~~~~~~~~~~~~~~~~~

All these files are the same as in lab 4.

ast.cc
   contains (part of) the implementations of the AST nodes.

ast.hh
   contains the definitions for the AST nodes. You will need to perform
   some safe downcasting in this lab, and the methods for doing that can
   be found here.

parser.y
   is the input file to bison.

quads.hh and quads.cc
   contain quad generation code. You won’t need to
   edit these files in this lab.

codegen.hh and codegen.cc
   contain assembler generation code. You won’t need
   to edit these files until the last lab.

error.hh, error.cc, symtab.hh, symbol.cc, symtab.cc, scanner.l, semantic.hh, and semantic.cc
   use your versions from the earlier labs.

main.cc
   this is the compiler wrapper, parsing flags and the like. Same as in
   the previous labs.

Makefile
   and **diesel** use the same files as in the last lab.

Declarations and routines available in *ast.hh*, *optimize.hh*, and *optimize.cc*
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The recursive :func:`~ast_node::optimize()` method common to all the AST classes takes no
arguments, and does not return anything; it either performs a
destructive update of the AST, or passes the call onward to its
children. The declarations can be found in ast.hh. You will have to
implement their method bodies yourself.

The :class:`ast_optimizer` class (currently) only contains one method of
external interest and some utility functions that you may want to use:

.. doxygenclass :: ast_optimizer
  :members: do_optimize, is_binop, fold_constants
  :no-link:

As mentioned before, it is quite likely that you will want to add
methods to the :class:`ast_optimizer` class. Feel free to do so at your
leisure, but remember to keep the interface as clean as possible. See
the code comments for more information.

Program specification
~~~~~~~~~~~~~~~~~~~~~

#. Your program must be able to handle optimization of all operations
   derived from the :class:`ast_binaryoperation` class.

#. Your program needs only optimize subtrees whose leaf nodes are
   instances of :class:`ast_real`, :class:`ast_integer` or :class:`ast_id` (only those
   :class:`ast_id` nodes that represent constants should be treated).

#. Your program does not need to be able to optimize expressions
   containing :class:`ast_cast` nodes, but feel free to implement it anyway as
   an exercise (it should not be too much extra work).

#. Your program does not need to handle optimizing of binary relations,
   but feel free to implement it anyway as an exercise. It will be very
   similar to handling binary operators.

#. Your program must preserve the code structure, i.e., the destructive
   updates must not change the result of running the compiled program in
   any way.

#. Optimization should be done one block at a time. Apart from the calls
   to :func:`~ast_optimizer::do_optimize()`, no optimization code is to
   reside in ``parser.y``.

Debugging help
~~~~~~~~~~~~~~

As previously, you can send all AST nodes to ``cout``.

How do I know it’s correct?
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Compare the AST before and after optimization has been performed. Study
the subtrees representing arithmetic expressions only involving
constants. Make sure that they have been replaced by their result. An
example is added as an enclosure.

Reporting your work
-------------------

* For the demonstration, run the test program
  for the test case ``opttest1.d`` and discuss your code.
* Hand in your code as described in :ref:`handing-in-results`.

Example execution
-----------------

Running ``make lab5`` will take the sample input and compare it to the
sample output. You should test your code on more examples than this.

.. code-block:: bash

  ./diesel -b -p -a ../testpgm/opttest1.d

.. literalinclude:: ../../code/trace/opttest1.trace
  :language: none
  :caption: trace/opttest1.trace

.. note ::

  If you optimize more nodes than required, the traces may be different
  in this and subsequent labs.
  If you do implement such extra features, it is suggested that you can
  easily disable it to compare against the provided traces to save time.
