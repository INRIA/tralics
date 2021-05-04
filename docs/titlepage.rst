Configuration files of Tralics: syntax and usage
================================================

1. Introduction
---------------

This document explains how to modify the output of *Tralics*:

-  A general configuration file, or a specialized *tcf* file, is used to
   find the document type;
-  This allows you to change the name of some elements, or the DTD of
   the XML document;
-  A mechanism is provided for collecting meta data at the start of the
   document when a command, for instance \\maketitle, is seen;
-  Additional files (of type *ult*, *plt*, and *clt*) can be used
   instead of package and classe files.

In this document, we explain the syntax of the configuration file using
`this <titlepage-conf.html>`__ configuration file. We consider two TeX
files `this one <titlepage-ex1.html#ex1>`__, and `that
one <titlepage-ex1.html#ex2>`__, and show the XML result `(this
XML) <titlepage-ex2.html#ex1>`__ and `(that
XML) <titlepage-ex2.html#ex2>`__ and the log file produced by Tralics
`(first) <titlepage-ex3.html#ex1>`__ and
`(second) <titlepage-ex3.html#ex2new>`__. We shall alse use the
following `tpa.tcf <titlepage-conf.html#tpa>`__ files.

2. What Tralics prints on the terminal
--------------------------------------

First example, using original syntax.

.. container:: tty_out

   ::

      grimm@medee$ tralics tpa -configfile ../config_tpa -verbose
      This is tralics 1.9t, a LaTeX to XML translator
      Copyright INRIA/MIAOU/APICS 2002-2004, Jos� Grimm
      Starting xml processing for tpa.
      <!-- S1 -->
      Trying config file from user specs:../config_tpa
      Read configuration file ../config_tpa.
      Configuration file has type \documentclass
      Seen \documentclass tpa
      <!-- S2 -->
      Potential type is tpa
      Defined type: default
      Defined type: tpa
      Defined type: unused
      Defined type: eurotex
      Using type tpa
      dtd is tpa from tpa.dtd (standard mode)
      Ok with the config file, dealing with the TeX file...
      <!-- S3 -->
      Error signaled at line 35:
      No value given for command \cmdp.
      <!-- S4 -->
      Seen 0 bibliographic entries
      Used 1237 commands
      math stats : formulas 0, non trivial kernels 0, cells 0,
         special 0 trivial 0, \mbox 0 large 0 small 0.
      List stats: short 0 inc 2 alloc 529
      Buffer realloc 0 string 1455 size 17863; merge 0
      Macros created 28 deleted 4
      Save stack +12 -12
      Attribute list search 191(119) found 156 in 1081 elements (1037 after boot)
      number of ref 0, of used labels 0, of defined labels 0, of ext. ref. 0
      Modules with 0, without 0, sections with 0, without 0
      There was no image.
      Output written on tpa.xml (1436 bytes).
      There was one error.
      (For more information, see transcript file tpa.log)

First example, using new syntax.

.. container:: tty_out

   ::

      grimm@medee$ tralics tpa -verbose 
      This is tralics 2.8, a LaTeX to XML translator
      Copyright INRIA/MIAOU/APICS 2002-2006, Jos\'e Grimm
      Licensed under the CeCILL Free Software Licensing Agreement
      Starting xml processing for tpa.
      <!-- S1 -->
      Configuration file identification: standard $ Revision: 2.24 $
      Read configuration file /user/grimm/home/cvs/tralics/.tralics_rc.
      Configuration file has type \documentclass
      Seen \documentclass tpa
      <!-- S2 -->
      Potential type is tpa
      Defined type: std
      Configuration file identification: tpa.tcf $ Revision: 1.3 $
      Read tcf file for type: ../confdir/tpa.tcf
      Using type tpa
      dtd is tpa from tpa.dtd (standard mode)
      Ok with the config file, dealing with the TeX file...
      <!-- S3 -->
      Error signaled at line 35 of file tpa.tex:
      No value given for command \cmdp.
      <!-- S4 -->
      Bib stats: seen 0 entries
      Seen 0 bibliographic entries
      Used 1810 commands
      Math stats: formulas 0, non trivial kernels 0,
         special 0, trivial 0, \mbox 0, large 0, small 0.
      Buffer realloc 0, string 1642, size 17963, merge 0
      Macros created 86, deleted 4.
      Save stack +24 -24.
      Attribute list search 1700(1549) found 1019 in 1495 elements (1450 after boot).
      Number of ref 0, of used labels 0, of defined labels 0, of ext. ref. 0.
      Modules with 0, without 0, sections with 0, without 0
      There was no image.
      Output written on tpa.xml (1378 bytes).
      There was one error.
      (For more information, see transcript file tpa.log)

Second example, non-verbose mode

.. container:: tty_out

   ::

      grimm@medee$ tralics bo
      This is tralics 2.8, a LaTeX to XML translator
      Copyright INRIA/MIAOU/APICS 2002-2006, Jos\'e Grimm
      Licensed under the CeCILL Free Software Licensing Agreement
      Starting xml processing for bo.
      Configuration file identification: standard $ Revision: 2.24 $
      Read configuration file /user/grimm/home/cvs/tralics/.tralics_rc.
      Configuration file identification: cedram.tcf $ Revision: 1.1 $
      Read tcf file for type: cedram.tcf
      Translation terminated after title page
      Bib stats: seen 0 entries
      Seen 3 bibliographic entries
      Used 1866 commands
      Math stats: formulas 16, non trivial kernels 10,
         special 0, trivial 0, \mbox 0, large 0, small 8.
      Buffer realloc 7, string 1881, size 25733, merge 2
      Macros created 114, deleted 10.
      Save stack +154 -154.
      Attribute list search 1847(1549) found 1010 in 1620 elements (1450 after boot).
      Number of ref 0, of used labels 0, of defined labels 0, of ext. ref. 0.
      Modules with 0, without 0, sections with 0, without 0
      There was no image.
      Output written on bo.xml (5209 bytes).
      No error found.
      (For more information, see transcript file bo.log)

3. Explanations
---------------

The lines above show the command used to call *Tralics*, and its
response. We have added some markers in order to make the explanations
easy to follow.

The first example was run in 2004, using an old version of the software.
We have rerun the example, with the new version of *Tralics*. As you can
see, the resulting XML is differerent (1378 bytes instead of 1536). You
can compare the `old version <titlepage-ex2.html#ex1old>`__ with the
`new version <titlepage-ex2.html#ex1new>`__. The differences are the
following: first, spaces before a ``\par`` command are removed, this
means that the XML contains no space before a closing ``</p>`` tag;
second, ``<foo></foo>`` is now printed as ``<foo/>``; third, the
``language`` atttribute is now added to the document element only if the
configuration file contains ``att_language="language"`` (the right hand
side of the assignment defines the attribute name).

3.1. Launching Tralics
~~~~~~~~~~~~~~~~~~~~~~

| For a list of all arguments of the Tralics command, see the page
  `Options of the program <raweb.html#options>`__. The command line used
  for the first example is:
|    ``tralics tpa -configfile ../config_tpa -verbose``
| It tells the program to translate a file named ``tpa.tex``, in verbose
  mode, using a given configuration file. The verbose option is
  equivalent to a ``\tracingall`` command in the TeX source (its effect
  is to print a lot of things in the transcript file); moreover some
  additional lines are printed on the screen (especially concerning
  configuration).

*Tralics* analyses first the arguments. All options start with a single
or double dash. Instead of ``-configfile foo``, you can say
``-configfile=foo`` (whitout spaces). For historical reasons, you can
also say ``-config_file=foo``, and, since version 2.8, the short form
``-config=foo`` is allowed. There is exactly one argument to the
``tralics`` command that does not start with a dash, it is the name of
the TeX input file (extension ``.tex`` is optional).

The transcript file (here ``tpa.log``) is opened after all arguments
have been successfully parsed. Almost everything printed on the terminal
is also printed to the transcript file. In the example, the version
number is printed, the Copyright notice, then the line: \`Starting xml
processing for tpa'. The TeX input file is read next, once and for all.

3.2. Finding the Potential Type
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Step S1 shows *Tralics* examining the configuration file, in order to
find the Document Type. This information will be used to select a part
of the configuration file, or the whole of a tcf file, using the rules
that follow.

#. If you say ``tralics -noconfig``, then no configuration file is read
   at all.
#. If you say ``tralics -configfile=foo``, then *Tralics* will print
   *Trying config file from user specs*, and try to use this file.
#. If you say ``tralics -configfile=foo.tcf``, then *Tralics* will print
   the same as above; it will also search the file in the \`confdir'
   directory.
#. If the source file contains
   ``% tralics configuration file 'foobar'``, then *Tralics* will print
   *Trying config file from source file*, and try to use this file. In
   case of failure, and if the name 'foobar' contains no dot, the suffix
   ``.tcf`` is added, and the next rule is applied.
#. If the source file contains
   ``% tralics configuration file 'foobar.tcf'``, then *Tralics* will
   print the same as above; it will also search the file in the
   \`confdir' directory.
#. The default configuration file is named ``.tralics_rc`` (or
   ``tralics_rc`` on Windows). The current directory is looked at first,
   then the tralicsdir, finally the home directory.
#. If you say ``tralics -dir TOTO``, or ``tralics -dir=TOTO``, then
   ``TOTO/src/.tralics_rc`` is the second try.
#. The home directory, or its ``src`` subdirectory, is searched next.
   (Depending on the operating system, this can fail, because there is
   no standard way of defining the home directory of the user).
#. If you set the shell variable ``TRALICSDIR`` to ``somedir``, or
   ``RAWEBDIR`` to ``somedir``, then ``somedir/src/.tralics_rc`` is the
   last try. If neither variable is set, then some default location will
   be used.

-  If a configuration file has been given on the command line, but not
   found according to rules 2 and 3, then no configuration file will be
   used.
-  For rules 4 and 5, only the first hundred lines of the source file
   are considered, the line must start with a percent character; you can
   use an upper case T in the string "tralics"; the configuration file
   name is delimited by single quotes; the line can hold additional
   characters that are ignored.
-  If a configuration file has been given in the source file, but not
   found according to rules 4 and 5, then the default configuration file
   will be used.
-  In the case of rules 3 and 5, a tcf file is looked at, say
   ``foo.tcf``. It is first searched in the current directory, then in
   all directories specified by the ``confdir`` options, then in a
   default directory. For instance, if you say
   ``tralics -confdir=mydir/`` (final slash optional), the second try
   would be ``mydir/foo.tcf``. If this file is not found, a default
   location, for instance ``../confdir/foo.tcf``, is tried. If you
   specify ``dir1`` then ``dir2``, then the order will be: current
   directory, dir2, dir1, default directory.

In the configuration file, you can use # or % as comment characters.
This file contains declarations inside groups (for instance
``BeginAlias`` ... ``End`` on lines
`11 <titlepage-conf.html#l11>`__--`14 <titlepage-conf.html#l14>`__ or
``BeginType`` ... ``End`` on lines
`16 <titlepage-conf.html#l16>`__--`25 <titlepage-conf.html#l25>`__). The
end of a group is defined by a line that starts with the 3 letters
\`End', followed by anything. Some commands are allowed outside groups.

In the example, you can see that the user defined configuration file has
been read and an important information extracted from line
`8 <titlepage-conf.html#l8>`__, namely
``'Configuration file has type \documentclass'`` and *Tralics* has
``Seen \documentclass tpa``.

The *documentclass* is defined as the argument of the first
``\documentclass`` command seen in the TeX source. This may be different
from the real document class because of one of the following

-  ``\csname documentclass\endcsname`` is used;
-  The real class is declared in another file, loaded via ``\input``;
-  There is a line break between the command and its argument, and the
   parser skips this one;
-  A line like ``%\documentclass{report}`` defines the type as report.
   If a double percent sign is used, the match fails. The translator
   will ignore such a line, if standard category codes are in effect.
-  In some cases, there is no ``\documentclass`` in the document. In
   theses cases, some default action is done.

If you say ``tralics -type FOO``, then the type will be FOO. Otherwise,
if the configuration file contains, at toplevel, the declaration
``Type =fOO``, then the type will be fOO. If the configuration file
contains ``Type =\documentclass``, then the type will be the
*documentclass*, as explained above. If you say nothing, then the type
is the documentclass. This will be called the *potential type*, that
needs to be converted into a real type.

3.3. Finding the Type
~~~~~~~~~~~~~~~~~~~~~

When *Tralics* enters step S2, it says: Potential type is tpa. It tries
to match this type with all the types that are defined. In this case
they are four types defined, default
(`l.16 <titlepage-conf.html#l16>`__), tpa
(`l.48 <titlepage-conf.html#l48>`__), unused
(`l.94 <titlepage-conf.html#l94>`__) and eurotex
(`l.134 <titlepage-conf.html#l134>`__), and one of them is to be
selected.

In case of failure, aliases are considered. There is line
`12 <titlepage-conf.html#l12>`__ that says that \`report' is the same as
\`article', but \`article' is undefined, so that this line discarded. In
the same fashion, \`inriaslides' and \`foiltex' are aliases to
\`slides', which is undefined. Near the end of the file, lines
`154 <titlepage-conf.html#l154>`__ to `156 <titlepage-conf.html#l156>`__
say that \`foo' and \`foo2004' are alias to tpa, which is undefined.
Thus we have only two useful lines, one that says that \`foo' is an
alias for \`TPA' and \`unused', while \`foo2003' is an alias for
\`unused'. Only the first valid alias for \`foo' is considered.

Hence, for this configuration file, aliasing adds two new types: \`foo'
for \`TPA', \`foo2003' for \`unused'.

In the case of the first example, new syntax, we use the standard
configuration file. This defines a single type, std, plus some aliases.
We show here the content of it (without the comments).

.. container:: ltx-source

   ::

      Type = \documentclass

      BeginType std
        DocType = std classes.dtd 
      End

      BeginAlias
        torture torture1 torture2 
        std report book article minimal
      End

Since version 2.8, tcf files are considered. This means that, if the
potential type is tpa, and the file ``tpa.tcf`` exists (or its alternate
location ``../confdir/tpa.tcf``), it will be used. This is what happens
in our example and *Tralics* says: *Read tcf file for type:
../confdir/tpa.tcf*. If the potential type is, as 'torture2' above,
aliased to \`torture', and if \`torture' is not a type defined in the
configuration file, then ``torture.tcf`` and ``../confdir/torture.tcf``
are considered; if the file exists, it will be read, and \`torture'
becomes the effective type.

If aliasing fails, a second try is made, ignoring final digits in the
name. Thus \`fooNNNN' is equivalent to \`tpa', whatever NNNN (if NNNN is
a number, different from 2003). Since the standard distribution contains
a file ``ra.tcf``, if you say ``\documentclass{ra2003}`` (and this is
required for the RAWEB, year 2003), the type will be \`ra'. See `example
of the raweb <raweb.html#main-doc>`__.

In case of failure, the first type of the configuration file is used
(this is \`std' for the default configuration file).

In case of failure (if the configuration file does specify no type) then
\`book', \`report' or \`article' classes are recognized as a special
case. In the same fashion, if no ``\documentclass`` command is seen in
the TeX source, then plainTeX input is assumed. No error is signaled,
but no parameterization code is executed.

In our example, we get the message *Using type tpa*. Everything that is
out of the \`tpa' scope is ignored from the configuration file. If a tcf
file, is given, its entire content is considered.

3.4 Using the Type
~~~~~~~~~~~~~~~~~~

What remains in the configuration file, after removing what is
irrelevant to the current type, consists in two blocks:
``BeginTitlePage ... End`` (lines `50 <titlepage-conf.html#l50>`__ --
`86 <titlepage-conf.html#l86>`__) ``BeginCommands ... End`` (lines
`87 <titlepage-conf.html#l87>`__ -- `89 <titlepage-conf.html#l89>`__)
and some isolated commands (in this example there is only line
`49 <titlepage-conf.html#l49>`__, but for other types lines
`17 <titlepage-conf.html#l17>`__ -- `20 <titlepage-conf.html#l20>`__ or
lines `29 <titlepage-conf.html#l29>`__ --
`43 <titlepage-conf.html#l43>`__ are considered).

From now on, all line numbers refer to the ``tpa.tcf`` file; the first
block is at lines
`311 <titlepage-conf.html#l311>`__--`347 <titlepage-conf.html#l347>`__,
the second at lines
`348 <titlepage-conf.html#l348>`__--`350 <titlepage-conf.html#l350>`__,
the isolated line at `308 <titlepage-conf.html#l308>`__.

The important command is the \`DocType' line. *Tralics* says: *dtd is
tpa from tpa.dtd (standard mode)*. It might say something like: *dtd is
raweb from raweb.dtd (mode RAWEB2003)*, case where you are creating
Inria's Raweb for year 2003.

-  A line of the form ``DocAttrib = test "foo"``
   (`l.31 <titlepage-conf.html#l31>`__ for instance) informs *Tralics*
   to add ``test='foo'`` to the attribute list of the document element.
   Comments are allowed after the string "foo".
-  A line of the form ``DocAttrib = test \specialyear``
   (`l.19 <titlepage-conf.html#l19>`__) informs *Tralics* to add
   ``test='2003'`` to the attribute list of the document element. The
   default year will be 2004 starting May, 1st. If you say
   ``tralics -year 2005``, then the special year is 2005.
-  A line of the form ``DocAttrib = test \foo``
   (`l.18 <titlepage-conf.html#l18>`__) informs *Tralics* to add
   ``test='fooval'`` to the attribute list of the document element. More
   precisely, ``\addattributestodocument{x}{\foo}`` is added to the
   document hook. This means that the ``\foo`` command is evaluated just
   after ``\begin{document}``.
-  A line of the form
   ``DocAttrib = z \the\day\space \the\month\space \the\year``
   (`l.20 <titlepage-conf.html#l20>`__) informs *Tralics* to add
   ``z='29 1 2004'`` to the attribute list of the document element.
   Every character on the line is considered. Comments are forbidden.
-  A line of the form ``makefo="xsltproc --catalogs -o %A.fo %B %C"``
   (`l.33 <titlepage-conf.html#l33>`__) is useful only for the RA. After
   translation is complete, the ``xsltproc`` command will be called with
   arguments depending on the file name, and other things. All the
   commands defined by the RA type (and commented in the example) follow
   these schema. See `Running Tralics -ps <raweb4.html#run3>`__.
-  A line of the form ``xml_labelitem_name = "Xlabel"``
   (`l.95 <titlepage-conf.html#l95>`__) tells *Tralics* to use the name
   Xlabel instead of the name label. See example elsewhere.
-  A block of the form
   ``BeginCommands          \def\recurse{\recurse\recurse}          EndCommands``
   is copied (without first and last line) into the TeX source, just
   before the ``\documentclass`` command (the one used to find the type;
   nothing is done if no such command exists). You can see this in the
   transcript file. It is marked \`Insert 2', because there is a second
   insertion of the same kind, see later.
   **Note** In the current version, it is inserted *before* the command.
   See `explanations <#inserts>`__.
-  Each line in a TitlePage block is analyzed, and a table is created.
   See later.

Now *Tralics* prints: *Ok with the config file, dealing with the TeX
file...*

3.5 Bootstrap code
~~~~~~~~~~~~~~~~~~

In the transcript file, at the start of Step S3, you can see that the
source file has 38 lines. After that, the transcript file contains, for
every source line its content, and for every command executed some
information (this is because we are in verbose mode).

Some lines are not shown, they are of the form ``\let\endgraf\par``.
Then comes a bunch of lines of the form
``Defining \InsertTitlepage as \TitlePageCmd 0``. Almost every line from
the TitlePage block is associated to such a line. In fact, every line
that start with a backslash (a LaTeX command) defines a command, and if
you say ``alias \foo``, then ``\foo`` is an alias to the command defined
on the previous line. In the case of ``<UR> -``, no command is created,
but such lines are nevertheless useful.

Each line is followed by a second line, of the form
``usual <abstract ab='AB1'/> (flags +par +env)``. We shall see in a
moment how the flags have to be interpreted.

| You can see this in the transcript file:
| ``[1] \cmdb{\cmdBval}``
| ``++ End of virtual file.``
| This is a consequence of the B or C flag, as explained later.

The transcript file contains ``[1] \def\cmdAval{CMDA}``, plus some other
lines, not shown here. This is the first line of the TeX source.

4. The titlepage info
---------------------

In the configuration file, there are some lines that define the
titlepage info. In this way, you can add, to *Tralics*, some commands
like ``\author``, ``\title``, etc, that can be used (exactly once, at
most once, at least once, as you like) before the ``\maketitle`` command
(you chose your own name, variants are allowed). These commands should
appear before the main text; they are forbidden in a paragraph (you can
always insert a ``\par`` before them). Only one ``\maketitle`` command
is allowed in the document.

4.1 Syntax
~~~~~~~~~~

Each line of the configuration file in the title page info section is
formed of one to four tokens; these have a type, say A, E, S, C; the
interpretation of the line depends on the lists of these types.

-  An A token corresponds to the string ``alias``, it could also be
   ``action`` or ``execute`` see below;
-  An E token corresponds to an element, of the form ``<...>`` (all
   characters up to the greater-than sign are read, it is assumed that
   the result is a valid XML element);
-  A S token corresponds to a character string of the form ``"..."``
   (all characters up to the closing double-quote are read);
-  A C token corresponds to a command, of the form ``\xxx`` (all letters
   following the backslash are read);
-  Tokens E, S, and C can be preceded by one or two modifiers. Currently
   characters ?+- (question, plus, minus) and letters ABCeEpq are
   allowed;
-  No modifier is allowed for the first and last token.

Our example starts with a CESS, followed by an AESS, and two CES. After
that comes a CEES, and the first E has plus as modifier. The order of
elements is important. There can be only one CESS element, and it has to
be the first. You can put an AESS only after an AESS or a CESS. You
cannot put an AS element after a CESS, AESS or E.

4.2 Semantics
~~~~~~~~~~~~~

We shall discuss the meaning of these token list, in the order of use in
the example.

CESS
   Example: ``\cmd <elt> "att1" "att2"``.

   This declaration has to be the first in the list. It defines a
   command ``\cmd``, that can be used only once in the document, after
   ``\begin{document}``. The effect is to insert the ``<elt>`` element
   into the XML tree. In what follows, we shall call it the *TPA
   element*. This element is formed of other elements defined by the
   titlepage info, the names of these elements are statically defined,
   their content is dynamic (i.e. the names depends on the configuration
   file, the content on the TeX document). The string ``att1`` is a list
   of attributes added to the TPA element and the string ``att2`` is a
   list of attributes added to the document element.

   There is an example on lines `312 <titlepage-conf.html#l312>`__ and
   `313 <titlepage-conf.html#l313>`__; the title page element is named
   \`titlepage', it has two attributes, with value \`att1' and \`att2'
   with value \`foo1' and \`foo2'. If used via the command defined on
   line `312 <titlepage-conf.html#l312>`__, it has two additional
   attributes \`a1' and \`a2', and the document element has attributes
   \`from_type' and \`from_tpa'. There is another example line
   `207 <titlepage-conf.html#l207>`__; the attribute name is empty, thus
   the attribute value will be ignored by the translator. See section
   `Special action <#titlepage-action>`__ for the use of this special
   value.

AESS
   Example: ``alias \cmda "att1p" "att2p"``. This declaration is valid
   only after a CESS declaration (or after another AESS declaration). It
   defines a command ``\cmda`` that can be used instead of ``\cmd``
   (only one of these commands can be used). The result is the same;
   however ``att1p`` is used instead of ``att1`` for the attribute list
   of the TPA element, and ``att2p`` is used instead of ``att2`` for the
   document element (same remark as above for special attribute values
   in ``att2p``).

   In what follows, the ``\TPA`` command means one of the commands
   defined by this rule or the preceding one.

CEES
   Example: ``\cmdb +<master> <local> "val"``. Note that the plus sign
   is required before the master element. Examples can be seen on line
   `316 <titlepage-conf.html#l316>`__ and
   `317 <titlepage-conf.html#l317>`__. See comments below.

   This declaration has as side effect that the TPA element will contain
   a ``<master>`` element, formed of a number of ``<local>`` elements.
   Initially there is only one, initialized with "val".

   The declaration has another effect, it defines a command ``\cmdb``,
   that has to be used before ``\TPA`` command. It takes one argument,
   and creates a ``<local>`` whose content is the translation of the
   argument. This element is added to the end of the ``<master>``
   element. Note that the default value is removed in case at least one
   value is given.

CES
   Example: ``\cmdc <elt> "value"``. The element can have the flags p,
   q, e or E, and the value can have the flags +ABC. The effect is to
   define a command ``\cmdc`` (or an environment ``cmdc`` if the E flag
   has been given), that can be used only before the ``\TPA`` command.
   The argument of the command, (or the content of the environment) is
   translated, put in a ``<elt>`` element, and added to the TPA element.
   If no flag is given for the element, paragraphs are forbidden in the
   argument. If you want to use paragraphs (either ``\par`` or ``\\``)
   you must use the P flag (lower-case letter). In the same fashion, a
   lower case E means environment without paragraphs, an upper case E
   means environment with paragraphs. If the q flag is given, paragraphs
   are forbidden, but you can use ``\\``, which is ignored. (in fact,
   the command reads an optional star, an optional argument, and the
   result is replaced by a space). Other flags are described in the
   section `Interpreting the CES flags <#titlepage-flags>`__ below.
   There is a special trick for the case where the name of the element
   associated to the command is empty. Assume that the configuration
   file contains ``\cmd <> +"text"``. In the case where the user does
   not use ``\cmd``, an error will be signaled, and ``text`` will appear
   in the resulting XML. If the user says ``\cmd{foo}``, then *Tralics*
   remembers the use and issues no complain. Moreover, it reads the
   argument, and pushes ``foo\par`` in the input stream (the reason why
   ``\par`` is executed is to make sure that *Tralics* remains in
   vertical mode).
CCS
   Example ``\cmdd \cmde "text"``. The effect is the same as
   ``\def\cmdd{\cmde{text}}``. However, "text" is not translated, it is
   taken verbatim.
AC
   If you say ``alias \foo``, then \\foo is an alias for the command
   defined on the previous line.
E
   Example: ``<UR> -``. The dash after the element is required. Another
   example is: ``<sUR fr='unit� de recherche' en='research unit'> -``.
   In this second example, we have an element named \`sUR', that has two
   attributes. The effect is to put, in the XML result, this element
   (with its attributes), and its content is a list of items declared in
   the configuration file (the list can be empty).
CE
   The syntax is ``\cmdf ?+ <elt>``. Example:
   ``\sURsop ?+ <sUR fr='dans le sud'>``. This has as effect to define a
   command, here ``\sURsop``, that takes no argument, whose effect is to
   insert, to the element ``<elt>`` (that must be defined by the
   previous rule) an empty element, whose name is ``cmdf``, and that has
   the attributes of ``<elt>``. In the example, we would get:
   ``<sURsop fr='dans le sud'/>``.
CEE
   The syntax is ``\cmdg ? <elt> <eltx>``. Same as above, but the
   element created is ``<eltx>`` instead of one named ``cmdf`` with the
   attributes of ``<elt>``. Example
   ``\sParis ?<sUR> <Rocq en='nearparis'>`` gives
   ``<Rocq en='nearparis'/>``.
S
   The character string is inserted verbatim in the XML tree.
AC
   The syntax is ``execute \cmd`` or ``action \cmd``. When *Tralics*
   sees the ``\InsertTitlepage`` command, it executes ``\xbox{}{\cmd}``,
   this gives an XML element that is inserted in the current tree.

**Note.** Since Version 2.8, *Tralics* can produce UTF-8 output. Some
arguments are copied verbatim from the input to the output,
independently of this setting. The example shown above for case 'E'
contains the string ``fr='unit� de recherche'`` (see configuration file,
line 71). If you read the file containing this string in UTF8 mode, an
error will be signaled by *Tralics*. If you use it in UTF8 output mode,
no error is signaled, but the result is invalid. The solution is to use
seven bit input and character entities, like ``&#xe9;``. See line 332 of
the configuration file, where this patch has been applied.

4.3. Interpreting the CES item
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If the titlepage contains ``\cmd ?<elt> ?"value"``, this defines a
command ``\cmd`` that produces an element ``<elt>``, with a default
value "value". The first question mark has to be replaced by nothing or
one of p, q, e or E, the second by one of A, B, C or plus sign. If e or
E is given, then an environment is defined, instead of a command.

Look at the second transcript file, line
`649 <titlepage-ex3.html#l649>`__. First, the translator sees a command;
here it is ``\lastname``, but what the translator knows is the command
code and its subcode; no name to is attached to this command code so
that you will see \``{(Unknown)}''. The translator executes a piece of
code that depends on the command code; this piece of code prints line
`650 <titlepage-ex3.html#l650>`__, namely \``{\titlepage 8}'', where 8
is the subcode of the command (the transcript file says, on line
`330 <titlepage-ex3.html#l330>`__, \``Defining \\lastname as
\\TitlePageCmd 8'', so that this is coherent.) The title page data
structure, contains for line 8, not only the name of the element to
create, but also the name of the command. Thus, you can see on line
`651 <titlepage-ex3.html#l651>`__ the name of the command,
\``{\titlepage 8=\lastname}'' and on line
`652 <titlepage-ex3.html#l652>`__ the start of the action \``{Push nom
1}''.

The effect of ``\cmd{foo}`` is roughly the same as
``\setbox8\xbox{elt}{foo}`` (replace \`\cmd' by the name of the command,
\`elt' by the name of the element, and \`\setbox8' by the effective
location in the titlepage structure, this can differ slightly from the
index of the command in the table). In the first version of *Tralics*
the argument of the command was read, translated, and the stack was
popped after that. As a consequence category codes were fixed once and
for all. When you say ``\setbox8=\hbox``, then TeX will execute the
``\hbox`` command, and push a special marker on the stack. When the
closing brace of the command is found, TeX will fix the glue of the box,
then pop its special stack, and then finish the assignment. No argument
is read, category changes are allowed, all modifications are local to
the group defined by the braces.

In the case of a titlepage element, another special marker is put on the
stack by *Tralics*. In general, the opening brace in ``\cmd{foo}`` does
not change the grouping level (this is different from ``\hbox``), so
that the transcript file will contain:
``+stack: level = 2 for titlepage argument`` (see transcript file, line
`653 <titlepage-ex3.html#l653>`__). In a simple case, for instance
``\Atitle``, the opening brace is sensed line
`539 <titlepage-ex3.html#l539>`__, the closing brace on line
`547 <titlepage-ex3.html#l547>`__, and this is followed by a line of the
form \``+stack: level - 2 for titlepage argument''. This means that the
current level will decrease from 2 to 1, which is the bottom level (what
you do *not* see in the current version is that the level is incremented
by one, when the special closing brace is seen).

Assume that ``\cmd@hook`` is defined. Then ``\cmd{foo}`` behaves like
``\setbox8 \xbox{elt} {\cmd@hook {foo}}``. This does not work if you use
\`cmd' as an environment. In fact, the argument of ``\foo`` is read, and
the token list ``{\cmd@hook {foo}}`` is pushed back in the input stream.
An example is given in the configuration line
`263 <titlepage-conf.html#l263>`__. The transcript file, on line
`653 <titlepage-ex3.html#l653>`__, shows the inserted brace, not the
initial one; on line `654 <titlepage-ex3.html#l654>`__, you see the call
of ``\lastname@hook``. On line `657 <titlepage-ex3.html#l657>`__, you
will see \``stack: level + 3'', this is because the hook expands to
``\textit{#1}``, and this is the same as ``{\it #1}``. On line
`660 <titlepage-ex3.html#l660>`__, you see the closing brace that
delimits the argument of the ``\it`` command, on line
`664 <titlepage-ex3.html#l664>`__ the closing brace of the titlepage
command, and on line `667 <titlepage-ex3.html#l667>`__ another closing
brace (input line 7 has the form ``\author{XYZ}``, the ``\author``
command command is defined to be ``\relax`` by the configuration file
line `238 <titlepage-conf.html#l238>`__, the 3 items X, Y, Z are
commands. The transcript file says, line
`666 <titlepage-ex3.html#l666>`__: \``{Pop 1: document_v nom_t}''. This
means that the current element (to be popped) is a ``<nom>`` in a
``<document>``.

Assume that ``\cmd@helper`` is defined. Then ``\cmd{foo}`` behaves like
``\setbox25 \xbox{elt} {\cmd@helper foo}``, and
``\begin{cmd}foo\end{cmd}`` behaves like
``\setbox25= \begin{xmlelement} {elt}\cmd@helper foo\end{xmlelement}``.
More precisely: if the command is defined, it is inserted and executed.
Braces are group delimiters. If we consider the case of \`Btitle', whose
execution starts line `554 <titlepage-ex3.html#l554>`__, we can see line
`556 <titlepage-ex3.html#l556>`__ that the opening brace that follows is
a special one (that leaves the level unchanged), and on line
`557 <titlepage-ex3.html#l557>`__ that the level is increased by one.
The closing brace is seen on line `568 <titlepage-ex3.html#l568>`__, and
line `570 <titlepage-ex3.html#l570>`__ shows that the level goes down
from 3 to 2 (as previously, you do not see it raising to 3). But you see
the effects: on line `569 <titlepage-ex3.html#l569>`__ the value of
``\@nomathml`` is restored. Line `572 <titlepage-ex3.html#l572>`__
matches line `557 <titlepage-ex3.html#l557>`__: the brace level goes
down from 2 to 1.

The situation is similar for environments. On lines
`805 <titlepage-ex3.html#l805>`__, `849 <titlepage-ex3.html#l849>`__,
``\begin{Aabstract}`` or ``\begin{Babstract}`` is seen. On lines
`806 <titlepage-ex3.html#l806>`__, `811 <titlepage-ex3.html#l811>`__
(and similarly, `850 <titlepage-ex3.html#l850>`__,
`855 <titlepage-ex3.html#l855>`__) we have two lines that say that the
current level increases or is unchanged. This is independent of the
existence of a helper or not. On line `842 <titlepage-ex3.html#l842>`__
and `887 <titlepage-ex3.html#l887>`__ we execute ``\end{Aabstract}`` or
``\end{Babstract}``. In general, this expands to ``\endAabstract``. But
this is a special case, so that, after restoring what is needed (line
`888 <titlepage-ex3.html#l888>`__ for instance), and unwinding the stack
(line `890 <titlepage-ex3.html#l890>`__), execution resumes with popping
the XML stack. As is the case of a non-environment, you see the level
decrease twice, but one of them (line `891 <titlepage-ex3.html#l891>`__)
is immediately corrected.

4.4. Interpreting the CES flags
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We finish the discussion of a command defined by:
``\cmd <elt> ?"value"``, where, instead of the question mark, you can
use one of A, B, C or plus sign. You can also say nothing. Then, if the
command is never used, ``<elt>value</elt>`` is added in proper place to
the TPA element.

On the titlepage example, lines `342 <titlepage-conf.html#l342>`__
--`345 <titlepage-conf.html#l345>`__, we define ``\cmdp``, ``\cmdA``,
``\cmdB``, ``\cmdC`` in a similar fashion, but add a flag before the
value. None of these commands is used in the TeX file (first example),
and you can observe the following facts.

-  *Tralics* complains with: *Error signaled at line 35: No value given
   for command \\cmdp.* In fact, when Tralics sees the
   ``\InsertTitlepage`` command, it notices that ``\cmdp`` has not been
   called and complains, because the plus-sign flags means: this value
   is required.
-  In the case where the flag is one of A, B or C, then ``"value"`` is a
   LaTeX command, that will be evaluated and translated (In fact,
   *Tralics* removes the double quotes, and inserts the characters in
   one of its buffers, or converts the string into a token list that is
   appended to the input token list). The non trivial point is: when is
   the command evaluated?
-  In the case of \`A' flag, the command is evaluated just before the
   ``\documentclass{TPA}`` command. It is marked `Insert
   1 <titlepage-ex3.html#insert1>`__ in the transcript file. Since there
   is a second insert, *Tralics* sees the following source lines : lines
   1 to 4 (this being the ``\documentclass`` line), the line 1 (for line
   of some insert), then line 88 (marked `Insert
   2 <titlepage-ex3.html#insert2>`__, this is a \`BeginCommands' block,
   starting at line `87 <titlepage-conf.html#l87>`__), then lines 5, 6,
   etc. Line `5 <titlepage-ex3.html#insert3>`__ is marked \`A destroyed'
   so that we know that the command is evaluated before this.
-  **Note:** Since version 2.9, the command is evaluated *before* the
   ``\documentclass`` command. See `explanations <#inserts>`__.
-  In the case of \`B' flag, the command is evaluated at begindocument.
   More precisely, ``\AtBeginDocument{\cmdb{\cmdBval}}`` is executed.
   This can been seen in the transcript file: after \`Defining \\cmdb',
   there is `the line <titlepage-ex3.html#insert4>`__
   ``[1] \cmdb{\cmdBval}``, followed by ``++ End of virtual file.`` This
   is a hack needed to convert the character string into a list of 4
   tokens (two braces, two commands), and this token list is fed to
   ``\AtBeginDocument``.
   These tokens are stored somewhere, and executed after the
   ``\begin{document}``, the result can be seen in the log in the region
   marked \`\ `documenthook <titlepage-ex3.html#documenthook>`__'.
-  In the case of \`C' flag, the command is evaluated when the TPA
   command is seen. In the transcript file is `the
   line <titlepage-ex3.html#insert5>`__ ``[1] \cmdc{\cmdCval}``,
   followed by ``++ End of virtual file.`` This is a hack, as above (the
   string is converted into a list of tokens, then executed).

4.5. Comments
~~~~~~~~~~~~~

The purpose of the CEES construction is to define a list of authors. See
example 1, where we have four authors, each defined by a single name.
Example 2 is more complicated, but incomplete. The idea would be the
following: the XML file contains a ``<auteur>`` element for each author
(the configuration file works for a single author, elements are
hard-coded `l.213 <titlepage-conf.html#l213>`__,
`l.221 <titlepage-conf.html#l221>`__. Authors can be divided into real
authors and contributors, the number of them is automatically computed
(`l.222 <titlepage-conf.html#l222>`__) and
`l.223 <titlepage-conf.html#l223>`__). It is easy to create the
``<nombre_auteurs>``: define a CES command, and do not use it, add flag
C; then some command is evaluated at the end, when the number of authors
is known.

The source file shows a ``\author`` command, containing the name
(structured via 3 commands) but not the affiliation (address, email,
etc), that are defined elsewhere. Very often, this is given via
footnotes, or special notes, like ``\thanks``. Some people use shorthand
notations like ``{A,B}@foo.bar`` for ``A@foo.bar`` and ``B@foo.bar``.

4.6. Special action
~~~~~~~~~~~~~~~~~~~

When you launch *Tralics* with the option ``-tpa_status=X``, this
controls what happens after the titlepage has been translated. If X is
``all`` (or starts with the letter \`a'), then the whole document is
translated as usual. If X is ``title`` (or starts with the letter \`t'),
then only the titlepage is translated. The resulting XML will contain
the TPA element, maybe some junk, maybe the bibliography. If X is
``config`` (or starts with the letter \`c'), then the result depends on
the configuration file, more precisely on the second attribute list of
the titlepage command (see examples lines
`207 <titlepage-conf.html#l207>`__ or
`312 <titlepage-conf.html#l312>`__). In the case where one of the
attributes of ``att2`` has the value ``'only title page'``, the
translation stops. Note that the string above contains two spaces and
two single quotes. If you really want this string to appear in the XML,
replace the quote by ``&apos;``. Any other value has the following
meaning: compilation stops if an error was signaled, continues
otherwise. By default, the configuration file has precedence.

If translation stops after the titlepage, the bibliography is not
inserted. Exception: if 'translate also bibliography' (with the quotes),
appears in the attribute list, then the equivalent of ``\nocite{*}``,
``\bibliography{\jobname}`` is executed. This has as side effect to
insert the bibliography at the end of the document.

Lines `244 <titlepage-conf.html#l244>`__,
`245 <titlepage-conf.html#l245>`__, `246 <titlepage-conf.html#l246>`__
of the configuration file contain the magic command ``\@reeavaluate``.
When you say ``\@reevaluate\foo\bar{gee}``, then commands ``\foo`` and
``\bar`` are called with ``gee`` as argument, as in
``\def\@reevaluate#1#2#3{#1{#3}#2{#3}}``, but the command can change the
category codes. A star is allowed, and the behaviour is as in
``\def\@reevaluate*#1#2#3{\begin{#1}#3\end{#1}\begin{#2}#3\end{#2}}``,
There is an example in the transcript file line
`375 <titlepage-ex3.html#l375>`__. We say
``\def \title {\@reevaluate \Atitle \Btitle}``. On line
`378 <titlepage-ex3.html#l378>`__ there is
``\def\abstract {\@reevaluate* {Aabstract} {Babstract}}``. The star form
of the command says that we consider environments. You can see the first
use on line `529 <titlepage-ex3.html#l529>`__.

First, *Tralics* reads the optional star, then two arguments. After
that, it reads all characters up to a closing brace, or (in the case of
a star) the end of the environment. It can happen that some characters
have already been read and tokenized. In this case, the token is
converted into a character string (with a leading backslash, and a
trailing space if the token has at least two characters, or has a single
character that is a letter (this is independent of the category code).
When characters are read from the buffer, the backslash character is
considered specially: the character that follows it is normal. Otherwise
an open brace increases the brace counter, a closing brace decreases the
brace counter. In the case of ``\@reevaluate*``, parsing ends when the
end of the environment is seen; this means that *Tralics* counts the
number of ``\begin`` and ``\end`` at brace-level zero (the command name
must be followed by a non-letter), and stops when there is one more
``\end`` than ``\begin`` (the characters in this ``\end`` token are not
read).

On line `530 <titlepage-ex3.html#l530>`__ and following, you can see see
the list of all these characters; they are put in a local file that has
a two lines, first is read at `l.534 <titlepage-ex3.html#l534>`__,
second is read at `l.551 <titlepage-ex3.html#l551>`__, end of file
detected at `l.574 <titlepage-ex3.html#l574>`__ The interesting part
starts on line `l.558 <titlepage-ex3.html#l558>`__. Originaly, we had
:literal:`\\catcode `\$=12 \@nomathml=1`, its effect is to replace the
math formula ``$L^\infy$`` by ``$L^\infty $``. Translation is `given
here <titlepage-ex2.html#ex2>`__. A better solution would be to use
``\@nomathml=-1`` (not category code changes!). Then the translation of
the math formula will be ``<texmath type='inline'>L^\infty </texmath>``.
The translation of the whole document is `given
here <titlepage-ex2.html#ex2new>`__.

In the case where ``\@nomathml`` is zero (normal case) or negative (as
above), parsing a math formula produces lines like
`l.543 <titlepage-ex3.html#l543>`__ (or
`l.564 <titlepage-ex3.html#l564>`__) because a math formula is a group,
maybe other lines that increase of decrease the grouping level, maybe
lines that show expansion of all local macros, followed by the
end-of-group line, followed by the list of all tokens read (see line
`l.827 <titlepage-ex3.html#l827>`__ for a more complicated example).
After that, the math formula is translated. You may see lines of the for
`l.829 <titlepage-ex3.html#l829>`__ whenever some tables are
reallocated.

Consider now line `l.781 <titlepage-ex3.html#l781>`__ in the transcript
file. We have the start of some environment. A new grouping level is
created (`line 784 <titlepage-ex3.html#l784>`__), and the command
associated to the environment is executed (`line
785 <titlepage-ex3.html#l785>`__), and this calls the magic command
``\@reevaluate``. All sources lines, from 24 to 27 are read (only the
first four characters of line 27 are consulted, none of them is read).
But *Tralics* reads one token (this should be the ``\end`` token) and a
sequence of characters (in the same way as the ``\end`` command reads is
argument); it does not test that this sequence is the current
environment. It closes the current environment (see lines
`791 <titlepage-ex3.html#l791>`__, `792 <titlepage-ex3.html#l792>`__).
After that, Tralics inserts the set of lines, of the form
``\begin {cmd1} text \end{cmd1}% \begin{cmd2}  text \end {cmd2}%``. As
above, a virtual file is created. It starts at line
`802 <titlepage-ex3.html#l802>`__ and lasts until
`893 <titlepage-ex3.html#l893>`__.

.. _cls-clo:

5. Classes and packages
-----------------------

.. _insert:

5.1 User configuration file
~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you compile file ``foo.tex``, then *Tralics* will insert the line
``\InputIfFileExists*{foo.ult}{}{}`` just near the line with the
documentclass declaration, if there is any. As mentioned above, there
are other insertions, and in the transcript file, they are all *after*
the line containing the ``\documentclass`` declaration. This is wrong in
the case where an optional argument follows the declaration on the
following line: we cannot insert text between the command and its
argument.

In the current version, all insertions are *before* the line containing
the ``\documentclass``. The order if the following: all commands from a
BeginCommand block in a configuration file (as Insert 2 above), then
insertions from the titlepage (as Insert 1 above) then the
InputIfFileExists line.

The semantics of ``\InputIfFileExists`` is the following. an argument is
read, here, ``foo.ult``. If the file exists, it is input; moreover the
first token list is added to the current stream, otherwise the second
list is inserted. Here both lists are empty, so that nothing special
happens. The star after the command has a special meaning: it says that
the @ character should be of category code 11 (letter) while reading the
file. The old category code is saved on a special stack.

If the file uses the document class 'bar', then ``bar.clt`` will be
used, and if package 'gee' is required, ``gee.plt`` will be used. The
files ``bar.clt`` and ``gee.plt`` are looked at it the current
directory, then in the directories specified by the ``confdir`` options
to the program, then in a default directory, in the same way as tcf
files, `described here <tcfpath>`__. No error is signalled if one of
these files is missing. The @ character has always category code 11
(letter) while loading these files. Normally, ``bar.clt`` contains the
same code as ``bar.cls`` (we shall explain later what is in the standard
classes), and ``gee.plt`` contains the same code as ``gee.sty``.

In order to implement commands like ``\LoadClassWithOptions``, a table
of all packages or classes, loaded not not, is maintained somewhere. A
global variable contains the current position in the table, it is saved
on the stack mentioned above, set to zero when a file opens, restored
when a file closes. A command like ``\DeclareOption`` adds an option to
the current class or package. If you say ``\input{baraux}``, the content
of the file \`baraux' does not belong to the current class or package.
In order to share options between standard classes, *Tralics* provides
``\InputClass{baraux}``. This loads the file ``baraux.clt`` (found in
the same way as ``aux.clt``), but the content is added to the current
class.

The following shows a bit what happens. The transcript file has a line
for every file whose existence is tested. The file 'article.clt'
identifies itself via ``\ProvidesClass`` line, it loads std.clt, that
identifies itself with a ``\ProvidesFile``. You can see how the integer
cur_file_pos is restored to zero (outside class or package, or to one,
the class). You can also see the options that are executed (in verbose
mode, you would see the code of the options as well). The first list
corresponds to the ``\ExecuteOptions`` command of the class file, the
second to the ``\ProcessOptions``

.. container:: log_out

   ::

      ++ file article.clt does not exist 
      ++ file ../confdir/article.clt exists 
      ++ Made @ a letter
      ++  Opened file ../confdir/article.clt; it has 25 lines
      Document class: article 2006/08/19 v1.0 article document class for Tralics
      ++ file std.clt does not exist 
      ++ file ../confdir/std.clt exists 
      ++ Made @ a letter
      ++  Opened file ../confdir/std.clt; it has 51 lines
      File: std 2006/08/19 v1.0 Standard LaTeX document class, for Tralics
      ++ End of file ../confdir/std.clt
      ++ cur_file_pos restored to 1
      {Options to execute->letterpaper,10pt,oneside,onecolumn,final}
      {Options to execute->}
      ++ End of file ../confdir/article.clt
      ++ cur_file_pos restored to 0
