#!/usr/bin/env python3
"""Doxygen input filter for VAMPS C source files.

Converts two legacy doc-comment styles to standard Doxygen /** ... */ format:

  /*C:funcname                       ->  /** @brief funcname
   *@return_type funcname(args)       ->   * @code return_type funcname(args) @endcode
   * Description text                ->   * Description text
   * Returns: value                  ->   * @return value
   */                                ->   */

  /*+Name: funcname                  ->  /** @brief funcname
   *  Prototype: return_type func()  ->   * @code return_type func() @endcode
   *  Description:  text             ->   * text
   *  Returns:  value+*/             ->   * @return value
                                         */

Usage (Doxygen INPUT_FILTER):
    python3 vamps_filter.py <filename>
"""

import re
import sys


def transform(text: str) -> str:
    lines = text.splitlines(keepends=True)
    out = []
    i = 0

    while i < len(lines):
        line = lines[i]

        # ------------------------------------------------------------------ #
        # Style 1: /*C:funcname ...  */                                       #
        # ------------------------------------------------------------------ #
        m = re.match(r'^(\s*)/\*C:(\S+)(.*)', line)
        if m:
            indent, name, rest = m.group(1), m.group(2), m.group(3).strip()
            out.append(f'{indent}/** @brief {name}\n')
            if rest:
                out.append(f'{indent} * {rest}\n')
            i += 1
            proto_seen = False
            while i < len(lines):
                l = lines[i]
                i += 1
                # End of block
                if re.search(r'\*/', l):
                    # Check for trailing Returns: before */
                    rm = re.match(r'\s*\*\s*Returns?:?\s*(.*?)\s*\*/', l, re.IGNORECASE)
                    if rm and rm.group(1):
                        out.append(f'{indent} * @return {rm.group(1)}\n')
                        out.append(f'{indent} */\n')
                    else:
                        out.append(f'{indent} */\n')
                    break
                # Prototype line: *@prototype
                pm = re.match(r'\s*\*@(.*)', l)
                if pm:
                    proto = pm.group(1).strip()
                    if not proto_seen:
                        proto_seen = True
                        out.append(f'{indent} * @code {proto} @endcode\n')
                    else:
                        # Subsequent *@ lines are inline code examples
                        out.append(f'{indent} * @code {proto} @endcode\n')
                    continue
                # Returns line
                rm = re.match(r'(\s*\*\s*)Returns?:?\s+(.*)', l, re.IGNORECASE)
                if rm:
                    out.append(f'{indent} * @return {rm.group(2)}\n')
                    continue
                # Strip inline @[text]@ markers
                l = re.sub(r'@\[([^\]]*)\]@', r'\1', l)
                out.append(l)
            continue

        # ------------------------------------------------------------------ #
        # Style 2: /*+Name: funcname ... +*/                                  #
        # ------------------------------------------------------------------ #
        m = re.match(r'^(\s*)/\*\+Name:\s*(.*)', line)
        if m:
            indent, name = m.group(1), m.group(2).strip()
            out.append(f'{indent}/** @brief {name}\n')
            i += 1
            while i < len(lines):
                l = lines[i]
                i += 1
                # End of block: +*/
                if re.search(r'\+\*/', l):
                    rm = re.match(r'\s*\*\s*Returns?:?\s*(.*?)\s*\+\*/', l, re.IGNORECASE)
                    if rm and rm.group(1):
                        out.append(f'{indent} * @return {rm.group(1)}\n')
                    out.append(f'{indent} */\n')
                    break
                # Prototype line
                pm = re.match(r'\s*\*\s*Prototype:\s*(.*)', l)
                if pm:
                    out.append(f'{indent} * @code {pm.group(1).strip()} @endcode\n')
                    continue
                # Description label — just strip it
                dm = re.match(r'(\s*\*\s*)Description:\s*(.*)', l)
                if dm:
                    rest = dm.group(2).strip()
                    if rest:
                        out.append(f'{indent} * {rest}\n')
                    continue
                # Returns line (without +*/)
                rm = re.match(r'\s*\*\s*Returns?:?\s+(.*)', l, re.IGNORECASE)
                if rm:
                    out.append(f'{indent} * @return {rm.group(1)}\n')
                    continue
                out.append(l)
            continue

        out.append(line)
        i += 1

    return ''.join(out)


if __name__ == '__main__':
    filename = sys.argv[1] if len(sys.argv) > 1 else '/dev/stdin'
    with open(filename, 'r', errors='replace') as fh:
        text = fh.read()
    sys.stdout.write(transform(text))
