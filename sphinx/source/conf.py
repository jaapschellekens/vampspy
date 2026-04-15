import os
import sys

sys.path.insert(0, os.path.abspath('../../'))

# -- Project information -------------------------------------------------------

project = 'VAMPS'
copyright = '2026, J. Schellekens'
author = 'J. Schellekens'
release = '1.0'

# -- General configuration -----------------------------------------------------

extensions = [
    'myst_parser',
    'sphinx.ext.autodoc',
    'sphinx.ext.napoleon',
    'sphinx.ext.mathjax',
    'sphinxcontrib.mermaid',
    'sphinxcontrib.bibtex',
    'breathe',
]

breathe_projects = {'VAMPS': '../_build/doxygen/xml'}
breathe_default_project = 'VAMPS'

bibtex_bibfiles = ['refs.bib']
bibtex_default_style = 'unsrt'

myst_enable_extensions = [
    'dollarmath',
    'colon_fence',
]

# Tell MyST to render ```mermaid fences as sphinxcontrib-mermaid directives
myst_fence_as_directive = ['mermaid']

templates_path = ['_templates']
exclude_patterns = [
    '_build',
    'Thumbs.db',
    '.DS_Store',
    '**/.ipynb_checkpoints',
]

source_suffix = {
    '.rst': 'restructuredtext',
    '.md': 'markdown',
}

# -- Options for HTML output ---------------------------------------------------

html_theme = 'sphinx_rtd_theme'
