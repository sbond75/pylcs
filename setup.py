from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
import sys
import setuptools
import os

# Config #
debugMode=False
# #


class get_pybind_include(object):
    """Helper class to determine the pybind11 include path
    The purpose of this class is to postpone importing pybind11
    until it is actually installed, so that the ``get_include()``
    method can be invoked. """

    def __init__(self, user=False):
        self.user = user

    def __str__(self):
        import pybind11
        return pybind11.get_include(self.user)


# https://stackoverflow.com/questions/4597228/how-to-statically-link-a-library-when-compiling-a-python-module-extension
static_libraries = ['libmba'] # Static libs
static_lib_dir = os.path.join(os.getcwd(), "libmba-0.8.10", "build", "Release")
libraries = [] # Dynamic libs
library_dirs = []

if sys.platform == 'win32':
    libraries.extend(static_libraries)
    library_dirs.append(static_lib_dir)
    extra_objects = []
else: # POSIX
    extra_objects = ['{}/lib{}.a'.format(static_lib_dir, l) for l in static_libraries]

#libs = {'libraries': libraries} if len(libraries) > 0 else {}

extra_link_args = {
    'msvc': ['/LIBPATH:' + x for x in [static_lib_dir]] + [x + '.lib' for x in static_libraries],
    'unix': ['-L' + x for x in [static_lib_dir]] + ['-l' + x.removeprefix('lib') for x in static_libraries],
}


ext_modules = [
    Extension(
        'pylcs',
        ['src/main.cpp'],
        include_dirs=[
            # Path to pybind11 headers
            get_pybind_include(),
            get_pybind_include(user=True)
        ],
        language='c++',
    extra_compile_args = (([
        "/Od", "/Zi" # build for debugging and produce pdb file
        , "/DLCS_DEBUG"
    ] if sys.platform == 'win32' else ['-O0', '-g3', "-DLCS_DEBUG"]) if debugMode else [
        "/Ox" if sys.platform == 'win32' else '-O3' # HACK: should be checking compiler instead
    ])[0],
    extra_link_args=extra_link_args['msvc'] if sys.platform == 'win32' else extra_link_args['unix'] # HACK: should be checking compiler instead        # https://stackoverflow.com/questions/49256000/setting-compile-arguments-in-setup-py-file-or-linker-arguments-lrt
    #**libs, # https://stackoverflow.com/questions/5710391/converting-python-dict-to-kwargs
    # library_dirs=library_dirs,
    # extra_objects=extra_objects

    ),
]


# As of Python 3.6, CCompiler has a `has_flag` method.
# cf http://bugs.python.org/issue26689
def has_flag(compiler, flagname):
    """Return a boolean indicating whether a flag name is supported on
    the specified compiler.
    """
    import tempfile
    with tempfile.NamedTemporaryFile('w', suffix='.cpp') as f:
        f.write('int main (int argc, char **argv) { return 0; }')
        try:
            compiler.compile([f.name], extra_postargs=[flagname])
        except setuptools.distutils.errors.CompileError:
            return False
    return True


def cpp_flag(compiler):
    """Return the -std=c++[11/14] compiler flag.
    The c++14 is prefered over c++11 (when it is available).
    """
    if has_flag(compiler, '-std=c++14'):
        return '-std=c++14'
    elif has_flag(compiler, '-std=c++11'):
        return '-std=c++11'
    else:
        raise RuntimeError('Unsupported compiler -- at least C++11 support '
                           'is needed!')


class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""
    c_opts = {
        'msvc': ['/EHsc'],
        'unix': []
    }

    if sys.platform == 'darwin':
        c_opts['unix'] += ['-stdlib=libc++', '-mmacosx-version-min=10.7']

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        if ct == 'unix':
            opts.append('-DVERSION_INFO="%s"' % self.distribution.get_version())
            opts.append(cpp_flag(self.compiler))
            if has_flag(self.compiler, '-fvisibility=hidden'):
                opts.append('-fvisibility=hidden')
        elif ct == 'msvc':
            opts.append('/DVERSION_INFO=\\"%s\\"' % self.distribution.get_version())
        for ext in self.extensions:
            ext.extra_compile_args = opts
        build_ext.build_extensions(self)

with open("README.md", "r") as fh:
    long_description = fh.read()

# Prepare dependencies
import subprocess
try:
    subprocess.run(['bash', 'buildDependencies.sh', os.getcwd()], stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=True)
except subprocess.CalledProcessError as exc:
    print("Status : FAIL", exc.returncode, exc.output)
    exit(exc.returncode)



setup(
    name='pylcs',
    version='0.0.6',
    author='Meteorix',
    author_email='lxhustauto@gmail.com',
    url='https://github.com/Meteorix/pylcs',
    description='super fast cpp implementation of longest common subsequence',
    long_description=long_description,
    long_description_content_type="text/markdown",
    ext_modules=ext_modules,
    install_requires=['pybind11>=2.2'],
    cmdclass={'build_ext': BuildExt},
    zip_safe=False,
    include_dirs=[
        os.path.join(os.getcwd(), "eigen"),
        os.path.join(os.getcwd(), "libmba-0.8.10", "src")
    ],
)
