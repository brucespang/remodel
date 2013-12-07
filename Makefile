src=src
bin=bin
test=test
build=build
include=include
executable=remodel

CC=clang
CFLAGS += -I. -g -Wall -Wextra -Werror -Wno-unused-function --coverage
LDFLAGS += -lck -lcrypto -lssl -lpthread

LEX    = flex
YACC   = bison -y
YFLAGS = -d

sources  := $(wildcard $(src)/*.c)
includes := $(wildcard $(include)/*.h)
tests    := $(wildcard $(test)/*.c)
objects  := $(filter-out $(build)/scan.o, $(filter-out $(build)/parse.o, $(sources:$(src)/%.c=$(build)/%.o)))
test_objects := $(tests:$(test)/%.c=$(build)/%.o)
test_binaries := $(tests:$(test)/%.c=$(build)/%)

.PHONY: all
all: compile run

.PHONY: clean
clean:
	rm -rf $(build)/
	rm -f $(bin)/$(executable)

$(build)/ $(bin)/:
	mkdir -p $@

$(src)/scan.c: src/scan.l src/parse.c
	$(LEX) $(LFLAGS) -o $@ $^

$(objects): $(build)/%.o : $(src)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(test_objects): $(build)/%.o : $(test)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(test_binaries)
.PHONY: $(test_binaries)
$(test_binaries): $(build)/% : $(test)/%.c
	rm -f $@.gcda $@.gcno
	$(CC) $(CFLAGS) $< $(filter-out $(build)/main.o, $(objects)) -o $@ $(LDFLAGS)
	$@

.PHONY: clean-coverage
clean-coverage:
	rm -f $(build)/*.gcda $(build)/*.gcno

$(bin)/$(executable): $(objects)
	mkdir -p $(bin)/
	$(CC) $(CFLAGS) $^ -o $(bin)/$(executable) $(LDFLAGS)

.PHONY: compile
compile: clean-coverage $(build)/ $(bin)/$(executable)

.PHONY: run
run: compile
	$(bin)/$(executable) remodel.remodel

.PHONY: check
check:
	./bin/clint.sh
	cppcheck --enable=all --error-exitcode=1 .

.PHONY: test
test: compile $(test_binaries)

.PHONY: coverage
coverage: test
	lcov -b . --no-checksum --capture --directory `pwd`/build --output-file coverage.info
	genhtml --ignore-errors=source --branch-coverage --highlight --legend --output-directory cov coverage.info
	open cov/index.html

.PHONY: cov
cov: coverage
