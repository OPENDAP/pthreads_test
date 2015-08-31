# Build libdap, libtest-types.a, getdap, getdap4

AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS =

AM_CPPFLAGS =
AM_CXXFLAGS = 

if COMPILER_IS_GCC
AM_CXXFLAGS += -Wall -W -Wcast-align
endif

# These are not used by automake but are often useful for certain types of
# debugging. The best way to use these is to run configure as:
# ./configure --disable-shared CXXFLAGS=...
# or ./configure --enable-developer --disable-shared
# the --disable-shared is not required, but it seems to help with debuggers.
CXXFLAGS_DEBUG = -g3 -O0  -Wall -W -Wcast-align 
TEST_COV_FLAGS = -ftest-coverage -fprofile-arcs

AM_CXXFLAGS += $(CXXFLAGS_DEBUG)

# SUBDIRS =

bin_PROGRAMS = p_test

p_test_SOURCES = MarshallerThread.cc MarshallerThread.h Error.cc Error.h \
	InternalErr.cc InternalErr.h p_test.cc
	  
p_test_LDADD = -lpthread

