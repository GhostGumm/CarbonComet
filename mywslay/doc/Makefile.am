# Wslay - The WebSocket Library

# Copyright (c) 2011, 2012 Tatsuhiro Tsujikawa

# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:

# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
SPHINX_DIR = $(srcdir)/sphinx
MANPAGES_WITH_SRC = \
SPHINX_MANPAGES_SRC = $(MANPAGES_WITH_SRC:%.3=$(SPHINX_DIR)/%.rst)

SPHINX_FILES = $(SPHINX_DIR)/Makefile \
	$(SPHINX_DIR)/api_reference.rst \
	$(SPHINX_DIR)/conf.py \
	$(SPHINX_DIR)/index.rst \
	$(SPHINX_DIR)/tutorial.rst \
	$(SPHINX_MANPAGES_SRC)

MANPAGES_DST = $(MANPAGES_SRC:${SPHINX_DIR}/%.rst=%.3)
MANPAGES = $(MANPAGES_WITH_SRC)
man_MANS = $(MANPAGES)
EXTRA_DIST = $(MANPAGES) $(SPHINX_FILES)

if HAVE_SPHINX_BUILD
$(MANPAGES_WITH_SRC): $(SPHINX_MANPAGES_SRC)
	echo $(MANPAGES_DST)
	@SPHINX_BUILD@ -b man $(SPHINX_DIR) man
endif # HAVE_SPHINX_BUILD
