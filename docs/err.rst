=========================
Error recovery in Tralics
=========================

This file list most of all errors produced by *Tralics*.

Accents
-------

The command ``\a`` allows you to put an accent on a character. In fact
``\a'e`` is the same as ``\'e``. The first line produces three errors,
the last line five é.

.. container:: ltx-source

   ::

      11 \a{\bb} \a{12} \a{W}x
      12 \a'e \a'{e} \a{'}e \a{'}{e}  \a {'} {e}

.. container:: log_out

   ::

      Error signaled at line 11 of file tty:
      Bad syntax of \a, argument not a character \bb.
      Error signaled at line 11 of file tty:
      wanted a single token as argument to \a.
      Error signaled at line 11 of file tty:
      Bad syntax of \a, argument is W.

After the accent command, for instance ``\^``, there should come a
single letter, with or without braces. The first line produces six
errors, the second is equivalent to añêañê

.. container:: ltx-source

   ::

      1 a\~{}b\^{}ca\~{}b\^{}c \^{ee}\^{\foo}
      2 a\~n\^ea\~{n}\^{e}

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      Error in accent, command = \~
      \~{} is the wrong way to put a tilde in an URL.
      Error signaled at line 1 of file tty:
      Error in accent, command = \^
      Things like {\'{}} are a bit strange.
      Error signaled at line 1 of file tty:
      Error in accent, command = \~
      \~{} is the wrong way to put a tilde in an URL.
      Error signaled at line 1 of file tty:
      Error in accent, command = \^
      Things like {\'{}} are a bit strange.
      Error signaled at line 1 of file tty:
      Error in accent, command = \^
      Wanted a single token.
      Error signaled at line 1 of file tty:
      Error in accent, command = \^
      Letter needed instead of \foo.

In some cases you can put a double accent on a letter. The first line
produces four errors signaled by ``\a``, the second line shows correct
syntax.

.. container:: ltx-source

   ::

      1 \^{\a\foo} \^{\a+e} \^{\a{}e} \^{\a'{ee}} 
      2 \^{\'e} \a^{\a'e}\a^{\a'{e}}

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      Bad syntax of \a, argument not a character \foo.
      Error signaled at line 1 of file tty:
      Bad syntax of \a,  argument is {Character + of catcode 12}.
      Error signaled at line 1 of file tty:
      Bad syntax of \a,  argument is {Character { of catcode 1}.
      Error signaled at line 1 of file tty:
      Error in accent, command = \^
      Wanted a single token.

*Tralics* allow you to put any accent on any letter. In the code that
follows, we redefine the command that creates a composite character.
Thus the third line provokes two errors while in general it gives none.
You cannot put an accent on a non-letter and you cannot put a double
accent on a letter if the result is not a Unicode character. Line four
yields four errors. The last line shows that you can put an accent on å,
æ and ø (and their upper case equivalent) entered as ``\aa``, ``\ae``
``\o``.

.. container:: ltx-source

   ::

      1 \makeatletter \def\@unicode@composite#1{\badchar#1}
      2 \def\badchar#1#2{\error{bad accent  on #1}}
      3 \`x  \u z
      4 \=+\=2\=é \^{\'i}
      5 \=\i\=\ae\'aa\'\o

.. container:: log_out

   ::

      Error signaled at line 3 of file tty:
      Error: bad accent on x.
      Error signaled at line 3 of file tty:
      Error: bad accent on z.
      Error signaled at line 4 of file tty:
      Error in accent, command = \=
      Cannot put accent on non-letter character +.
      Error signaled at line 4 of file tty:
      Error in accent, command = \=
      Cannot put accent on digit 2.
      Error signaled at line 4 of file tty:
      Error in accent, command = \=
      Cannot put accent on a non 7-bit character.
      Error signaled at line 4 of file tty:
      Error in accent, command = \^\'
      Cannot put accent on letter i.

Errors in tables
----------------

The first argument to ``\newcolumntype`` has to be a 7- bit character.
The first two lines provoke some errors

.. container:: ltx-source

   ::

      1 \newcolumntype{}{xx}\newcolumntype{foo}{xx}
      2 \newcolumntype{é}{xx}\newcolumntype{\foo}{xx}
      3 \newcolumntype{C}{{>$}c<{$}}
      4 \newcolumntype{d}[1]{>{\rightdots{#1}}r<{\endrightdots}}

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      Empty argument to \newcolumntype.
      Error signaled at line 1 of file tty:
      More than one token in argument to \newcolumntype.
      Error signaled at line 2 of file tty:
      Argument to \newcolumntype is not a 7bit character.
      Error signaled at line 2 of file tty:
      Argument to \newcolumntype is not a 7bit character.

If you say ``\newcolumntype{D}{xx}``, then D will be replaced by xx, and
this is checked again; an error is signaled if a loop is detected. After
the replacement, errors are signaled if the result is not a known
character.

.. container:: ltx-source

   ::

      1 \newcolumntype{D}{D}
      2 \newcolumntype{E}{fg}
      3 \def\foo{33}\newcolumntype{F}{\foo}
      4 \begin{tabular}{D} x \end{tabular}
      5 \begin{tabular}{E} x \end{tabular}
      6 \begin{tabular}{F} x \end{tabular}

.. container:: log_out

   ::

      Error signaled at line 4 of file tty:
      array preamble expansion: Infinite loop?.
      Error signaled at line 5 of file tty:
      Wrong character in array preamble f.
      Error signaled at line 5 of file tty:
      Wrong character in array preamble g.
      Error signaled at line 6 of file tty:
      Wrong character in array preamble \foo.

The last line here shows a list of all possible characters in an array
preamble. Some characters take an argument (shown in braces here). This
may signal an error. There are some restrictions you cannot use a
less-than-sign, an at-sign or a exclamation point anywhere.

.. container:: ltx-source

   ::

      1 \def\foo#1{\begin{tabular}{#1} x \end {tabular}}
      2 \foo{!}\foo{@}\foo{c<}\foo{>}\foo{m}\foo{p}\foo{b}
      3 \foo{c>{}@{}}\foo{c>{}!{}}\foo{<{x}}
      4 \begin{tabular}{|clr!{1}@{2}c<{3}>{4}m{5}p{6}b{7}} x \end{tabular}

.. container:: log_out

   ::

      Error signaled at line 2 of file tty:
      Array preamble: argument missing for !.
      Error signaled at line 2 of file tty:
      Array preamble: argument missing for @.
      Error signaled at line 2 of file tty:
      Array preamble: argument missing for <.
      Error signaled at line 2 of file tty:
      Array preamble: argument missing for >.
      Error signaled at line 2 of file tty:
      Array preamble: argument missing for m.
      Error signaled at line 2 of file tty:
      Array preamble: argument missing for p.
      Error signaled at line 2 of file tty:
      Array preamble: argument missing for b.
      Error signaled at line 3 of file tty:
      illegal ! or @.
      Error signaled at line 3 of file tty:
      illegal ! or @.
      Error signaled at line 3 of file tty:
      unexpected less-than sign.

If the array preamble contains ``>{foo}``\ or ``<{bar}`` then the token
list ``foo`` is inserted before the corresponding cell, and then token
list ``bar`` after the cell. These lists are called the *u*- and
*v*-parts. A special marker is inserted after the *v*-part; it is called
the endv token. In the code that follows, the command ``\foo`` reads
this token. When *Tralics* sees ``\end{tabular}`` it finishes the
current cell; however, in the second (commented out) line, the ``\foo``
command gobbles then endv token and the end of the cell is never seen;
*Tralics* loops without end. On the third line there is a ``\cr``; this
end the current row (and thus the current cell, without inserting a
*v*-part). On line four, we show that ``\cr`` or the endv token cannot
be used outside a table.

.. container:: ltx-source

   ::

      1 \def\foo#1{\gdef\Bar{#1}} 
      2 %\begin{tabular}{c<{\foo}} x& y\end{tabular}
      3 \begin{tabular}{c<{tt\foo}} x& y\cr\end{tabular}
      4 \cr \Bar

.. container:: log_out

   ::

      Error signaled at line 4 of file tty:
      bad \cr.
      Error signaled at line 4 of file tty:
      Bad endv token.

Arithmetic errors
-----------------

The basic commands like ``\advance``, ``\multiply`` and ``\divide`` do
not always test for overflow. (the ``\XX`` macro produces never an
overflow, while the second call of ``\YY`` signals an error).

.. container:: ltx-source

   ::

      1 \def\XX{\advance \dimen0 by15000pt }
      2 \def\YY{\advance \dimen0 by \dimen0 }
      3 \dimen0= 17pt \XX \XX \XX \XX \XX \XX \XX
      4 \dimen0= 15000pt \YY \YY
      5 \count0=1234 \dimen0=5678pt \skip0= 3pt plus 5pt minus 6pt
      6 \divide \count0 by 0 \divide \dimen0 by 0\divide \skip0 by 0
      7 \dimen0=3000em \dimen0=4000ex \count0=9876543210

.. container:: log_out

   ::

      Error signaled at line 4 of file tty:
      Arithmetic overflow, threshold=1073741823, cur val=1966080000.
      Error signaled at line 6 of file tty:
      Arithmetic overflow, division by 0
      in 1234/0.
      Error signaled at line 6 of file tty:
      Arithmetic overflow, division by 0
      in 372113408/0.
      Error signaled at line 6 of file tty:
      Arithmetic overflow, division by 0.
      Error signaled at line 7 of file tty:
      Arithmetic overflow, threshold=2^{30}
      in 3000*655360+0.
      Error signaled at line 7 of file tty:
      Arithmetic overflow, threshold=2^{30}
      in 4000*278528+0.
      Error signaled at line 7 of file tty:
      number too big.

On the other hand commands like ``\numexpr`` perform regular overflow
checks. The error messages are also a bit different.

.. container:: ltx-source

   ::

       1 \def\N#1{\the\numexpr #1\relax}
       2 \def\D#1{\the\dimexpr #1\relax}
       3 \def\G#1{\the\glueexpr #1\relax}
       4 \D{15000pt+15000pt}
       5 \N{12/0}\D{12pt/0}\G{12pt/0}
       6 \N{12*3/0}\D{12pt*3/0}\G{12pt*3/0}
       7 \D{1073741824sp}\G{1073741824sp}\G{0pt plus 1073741824sp}
       8 \D{16384pt} \G{16384pt plus 16384pt minus 16384pt}
       9 \dimen0= 1639pt \multiply\dimen0 by 10 \D{16384pt*10}
      10 \D{16384pt*10}  \G{15000pt *3/2} \G{15000pt *2/3}
      11 \N{2147483000+ 700}\G{0pt plus 15000pt+0pt plus 15000pt}

.. container:: log_out

   ::

      Error signaled at line 4 of file tty:
      Arithmetic overflow, threshold=2^{30}
      in 983040000+983040000.
      Error signaled at line 5 of file tty:
      Division by zero.
      Error signaled at line 5 of file tty:
      Division by zero.
      Error signaled at line 5 of file tty:
      Division by zero.
      Error signaled at line 6 of file tty:
      Division by zero.
      Error signaled at line 6 of file tty:
      Division by zero.
      Error signaled at line 6 of file tty:
      Division by zero.
      Error signaled at line 7 of file tty:
      Arithmetic overflow, threshold=1073741823, cur val=1073741824.
      Error signaled at line 7 of file tty:
      Arithmetic overflow, threshold=1073741823, cur val=1073741824.
      Error signaled at line 7 of file tty:
      Arithmetic overflow, threshold=1073741823, cur val=1073741824.
      Error signaled at line 8 of file tty:
      Arithmetic overflow, threshold=2^{14}
      for 16384.
      Error signaled at line 8 of file tty:
      Arithmetic overflow, threshold=2^{14}
      for 16384.
      Error signaled at line 8 of file tty:
      Arithmetic overflow, threshold=2^{14}
      for 16384.
      Error signaled at line 8 of file tty:
      Arithmetic overflow, threshold=2^{14}
      for 16384.
      Error signaled at line 9 of file tty:
      Arithmetic overflow, threshold=2^{30}=16384pt
      in 107413504*10.
      Error signaled at line 9 of file tty:
      Arithmetic overflow, threshold=2^{14}
      for 16384.
      Error signaled at line 9 of file tty:
      Arithmetic overflow, threshold=2^{30}=16384pt
      in 1073741823*10.
      Error signaled at line 10 of file tty:
      Arithmetic overflow, threshold=2^{14}
      for 16384.
      Error signaled at line 10 of file tty:
      Arithmetic overflow, threshold=2^{30}=16384pt
      in 1073741823*10.
      Error signaled at line 10 of file tty:
      Arithmetic overflow
      in 983040000*3/2.
      Error signaled at line 11 of file tty:
      Arithmetic overflow, threshold=2^{31}
      in 2147483000+700.
      Error signaled at line 11 of file tty:
      Arithmetic overflow, threshold=2^{30}
      in 983040000+983040000

There are at most 256 possible fonts, and you might see *fatal: font
table overflow* after too many uses of ``\newfont``. Trying to modify a
parameter of non-existing font is an error. Moreover, indices must be in
the range zero to 100000. The last line here is correct.

.. container:: ltx-source

   ::

      1 \newfont\foo=cmr10
      2 \skewchar \empty 10 \fontdimen0 \empty 10pt
      3 \fontdimen -3 \foo 10pt \fontdimen 123456 \foo 10pt
      4 \skewchar \foo 10 \fontdimen0 \foo 10pt

.. container:: log_out

   ::

      Error signaled at line 2 of file tty:
      Missing font identifier.
      Error signaled at line 2 of file tty:
      attempt to modify unexistent font param.
      Error signaled at line 2 of file tty:
      Missing font identifier.
      Error signaled at line 2 of file tty:
      attempt to modify unexistent font param.
      Error signaled at line 3 of file tty:
      attempt to modify unexistent font param.
      Error signaled at line 3 of file tty:
      attempt to modify unexistent font param.

We show some errors signaled while parsing numbers

.. container:: ltx-source

   ::

       1 \catcode0=17 \mathcode0=32800 \sfcode0=32800 \delcode0=1234567890
       2 \lccode0=66000 \uccode0=66000 \lccode0=-1
       3 \advance\par by 3pt
       4 \count0=1234567890
       5 \advance \skip0by \muskip0 \advance \muskip0by \skip0
       6 \advance \muskip0by 3pt \advance \muskip0by 3mu
       7 \advance \skip0by 3mu
       8 \advance\skip0 by 3ex plus 2fillll
       9 \count32000=zero \count0=12345678901

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      Invalid code (out of bounds)
      \catcode wants 0<=N<=15, got N=17.
      Error signaled at line 1 of file tty:
      Invalid code (out of bounds)
      \mathcode wants 0<=N<=32768, got N=32800.
      Error signaled at line 1 of file tty:
      Invalid code (out of bounds)
      \sfcode wants 0<=N<=32767, got N=32800.
      Error signaled at line 1 of file tty:
      Invalid code (out of bounds)
      \delcode wants 0<=N<=16777215, got N=1234567890.
      Error signaled at line 2 of file tty:
      Invalid code (out of bounds)
      \lccode wants 0<=N<=65535, got N=66000.
      Error signaled at line 2 of file tty:
      Invalid code (out of bounds)
      \uccode wants 0<=N<=65535, got N=66000.
      Error signaled at line 2 of file tty:
      Invalid code (out of bounds)
      \lccode wants 0<=N<=65535, got N=-1.
      Error signaled at line 3 of file tty:
      Invalid argument for \advance.
      Error signaled at line 5 of file tty:
      Incompatible glue units in \advance
      in conversion from mu to glue.
      Error signaled at line 5 of file tty:
      Incompatible glue units in \advance
      in conversion from glue to mu.
      Error signaled at line 6 of file tty:
      Incompatible glue units in \advance
      Expected mu unit, found something else.
      Error signaled at line 7 of file tty:
      Missing unit (replaced by pt) m.
      Error signaled at line 8 of file tty:
      Illegal unit of measure (replaced by filll).
      Error signaled at line 9 of file tty:
      Bad register code replaced by 0
      \count wants 0<=N<=1023, with N=32000.
      Error signaled at line 9 of file tty:
      Missing number, treated as zero scanned by \count.
      Error signaled at line 9 of file tty:
      number too big.

The first argument of ``\setlength`` has to be a command name, generally
a register allocated by ``\newlength``. Since there is only a finite
number of counters, you may see errors like *Overflow in \\newdimen; max
value is 1023*. The argument to ``\newif`` must start with the two
letters I and F. The ``\read`` command expands its arguments when
looking at a 'to' keyword, this is why we make sure ``\foo`` exists. We
show some errors signaled by the ``calc`` package. **Note:** the last
line number should be 9 and not 8. This is because of the
``\usepackage`` command in interactive mode; if you put the code in a
file, everything will be OK; however, if the code follows
``\begin{document}``, you will get another error; *Bad \\usepackage
command. The following command is not in outer vertical mode:
\\usepackage*. Note also that the line without a number should not be
put in the file, but given as answer to ``\read``. If nevertheless it is
in the file, then line you type will be executed. in particular, you get an
endless loop if the answer contains ``\foo``.

.. container:: ltx-source

   ::

       1 \newlength\mylength \newif\myif
       2 \setlength{\dimen0}{2cm}\setlength{\mylength}{2cm}
       3 \the\par
       4 \let\foo\relax \read0 \foo
           This line is read into \foo
       5 \the\numexpr(2=3)
       6 \the\fontdimen2\relax
       7 \par\usepackage{calc}
       8 \setlength{\parindent}{3Pt/0}\setlength{\parindent}{3Pt=0}
       8 \setbox0=\par

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      token list \newif does not start with if.
      Error signaled at line 2 of file tty:
      The command \setlength takes one token as argument.
      Error signaled at line 3 of file tty:
      You can't use `\par' after \the.
      Error signaled at line 4 of file tty:
      Missing to inserted.
      Error signaled at line 5 of file tty:
      Expected + - * / or ); closing paren inserted.
      Error signaled at line 6 of file tty:
      Missing font identifier.
      Error signaled at line 8 of file tty:
      Arithmetic overflow, division by 0
      in 196608/0.
      Error signaled at line 8 of file tty:
      unexpected token in calc
      {Character = of catcode 12}.
      Error signaled at line 8 of file tty:
      A box was assumed to be here, got \par.

Errors in the bibliography
--------------------------

At the end of the job, *Tralics* reads the bibliography, in order to
solve all unsolved references, and translates the corresponding. parts.
The bibliography file shown here has some errors. We shall comment them
one after the other.

For Inria's activity report you can see additional errors of the form
*entry moved from refer to year because* either there is no year field,
it is invalid, is the current year, or because the reference is not yet
published. There are errors of the form *the year field of this entry
should be* XXX, it is missing or YYY.

.. container:: ltx-source

   ::

      26 \bibliography{test}
      27 \cite{foo, foo1,foo2,foo3,foo4,foo5,foo6,foo7,foo8} 
      28 \bibitem {foo2}\bibitem {foo2}
      29 \section{foo}\XMLsolvecite{foo3}

This is the bibliography database

.. container:: ltx-source

   ::

       3 @article {foo, title = {\cite{bar} etc}}
       4 @article {foo1, title = ok}
       5 @article {foo1, title = {ok}}
       6 @article{foo4, 1=2,
       7   author={,, and {},{},{}}, % not a comment
       8   énot7bit is bad
       9   () parens are bad either
      10   xx={} ,author =0,  yy = "foo"-"bar",
      11   author={123}, 
      12  xx=aa(),
      13  author={123})
      14 @article thiswordwrong (foo5, author={Jos{\'e} Gr{\^\i} mm} )
      15 @article{foo6, author="foo&sons", editor = "a,b,c,d"}
      16 @article{foo7, author="foo{ and sons}}",  }
      17 @article{foo8, author="foo \and sons and Jos\'e Gr\^\i mm",  title="foo \and sons",  }
      18 @article

The effect of ``\bibitem{foo}`` is to mark the reference as solved; you
can solve it only once. The effect of ``\XMLsolvecite{foo3}`` acts like
``\bibitem``; it says that the current XML element solves the reference.
This command may fail if the current element has already a unique id
(here the current XML element is the section). The last line here is not
an error: it is just an information, *Tralics* has seen 9 citations, and
has solved one.

.. container:: log_out

   ::

      Error signaled at line 28 of file te.tex:
      Bibliography entry already defined foo2.
      Translating section command div0: foo.
      Error signaled at line 29 of file te.tex:
      Cannot solve (element has an Id) foo3.
      Bib stats: seen 9(1) entries.

There are two entries called ``foo1``, the second one will be ignored.
The first one has ``title=ok``, and *Tralics* needs a value for the
macro ``ok``.

.. container:: log_out

   ::

      Error detected at line 4 of bibliography file test.bib
      in entry foo1 started at line 4
      undefined macro ok.
      Error detected at line 5 of bibliography file test.bib
      in entry foo1 started at line 5
      duplicate entry ignored.

Entry ``foo4`` has a lot of errors. We have first ``1=2``, this is
illegal; a field cannot start with a percent sign, a 8bit-character, an
open parenthesis, etc. In all these cases, the remainder of the current
line will be ignored (and the parser may be in a silly state). You say
``foo#bar`` if you want to concatenate two values; on line 10 there is a
dash, and *Tralics* complains as it expected a comma. The last error is
that the entry is terminated by a parenthesis instead of a closing brace
(a closing parenthesis is allowed, provided that the entry starts with a
parenthesis, see entry foo5).

.. container:: log_out

   ::

      Error detected at line 6 of bibliography file test.bib
      in entry foo4 started at line 6
      bad syntax for a field name
      it cannot start with a digit
      remaining characters on current line will be ignored.
      Error detected at line 7 of bibliography file test.bib
      in entry foo4 started at line 6
      bad syntax for a field name
      it cannot start with `%'
      (A percent sign is not a comment character in bibtex)
      remaining characters on current line will be ignored.
      Error detected at line 8 of bibliography file test.bib
      in entry foo4 started at line 6
      bad syntax for a field name
      it cannot start with `é'
      remaining characters on current line will be ignored.
      Error detected at line 9 of bibliography file test.bib
      in entry foo4 started at line 6
      bad syntax for a field name
      it cannot start with `('
      remaining characters on current line will be ignored.
      Error detected at line 10 of bibliography file test.bib
      in entry foo4 started at line 6
      duplicate field `author' ignored.
      Error detected at line 10 of bibliography file test.bib
      in entry foo4 started at line 6
      expected comma before a field.
      Error detected at line 10 of bibliography file test.bib
      in entry foo4 started at line 6
      bad syntax for a field name
      expected `=' sign
      remaining characters on current line will be ignored.
      Error detected at line 11 of bibliography file test.bib
      in entry foo4 started at line 6
      bad syntax for a field type
      it cannot end with `='
      expecting `,', `#' or `}'
      remaining characters on current line will be ignored.
      Error detected at line 12 of bibliography file test.bib
      in entry foo4 started at line 6
      bad syntax for a field type
      it cannot end with `('
      expecting `,', `#' or `}'
      remaining characters on current line will be ignored.
      Error detected at line 13 of bibliography file test.bib
      in entry foo4 started at line 6
      duplicate field `author' ignored.
      Error detected at line 13 of bibliography file test.bib
      in entry foo4 started at line 6
      bad end delimiter.

On line 15 there is some junk between the keyword ``@article`` and the
opening brace. On line 16 braces do not balance in the string. Finally,
the file ends in the middle of an entry.

.. container:: log_out

   ::

      Error detected at line 14 of bibliography file test.bib
      bad syntax for an entry type
      expected `('  or `{'
      all characters up to next `('  or `{' ignored.
      Error detected at line 16 of bibliography file test.bib
      in entry foo7 started at line 16
      illegal closing brace.
      Error detected at line 18 of bibliography file test.bib
      Illegal end of bibtex database file.

After finishing analysis of the data base, *Tralics* signals an error:
reference foo3 was not found. Some additional errors are signaled. A
name is formed of three parts; last name, first name, junior, separated
by commas. *Tralics* insists that an empty slot be marked by ``{}``.
Moreover at least one of these fields has to be non-empty (if no comma
is given, the only fields contains the first and the last name).

.. container:: log_out

   ::

      Error signaled while handling entry cite:foo3
      undefined reference.
      Error signaled while handling entry cite:foo4 (line 6)
      misplaced comma in bibtex name
      you should say "{},{},foo", instead of  ",,foo" in 
       ,, and {},{},{}.
      Error signaled while handling entry cite:foo4 (line 6)
      misplaced comma in bibtex name
      you should say "{},{},foo", instead of  ",,foo" in 
       ,, and {},{},{}.
      Error signaled while handling entry cite:foo4 (line 6)
      empty name in
       ,, and {},{},{}.

Multiple names should be separated by ``and``, not by commas or an
ampersand. A particularity of bibtex is that commands should not appear
at top-level. You should for instance say ``Jos{\'e}``. *Tralics* allows
``Jos\'e`` as well as ``José``. It signals an error if you want to put
an accent on a non-letter as in ``Gr\^\i mm``. Note also that *Tralics*
considers ``mm`` as the last in this case.

.. container:: log_out

   ::

      Error signaled while handling entry cite:foo6 (line 15)
      unexpected character `&' (did you mean `and' ?)
      bad syntax in author or editor name
      error occurred at character position 4 in the string
       foo&sons.
      Error signaled while handling entry cite:foo6 (line 15)
      too many commas (namely 3) in name
       a,b,c,d.
      Error signaled while handling entry cite:foo8 (line 17)
      commands allowed only within braces
      bad syntax in author or editor name
      error occurred at character position 5 in the string
       foo \and sons and Jos\'e Gr\^\i mm.
      Error signaled while handling entry cite:foo8 (line 17)
      bad accent construct
      bad syntax in author or editor name
      error occurred at character position 28 in the string
       foo \and sons and Jos\'e Gr\^\i mm.
      Error signaled while handling entry cite:foo8 (line 17)
      commands allowed only within braces
      bad syntax in author or editor name
      error occurred at character position 30 in the string
       foo \and sons and Jos\'e Gr\^\i mm.

*Tralics* tells us that it extract seven entries from the data base
(remember that one entry was already solved, and another one missing).
Finally, the translation of entry foo had ``\cite`` command in it. This
is illegal.

.. container:: log_out

   ::

      Seen 7 bibliographic entries.
      Error signaled at line 12:
      Citation after loading biblio?.
      There were 28 errors.

Other errors
------------

The ``\end{figure}`` commands performs some checks, that fail if
``\begin{figure}`` is redefined. You cannot put ``\begin{document}``
inside a group; you cannot use ``\endcsname`` before ``\csname``. You
cannot use hat or underscore characters outside of math mode. There are
30 commands, for instance ``\lastkern``, that are read-only. You should
use them as ``\the\lastkern`` or ``\skip0=\lastkern``; in other cases an
error is signaled; if the command takes an argument, they are not read.
If you say ``\@onlypreamble\foo``, then ``\foo`` cannot be used after
``\begin{document}``. Assume that ``\begin{foo}`` had not been used.
Then ``\endfoo`` is an error, and ``\endfoo`` is not called. But if this
appears in the text then either *Tralics* complains that the command is
undefined, or does nothing, or says that the stack does not contain some
object; There are a few commands (like math environments) that say that
``\begin`` is missing.

.. container:: ltx-source

   ::

      1 \makeatletter
      2 \def\figure {} \begin{figure}  \end{figure}
      3 \def\table {} \begin{table} \end{table}
      4 \@onlypreamble\url.
      5 \begingroup \begin{document} \endcsname ^_
      6 \the\lastkern \skip0=\lastkern
      7 \lastkern \numexpr \fontcharwd \url
      8 \end{foo}\endfoo \endlist \endcenter \endequation

.. container:: log_out

   ::

      Error signaled at line 2 of file tty:
      no figure/table on stack.
      Error signaled at line 2 of file tty:
      Error in pop; stack holds document; trying to pop figure.
      Error signaled at line 3 of file tty:
      no figure/table on stack.
      Error signaled at line 3 of file tty:
      Error in pop; stack holds document; trying to pop table.
      Error signaled at line 5 of file tty:
      \begin{document} not at level 0.
      Error signaled at line 5 of file tty:
      Extra \endcsname.
      Error signaled at line 5 of file tty:
      Missing dollar not inserted, token ignored: {Character ^ of catcode 7}.
      Error signaled at line 5 of file tty:
      Missing dollar not inserted, token ignored: {Character _ of catcode 8}.
      Error signaled at line 7 of file tty:
      Read only variable \lastkern.
      Error signaled at line 7 of file tty:
      Read only variable \numexpr.
      Error signaled at line 7 of file tty:
      Read only variable \fontcharwd.
      Error signaled at line 7 of file tty:
      Can be used only in preamble: \url.
      Error signaled at line 8 of file tty:
      cannot close environment foo.
      Error signaled at line 8 of file tty:
      Undefined command \endfoo.
      Error signaled at line 8 of file tty:
      Error in pop; stack holds document; trying to pop list.
      Error signaled at line 8 of file tty:
      missing \begin environment \endequation.

Errors signaled by the the date package

.. container:: ltx-source

   ::

      207 \usepackage{datenumber}
      208 \setdatenumber{2011}{04}{30} \datedate
      209 \setdatenumber{2011}{13}{30}\setdatenumber{2011}{-5}{30}
      210 \setdatenumber{2011}{04}{32}\setdatenumber{2011}{05}{32}
      211 \setdatenumber{2011}{04}{-3}\setdatenumber{-10}{04}{32}
      212 \setdatenumber{2011}{02}{30}\setdatenumber{2012}{02}{30}

.. container:: log_out

   ::

      Error signaled at line 209 of file tty:
      Date error: month>12 2011/13/30.
      Error signaled at line 209 of file tty:
      Date error: month<1 2011/-5/30.
      Error signaled at line 210 of file tty:
      Date error: day>30 2011/4/32.
      Error signaled at line 210 of file tty:
      Date error: day>31 2011/5/32.
      Error signaled at line 211 of file tty:
      Date error: day<1 2011/4/-3.
      Error signaled at line 211 of file tty:
      Date error: year<1 -10/4/32.
      Error signaled at line 211 of file tty:
      Package datenumber: datenumber < startyear (-10 < \@startyear ).
      Error signaled at line 212 of file tty:
      Date error: day>28 2011/2/30.
      Error signaled at line 212 of file tty:
      Date error: day>29 2012/2/30.

Error signaled by the fp package
--------------------------------

.. container:: ltx-source

   ::

       9 \FPadd\foo{999999999999999999}{.5} ok
      10 \FPmul\fooe\foo{1.0000001} overflow
      11 \FPadd\fooe\foo{.5} bad
      12 \FPdiv\fooe\foo{0.99} 
      13 \FPpascal\foo{200}
      14 \FPtrunc\foo{1234}{-3}
      15 \FPround\foo{1234}{-4}
      16 \FPdiv\foo{3}{0}
      17 \FPln\foo{-3.4}\FPln\foo{0}
      18 \FPexp\foo{41} \FPexp\foo{42}
      19 \FPpow\foo{-1}{2}\FPpow\foo{0}{-2}
      20 \FParccos\foo{1}
      21 \FParccos\foo{1.2}
      22 \FParcsin\foo{1.2}
      23 \FPround\foo{12345678901234567890}{4}

.. container:: log_out

   ::

      Error signaled at line 10 of file te.tex:
      Overflow in FPmul.
      Error signaled at line 11 of file te.tex:
      Overflow in FPadd.
      Error signaled at line 12 of file te.tex:
      Overflow in FPdiv.
      Error signaled at line 13 of file te.tex:
      out of bound 200 max value is 63.
      Error signaled at line 14 of file te.tex:
      Negative number -3 in truncate.
      Error signaled at line 15 of file te.tex:
      Negative number -4 in round.
      Error signaled at line 16 of file te.tex:
      division by zero.
      Error signaled at line 17 of file te.tex:
      Log of negative number.
      Error signaled at line 17 of file te.tex:
      Log of 0.
      Error signaled at line 18 of file te.tex:
      Overflow in FPexp.
      Error signaled at line 19 of file te.tex:
      First argument negative.
      Error signaled at line 21 of file te.tex:
      Number greater than one for \FP@arccos.
      Error signaled at line 22 of file te.tex:
      Number greater than one for \FP@arcsin.
      Error signaled at line 23 of file te.tex:
      overflow in FPread.

Error in math mode
------------------

In a number of cases, *Tralics* stops scanning tokens in math mode when
it sees an error, because it generally minimizes the number of further
errors. Here are some errors.

.. container:: ltx-source

   ::

       1 $\left+\right -$
       2 $\left ( no right;$ 
       3 $no left but \right.$
       4 $a}  we are out of math here
       5 $a \par we are out of math here
       6 $a \frac{b} \par  we are out of math here
       7 $ \end{displaymath} we are out of math here
       8 $$ x $ we are out of math here
       9 $x {  $ y} $ we are out of math here
      10 $ \frac {$} 2$ we are out of math here
      11 $\begin{array}{c} $ x \end{array}$  we are out of math here

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      Invalid character in \left or \right
      {Character + of catcode 12}.
      Error signaled at line 1 of file tty:
      Invalid character in \left or \right
      {Character - of catcode 12}.
      Error signaled at line 2 of file tty:
      Missing \right. inserted.
      Error signaled at line 3 of file tty:
      Unexpected \right.
      Error signaled at line 4 of file tty:
      Unexpected }.
      Error signaled at line 5 of file tty:
      Unexpected \par.
      Error signaled at line 6 of file tty:
      Unexpected \par while scanning argument of \frac.
      Error signaled at line 6 of file tty:
      Unexpected }.
      Error signaled at line 7 of file tty:
      Bad \end{displaymath}.
      Error signaled at line 8 of file tty:
      Display math should end with $$.
      Error signaled at line 9 of file tty:
      Out of scope $ ignored, maybe a } is missing here.
      Error signaled at line 10 of file tty:
      Extra $ ignored while scanning argument of \frac.
      Error signaled at line 11 of file tty:
      Extra $ ignored....

In a table, the command ``\multicolumn`` takes 3 arguments. In the first
case shown here, there are too many tokens; this is harmless. In the
second case, there are two few arguments and the ampersand is considered
the last argument. In the third case, the ``\end`` token is the
argument, and *Tralics* is in a bad state; it needs two ``\par``\ s to
get out of math mode. There is a problem on the next line: the ``array``
environment has an optional argument, and the closing bracket is
missing; the mandatory argument is also missing. Some environments like
``gather`` enter display math mode; they are forbidden in math mode. You
can use ``\eqno`` only in display math mode. Only one label is allowed
in a formula. Note the error on the last line: in some case it is not
possible to convert the math expression into MathML.

.. container:: ltx-source

   ::

      1 $\begin{array}{c} \multicolumn{2}{a}{b}{c} & d\end{array} $
      2 $\begin{array}{c} \multicolumn{2}{a}& \end{array} $
      3 $\begin{array}{c} \multicolumn{2}{a}\end{array} $ \par\par
      4 $\begin{array} [ \end{array} $ 
      5 $x \begin {gather} xx \end{gather}y $
      6 $x\eqno y \label {a}\label {b}$
      7 $a\over b \over c  a^b^c a_b_c  a^ ^ b__  c-^d_$
      8 $\hbox{\vbox{foo}} \xbox{foo}{bar}$

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      Cell contains garbage after \multicolumn.
      Error signaled at line 2 of file tty:
      Wrong goup delimiter
      got `cell', expected `math'.
      Error signaled at line 2 of file tty:
      Wrong goup delimiter
      got `math', expected `cell'.
      --- {Character & of catcode 4}
      Error signaled at line 2 of file tty:
      Bad math expression.
      Error signaled at line 3 of file tty:
      Missing { inserted before unexpected } in argument of {Character $ of catcode 3}.
      Error signaled at line 3 of file tty:
      Bad \end{}.
      Error signaled at line 3 of file tty:
      Extra $ ignored....
      Error signaled at line 3 of file tty:
      Unexpected \par.
      Error signaled at line 3 of file tty:
      Unexpected \par.
      Error signaled at line 3 of file tty:
      Cell contains garbage after \multicolumn.
      Error signaled at line 4 of file tty:
      missing ].
      Error signaled at line 4 of file tty:
      missing argument.
      Error signaled at line 5 of file tty:
      bad math env gather.
      Error signaled at line 6 of file tty:
      Command \eqno allowed only in display math.
      Error signaled at line 6 of file tty:
      Label will be lost: a.
      Error signaled at line 7 of file tty:
      Too many commands of type \over.
      Error signaled at line 7 of file tty:
      double superscript.
      Error signaled at line 7 of file tty:
      double subscript.
      Error signaled at line 7 of file tty:
      Two consecutive _ or ^ characters.
      Error signaled at line 7 of file tty:
      Two consecutive _ or ^ characters.
      Error signaled at line 7 of file tty:
      Math formula should not finish with _.
      Error signaled at line 8 of file tty:
      bad hbox (see transcript file for details).
      --- \xbox
      Error signaled at line 8 of file tty:
      Bad math expression.

Parsing errors
--------------

| In some cases *Tralics* expects an opening brace and sees none, or
  sees a closing brace. Various error messages may be issued:
| *A left brace was mandatory here, so I've put one in*,
| *Extra closing brace (scanning started at line 17) for argument to
  \\foo*,
| *Too many closing braces*
| *Missing { inserted before unexpected } in argument of \\foo*,
| *Spurious close brace removed*,
| *missing brace for token register \\everyhbox*,
| *too many closing braces in \\read*.

| A *runaway* error is signaled when scanning of an argument is
  interrupted by an end-of-file or sometimes ``\par`` or any macro
  declared ``\outer``. You could see messages like
| *End of data reached while scanning end of environment foo*,
| *End of data reached while scanning a group; scanning argument of
  \\foo*
| *End of data reached while scanning argument of \\foo*
| *End of data reached while scanning start of argument*
| *Runaway argument?; Paragraph ended while scanning an argument of
  \\foo*
| *End of data reached while scanning \\fi for condition started on line
  17*
| *End of data reached while expecting \\endcsname*
| *End of data reached while scanning content of foo*;
| *end of data instead of a number after #*
| *Unterminated \\if*
| *\\if at end of file*
| *unbalanced \\read*.

On line 3, the name of the new command is illegal and the body is
missing. On line four, the open brace is missing after the (optional)
arguments of ``\hbox``. Note the that end-of-file after ``\iffalse``
confuses *Tralics* a bit.

.. container:: ltx-source

   ::

       1 \def\foo#1{} \foo{\par}
       2 \outer\def\Foo{} \def\foo#1{} \foo{\Foo}
       3 \newcommand{\foo\bar}. 
       4 \hbox foo}
       5 \iffalse

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      Runaway argument?
      Paragraph ended while scanning an argument of \foo.
      Error signaled at line 1 of file tty:
      Extra }.
      Error signaled at line 2 of file tty:
      Runaway argument?
      Forbidden control sequence \Foo found while scanning use of \foo.
      Error signaled at line 2 of file tty:
      Extra }.
      Error signaled at line 3 of file tty:
      Attempt to define: \foo \bar .
      Error signaled at line 3 of file tty:
      Only one token allowed in argument list of \newcommand.
      Error signaled at line 3 of file tty:
      A left brace was mandatory here, so I've put one in.
      Argument was replaced by {}.
      Error signaled at line 4 of file tty:
      A left brace was mandatory here, so I've put one in.
      Error signaled at line 5 of file tty:
      Incomplete \if? missing \fi inserted
      End of file in conditional started at line 5.
      Error signaled at line 5 of file tty:
      End of data reached while scanning \fi
      for condition started on line 5.
      Error signaled at line 5 of file tty:
      Extra \fi
      I'm ignoring this; it doesn't match any \if.

On the first line the specification of the the number of arguments is
wrong. On the second line there should be ``#1`` instead of ``#2``. On
line 3, there are attempt to insert ``\char`` inside a name. On line 4,
we try to globally set the counter ``foo``; this is an error since the
counter is undefined. Note the assignment is global, so that ``\global``
is useless; in this case, the prefix applies to ``{0}`` since other
tokens have been discarded. Line 5 shows that the prefix is usually
accepted, but other prefixes like ``\log`` or ``\outer`` apply only to
``\def``. One line 6 we have a silly definition of ``\c@foo``.

.. container:: ltx-source

   ::

      1 \newcommand\fooA[25]{} \newcommand\fooB[\one]{} \newcommand\fooC[x]{} 
      2 \def\foo#2{}
      3 \csname \char 32\endcsname \newcounter{\char 32}
      4 \global\relax\setcounter{foo}{0}
      5 \global\setcounter{page}{0}\long\outer\setcounter{page}{0}
      6 \makeatletter\let\c@foo\relax\value{foo}

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      Bad syntax in optional argument
      Only one token allowed
      I will assume that the command takes no argument.
      Error signaled at line 1 of file tty:
      Bad syntax in optional argument
      Commands are not allowed
      I will assume that the command takes no argument.
      Error signaled at line 1 of file tty:
      Bad syntax in optional argument
      Argument should be a digit
      I will assume that the command takes no argument.
      Error signaled at line 2 of file tty:
      Error while scanning definition of \foo
      got #{Character 2 of catcode 12}, expected #1.
      Error signaled at line 3 of file tty:
      Invalid token \char
      found between \csname and \endcsname.
      Error signaled at line 3 of file tty:
      Extra \endcsname.
      Error signaled at line 3 of file tty:
      Invalid token \char
      found in counter name.
      Error signaled at line 3 of file tty:
      Extra \endcsname.
      Error signaled at line 4 of file tty:
      Unknown counter `foo'.
      Error signaled at line 4 of file tty:
      You cannot use the prefix \global before {Character { of catcode 1}.
      Error signaled at line 5 of file tty:
      You cannot use the prefix \global\long\outer  before \c@page.
      Error signaled at line 6 of file tty:
      Invalid counter `foo'.

We show here some errors produced by the ``\verb`` command. It cannot be
an argument of a command, and the argument of ``\verb`` must be on the
current line. You can define a short-verb command, so that ``|foo|`` is
the same as ``\verb+foo+``. You can save verbatim text (see line 5). The
verbatim text has to be on the current line. At line 8, we see that a
verbatim-like environment cannot be an argument of a command.

At line 9 is an instance of a special error: in some cases, *Tralics*
expects an internal buffer to be empty. This is normally the case if
``\item`` immediately follows the start of the environment, this is also
the case when the environment is not buried in a box or a table like
here. A remedy might be to insert a ``\par`` token before the ``\item``

.. container:: ltx-source

   ::

      1 \def\foo{\verb=bar=} \foo ?
      2 \verb+foo
      3 \DefineShortVerb{\|}\DefineShortVerb{\|}\UndefineShortVerb{\+}
      4 \DefineShortVerb{}\DefineShortVerb{abc}\DefineShortVerb{\abc}
      5 \SaveVerb{foo}|bar| \SaveVerb{foo}
      6 \endlinechar=-1
      7 \verb
       ok \def\foo{\begin{comment} etc} \foo
      9 \hbox{\begin{description} bad\item o\end{description}}

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      \verb in argument of a command.
      Error signaled at line 2 of file tty:
      \verb terminated by end of line.
      Error signaled at line 3 of file tty:
      \DefineShortVerb: Already a short verb \|.
      Error signaled at line 3 of file tty:
      \UndefineShortVerb: Not a short verb \+.
      Error signaled at line 4 of file tty:
      \DefineShortVerb: One argument required.
      Error signaled at line 4 of file tty:
      \DefineShortVerb: One argument required.
      Error signaled at line 4 of file tty:
      \DefineShortVerb: A one-char control sequence is needed instead of \abc.
      Error signaled at line 5 of file tty:
      \SaveVerb : no delimiter found.
      Error signaled at line 7 of file tty:
      \verb at end of line.
      Error signaled at line 8 of file tty:
      Verbatim-like environment in argument : comment.
      Error signaled at line 9 of file tty:
      In \item, non-empty buffer 
       bad
      Some text may be lost.

Macros
------

We start with showing what happens when an undefined command or
environment is seen. We also show some errors that happen when defining
a command. The last error on line 9 is because\ ``\@ifdefinable``
signals an error when the command cannot be defined.

.. container:: ltx-source

   ::

      1 \undefined \begin{undefined} \end{undefined}
      2 \newcommand{b}\renewcommand{b}
      3 \newcommand{\b}\renewcommand{\B}
      4 \newcommand{}{}\providecommand{12}{}
      5 \newboolean{foo}\newboolean{foo}\newboolean{\par}
      6 \setboolean {foo}{OK}\setboolean {bar}{false}
      7 \def\foo\bar#1{#1} \foo c
      8 \def\foo#x#1{xx #x#3}
      9 \makeatletter \@ifdefinable{foo}{ok}\@ifdefinable\bar{ok}

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      Undefined command \undefined.
      Error signaled at line 1 of file tty:
      Undefined environment undefined.
      Error signaled at line 2 of file tty:
      Error signaled at line 2 of file tty:
      A left brace was mandatory here, so I've put one in.
      Argument was replaced by {}.
      Error signaled at line 3 of file tty:
      A left brace was mandatory here, so I've put one in.
      Argument was replaced by {}.
      Error signaled at line 3 of file tty:
      Error signaled at line 3 of file tty:
      Undefined command \B.
      Error signaled at line 4 of file tty:
      Empty argument list for \newcommand.
      Error signaled at line 4 of file tty:
      Attempt to define: 12.
      Error signaled at line 4 of file tty:
      Only one token allowed in argument list of \providecommand.
      Error signaled at line 5 of file tty:
      boolean \iffoo already defined in \newboolean.
      Error signaled at line 5 of file tty:
      Invalid token \par found while scanning newboolean.
      Error signaled at line 5 of file tty:
      Extra \endcsname.
      Error signaled at line 6 of file tty:
      boolean value neither true nor false: ok.
      Error signaled at line 6 of file tty:
      Undefined boolean \barfalse.
      Error signaled at line 7 of file tty:
      Use of \foo doesn't match its definition;
      got c, expected \bar.
      Error signaled at line 8 of file tty:
      Error while scanning definition of \foo
      got #x, expected #1.
      Error signaled at line 8 of file tty:
      bad token (x) after #.
      Error signaled at line 8 of file tty:
      bad token ({Character 3 of catcode 12}) after #.
      Error signaled at line 9 of file tty:
      Wrong syntax in \@ifdefinable.
      Error signaled at line 9 of file tty:
      Undefinable command \bar.

The commands ``&`` and ``\\`` can be used in an array. The command
``\\`` also be used as end of paragraph marker, in some cases; otherwise
an error will be signaled. An error is signaled for the two ``\url``
commands for historical reasons. The error message of ``\href`` is a bit
strange.

.. container:: ltx-source

   ::

      1 \begin{tabular}{c}\newline 1\\2 \end{tabular}
      2 \hbox{a\\ b\newline c} &
      3 \sin  o
      4 \color{blue}
      5 \url{()}\url{}
      6 \href{\texttt{x}}{y}  

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      Bad use of \newline.
      Error signaled at line 2 of file tty:
      Bad use of \\.
      Error signaled at line 2 of file tty:
      Bad use of \newline.
      Error signaled at line 2 of file tty:
      Unexpected &.
      Error signaled at line 3 of file tty:
      Math only command \sin.
      (Contrarily to TeX, Tralics does not switch to math much in such a case.)
      Error signaled at line 4 of file tty:
      Undefined color blue.
      Error signaled at line 5 of file tty:
      Invalid URL value.
      Error signaled at line 5 of file tty:
      Invalid URL value.
      Error signaled at line 6 of file tty:
      unexpected font change tt.
      you should perhaps use \Href{\url{x}}{y}
        instead of \Href{y}{\url{x}}

The commands ``&`` and ``\\`` can be used in an array. The command
``\\`` also be used as end of paragraph marker, in some cases; otherwise
an error will be signaled

Some other errors

.. container:: ltx-source

   ::

      1 \count0=`\bar
      2 \includegraphics[1=2]{foo}
      3 \ifnum 1 2 \fi \hbox foo}
      4 \label {foo}\label{foo}
      5 \end{} \unless\par
      6 \shipout\box255
      7 \DeclareTextCommand \foo{body of foo}
      8 \xbox{a{\bf b}$c$}{the box}

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      Improper alphabetic constant \bar.
      Error signaled at line 2 of file tty:
      Invalid key in \includegraphics 1 = 2.
      Error signaled at line 3 of file tty:
      Missing = inserted for comparison {Character 2 of catcode 12}.
      Error signaled at line 3 of file tty:
      A left brace was mandatory here, so I've put one in.
      Error signaled at line 4 of file tty:
      Label 'foo' multiply defined (first use line 4 file tty).
      Error signaled at line 5 of file tty:
      Illegal \end{}.
      Error signaled at line 5 of file tty:
      You cannot use \unless before \par.
      Error signaled at line 6 of file tty:
      \shipout is undefined.
      Error signaled at line 7 of file tty:
      Unimplemented NFSS command \DeclareTextCommand.
      Error signaled at line 8 of file tty:
      A left brace was mandatory here, so I've put one in.
      Argument was replaced by {}.

There are some not-yet implemented commands.

.. container:: ltx-source

   ::

      1 \middle \accent 27 \delimiter 17
      2 \halign {etc} \insert 12{value} \mathaccent 12 \mathchar 13
      3 \noalign \noboundary \omit \radical 12
      4 \vadjust {code}
      5 \valign \span \dump \widthof \heightof \depthof 
      7 \vcenter{a box}
      7 \textquotestraightbase
      8 \textquotestraightdblbase

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      Unimplemented e-TeX extension \middle.
      Error signaled at line 1 of file tty:
      Unimplemented command \accent.
      Error signaled at line 1 of file tty:
      Unimplemented command \delimiter.
      Error signaled at line 2 of file tty:
      Unimplemented command \halign.
      Error signaled at line 2 of file tty:
      Unimplemented command \insert.
      Error signaled at line 2 of file tty:
      Unimplemented command \mathaccent.
      Error signaled at line 2 of file tty:
      Unimplemented command \mathchar.
      Error signaled at line 3 of file tty:
      Unimplemented command \noalign.
      Error signaled at line 3 of file tty:
      Unimplemented command \noboundary.
      Error signaled at line 3 of file tty:
      Unimplemented command \omit.
      Error signaled at line 3 of file tty:
      Unimplemented command \radical.
      Error signaled at line 4 of file tty:
      Unimplemented command \vadjust.
      Error signaled at line 5 of file tty:
      Unimplemented command \valign.
      Error signaled at line 5 of file tty:
      Unimplemented command \span.
      Error signaled at line 5 of file tty:
      Unimplemented command \dump.
      Error signaled at line 5 of file tty:
      Unimplemented command \widthof.
      Error signaled at line 5 of file tty:
      Unimplemented command \heightof.
      Error signaled at line 5 of file tty:
      Unimplemented command \depthof.
      Error signaled at line 6 of file tty:
      Unimplemented command \vcenter.
      Error signaled at line 7 of file tty:
      Unimplemented command \textquotestraightbase.
      Error signaled at line 8 of file tty:
      Unimplemented command \textquotestraightdblbase.

Keywords
--------

We show here some errors signaled by the xkeyval package.

.. container:: ltx-source

   ::

      30 \makeatletter\usepackage{xkeyval}\par
      31 \define@key {XK\relax V}{foo}
      32 \define@key {XKV}{foo\char16 etc}{12}
      33 \disable@keys [XKV]{XKV}{foo}
      34 \define@key {aa}{a}{}\define@key {aa}{b}{}\define@key {aa}{c}{}
      35 \setkeys{aa}{a=1,\savevalue{b}=2, \savevalue{c}=\usevalue{b}}
      36 \setkeys{aa}{a=1,\savevalue{d}=2, \savevalue{c}=\usevalue{a}}
      37 \setkeys{aa}{a=\usevalue\bar}
      38 \setkeys{aa}{\savevalue{c}=\usevalue{c}}
      39 \setkeys{aa}{a,b=c,d=e}

.. container:: log_out

   ::

      Error signaled at line 31 of file tty:
      Bad command \relax in XKV family (more errors may follow).
      Error signaled at line 31 of file tty:
      A left brace was mandatory here, so I've put one in.
      Argument was replaced by {}.
      Error signaled at line 32 of file tty:
      bad key name.
      Error signaled at line 32 of file tty:
      A left brace was mandatory here, so I've put one in.
      Argument was replaced by {}.
      Error signaled at line 32 of file tty:
      Extra \endcsname.
      Error signaled at line 33 of file tty:
      xkeyval: `XKV' prefix is not allowed'.
      Error signaled at line 33 of file tty:
      Undefined key cannot be disabled: foo.
      Error signaled at line 36 of file tty:
      Undefined key: d.
      Error signaled at line 36 of file tty:
      No value recorded for key `a'; ignored.
      Error signaled at line 37 of file tty:
      Invalid \usevalue token.
      Error signaled at line 38 of file tty:
      Replace pointer aborted, (infinite loop?).
      Error signaled at line 39 of file tty:
      No value specified for key `a'.
      Error signaled at line 39 of file tty:
      Undefined key: d.

We show here some errors produced by the kvoptions package. There are a
bunch of errors due to the fact that the Declare-XXX-Option commands
check that no macro will be overwritten (note that, in case of error,
some arguments are not read, this might signal further errors). When
loading the package there is an error as BH expects a boolean (true or
false) and VA takes no argument. We first show the TeX file:

.. container:: ltx-source

   ::

      \usepackage[BH=TRUE,VA=0]{kvo}

This is the package file

.. container:: ltx-source

   ::

       4 \ProvidesPackage{kvo}[2007/10/18 kvoptions test]
       5 \RequirePackage{kvoptions}
       6 \SetupKeyvalOptions{family=MCS,prefix=MCS@}
       7 \DeclareVoidOption{VA}{\let\seeva\relax}
       8 \DeclareVoidOption{VB}{}
       9 \DeclareBoolOption[true]{BH}
      10 \DeclareStringOption[foo]{SB}
      11 \def\MCS@SA{}\DeclareStringOption{SA}
      12 \def\MCS@SB{}\DeclareVoidOption{SB}
      13 \def\MCS@SCtrue{}\DeclareBoolOption{SC}
      14 \def\MCS@SDfalse{}\DeclareBoolOption{SD}
      15 \def\ifMCS@SE{}\DeclareBoolOption{SE}
      16 \def\MCS@SFtrue{}\DeclareComplementaryOption{SF}{BH}
      17 \def\MCS@SGtrue{}\DeclareComplementaryOption{SG}{BH}
      18 \DeclareComplementaryOption{SH}{Sh}
      19 \ProcessKeyvalOptions*

.. container:: log_out

   ::

      Error signaled at line 11 of file kvo.plt:
      Cannot redefine \MCS@SA.
      Error signaled at line 11 of file kvo.plt:
      Cannot redefine \MCS@SB.
      Error signaled at line 12 of file kvo.plt:
      Cannot redefine \MCS@SCtrue.
      Error signaled at line 13 of file kvo.plt:
      Cannot redefine \MCS@SDfalse.
      Error signaled at line 14 of file kvo.plt:
      Cannot redefine \ifMCS@SE.
      Error signaled at line 15 of file kvo.plt:
      Cannot redefine \MCS@SFtrue.
      Error signaled at line 16 of file kvo.plt:
      Cannot redefine \MCS@SGtrue.
      Error signaled at line 17 of file kvo.plt:
      Cannot generate code for `SH', no parent Sh.
      Error signaled at line 18 of file kvo.plt:
      Illegal boolean value TRUE ignored.
      Value  should be true or false in package kvo.
      Error signaled at line 18 of file kvo.plt:
      Option VA takes no argument.

We show here errors produced by the date parser. The first line is OK
(unambiguous). Line 5 is OK (order of items is specified).

.. container:: ltx-source

   ::

      1 \formatdate{23/11/2009}\formatdate{23 jan 2009}
      2 \formatdate{23 jan 29} \formatdate{2003 jan feb}
      3 \formatdate{1 32 33}\formatdate{12345 32 33}
      4 \formatdate{today}\formatdate{1 2}
      5 \formatdate{1 2 3 YMD}
      6 \formatdate{1 22 3 YMD}
      7 \formatdate{1 2 33 YMD}
      8 \formatdate{jan 2 3 YMD} \formatdate{2 3 jan  YMD}

.. container:: log_out

   ::

      Error signaled at line 2 of file tty:
      Unable to distinguish between year day and month.
      Error signaled at line 2 of file tty:
      Date has two month fields.
      Error signaled at line 3 of file tty:
      Date has two fields with value>31.
      Error signaled at line 3 of file tty:
      Too many digits in date field.
      Error signaled at line 4 of file tty:
      Expected digits or a month in letters.
      Error signaled at line 4 of file tty:
      Missing fields in date.
      Error signaled at line 6 of file tty:
      Bad month field.
      Error signaled at line 7 of file tty:
      Bad day field.
      Error signaled at line 8 of file tty:
      Bad year field.
      Error signaled at line 8 of file tty:
      Bad day field.

Classes and styles
------------------

Some errors are signaled by classes or packages. Here are some examples.

.. container:: ltx-source

   ::

      1 \ClassError {myclass}{foo}{bar}
      2 \ClassError {myclass}{error line1\MessageBreak error line2}{unused}
      3 \PackageError {mypack}{foo}{bar}
      4 \PackageError {mypack}{error line1\MessageBreak error line2}{unused}
      5 \GenericError{myprefix}{error line1\MessageBreak error line2}{unused}{unused}
      6 \csname @latex@error\endcsname {aa}{unused}
      7 \error{this is an error}

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      Class myclass Error: foo.
      Error signaled at line 2 of file tty:
      Class myclass Error: error line1
      (myclass)            error line2.
      Error signaled at line 3 of file tty:
      Package mypack Error: foo.
      Error signaled at line 4 of file tty:
      Package mypack Error: error line1
      (mypack)              error line2.
      Error signaled at line 5 of file tty:
      error line1
      myprefixerror line2.
      Error signaled at line 6 of file tty:
      Tralics Error: aa.
      Error signaled at line 7 of file tty:
      Error: this is an error.

There are some restrictions on the use of ``\documentclass{foo}`` namely
that you can use it only once, only before ``\begin{document}`` and not
inside a paragraph. In all these cases, the same error message is
signaled *Not in outer vertical mode*. *Tralics* read the file foo.plt
if it exists, but does nothing if the file does not exist. In the case
of ``\InputClass`` an error is signaled.

.. container:: ltx-source

   ::

      1 \LoadClass{foo}\InputClass{foo}
      2 \documentclass{foo} \usepackage{foo}
      3 \documentclass{foo} \usepackage{foo}
      4 text
      5 \usepackage{foo}\par \usepackage{foo}
      6 \begin{document} \usepackage{foo}
      7 \begin{document} 

.. container:: log_out

   ::

      Error signaled at line 1 of file tty:
      Cannot input foo.clt.
      Error signaled at line 3 of file tty:
      Bad \documentclass
      The following command is not in outer vertical mode: \documentclass.
      Error signaled at line 5 of file tty:
      Bad \usepackage command
      The following command is not in outer vertical mode: \usepackage.
      Error signaled at line 7 of file tty:
      Two environments named document.

Errrors for the activity report (old version)
---------------------------------------------

The Inria activity report uses some keywords like ``\projet``,
``localisation`` etc. They may signal an error of the form *There is a
problem with the command* followed by *It should be placed before
\\\begin{document}* or *It should be given only once*. A team name has
to be given in upper-case and mixed-case letters, like ``APICS`` and
``Apics``. In case of mismatch *Tralics* complains with *Bad Team name
foo should be case equivalent to bar*. The activity report is split into
modules, that have a name and a title. The name of a module is the name
of the file in which the content of the module will be stored. You may
see *Empty module name* or *invalid module name (\\\ and $ forbidden)*
or. *Module name \`foo' already seen* . Modules are grouped into
sections; which are specified by an optional argument. This may provoke
the following errors *Invalid section name* or *No default section*. You
may also meet *Environment foo is not closed* , or *unexpected end of
file* or *Characters ignored before open brace* or *Characters ignored
at end of line*

The command for producing glossaries in hidden in the current version of
*Tralics*. However, you can still use it. An error is signaled if the
environment is empty.

.. container:: ltx-source

   ::

      1 \begin{@glossaire}\csname@glo\endcsname{foo}{bar}\end{@glossaire}
      2 \begin{@glossaire}\end{@glossaire}

.. container:: log_out

   ::

      Error signaled at line 2 of file tty:
      empty glossaire.

Errrors for the activity report (current version)
-------------------------------------------------

Assume that your document starts with ``\documentclass{ra2009}``. In
this case *Tralics* enters RA mode. It may complain with *Fatal error:
Input file name must be team name followed by 2010* (where 2010 could be
replaced by the another quantity). Let's rename the file to test2010.
You will see *Illegal document class ra2009 should be ra2010*. The
reason is a mismatch between the year in the file name and the class
value. Let's rename the file to Team2010. You will see *Fatal error Only
lowercase letters allowed: Team*. If the team name is empty, you will
also see *Illegal file name of the form safir/2002.tex*

Translation of the Inria Activity Report requires a well-formed
configuration file. You might see errors of the form *The configuration
file for the RA is ra2007.tcf or ra.tcf. It must define a value for the
parameter theme_vals*. Other parameters are ur_vals, affiliation_vals,
profession_vals, section_vals. A variant of section_vals is
fullsection_vals. If not at least two sections are defined by one of
these lists, you will see *Config file did not provide sections*. These
errors are fatal: no XML is produced.

The ra2010.cls class file must provide a bunch of commands, otherwise
you may see errors of the form: *Undefined command
\\rawebstartdocument*. The command ``\projet`` is mandatory. It takes
three argument; if the first is not equal to the file name (without the
year), it might complain with *Package Raweb Error: Invalid Team name
exemple.* The command ``\theme`` was once mandatory; it could produce
error messages the form *Empty or missing theme* or *Invalid theme*; as
a consequence theme_vals is no longer required. The first argument of
the ``module`` environment is the section. In case of error you may see
*Invalid section* if the argument is not in the list of sections defined
by the configuration file, or *Illegal access to fullsection list* (this
is weird) or *No default section* (after the first invocation, the
argument becomes the default section, and there is no need to repeat
it), or *Only one module accepted in composition* (in the current
version, 'composition' is an environment, its translation should be the
only module in the composition section). You may also see *Bad section
fondements after logiciels* if modules are not in the right order.
Module titles should not be empty, otherwise you will get *Empty module
name replaced!* (this is an awful hack). The command ``\UR``, specifies
the localisation of the team; it may produce errors like *Empty
localisation value* or *Illegal localisation value*. One argument to
``\pers`` is a localisation. Errors may be of the form *No default
Research Centre defined* or *Invalid Unit Centre*.

Other Errrors
-------------

If *Tralics* cannot find its input file, it complains with *Fatal error:
Cannot open input file foo.tex*. If two arguments appear to *Tralics* to
be input file names, it complains with *Fatal error: Seen two source
files foo.tex and bar*. No file is translated in these cases.

The ``filecontents`` environment may signal *unable to open file for
writing*. It may signal *bad end of filecontents env* if the ``\end``
command is not placed at the start of the line.

When *Tralics* sees a ``\par`` or ``\noindent`` command in horizontal
mode, it expects the current element to be ``<p>``, this may fail and an
*Invalid \\par command* may be signaled. In general a ``<p>`` element is
started. This may confuse the caller. For instance ``\hbox{\noindent}``
signals *Error in pop; stack holds p; trying to pop hbox.* In the case
of ``\vspace`` you could get *Expected a p element on the stack*

| You may encounter errors like
| *Text found in a mode where no text is allowed*.
| *Command \\foo should occur in bibliographic mode only*
| *Current command should occur in tables only*

If an end of file is reached while in a special environment like
``verbatim`` or ``comment`` you will get *End of data reached while
scanning end of environment verbatim*. In other cases no error is
signaled. You may notice that there is a problem by examining the line
``Save stack +24   -17`` printed at the end. It says that the stack is
not empty.

There may be errors while parsing incorrect XML files
