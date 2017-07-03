# CFLAGS and CXXFLAGS are either the name of environment variables or of
# Makefile variables that can be set to specify additional switches to be
# passed to a compiler in the process of building computer software.

# Generate debug information.
CFLAGS += -g

# Moderate level of optimization.
CFLAGS += -O2

# Turns on certain warning options.
# https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
CFLAGS += -Wall

# Enables extra warning flags that are not enabled by -Wall.
CFLAGS += -Wextra
CFLAGS += -Isrc
# CFLAGS += -rdynamic
CFLAGS += -DNDEBUG
CFLAGS += $(OPTFLAGS) # Augment the build options as needed

# LDFLAGS is a variable used in Makefiles. Specifically its value is added to
# linker command lines when the linker is invoked by an implicit rule.
LDFLAGS = -ldl $(OPTLIBS)

# Set an optional variable called PREFIX that will only have this value
# if the person running the makefile didn't already give a prefix setting.
# That's what the ?= does.
PREFIX?=/usr/local

# Dynamically create the SOURCES variable by doing a wildcard search
# for all *.c files in the src/ directory.
SOURCES=$(wildcard src/**/*.c src/*.c)

# Once we have the source files, we can use pattern substitution
# to convert them to object files.
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

# Find all the source files for unit tests
TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

TARGET=build/libhashmap.a
SO_TARGET=$(patsubst %.a,%.so,$(TARGET))

# The Target Build
# all: $(TARGET) $(SO_TARGET) tests
all: $(TARGET) tests

dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra $(OPTFLAGS)
dev: all

$(TARGET): CFLAGS += -fPIC
$(TARGET): build $(OBJECTS)
	ar rcs $@ $(OBJECTS)
	ranlib $@

build:
	@echo $(OBJECTS)
	@mkdir -p build
	@mkdir -p bin

# The Unit Tests

# A phony target is one that is not really the name of a file; rather it is
# just a name for a recipe to be executed when you make an explicit request.
# There are two reasons to use a phony target: to avoid a conflict with a file
# of the same name, and to improve performance.
#
# https://www.gnu.org/software/make/manual/html_node/Phony-Targets.html
.PHONY: tests
tests: LDLIBS += $(TARGET)
tests: $(TESTS)
	sh ./tests/runtests.sh

valgrind:
	VALGRIND="valgrind --log-file=/tmp/valgrind-%p.log" $(MAKE)

# The Cleaner
clean:
	rm -rf build $(OBJECTS) $(TESTS)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`

# The Install
install: all
	install -d $(DESTDIR)/$(PREFIX)/lib/
	install $(TARGET) $(DESTDIR)/$(PREFIX)/lib/

# The Checker
check:
	@echo Files with potentially dangerous functions.
	@egrep '[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)' $(SOURCES) || true

ctags:
	ctags --c-kinds=+defgmpstux -R
