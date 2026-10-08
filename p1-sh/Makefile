#
# Simple Makefile
# Mike Lam, James Madison University, August 2016
#
# This makefile builds a simple application that contains a main module
# (specified by the EXE variable) and a predefined list of additional modules
# (specified by the MODS variable). If there are any external library
# dependencies (e.g., the math library, "-lm"), list them in the LIBS variable.
# If there are any precompiled object files, list them in the OBJS variable.
#
# By default, this makefile will build the project with debugging symbols and
# without optimization. To change this, edit or remove the "-g" and "-O0"
# options in CFLAGS and LDFLAGS accordingly.
#
# By default, this makefile build the application using the GNU C compiler,
# adhering to the C99 standard with all warnings enabled.


# application-specific settings and run target

EXE=dukesh
UTILS=ls loop segv chmod head mkdir rm touch file repeat cat tee
MODS=main.o process.o shell.o builtins.o cmd.o hash.o
OBJS=
LIBS=

default: build $(EXE)

build:
	@mkdir build

bin:
	@mkdir bin

ifeq (test,$(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "test"
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  # ...and turn them into do-nothing targets
  $(eval $(RUN_ARGS):;@:)
else ifeq (check,$(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "check"
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  # ...and turn them into do-nothing targets
  $(eval $(RUN_ARGS):;@:)
else ifeq (submit,$(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "submit"
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  # ...and turn them into do-nothing targets
  $(eval $(RUN_ARGS):;@:)
else ifeq (status,$(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "status"
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  # ...and turn them into do-nothing targets
  $(eval $(RUN_ARGS):;@:)
endif

test: build $(EXE) utils
	@make -C tests test $(RUN_ARGS)

utest: build $(EXE)
	@make -C tests utest

check: build $(EXE) utils
	@make -C tests check "$(RUN_ARGS)"

submit: build $(EXE) utils
	@make -C tests submit "$(RUN_ARGS)"

status: build $(EXE) utils
	@make -C tests status

style: $(EXE)
	@make -C tests style

# compiler/linker settings

CC=gcc
CFLAGS=-g -O0 -Wall -Werror -std=c99 -pedantic -D_POSIX_C_SOURCE=200809L
LDFLAGS=-g -O0


# build targets

BUILD=$(addprefix build/, $(MODS))

$(EXE): build/main.o $(BUILD) $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXE) $^ $(LIBS)

build/%.o: src/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

BINUTILS=$(addprefix bin/, $(UTILS))

utils: bin $(BINUTILS)

bin/%: utils/%.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf $(EXE) build bin
	make -C utils clean
	make -C tests clean

.PHONY: default clean test check style submit status

