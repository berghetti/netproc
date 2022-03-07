
# https://www.gnu.org/software/make/manual/make.html#Pattern-Examples
# http://wiki.inf.ufpr.br/maziero/doku.php?id=prog2:o_sistema_make

# CC				compilador a ser usado
# CPPFLAGS	opções para o preprocessador (-DDEBUG ou -I $(DIR_INCLUDES))
# CFLAGS		opções para o compilador (-std=c99)
# LDFLAGS		opções para o ligador (-static -L/opt/opencv/lib)
# LDLIBS		bibliotecas a ligar ao código gerado (-lpthreads -lmath)

# https://www.gnu.org/software/make/manual/make.html#Automatic-Variables
# $@ - representa o alvo
# $^ - representa a lista de dependencia
# $< - representa a primeira dependencia

# alvo: dependencia
# <tab>regra

PROG_NAME=netproc

prefix ?= /usr/local

PATH_DOC_INSTALL=$(DESTDIR)$(prefix)/share/man/man8
PATH_INSTALL=$(DESTDIR)$(prefix)/sbin

SRCDIR=./src
OBJDIR=./obj
BINDIR=./bin
DOCDIR=./doc

CC ?= gcc
CFLAGS += -Wall -Wextra -pedantic -Wformat=2

# environment var
ifdef DEBUG
	CFLAGS += -O0 -ggdb -fsanitize=address
	LDFLAGS += -fsanitize=address
else ifdef DEV
	CFLAGS += -O0 -ggdb
else
	CPPFLAGS += -D NDEBUG
	CFLAGS += -O2 -flto
	LDFLAGS += -s -O2 -flto
endif

LDLIBS=$(shell ncursesw6-config --libs 2> /dev/null)

# if LDLIBS is empty
ifeq ($(LDLIBS),)
	LDLIBS=$(shell  ncursesw5-config --libs 2> /dev/null)
endif

LDLIBS += -lpthread

#.c files all subdirectories
C_SOURCE= $(wildcard $(SRCDIR)/*.c) $(wildcard $(SRCDIR)/*/*.c)

# .h files all subdirectories
H_SOURCE=$(wildcard $(SRCDIR)/*.h) $(wildcard $(SRCDIR)/*/*.h)

# https://www.gnu.org/software/make/manual/make.html#General-Search
# search path prerequisits
VPATH= src src/resolver

# Object files
# todos arquivos .c trocado a extensão para .o
# transforma 'src/foo.c' em 'obj/foo.o'
OBJECTS=$(addprefix $(OBJDIR)/, $(notdir $(C_SOURCE:.c=.o) ) )

# alvos fake, não são arquivos
.PHONY: all clean distclean run install uninstall format man tarball

all: $(BINDIR)/$(PROG_NAME)

# linka os arquivos ojetos para o executavel
# regra ja é implitica dessa forma,
# adicionado apenas para 'clareza'

$(BINDIR)/$(PROG_NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# mascara generica para compilar arquivos .o
# utiliza todos os arquivos headers como dependencia,
# caso algum seja atualizado, todo os objetos são recompilados

$(OBJDIR)/%.o: %.c $(H_SOURCE)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	@ find . -type f -name '*.o' -delete
	@ echo "Object files removed"

distclean: clean
	@ find $(BINDIR) -name $(PROG_NAME) -delete
	@ echo "Removed "$(BINDIR)"/"$(PROG_NAME)

run:
	sudo $(BINDIR)/$(PROG_NAME)

install:
	@ install -d -m 755 $(PATH_INSTALL)
	@ install --strip $(BINDIR)/$(PROG_NAME) $(PATH_INSTALL); \
		echo "Binary instaled in "$(PATH_INSTALL)"/"$(PROG_NAME)
	@ install -d -m 755 $(PATH_DOC_INSTALL)
	@ install -g 0 -o 0 -m 0644 $(DOCDIR)/$(PROG_NAME).8 $(PATH_DOC_INSTALL); \
		gzip -9 -f $(PATH_DOC_INSTALL)/$(PROG_NAME).8 ; \
		echo "Man page instaled in "$(PATH_DOC_INSTALL)/$(PROG_NAME)".8.gz"

uninstall:
	@ find $(PATH_DOC_INSTALL) $(PATH_INSTALL) -type f -name "$(PROG_NAME)*" \
		-delete -exec echo "Removed "{} \;

format:
	@ echo "Formating code"
	@ clang-format -i $(SRCDIR)/*.[ch]
	@ clang-format -i $(SRCDIR)/*/*.[ch]
	@ clang-format -i tests/*.[ch]

man:
	txt2man -t $(PROG_NAME) -v "$(PROG_NAME) man" -s 8 \
		$(DOCDIR)/$(PROG_NAME).8.txt > $(DOCDIR)/$(PROG_NAME).8

VERSION = $(shell ./get_version.sh)
tarball:
	git archive -o "netproc-$(VERSION).tar.gz" $(VERSION)
