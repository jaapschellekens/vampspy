Command-Line Reference — vamps(1)
==================================

This page is derived from the ``vamps.1`` man page.


Name
----

**vamps** — a model of Vegetation-AtMosPhere-Soil water flow


Synopsis
--------

.. code-block:: text

   vamps [--help] [--Header] [--copyright] [--license]
         [--verbose] [--showdef] [--fit] [--noinputdump]
         [--Comment commentchar]
         [--Output setname]
         [--Determine variable]
         [--output filename]
         [--Logfile filename]
         [--Setvar section name value]
         [--spEed speed]
         [--interFace]
         filename


Description
-----------

``vamps`` is a one-dimensional model for describing the flow of water through
a forested ecosystem.  VAMPS has been designed to be flexible and fast.

Flexibility allows users to adapt the programme by changing parameters in the
input file or by hooking Python scripts into the run via the ``[vamps] xtrapy``
configuration key.

.. note::

   In version 1.0 the S-Lang scripting engine present in earlier versions has
   been replaced by embedded CPython.  S-Lang intrinsics and the interactive
   S-Lang interface are no longer available.  User scripting is now done in
   Python 3 (see :doc:`../user_guide/python_scripting`).

A Python package ``vampspy`` is provided for driving VAMPS entirely from
Python with no subprocess invocation.  It supports single-column runs and
parallel 2-D grid runs.  See :doc:`../vampspy/api` for the API reference.

Input is read from *filename* or from standard input if *filename* is ``-``.
Output is written to stdout, or to *filename* if the ``-o`` option is used.

VAMPS intercepts some signals (like keyboard interrupts) and ignores them
MAXSIG times (usually 5).  To stop VAMPS you must send the interrupt MAXSIG
times.


Options
-------

Options may be given as standard one-letter options or GNU-style long options
(prefixed with ``--``).  Long options may be truncated to any unambiguous
abbreviation.  The short form is the first letter of the long option, or the
first capitalised letter if one exists (e.g. ``-H`` for ``--Header``,
``-E`` for ``--spEed``).

``--Header``  (``-H``)
    Omit the headers in output.

``--copyright``  (``-c``)
    Show copyright information and exit.

``--license``  (``-l``)
    Print license information and exit.

``--help``  (``-h``)
    Print help text and exit.

``--verbose``  (``-v``)
    Enter verbose mode; programme progress is displayed on stderr.
    Default is silent.

``--noinputdump``  (``-n``)
    By default the contents of the input file is included in the output.
    Use this option to suppress that.

``--Output setname``  (``-O setname``)
    Dump the specified *setname* to a separate file in ts(5) format.
    See :doc:`config_reference` for a list of allowed set names.

``--Determine variable``  (``-D variable``)
    Determine *variable* (single variable or time series).  This is normally
    set in the input file.  Multiple ``--Determine`` options are allowed.

``--Logfile filename``  (``-L filename``)
    Log this session to *filename*.

``--Setvar section name value``  (``-S section name value``)
    Set the value of variable *name* in section *section* to *value*.
    Overrides the input file setting.

``--Comment commentchar``  (``-C commentchar``)
    Set the comment character.  Do not change this without a good reason.

``--showdef``  (``-s``)
    Send programme defaults to stdout.  Useful for creating a default file
    from scratch.

``--output filename``  (``-o filename``)
    Send output to *filename* instead of stdout.

``--spEed speed``  (``-E speed``)
    Set calculation speed (1 = slow/accurate, 6 = fastest/least accurate).
    See the ``speed`` variable in :doc:`config_reference` for details.

``--fit``  (``-f``)
    Fit data to settings in the ``[fit]`` section.

``--interFace``  (``-F``)
    Start the interactive interface.  This option must be the last option on
    the command line (before *filename* if any).

*filename*
    Read input from *filename*.  Use ``-`` to read from standard input.


Usage
-----

If ``--verbose`` is set (via the command line or the input file), VAMPS shows
progress on stderr.

On UNIX systems with X11 and ``gnuplot`` installed, progress can also be
displayed graphically.  Set ``showgraph = 1`` and ``graphcommand`` to the
gnuplot executable in the ``[vamps]`` section.  Note that this slows
calculations.


Python scripting
----------------

VAMPS embeds a CPython interpreter.  Python hooks are loaded at startup via
the ``xtrapy`` variable in the ``[vamps]`` section:

.. code-block:: ini

   [vamps]
   xtrapy = myscript.py

The script may define ``at_start()``, ``each_step()``, and ``at_end()``
functions.  Import the ``vamps`` module to access simulation state.

See :doc:`../user_guide/python_scripting` for full details.


Files
-----

``$VAMPSLIB/vamps_startup.py``
    Python startup script auto-loaded before every user script.

``$HOME/.vampsrc`` (UNIX) or ``vamps.rc`` (Windows)
    User defaults file.  Syntax is identical to the input file.

``$VAMPSLIB/``
    Directory containing Python library modules.  Set via the ``VAMPSLIB``
    environment variable.


Environment
-----------

``VAMPSLIB``
    Path to the VAMPS library directory.  Must contain ``vamps_startup.py``
    and the share modules.

``VAMPS_BINARY``
    Path to the ``vamps`` executable (used by the ``vampspy`` fallback mode).


See also
--------

* :doc:`config_reference` — vamps(5) input/output file reference
* :doc:`../user_guide/configuration` — tutorial-style configuration guide
* :doc:`../vampspy/api` — vampspy Python API reference
