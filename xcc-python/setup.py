# setup.py
from distutils.core import setup, Extension
setup (name = "python-xcc",
    version = "0.5.2",
    maintainer = "Andrew Bruno",
    maintainer_email = "aeb@qnot.org",
    description = "Python bindings for Mark Logic",
    url = "https://code.qnot.org/svn/projects/marklogic/xcc-python/",
    packages=["xcc"],
    ext_modules = [Extension('_xcc',
                  include_dirs = ['/usr/include'],
                  library_dirs=['/usr/lib'],
                  libraries = ['mlxcc'],
                  sources=['xcc/xcc-python.c'])])
