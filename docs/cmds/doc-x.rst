All commands, alphabetic order; letter X
========================================

| This page contains the description of the following commands
  `\\xbox <#cmd-xbox>`__, `\\xdef <#cmd-xdef>`__, `\\xi <#cmd-xi>`__,
  `\\Xi <#cmd-cxi>`__, `\\xiint, \\xiiint, \\xiiiint <#cmd-xiint>`__,
  `\\@xiipt <#cmd-xpt>`__, `\\@xiipt <#cmd-xpt>`__,
  `\\@xipt <#cmd-xpt>`__, `\\@XKV@cc <#cmd-xkvcc>`__,
  `\\@xfontsize <#cmd-xsetfontsize>`__, `\\xleaders <#cmd-xleaders>`__,
  `\\xleftarrow <#cmd-xleftarrow>`__, `\\XML@A@id <#cmd-xmlAid>`__,
  `\\XML@A@name <#cmd-xmlAname>`__, `\\XMLaddatt <#cmd-XMLaddatt>`__,
  `\\XML@B@id <#cmd-xmlAid>`__, `\\XML@B@name <#cmd-xmlAname>`__
  `\\XMLbox@id <#cmd-xmlAid>`__, `\\XMLbox@name <#cmd-xmlAname>`__,
  `\\xmlcomment <#cmd-xmlcomment>`__, `\\XML@copyA <#cmd-xmlcopyA>`__,
  `\\XML@copyB <#cmd-xmlcopyA>`__,
  `\\XMLcurrentid <#cmd-xmlcurrentid>`__,
  `\\XMLcurrentcellid <#cmd-xmlcurrentcellid>`__,
  `\\XMLcurrentrowid <#cmd-xmlcurrentcellid>`__,
  `\\XMLcurrenttableid <#cmd-xmlcurrentcellid>`__,
  `\\XML@delete <#cmd-XMLget>`__, `\\xmlelt <#cmd-xmlelt>`__,
  `\\xmlemptyelt <#cmd-xmlemptyelt>`__, `\\XML@fetch <#cmd-XMLfetch>`__,
  `\\XMLgetatt <#cmd-XMLgetatt>`__, `\\XML@get <#cmd-XMLget>`__,
  `\\XML@insert <#cmd-XMLget>`__, `\\XMLlastid <#cmd-xmllastid>`__,
  `\\xmllatex <#cmd-xmllatex>`__, `\\XML@moveAB <#cmd-xmlmoveAB>`__,
  `\\XML@moveBA <#cmd-xmlmoveAB>`__, `\\XML@parent <#cmd-xmlparent>`__,
  `\\XMLref <#cmd-xmlref>`__, `\\XML@set <#cmd-XMLget>`__,
  `\\XML@setA <#cmd-xmlsetA>`__, `\\XML@setB <#cmd-xmlsetA>`__,
  `\\XML@swapAB <#cmd-xmlmoveAB>`__, `\\@xpt <#cmd-xpt>`__,
  `\\xrightarrow <#cmd-xrightarrow>`__, `\\xscale <#cmd-xscale>`__,
  `\\xscaley <#cmd-xscaley>`__, `\\xscape <#cmd-xspace>`__,
  `\\XMLsolvecite <#cmd-XMLsolvecite>`__,
  `\\xspaceskip <#cmd-xspaceskip>`__, `\\x@tag, \\@xtag <#cmd-xtag>`__,
  `\\@xviipt <#cmd-xpt>`__, `\\@xxpt <#cmd-xpt>`__,
  `\\@xxvpt <#cmd-xpt>`__,
| and environments `xalignat <doc-a.html#env-align>`__,
  `xxalignat <doc-a.html#env-align>`__,
  `xmlelement <#cmd-xmlelement>`__, as well as a note on how to
  `manipulate the XML tree <#manipulate>`__.

--------------

.. _cmd-xbox:

\\xbox (Tralics command)
------------------------

In TeX, you can construct boxes that have a type (vertical or
horizontal), and you can try to modify the size, see
```\vbox`` <doc-v.html#cmd-vbox>`__. In *Tralics*, ``\vbox {xx}`` is the
same as ``\xbox{}{xx}``. This creates an anonymous element containing
``xx``. If you say ``\xbox{a}{xx}``, then the box is named a. The
example below shows that assignments inside the second argument are
local to the group.

.. container:: ltx-source

   ::

      \xbox{foo}{\let\FOO=xA\xbox{bar}{\let\FOO=yB\FOO} C\FOO}

Translation:

.. container:: xml_out

   ::

      <foo>A<bar>By</bar> Cx</foo>

| You can say: ``\setbox13=\xbox{foo}{\large x}``. This will put the XML
  element into box 13, and you can use it later.
| |See...| See ```\pers`` <doc-p.html#cmd-pers>`__ for an example of
  use. See also the ```xmlelement`` <doc-x.html#cmd-xmlelement>`__
  environment. **Note:** Not every character string is a valid XML name,
  and *Tralics* does not check for it.

.. _cmd-xdef:

\\xdef
------

You can say ``\xdef\foo{bar}``. The syntax is the same as
```\def`` <doc-d.html#cmd-def>`__, but the token list in the body is
fully expanded (tokens that come from ``\the`` or ``\unexpanded`` are
not expanded).

You can put the prefix ``\global`` before ``\xdef``, this is however
useless, since ``\xdef`` is the same as ``\global\edef``. The following
example puts a brace in ``\foo``. The ``\string`` command can be
expanded, the value is the name of the command (preceded by a backslash,
or whatever the value of the escape character is). Here the assignment
to the escape character is local, the assignment to ``\foo`` is global.

.. container:: ltx-source

   ::

      {\escapechar=-1 \xdef\foo{\string\}}}

.. _cmd-xsetfontsize:

\\@xfontsize (Tralics internal integer)
---------------------------------------

In *Tralics*, since version 2.13.1, there are eleven font sizes, named,
starting with the smaller, ``\Tiny``, ``\tiny``, ``\SMALL``, ``\Small``,
``\small``, ``\normalsize`` ``\large``, ``\Large``, ``\LARGE``,
``\huge`` and ``\Huge``. The commands ``\scriptsize`` and
``\footnotesize`` are aliases for ``\SMALL`` and ``\Small``. The current
font size can be changed by setting the integer ``\@xfontsize``. This
allows two additional sizes TINY and HUGE. The example below shows how
to implement commands ``\larger`` and ``\smaller``. Note that by default
*Tralics* translates ``{\large x}`` and ``{\Large x}`` in the same way,
but in the example below, y is in normal size.

.. container:: ltx-source

   ::

      \newcommand{\smaller}[1][1]{\larger[-#1]}
      \newcommand{\larger}[1][1]{%
        \count@\@xfontsize \advance\count@#1\relax
        \@xfontsize \count@}
      {\large x\smaller y\smaller z}{\Large w\smaller x\smaller y}

.. _cmd-xi:

\\xi (math symbol)
------------------

The ``\xi`` command is valid only in math mode. It generates a Greek
letter: ``<mi>&xi;</mi>`` (Unicode character U+3BE, ξ).

.. _cmd-cxi:

\\Xi (math symbol)
------------------

The ``\Xi`` command is valid only in math mode. It generates an
uppercase Greek letter: ``<mi>&Xi;</mi>`` (Unicode character U+39E, Ξ).

.. _cmd-xiint:

\\xiint, \\xiiint, \\xiiiint (math symbol)
------------------------------------------

These are math-only commands, producing double, triple and quadruple
integrals. See ```\iint`` <doc-i.html#cmd-iint>`__ for an alternate
command. Translation is an ``<mo>&iint;</mo>`` or similar; double
integral sign is Unicode U+222C, ∬, triple integral is U+222D, ∭,
quadruple integral is U+2A0C, ⨌.

.. _cmd-xkvcc:

\\XKV@cc
--------

This command is provided by the `xkeyval <doc-k.html#cmd-keyval>`__
package.

.. _cmd-xleaders:

\\xleaders
----------

The ``\xleaders`` commands reads a box and names it ``xleaders``. In
*Tralics*, a box is an XML element, produced by ``\hbox``, ``\vbox``,
``\xbox``, or one of the box registers (for instance ``\box15``), a copy
of a box register (for instance ``\copy12``), the ``\lastbox``, some
specific LaTeX commands like ``\parbox`` ``\marginpar``. Note that
``\vsplit`` produces an empty box. Example.

.. container:: ltx-source

   ::

      \setbox0=\hbox{text}
      \leaders\copy0\cleaders\hbox{TEST}\cleaders\xbox{foo}{bar}
      \leaders\vsplit 0to 3pt \xbox{bar}{foo}\xleaders\lastbox
      \cleaders\parbox{3pt}{etc}\cleaders\marginpar{etc}

.. container:: xml_out

   ::

      <leaders>text</leaders>
      <cleaders>TEST</cleaders>
      <cleaders><foo>bar</foo></cleaders>
      <leaders/>
      <xleaders><bar>foo</bar></xleaders>
      <cleaders>etc</cleaders>
      <cleaders><marginpar>etc</marginpar></cleaders>

.. _cmd-xleftarrow:

\\xleftarrow (math command)
---------------------------

| There is no difference between ``\xleftarrow{foo}`` and
  ``\stackrel{foo}{\leftarrow}``. It puts foo over an arrow pointing to
  the left.
| |See...| See ```\stackrel`` <doc-s.html#cmd-stackrel>`__. The command
  accepts an optional argument placed below the arrow. Example.

.. container:: ltx-source

   ::

      $\xleftarrow{U}  \xleftarrow[D]{} \xleftarrow[U]{D}$ 
      $\xrightarrow{U}  \xrightarrow[D]{} \xrightarrow[U]{P}$ 

Translation:

.. container:: xml_out

   ::

      <formula type='inline'>
        <math xmlns='http://www.w3.org/1998/Math/MathML'>
          <mrow>
            <mover><mo>&leftarrow;</mo> <mi>U</mi></mover>
            <munder><mo>&leftarrow;</mo> <mi>D</mi></munder>
            <munderover><mo>&leftarrow;</mo> <mi>U</mi> <mi>D</mi></munderover>
          </mrow>
        </math>
      </formula>
      <formula type='inline'>
        <math xmlns='http://www.w3.org/1998/Math/MathML'>
          <mrow>
            <mover><mo>&rightarrow;</mo> <mi>U</mi></mover>
            <munder><mo>&rightarrow;</mo> <mi>D</mi></munder>
            <munderover><mo>&rightarrow;</mo> <mi>U</mi> <mi>P</mi></munderover>
          </mrow>
        </math>
      </formula>

.. _cmd-XMLaddatt:

\\XMLaddatt (Tralics command)
-----------------------------

The ``\XMLaddatt`` command takes as arguments an optional star, an
optional number, an attribute name, an attribute value. It puts the
attribute pair on the XML element defined by the number. If the number
is not given, the current element will be used; this is the same as
specifying ``\XMLcurrentid`` as number. If a star is used, you can
overwrite an existing attribute pair. **Note**: There is currently no
check on the name; in particular using space, equal sign, quotes,
greater than, may result in illegal (unparsable) XML code. The command
``\XMLgetatt`` takes as arguments an optional number and an attribute
name; its expansion is the value associated to the name of the XML
element defined by the number (all characters but space are of category
code 12). Example:

.. container:: ltx-source

   ::

      {\everyxbox{Vest}
      \setbox0=\xbox{foo}{1\xbox{bar}{2} %
         \XMLaddatt[\XMLlastid]{x}{1}\XMLaddatt[\XMLcurrentid]{y}{2}\XMLaddatt{y}{22}3%
         \edef\values{x=\XMLgetatt[\XMLlastid]{x}, y=\XMLgetatt{y}, z=\XMLgetatt{z}.}%
         \show\values}
      \showbox0
      }
      {\everyxbox{West}
      \setbox0=\xbox{foo}{1\xbox{bar}{2} %
      \XMLaddatt*[\XMLlastid]{x}{1}\XMLaddatt*[\XMLcurrentid]{y}{2}\XMLaddatt*{y}{4}3%
         \edef\values{x=\XMLgetatt[\XMLlastid]{x}, y=\XMLgetatt{y}, z=\XMLgetatt{z}.}%
         \show\values}
      \showbox0
      }

The following will be printed on the screen.

.. container:: tty_out

   ::

      \values=macro: ->x=1, y=2, z=..
      Box 0: <foo y='2'>Vest1<bar x='1'>Vest2</bar> 3</foo>
      \values=macro: ->x=1, y=4, z=..
      Box0: <foo y='4'>West1<bar x='1'>West2</bar> 3</foo>

The main XML element has *ID* number one, the bibliography has number
three, the table of contents has number four, the index has number five,
the glossary has number six. Never use number two.

If the attribute name is empty, the element name is used instead of the
attribute value. Thus the expansion of ``\XMLgetatt{}`` is the name of
the current element; and ``\XMLaddatt*{}{foo}`` changes it to foo (the
star is mandatory here). In some cases no element is associated to the
number. For instance, you cannot use this mechanism to get or change the
bibliography name. Example

.. container:: ltx-source

   ::

      \setbox0\xbox{José}{Box is named \XMLgetatt{}\XMLaddatt*{}{bar}}
      \showbox0
      %prints: Box0: <bar>Box is named José</bar>

.. _cmd-xmlAid:

\\XML@A@id, \\XML@B@id, \\XMLbox@id (Tralics readonly integer)
--------------------------------------------------------------

The command ``\XMLbox@id`` scans a register number, say *N* (see
`scanint <doc-s.html#fct-scanint>`__ for details of how the number after
the command is scanned) and returns the *ID* of the element that is in
box register *N*. The commands ``\XML@A@id`` and ``\XML@B@id`` return
the *ID* of the special elements xmlA and xmlB; see section
`manipulating the XML tree <#manipulate>`__ for examples. Such a command
can be used only when *Tralics* reads a number, in other cases you get
an error of the form *Read only variable \\XMLbox@id.* For instance
``\count0 \XML@A@id`` stores the *ID* in count0, while
``\showthe \XML@B@id`` prints the quantity on the terminal and the
transcript file. This can also be used as an argument to ``\XMLaddatt``.

.. _cmd-xmlAname:

\\XML@A@name, \\XML@B@name, \\XMLbox@name (Tralics readonly token list)
-----------------------------------------------------------------------

The command ``\XMLbox@name`` scans a register number, say *N* (see
`scanint <doc-s.html#fct-scanint>`__ for details of how the number after
the command is scanned) and returns the name of the element that is in
box register *N*. The commands ``\XML@A@name`` and ``\XML@B@name``
return the name of xmlA and xmlB; see section `manipulating the XML
tree <#manipulate>`__ for examples. Such a command can be used only when
*Tralics* reads a token list, in other cases you get an error of the
form *Read only variable \\XMLbox@name*. For instance
``\showthe   \XML@B@name`` prints the quantity on the terminal and the
transcript file. The invocation ``\edef\foo{\the\XMLbox@name3}`` puts in
``\foo`` the name of the box register 3. Every character has category
code 12, except for space characters (see
```\glueshrink`` <doc-g.html#cmd-glueshrink>`__, where we explain how to
use ``\detokenize`` in order to compare the name of the box with a
constant character string).

.. _cmd-xmlcomment:

\\xmlcomment (Tralics command)
------------------------------

This command reads an argument and puts it as a cmment in the XML tree.
For instance ``\xmlcomment{abc and d}`` produces ``<!--abc and d-->``.

.. _cmd-xmlcopyA:

\\XML@copyA, \\XML@copyB (Tralics command)
------------------------------------------

Whenever *Tralics* expects a box, you can use these commands; the result
is a full copy of one the two special box registers xmlA and xmlB. See
section `manipulating the XML tree <#manipulate>`__ for examples.

.. _cmd-xmlcurrentid:

\\XMLcurrentid (Tralics internal integer)
-----------------------------------------

Expansion of ``\the\XMLcurrentid`` is the *ID* of the current XML
element. This *ID* can be used as optional argument for the
```\XMLaddatt`` <#cmd-XMLaddatt>`__ command. Using the command outside a
context where a number is read yields the error *Read only variable
\\XMLcurrentid* since you cannot modify the *ID* of an element.

.. _cmd-xmlcurrencellid:

\\XMLcurrentcellid \\XMLcurrentrowid \\XMLcurrentarrayid (Tralics internal integer)
-----------------------------------------------------------------------------------

These commands expand to the *ID* of the current cell row or table, to
zero if there is no such thing. Same comments as for
``\the\XMLcurrentid`` above. Note:
``\xbox{cell}{\the \XMLcurrentcellid}`` translates to ``<cell>0</cell>``
because the element is not a cell.

.. container:: ltx-source

   ::

       \begin{tabular}{}
        \xbox{foo}{\XMLaddatt[\XMLcurrentcellid]{a}{b}}
      \end{tabular}

Translation

.. container:: xml_out

   ::

      <table rend='inline'><row><cell a='b'><foo/></cell>
      </row></table>

.. _cmd-xmlelement:

xmlelement (Tralics environment)
--------------------------------

The ``xmlelement`` environment takes one argument, say X. It creates an
XML element, named X, whose content is the content of the environment.
As the example shows, you can add an attribute via the
``\AddAttToCurrent`` command. Moreover, each element has a unique *ID*,
and the last unique *ID* allocated can be used to define the latest XML
element, and ``\AddAttToLast`` modifies the attribute list of this
element. In the example that follows, we explain how an apostrophe can
be inserted in the attribute value (note however that *Tralics* does no
check for the attribute name, neither the element name, it's up to you
to make sure it is a valid name). Example:

.. container:: ltx-source

   ::

      \begin{xmlelement}{foo}
      \begin{xmlelement}{subelt1}
      texte1
      \end{xmlelement}
      \begin{xmlelement}{subelt2}
      texte2
      \end{xmlelement}
      \AddAttToLast{sb2-att}{value1}
      \AddAttToCurrent{foo-att}{att-value''}
      \end{xmlelement}

Translation

.. container:: xml_out

   ::

      <foo foo-att='att-value&apos;&apos;'>
      <subelt1>
      texte1
      </subelt1>
      <subelt2 sb2-att='value1'>
      texte2
      </subelt2>
      </foo>

If you try to specify twice the value of the same attribute, there are
two cases: if the command is followed by a star, the old value is
removed, otherwise, the new value is ignored. See example below.

.. container:: ltx-source

   ::

      \begin{xmlelement}{foo}
      \AddAttToCurrent{x}{1} % ignored
      \AddAttToCurrent{y}{2} % ok
      \AddAttToCurrent*{x}{3} % ok
      \AddAttToCurrent{y}{4} % ignored
      \end{xmlelement}

In this translation, you can notice lots of spaces, one space for each
line that has a comment (the space before the comment is not discarded);
the space at the end of the ``\begin`` line is ignored.

.. container:: xml_out

   ::

      <p><foo x='3' y='2'>    </foo>
      </p>

You can put a star after the name of the environment. The effect is the
following. If no star is used, then horizontal mode is entered before
creating the element (in the example above, the ``<foo>`` is inside a
``<p>``). Otherwise, the element just created is typeset in vertical
mode (said otherwise, in the case of ``bar`` below, the character
sequence ``ok`` creates a new paragraph, hence a new element; in the
case of ``foo`` no new element is created, and the meaning of \`Current'
does not change).

.. container:: ltx-source

   ::

      ok
      \begin{xmlelement*}{bar}
      \AddAttToCurrent{x}{1}
      ok %
      \AddAttToCurrent{x}{2}
      \end{xmlelement*}


      ok
      \begin{xmlelement}{foo}
      \AddAttToCurrent{x}{1}
      ok %
      \AddAttToCurrent{x}{2}
      \end{xmlelement}

Translation:

.. container:: xml_out

   ::

      <p>ok
      </p>
      <bar x='1'><p x='2'>ok 
      </p>
      </bar><p>ok
      <foo x='1'>
      ok 
      </foo>
      </p>

You can say ``\begin{xmlelement+}...\end{xmlement+}``. In this case
``\leavevmode`` is called (as in the case of ``\begin{xmlelement}``) but
translation starts in vertical mode (so that the box might start with a
new paragraph).

.. _cmd-XMLfetch:

\\XML@fetch (Tralics command)
-----------------------------

This function reads an integer, say *N*, and puts in a special location
xmlA the XML element whose *ID* is *N*. See section `manipulating the
XML tree <#manipulate>`__ for examples.

.. _cmd-XMLget:

\\XML@get, \\XML@set, \\XML@delete, \\XML@insert (Tralics command)
------------------------------------------------------------------

These four commands read an integer, say *N*. The first command puts in
xmlB the child of index *N* of xmlA. The second command replaces the
child of index *N* of xmlA by xmlB, the third command erases it; the
last inserts xmlB before the child of index *N* of xmlA. These command
may signal an error when scanning the integer, but not after that. In
the case of ``\XML@get``, if *N* is out of bounds (in particulatr if
xmlA is void), then xmlB will be void. For the other command, nothing
happens if xmlA is void or *N* is negative, or out of bounds. If *N* is
the number of children of xmlA then ``\XML@insert`` inserts xmlB at the
end. The two commands ``\XML@set`` and ``\XML@insert`` always use a deep
copy of xmlB. In the example below ``\copy0`` is like ``\box0``, except
that the content is not erased; in particular nothing is copied. The
important point is that ``\XML@insert2`` makes a copy. The resulting XML
is ``<foo>bargee<foo>bargee</foo></foo>``. See section `manipulating the
XML tree <#manipulate>`__ for more examples.

.. container:: ltx-source

   ::

      \setbox0\xbox{foo}{bar}\copy0
      \XML@setA\copy0
      \XML@setB\hbox{gee}
      \XML@insert1
      \XML@moveAB
      \XML@insert2 

.. _cmd-XMLgetatt:

\\XMLgetatt (Tralics command)
-----------------------------

This command returns the value of an attribute. See
```\XMLaddatt`` <#cmd-XMLaddatt>`__.

.. _cmd-xmlmoveAB:

\\XML@moveAB, \\XML@moveBA, \\XML@sawpAB (Tralics command)
----------------------------------------------------------

*Tralics* has two special registers xmlA and xmlB. These commands copy
xmlA in xmlB, xmlB in xmlA, or swap the values. See section
`manipulating the XML tree <#manipulate>`__ for examples.

.. _cmd-XMLsolvecite:

\\XMLsolvecite (Tralics command)
--------------------------------

The ``\XMLsolvecite[xid][type]{key}`` command takes two optional
argument and a required argument. A star is allowed after the command
name. The call ``\bibitem[ign]{key}`` is the same as
``\par \leavevmode \xbox{bibitem}{\XMLaddatt{bib-key}{ign}} \XMLsolvecite*[][]{key}``.

If the optional type is \`year' or \`refer', it will be replaced by an
empty one (note that 'refer' is the same as 'year' only in some cases,
for details see section `2.8 Bibliography <raweb.html#rabib>`__ of the
raweb documentation). Two quantities are computed by *Tralics*, an eid,
and a bid. The bid becomes the value of the ``id`` attribute of the
element numbered eid, as in ``\XMLaddatt[xid]{id}{bid25}``, see the
description of `\\XMLaddatt <#cmd-XMLaddatt>`__ for details; in
particular the current XML element is used if the first optional
argument is omitted or if its value is empty.

The bid is the same as the value of the ``target`` attribute for the
citation ``\cite[type][]{key}``, (see
```\cite`` <doc-c.html#cmd-cite>`__ for an example). For every cite, a
solvecite is required. This might be done by calling bibtex, as
``\bibitem`` solves a citation. If *Tralics* is used, it generates some
``\cititem`` and solves everything. But you can also use
``\XMLsolvecite`` directly. If you say ``\cite[foo][]{key}``, then
``\XMLsolvecite*[][foo]{key}`` solves the entry; however, if there is no
such entry to solve, then the first entry with value 'foo', whatever the
type is solved. This is the expected behavior for ``\bibitem``. If no
star is given, this allows you to put a cite command after its target.

It is an error if the entry is already solved (because the cite element
has a unique target) or if the element has an id (because ids are
unique). In the case of a ``\bibitem``, the element that gets an id is
in general the ``<p>`` element that comes from the ``\leavevmode``.

Example. The translation of
``\xbox{bibitem}{\XMLsolvecite[][foot]{toto}{key1}}`` could be
``<bibitem id='bid0'>key1</bibitem>``

.. _cmd-xmlelt:

\\xmlelt (Tralics command)
--------------------------

The translation of ``\xmlelt{foo}{bar}`` is ``<foo>bar</foo>``. See also
```\xbox`` <doc-x.html#cmd-xbox>`__ whose syntax is more or less the
same. However, ``\xmlelt`` starts a new paragraph, if needed, while
``\xbox`` is independent of the mode.

.. _cmd-xmlemptyelt:

\\xmlemptyelt (Tralics command)
-------------------------------

The translation of ``\xmlemptyelt{foo}`` is ``<foo/>``. This command
does not start a paragraph. Use ``\xmlelt{foo}{}`` or
``\leavevmode\xmlemptyelt{foo}`` if needed.

.. _cmd-xmllastid:

\\XMLlastid (Tralics readonly internal integer)
-----------------------------------------------

The command ``\XMLlastid`` is a reference to the current *ID*, i.e., the
*ID* of the last created XML element. You can use it to modify the
attribute list (see the ```\XMLaddatt`` <#cmd-XMLaddatt>`__ command), or
modify the element (see section `manipulating the XML
tree <#manipulate>`__). The invocation ``\the\XMLlastid`` returns the
*ID* as a character string.

.. _cmd-xmllatex:

\\xmllatex (Tralics command)
----------------------------

This command is defined in tralics.sty as
``\newcommand\xmllatex[2]{#2}``. Said otherwise, it takes two arguments,
and ignores the first one. However *Tralics* ignores the second
argument, and handles only the first.

In the first argument, commands are not expanded, but replaced by their
names. Said otherwise, ``\foo`` is replaced by ``foo``, this is not
interesting; however ``\<\&\>`` is replaced by ``<&>``. If you do not
care, this might generate invalid XML. In the example that follows, we
define a command ``\strok``. You can apply it to the letters d, h, l, t
(uppercase and lower case), the result is a character entity defined in
the isolat2.ent file. We also give an example that shows how to put a
double accent on a character.

.. container:: ltx-source

   ::

      \def\entity#1{\xmllatex{\&#1;}{}}
      \def\strok#1{\entity{#1strok}}
      \def\Han{Hàn Th\xmllatex{&\#x1ebf;}{\'{\^e}} Thành}

      \Han\ designed \xmllatex{\pdf\<TeX/\>}{}. \l\ and \strok{l}.\\
      In Tralics version 2, you can say Th{\'{\^e}}

The resulting XML is the following.

.. container:: xml_out

   ::

      <p>Hàn Th&#x1ebf; Thành designed pdf<TeX/>. &#x142; and &lstrok;.</p>
      <p noindent='true'>In Tralics version 2, you can say Th&#x1EBF;</p>

Preview: |example of xmllatex|

.. _cmd-xmlparent:

\\XML@parent (Tralics command)
------------------------------

This puts in the special register xmlB the parent of xmlA. Since you can
manipulate the XML tree as you like, it may happen that the element has
no parent, or more than one (case where the first parent in the list is
returned). In case of failure, the result is void. See section
`manipulating the XML tree <#manipulate>`__ for examples. The example
contains a math formula, with an ex and an alpha inside a ``<mi>``
element. We explain how to fetch the alpha entity and put it in place of
the ex. Now, there are three elements that have the alpha entity as
child (because there is bold alpha). The ``\XML@parent`` command finds
neither of them, since it ignores all elements created at boot-strap
code.

.. _cmd-xmlref:

\\XMLref (math command)
-----------------------

The effect of ``\XMLref[13]{foo}`` is to make a reference from element
number 13 to the element defined by the label foo. If this element has
unique id uid27, then the effect is the same as
``\XMLaddatt[13]{target}{27}``. The first argument is optional, and has
the same meaning as in ```\XMLaddatt`` <#cmd-XMLaddatt>`__. There is no
difference between ``\ref{foo}`` and ``\xbox{ref}{\XMLref{foo}}``.
Example. Consider the following piece of code. The two calls to
``\newlabel`` are taken from an auxiliary file written by LaTeX with the
hyperef package. They memorize they section number and section title.
that will be used by ``\myref``.

.. container:: ltx-source

   ::

      \def\newlabel#1#2{\newlabelxx{#1}#2}
      \def\newlabelxx#1#2#3#4#5#6{%
          \@namedef{label@value@#1}{#2}%
          \@namedef{label@title@#1}{#4}}
      \def\myref#1{\xbox{ref}{%
         \XMLref{#1}\@nameuse{label@title@#1}%
         \XMLaddatt{value}{\@nameuse{label@value@#1}}}}

      \newlabel{labA}{{1.1}{1}{Introduction\relax }{subsection.1.1}{}}
      \newlabel{labB}{{1.2}{1}{Continuation\relax }{subsection.1.2}{}}

Consider now the following lines. The commands ``\myref`` can be placed
before or after the associated label (but should be placed before the
``\newlabel``).

.. container:: ltx-source

   ::

      Text. \anchor\label{labA}
      \myref{labA}\myref{labB}
      \anchor\label{labB}

Translation

.. container:: xml_out

   ::

      <p>Text. <anchor id-text='1' id='uid1'/>
      <ref target='uid1' value='1.1'>Introduction</ref>
      <ref target='uid2' value='1.2'>Continuation</ref>
      <anchor id-text='2' id='uid2'/></p>

.. _cmd-xmlsetA:

\\XML@setA, \\XML@\setB (Tralics command)
-----------------------------------------

These two commands behave like ``\setbox0``, this means that they read a
box, but the result is stored in one of two special box registers, xmlA
and xmlB. See section `manipulating the XML tree <#manipulate>`__ for
examples.

.. _cmd-xpt:

\\@xpt, \\@xipt, \\@xiipt, \\@xivpt, \\@xviipt, \\@xxpt, \\@xxvpt (constant)
----------------------------------------------------------------------------

These seven commands are used for font sizes; their expansion is
respectively 10, 10.95, 12, 14.4, 17.28, 20.74 and 24.88.

.. _cmd-xrightarrow:

\\xrightarrow (math command)
----------------------------

| There is no difference between ``\xrightarrow{foo}`` and
  ``\stackrel{foo}{\leftarrow}``. It puts foo over an arrow pointing to
  the right.
| |See...|\ See ```\stackrel`` <doc-s.html#cmd-stackrel>`__. The command
  accepts an optional argument placed below the arrow. See
  ```\xleftarrow`` <doc-x.html#cmd-xleftarrow>`__.

.. _cmd-xscale:

\\xscale
--------

The ``\xscale`` command is defined by the curves package. The value is
``1.0``. See ```\arc`` <doc-a.html#cmd-arc>`__.

.. _cmd-xscaley:

\\xscaley
---------

The ``\xscaley`` command is defined by the curves package. The value is
``0.0``. See ```\arc`` <doc-a.html#cmd-arc>`__.

.. _cmd-xspace:

\\xspace
--------

The ``\xspace`` command looks at a token, its expansion is a space
unless the space is useless, where where the expansion is empty. No
space is needed if the token is an opening brace, a closing brace, a
space (or a command that is made equivalent via ``\let`` to one these),
or a punctuation character, one of ``.!,:;?/')-~``. In the example that
follows, the effect is to put a space between the two As, and not
between the A and the exclamation point. Note that, in French, there
should be a space there. On the other hand, there is a double space
between the Bs, because the token that follows ``\xspace``, namely
``\cmdB``, is not in the list (it is not expanded by ``\xspace``).

.. container:: ltx-source

   ::

      \def\cmdA{A\xspace}
      \def\cmdB{ B\xspace}
      \cmdA \cmdA !
      \cmdB \cmdB !

.. _cmd-xspaceskip:

\\xspaceskip (rubber length)
----------------------------

You can say ``\xspaceskip=10pt plus 2pt minus 3pt``. This explains to
TeX that it should put 10pt (maybe up to 2pt more, maybe up to 3pt less)
of glue between sentences. Unused by *Tralics*. (See
`scanglue <doc-s.html#fct-scanglue>`__ for details of argument
scanning). (See ```\spaceskip`` <doc-s.html#cmd-spaceskip>`__ for
additional details).

.. _cmd-xtag:

\\x@tag, \\@xtag (Tralics commands)
-----------------------------------

Internal commands used for equation numbering, see
```\tag`` <doc-t.html#cmd-tag>`__.

.. _manipulate:

Manipulation of the XML tree
----------------------------

*Tralics* (since version 2.15.3) provides some commands to manipulate
the XML tree it constructs. In order to make sure that the result
remains a tree, a complete copy of some objects has to be made; the
copies may be avoided by using one of the two pointers xmlA and xmlB.
One can get the name of an element; however ``\XMLname \hbox{...}`` is
difficult to implement and a bit strange: this creates a box, gets its
name, then discards the box (but the box is never destroyed). For this
reason, you can fetch only the name of a box register or one of xmlA or
xmlB.

You can say ``\XML@moveAB``, ``\XML@moveBA`` and ``\XML@swapAB``. The
effect is to copy xmlA into xmlB, or xmlB into xmlA or to swap these two
quantities. You can say ``\XML@copyA`` or ``\XML@copyB``. What you get
is a complete copy of xmlA or xmlB; you can put it in a register as
``\setbox0 \XML@copyA``. You can use ``\XML@setA`` or ``\XML@setB``,
this will read a box and put it into xmlA or xmlB. Example
``\XML@setA\box0`` put box register zero into xmlA. You can say
``\XML@A@id``; this returns the unique *ID* of xmlA. You can say
``\count0=\XML@A@id``, or ``\ifnum 0<\XML@A@id   good \else  bad\fi``.
There is also ``\XML@B@id``. You can say ``\XML@A@size``; this returns
the size of xmlA. The value is -1 in case xmlA is void, and -2 in case
xmlA is not an element.

You can say ``\XML@fetch``; this reads an integer N and puts in xmlA the
XML element whose *ID* is N. If no element is found, a void element is
used (you can use ``\XML@A@id`` to check the result). You can say
``\XML@parent``. This puts in xmlB an element whose child is xmlA, if
such an object exists; an empty box otherwise. You can say
``\XML@insert``, ``\XML@delete``, ``\XML@set`` or ``\XML@get``. These
commands read an integer N. In the case of "get", the N-th child of xmlA
is put in xmlB; in the case of "delete", it is deleted; in the case of
"put" it is replaced by a copy of xmlB, and in the case of "insert", a
copy is inserted before the element at position N; Nothing is done if
xmlA has not at least N children; except that , in the case of "insert",
N can be the size, in the case, the insert is after the last element.

We give here an example of how these commands can be used. In blue we
show what *Tralics* prints on the terminal.

.. container:: ltx-source

   ::

      before
      \xbox{foo} {  b \xbox{bar}{s \global\count0=\XMLlastid} c }
      after
      $x= \alpha^2$
      \count1=\XMLcurrentid
      \showthe\count0 \showthe\count1
      \show: 1124
      \show: 1122

At the end of the compilation *Tralics* may print someting like
*Attribute list search 1529(1509) found 1083 in 1132 elements (1120 at
boot)* . This means the total number of elements created was 1132, and
1120 elements were created at bootstrap time (the other numbers are
statistic about attributes of these elements). The commands
``\XML@fetch`` and ``\XML@parent`` do not consider these elements. For
some strange reason, the first element created after bootstrap is
``<thepage/>``, then comes the main element, then a paragraph, the *ID*
of this paragraph is in ``\count1``; after that comes ``<foo>`` and
``<bar>`` (which *ID* is in ``\count0``), this is followed by the math
formula. Consider now

.. container:: ltx-source

   ::

      \makeatletter
      \XML@fetch\count1
      %\show@xmlA
      \showthe\XML@A@size
      \show: 4
      \XML@fetch\count0
      \show@xmlA
      xmlA: <bar>s </bar>
      \XML@parent
      \show@xmlB
      xmlB: <foo> b <bar>s </bar> c </foo>

The commented line explains one how could look at the element with *ID*
1122 (this the paragraph); it has four children; two pieces of text,
``<foo>`` and the math formula.

Let's swap xmlA and xmlB, fetch the *ID* of xmlA (this should be 1123),
modify the attribute list, and show A again

.. container:: ltx-source

   ::

      \XML@swapAB \count2=\XML@A@id 
      \XMLaddatt[\count2]{mayatt}{myval}
      \show@xmlA 
      xmlA: <foo myatt='myval'> b <bar>>s </bar> c </foo>

Let's get the parent of xmlA, and the children of index 3, then 0 then 0
(this is a ``<mrow>`` element with three children, a letter, an equals
signs and a square. We store in count4 the *ID* of the row, then take
the square, then the alpha. We put the alpha in a box register, then
inset a copy of it in the XML tree. Note that the size is now 6, as one
of the previous line inserted a space after the math formula.

.. container:: ltx-source

   ::

      \XML@parent \XML@moveBA \XML@get3  \XML@swapAB \XML@get0 %
      \XML@swapAB \XML@get0 \count4=\XML@B@id 
      \XML@swapAB \XML@get2 \XML@swapAB \XML@get0 
      \show@xmlB \setbox0=\XML@copyB \showbox0  \copy0 %
      xmlB: <mi>&alpha;</mi>
      Box0: <mi>&alpha;</mi>
      \copy0 \XML@fetch\count1\showthe\XML@A@size
      \XML@fetch\count1\showthe\XML@A@size
      \show: 6

The *Tralics* distribution contains a file with a single line of code:
``Hello, world!``. Let's compile it into hello.xml, and read this file.
If this succeeds, then the content of the file will be in xmlA. As you
can see, *Tralics* created an element called ``root``, with seven sons.
These are, in order, the XML declaration, a newline character, the
DOCTYPE declaration, a newline character, a comment indicating the
*Tralics* version and compilation date, a newline character, and the
real content of the file. If we ask for the *ID* of the element, we get
the values shown below. Here zero means that the XML element is a
character string; minus one means that it is a comment, minus two that
it is a declaration (for instance the DOCTYPE), minus three a processing
instruction and minus four that it is void ( this means that
``\XML@get7`` failed).

.. container:: ltx-source

   ::

      \readxml{hello.xml}\showthe\XML@A@size\showthe\XML@A@name
      \show: 7
      \show: root
      \XML@get0 \showthe\XML@B@id
      \show: -3
      \XML@get1 \showthe\XML@B@id 
      \show: -0
      \XML@get2 \showthe\XML@B@id 
      \show: -2
      \XML@get4 \showthe\XML@B@id
      \show: -1
      \XML@get7 \showthe\XML@B@id 
      \show: -4

There is a possibility to specify the name of the main element (see
`this documentation <titlepage.html>`__), generally based on the current
document class; since our document lacks a ``\documentclass``
declaration, the name will be ``unknown``. You can put the name into a
macro, and compare it with something (so you could do something with the
first child that has a given name); note that the name is formed of
characters of category code 12, so you have to use the ``\detokenize``
trick. For some reason, the first child of the main element is a newline
character; the second one is ``<p>Hello,   world!</p>``, with a newline
character (since there was one in the original file). We close the
current paragraph and insert the element in the current XML tree.

.. container:: ltx-source

   ::

      \XML@get6 \showthe\XML@B@name
      \show: unknown
      \edef\pfoo{\detokenize{unknown}}
      \edef\foo{\the\XML@B@name} \ifx\foo\pfoo\else \ooups\fi 
      \XML@swapAB \XML@get1 \show@xmlB  \par\XML@copyB 
      xmlB: <p>Hello, world!
      </p>

Let's do something with box0. We put it in xmlA via its *ID*. We add an
attribute list, copy it in box1, then modify the attribute list of the
copy. We give some examples of box manipulations.

.. container:: ltx-source

   ::

      \XML@fetch\XMLbox@id0 \show@xmlA
      xmlA: <mi>&alpha;</mi>
      \XMLaddatt[\XMLbox@id0]{att-name}{att-val0}
      \setbox1=\XML@copyA \leavevmode\copy1
      \XMLaddatt*[\XMLbox@id1]{att-name}{att-val1}%
      \XML@setB\hbox{E1}\XML@insert0
      \XML@setB\hbox{E3}\XML@insert2
      \XML@setB\hbox{E2}\XML@set1 
      \show@xmlA 
      xmlA: <mi att-name='att-val0'>E1E2E3</mi>

Let's do something with box1. We put it in xmlA via a box assignment
(note that ``\copy1`` could also have been used). We put the content of
the box in box0, then erase it. So, the XML tree will contain an empty
``mi`` element. Then we come back to the box whose *ID* is in count4.
This is the ``<mrow>``; now we replace the first child of the first
child by the content of box0.

.. container:: ltx-source

   ::

      \XML@setA\box1 \show@xmlA \showbox1
      mlA: <mi att-name='att-val1'>&alpha;</mi>
      Box 1: empty.
      \XML@get0 \setbox0=\XML@copyB \XML@delete0
      \XML@fetch\count4 \XML@get0 \XML@swapAB 
      \XML@setB\box0 \XML@set0

This is now the translation

.. container:: xml_out

   ::

      <?xml version='1.0' encoding='UTF-8'?>
      <!DOCTYPE unknown SYSTEM 'unknown.dtd'>
      <!-- Translated from latex by tralics 2.15.3, date: 2015/07/29-->
      <unknown>
      <p>before
      <foo myatt='myval'> b <bar>s </bar> c </foo>
      after
      <formula type='inline'><math xmlns='http://www.w3.org/1998/Math/MathML'>NL
      <mrow><mi>α</mi><mo>=</mo><msup><mi>α</mi>NL
      <mn>2</mn> </msup></mrow></math></formula>

      <mi att-name='att-val0'>E1E2E3</mi></p>
      <p>Hello, world!
      </p><p><mi att-name='att-val1'/></p></unknown>

.. |See...| image:: /img/AR.png
.. |example of xmllatex| image:: /img/img_64.png
