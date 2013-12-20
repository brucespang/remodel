src=src
bin=bin
test=test
build=build
include=include
executable=remodel

CC=clang
CFLAGS += -I. -I./ck/usr/local/include -g -Wall -Wextra -Werror -Weverything -Wno-unused-function -Wno-used-but-marked-unused -Wno-padded -Wno-cast-align -Wno-language-extension-token -Wno-shorten-64-to-32
LDFLAGS += -lck -lssl -lcrypto -lpthread

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
	$(CC) $(CFLAGS) -L./ck/usr/local/lib $< $(filter-out $(build)/main.o, $(objects)) -o $@ $(LDFLAGS)
	LD_LIBRARY_PATH=./ck/usr/local/lib/ $@

.PHONY: clean-coverage
clean-coverage:
	rm -f $(build)/*.gcda $(build)/*.gcno

ck/:
	git clone https://github.com/sbahra/ck.git

./ck/usr/local/lib/libck.so: ck/
	bash -c 'cd ck && ./configure && make && DESTDIR=. make install'

$(bin)/$(executable): ./ck/usr/local/lib/libck.so $(objects)
	mkdir -p $(bin)/
	$(CC) $(CFLAGS) -L./ck/usr/local/lib $^ -o $(bin)/$(executable) $(LDFLAGS)

.PHONY: compile
compile: clean-coverage $(build)/ $(bin)/$(executable)

.PHONY: run
run: compile
	LD_LIBRARY_PATH=./ck/usr/local/lib/ $(bin)/$(executable) remodel.remodel

.PHONY: check
check:
	./bin/clint.sh
	cppcheck --enable=all --error-exitcode=1 -I src/parse.c -I src/scan.c .

.PHONY: test
test: compile $(test_binaries)

.PHONY: report
report: report.tex report.bib
	bibtex report
	pdflatex report
