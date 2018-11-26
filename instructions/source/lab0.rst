.. _lab0:

Lab 0: Grammars and Formal Languages
====================================

Theory assignment
-----------------

These assignments are **optional** for those who need to revise their
theory knowledge of regular expressions and formal languages. The lab is
intended to make you practice using the basic concepts used in
descriptions of formal languages.

#. Given the alphabet :math:`A = \{ 1, 2, 3 \}` and the strings x=\ ``2``,
   y=\ ``13``, z=\ ``323``:

   #. What are the strings xz, zyx, z\ :sup:`2`, z\ :sup:`7`; and what are
      their lengths?

   #. What are A\ :sup:`1`, A\ :sup:`2`, A\ :sup:`0`?

   #. Describe A\ :sup:`*` and A\ :sup:`+`.

#. Given:

   .. code-block :: bnf
     :caption: Grammar for arithmetic expressions :math:`G(<exp>)`
     :name: lab0_grammar_exp

     <exp> ::= <term> | <exp> + <term> | <exp> - <term>
     <term> ::= <factor> | <term> * <factor> | <term> / <factor>
     <factor> ::= ( <exp> ) | <ident>
     <ident> ::= A | B | C ... | Z

   .. code-block :: none
      :caption: Example statements in :math:`G(<exp>)`
      :name: lab0_statements_exp

      A*B-C
      A*(B-C)
      A/B/C
      -A*B


   .. _ex_draw_parse_tree:

   #. Find derivations and draw a parse tree for the statements in :numref:`lab0_statements_exp`.

   #. State the handle in statement form ``<term> * <factor> + B/C``

   #. State a canonical derivation of one of the statements in
      :numref:`lab0_statements_exp`.

   #. Why is ``A * <exp> * D`` not in statement form?

   #. What are :math:`V`, :math:`\Sigma`, and :math:`N` in this grammar?

   #. Describe :math:`\Sigma^+` and :math:`L(G)`. State some string in
      :math:`\Sigma^+` which is not in :math:`L(G)`.

#. Using the grammar in :numref:`lab0_grammar_exp`, draw the parse tree for the
   statement:

   .. code-block :: bnf

     A - <term> * B + ( <term> )

   Use this to explain why there is no canonical derivation of the
   statement.

#. Construct a context-free grammar for even integers which may not
   begin with zeros (this also applies to a single zero).

#. provide a regular expression for strings consisting of binary numbers
   separated by the operators :math:`+` or :math:`-`. Example:

   ``10+1001-01 111001``

   Draw the respective state diagram, too.

#. Given the alphabet :math:`A = \{ a, b, c, d, e, f, g, h, i \}`:

   #. Construct a context-free grammar for words of one syllable (i.e.
      words containing exactly one vowel).

   #. Construct a context-free grammar for multi-syllable words.

#. Given the alphabet :math:`A = \{ a, b, c \}`:

   #. Write regular a expression which describes all strings starting
      with a sequence of zero or several ``a``\ 's
      followed by an arbitrary number of ``b``\ 's and ``c``\ 's mixed
      together. The strings finish with another sequence of ``a``\ 's.
      (Example: ``aaabbcbca``, ``bbca``, ``aa``).

   #. Can the expression :math:`a^{n}b^{n}c^{n}` be described using a
      regular expression? If so, write down the regular expression. If
      not, motivate why.

#. Describe the languages (sets) that the following grammars generate:
   ``<S>`` is the start symbol.

   #.
     .. code-block :: bnf

       <S> ::= <A> <A>
       <A> ::= 1 <A> 0 | 10

   #.
      .. code-block :: bnf

        <S> ::= 1 <S> 0 | <B>
        <B> ::= 0 <B> 1 | ε

   #.
      .. code-block :: bnf

        <S> ::= 1 <A> | <B> 0
        <A> ::= 1 <A> | <C>
        <B> ::= <B> 0 | <C>
        <C> ::= 1 <C> 0 | ε
