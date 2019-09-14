SWIG_INTERFACE = lex_tools.i

BUILT_SOURCES = %_wrap.cpp

%_wrap.cpp: $(SWIG_INTERFACE) setup.py
	$(PYTHON) setup.py build

install-exec-local:
	$(PYTHON) setup.py install $(PYTHON_INSTALL_PARAMS)
