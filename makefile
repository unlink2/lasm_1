CC=g++
EXT=.cc
DBG=gdb
BIN=lasm
PKGCONFIG = $(shell which pkg-config)

# where is core code defined?
INCLUDEDIR=./src
SRCDIR=./src
INCLUDEDIR_FRONT=./frontend
SRCDIR_FRONT=./frontend

# where are tests defined?
INCLUDEDIR_TEST=./tests
SRCDIR_TEST=./tests

# sdl frontend objects
ODIR=./obj
# test objects
ODIR_TEST=$(ODIR)/tests
# all binaries
BINDIR=./bin

DEBUG=-DDEBUG
LIBS=
CFLAGS=-Wall -g -std=c++17 -Ilibs/argcc/src/ -Isrc/
CFLAGS_LIB=-Wall -g
LIBS_TEST=-lm
CFLAGS_TEST=-Wall -g -std=c++17 -Ilibs/argcc/src/ -Isrc/
OTHER_FLAGS=-rdynamic
MAIN = main # main for frontend
TEST_MAIN = test # main for test
INSTALLDIR = /usr/local/bin

# test source modules
MODULES_TEST = test_scanner test_token test_object test_instruction test_utility test_expr test_parser test_interpreter\
			   test_enviorment

# modules from core that can be testsd
TESTABLE_MODULES = scanner token object instruction error utility expr astprinter parser interpreter stmt enviorment callable

# modules for tui frontend
MODULES_FRONT =

# dependencies and objects for sdl froentend
FRONT_DEPS=$(patsubst %,$(INCLUDEDIR_FRONT)/%.h,$(MODULES_FRONT))
DEPS=$(patsubst %,$(INCLUDEDIR)/%.h,$(TESTABLE_MODULES))
OBJ=$(patsubst %,$(ODIR)/%.o,$(MODULES_FRONT))
OBJ+=$(patsubst %,$(ODIR)/%.o,$(TESTABLE_MODULES))
OBJ+=$(patsubst %,$(ODIR)/%.o,$(MAIN))

# dependencies and objects for tests
TEST_DEPS=$(patsubst %,$(INCLUDEDIR_TEST)/%.h,$(MODULES_TEST))
TESTABLE_DEPS+=$(patsubst %,$(INCLUDEDIR)/%.h,$(TESTABLE_MODULES))
TEST_OBJ=$(patsubst %,$(ODIR_TEST)/%.o,$(TESTABLE_MODULES))
TEST_OBJ+=$(patsubst %,$(ODIR_TEST)/%.o,$(MODULES_TEST))
TEST_OBJ+=$(patsubst %,$(ODIR_TEST)/%.o,$(TEST_MAIN))

# =====================
# main
# =====================

# core source code
$(ODIR)/%.o: $(SRCDIR)/%$(EXT) $(DEPS) | init
	$(CC) $(OTHER_FLAGS) -c -o $@ $< $(CFLAGS)

# tui froentend
$(ODIR)/%.o: $(SRCDIR_FRONT)/%$(EXT) $(FRONT_DEPS) | init
	$(CC) $(OTHER_FLAGS) -c -o $@ $< $(CFLAGS)

${BIN}: $(OBJ)
	$(CC) $(OTHER_FLAGS) -o $(BINDIR)/$@ $^ $(LIBS)

# =====================
# test
# =====================

# core source code
$(ODIR_TEST)/%.o: $(SRCDIR)/%$(EXT) $(TESTABLE_DEPS) | init
	$(CC) -c -o $@ $< $(CFLAGS_TEST)

# test source code
$(ODIR_TEST)/%.o: $(SRCDIR_TEST)/%$(EXT) $(TEST_DEPS) | init
	$(CC) -c -o $@ $< $(CFLAGS_TEST)

build_test: $(TEST_OBJ)
	@echo $(TEST_OBJ)
	$(CC) -o $(BINDIR)/${TEST_MAIN} $^ $(LIBS_TEST) -l cmocka

test: build_test
	$(BINDIR)/$(TEST_MAIN)

leaktest: build_test
	valgrind -s $(BINDIR)/$(TEST_MAIN)

# =====================
# library
# =====================

$(ODIRLIB)/%.o: $(SRCDIR)/%$(EXT) $(DEPS) | init
	$(CC) -c -o $@ $< $(CFLAGS_LIB)

libmlisp.a: $(LIB_OBJ)
	ar rcs $(BINDIR)/$@ $^

# =====================
# other useful things
# =====================

.PHONY: clean
clean:
	@echo Cleaning stuff. This make file officially is doing better than you irl.
	rm -f $(ODIR)/*.o
	rm -f $(BINDIR)/*
	rm -f $(ODIR_TEST)/*.o

.PHONY: setup
init:
	mkdir -p $(ODIR)
	mkdir -p $(BINDIR)
	mkdir -p $(ODIR_TEST)

.PHONY: install
install:
	mkdir -p ${INSTALLDIR}/view
	cp ${BINDIR}/${BIN} ${INSTALLDIR}/${BIN}
