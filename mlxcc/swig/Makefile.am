
EXTRA_DIST = perl.i python.i ruby.i constants.i \
             perl-typemaps.i python-typemaps.i \
             ruby-typemaps.i xcc.i

swig:
	swig -ruby -o xcc-ruby.c xcc.i
	swig -perl -o xcc-perl.c xcc.i
	swig -python -o xcc-python.c xcc.i

clean:
	rm -Rf xcc-ruby.c xcc-perl.c xcc-python.c *.pm *.py
