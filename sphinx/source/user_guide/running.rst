Running VAMPS
=============

Synopsis
--------

.. code-block:: text

   vamps [--help] [--Header] [--copyright] [--license] [--verbose]
         [--showdef] [--fit] [--noinputdump]
         [--Comment commentchar] [--Output setname]
         [--Determine variable] [--output filename]
         [--Logfile filename] [--Setvar section name value]
         [--spEed speed] filename


Command-line options
--------------------

Options may be given as standard one-letter options or GNU-style long
options.  Long options start with two dashes ``--`` and may be truncated to
any unambiguous abbreviation.  The short option is the first letter of the
long option, or the first capitalised letter if one is present (e.g. ``-H``
for ``--Header``, ``-E`` for ``--spEed``).

``--Header``
    Omit the headers in output.

``--copyright``
    Show copyright information and exit.

``--license``
    Print license information and exit.

``--help``
    Print help text and exit.

``--verbose``
    Enter verbose mode; programme progress is displayed on stderr.  Default
    is silent.

``--noinputdump``
    By default the contents of the input file is included in the output.
    Use this option to suppress that.

``--Output setname``
    Dump the specified *setname* to a separate file in ts(5) format.  See
    the configuration reference for a list of allowed set names.

``--Determine variable``
    Determine *variable* (can be a single variable or a time series).  This
    is normally set in the input file.  Multiple options are allowed.

``--Logfile filename``
    Log this session to *filename*.

``--Setvar section name value``
    Set the value of variable *name* in section *section* to *value*.  This
    overrides the setting in the input file.

``--Comment commentchar``
    Set the comment character to *commentchar*.  Do not change this without
    good reason.

``--showdef``
    Send programme defaults to stdout.  This can be used to create a defaults
    file from scratch.

``--output filename``
    Send output to *filename* instead of stdout.

``--spEed speed``
    Set calculation speed (1 = slow/accurate, 6 = fastest/least accurate).
    See the ``speed`` variable in ``[soil]`` for details.

``--fit``
    Fit data to the settings in the ``[fit]`` section.

``filename``
    Read input from *filename*.  If given as a single dash ``-`` it is taken
    from standard input.


Usage modes
-----------

VAMPS runs in two modes:

1. **Batch mode** — started with command-line options, runs and exits when
   finished.  This is the normal mode.
2. **Interactive mode** — starts an interactive interface (``--interFace`` or
   ``-F``).  This option must be the last option on the command line.
   *Note: the S-Lang interactive interface from earlier versions is no longer
   available in version 1.0.  User scripting is now done in Python 3 via
   the* ``xtrapy`` *mechanism.*

If ``--verbose`` is set VAMPS shows the progress of calculations on stderr.

VAMPS intercepts some signals (like keyboard interrupts) and ignores them
MAXSIG times (usually 5).  To stop VAMPS using ``^C`` you must press it
MAXSIG times.


The vsel utility
----------------

``vsel`` is the recommended tool for extracting time-series variables from
VAMPS ``.out`` output files.  It can produce two-column ASCII output suitable
for import into plotting programmes (``gnuplot``) or spreadsheets.

From Python, the equivalent functionality is available in ``share/util.py``
as the ``vsel()`` and ``vprof()`` functions.


.. _sec-troubles:

Troubleshooting
---------------

Variable missing
~~~~~~~~~~~~~~~~

If VAMPS issues a message like::

   deffile.c: Fatal:  could not find  ->steps<-
       in section     ->time<-
       in file         ->example1.inp<-
   deffile.c:  error message:
       from: deffile.c
       description: Var not found or invalid, see above.

you have not specified a variable that VAMPS needs to run.  This can be a
variable that is always required, or one that is needed because of another
variable you have set.  For example, if you specify the Gash method for
interception you must also specify ``S`` in the ``[interception]`` section.


Mass-balance error warning
~~~~~~~~~~~~~~~~~~~~~~~~~~

VAMPS issues a warning when large jumps in the mass-balance error are
encountered.  You can ignore them as long as you are aware that the solution
is not optimal.

.. note::
   The mass balance is *not* calculated correctly when using lateral drainage.

To reduce mass-balance errors, in order:

1. Decrease ``dtmax`` — prevents VAMPS from making large timestep estimates.
2. Decrease ``dtmin`` and/or ``thetol``.
3. Increase ``maxitr``.
