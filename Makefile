CC=gcc
CFLAGS=-c -Wall -Wextra -Werror -std=c11
LDFLAGS=-lcheck -lm
LIBSRC=$(filter-out main.c, $(wildcard *.c))
LIBOBJ=$(LIBSRC:.c=.o)
EXECUTABLE=result_file
LCOVEXEC=$(EXECUTABLE).info
REPORTDIR=report
STATICLIB=s21_decimal.a

.PHONY: all build test gcov_report clean rebuild

all: build

build: clean main.o $(STATICLIB)
	$(CC) main.o $(STATICLIB) -o $(EXECUTABLE) $(LDFLAGS)

test: build
	./$(EXECUTABLE)

gcov_report: CFLAGS+=--coverage
gcov_report: LDFLAGS+=--coverage
gcov_report: test
	lcov -t $(EXECUTABLE) -o $(LCOVEXEC) -c -d .
	genhtml -o $(REPORTDIR) $(LCOVEXEC)

%.o: %.c
	$(CC) $(CFLAGS) $^ -o $@

style:
	cppcheck --enable=all --suppress=missingIncludeSystem ./
	python3 ../materials/linters/cpplint.py --extensions=c,h --recursive ./

clean:
	rm -rf *.o $(EXECUTABLE) $(STATICLIB) *.gcno *.gcda *.gcov $(LCOVEXEC) $(REPORTDIR)

rebuild: clean all

$(STATICLIB): $(LIBOBJ)
	ar rcs $@ $^
	ranlib $@
