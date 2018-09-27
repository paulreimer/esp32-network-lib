#!/usr/bin/env python
from __future__ import print_function

import re
import sys

from jinja2 import BaseLoader, Environment

output_template = '''
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
{% for sym in skip_symbols -%}
/* {{ sym.addr }} {{ sym.tag }} {{ sym.name }} */
{% endfor -%}
#include "sym.h"
{% for sym in data_symbols %}
static const char __D{{ loop.index0 }}[] = "{{ sym.name }}";
{%- endfor %}
{% for sym in data_symbols %}
extern int {{ sym.name }};
{%- endfor %}

const int sym_objects_count = {{ data_symbols | length }};
const struct symbol sym_objects[{{ data_symbols | length }}] = {
{%- for sym in data_symbols %}
  { (const char *)__D{{ loop.index0 }}, { .obj = (void *)&{{ sym.name }} } },
{%- endfor %}
};
{% for sym in text_symbols %}
static const char __T{{ loop.index0 }}[] = "{{ sym.name }}";
{%- endfor %}
{% for sym in text_symbols %}
extern int {{ sym.name }}();
{%- endfor %}

// built-ins
int printf(const char *, ...);
int sprintf(char *, const char *, ...);
void *malloc();
//void *memcpy();
void *memset();
void *memmove();
char *strcpy();

double sin();
double cos();
float sinf(float);
float cosf(float);

const int sym_functions_count = {{ text_symbols | length }};
const struct symbol sym_functions[{{ text_symbols | length }}] = {
{%- for sym in text_symbols %}
  { (const char *)__T{{ loop.index0 }}, { .func = (fn)&{{ sym.name }} } },
{%- endfor %}
};
#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus
'''

if __name__ == '__main__':
    skip_symbols = []
    text_symbols = []
    data_symbols = []

    # Valid tags from: https://sourceware.org/binutils/docs/binutils/nm.html
    parse_symbol = re.compile(
        r'''
        (?P<addr>\b[0-9a-fA-F]+\b)\s+                   # e.g. 40088ec8
        (?P<tag>\b([ABbCDdGgiINPRrSsTtUuVvWw?-])\b)\s+  # e.g. T
        (?P<name>.*)                                    # e.g. printf
        ''',
        re.VERBOSE
    )

    #is_skip = lambda sym: sym.tag in list('')
    def is_skip(sym): return sym['name'] in (
        'sym_objects',
        'sym_objects_count',
        'sym_functions',
        'sym_functions_count',

        '__cxa_call_unexpected',
        'DW.ref.__gxx_personality_v0',

        'printf',
        'sprintf',

        'malloc',
        #        'memcpy',
        'memset',
        'memmove',
        'strcpy',

        'sin',
        'cos',
        'sinf',
        'cosf',
    )
    #is_data = lambda sym: sym['tag'] in list('BCDGRS')

    def is_data(sym): return sym['tag'] in list('BCDGRSAVWw')

    def is_text(sym): return sym['tag'] in list('T')

    for line in sys.stdin:

        # Check for regex match of any kind of symbol
        match = parse_symbol.match(line)

        if match is not None:
            tag = match.group('tag')
            symbol = {
                'addr': match.group('addr'),
                'name': match.group('name'),
                'tag':  match.group('tag'),
            }

            # Store the symbol in the appropriate set
            if is_skip(symbol):
                skip_symbols.append(symbol)

            elif is_data(symbol):
                data_symbols.append(symbol)

            elif is_text(symbol):
                text_symbols.append(symbol)

    # Render the template
    template_obj = Environment(loader=BaseLoader).from_string(output_template)
    rendered = template_obj.render(locals())

    print(rendered)
