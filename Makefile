
# https://www.gnu.org/software/make/manual/make.html#Pattern-Examples
# http://wiki.inf.ufpr.br/maziero/doku.php?id=prog2:o_sistema_make

# CC				compilador a ser usado
# CPPFLAGS	opções para o preprocessador (-DDEBUG ou -I $(DIR_INCLUDES))
# CFLAGS		opções para o compilador (-std=c99)
# LDFLAGS		opções para o ligador (-static -L/opt/opencv/lib)
# LDLIBS		bibliotecas a ligar ao código gerado (-lpthreads -lmath)


# $@ - representa o alvo
# $^ - representa a lista de dependencia
# $< - representa a primeira dependencia

# alvo: dependencia
# <tab>regra

PROG_NAME=netproc

PATH_DOC_INSTALL=$(DESTDIR)/usr/local/share/man/man8
PATH_INSTALL=$(DESTDIR)/usr/local/sbin

SRC=./src
BIN=./bin
DOC=./doc

CC=gcc
# CPPFLAGS=
# CFLAGS=

# environment var
ifdef DEBUG
	CFLAGS+=-Wall -Wextra -pedantic -pedantic-errors -O0 -g
else
	CPPFLAGS=-D NDEBUG
	CFLAGS+= -O2 -march=native -Wall -Wextra
endif

LDLIBS=-lncursesw -lpthread

#.c files
C_SOURCE=$(wildcard $(SRC)/*.c)
C_SOURCE+=$(wildcard $(SRC)/resolver/*.c)
#
# .h files
H_SOURCE=$(wildcard $(SRC)/*.h)
H_SOURCE+=$(wildcard $(SRC)/resolver/*.h)
#
# Object files
# todos arquivos .c trocado a extensão para .o
OBJECTS=$(C_SOURCE:.c=.o)

# alvos fake, não são arquivos
.PHONY: all clean distclean run install uninstall format man

all: $(BIN)/$(PROG_NAME)

# linka os arquivos ojetos para o executavel
# regra ja é implitica dessa forma,
# adicionado apenas para 'clareza'
$(BIN)/$(PROG_NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# mascara generica para compilar arquivos .o
# utiliza todos os arquivos headers como dependencia,
# caso algum seja atualizado, todo os objetos são recompilados
%.o: %.c $(H_SOURCE)
	 $(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	@ find . -type f -name '*.o' -delete
	@ echo "Object files removed"

distclean: clean
	@ find $(BIN) -name $(PROG_NAME) -delete
	@ echo "Removed "$(BIN)"/"$(PROG_NAME)

run:
	sudo $(BIN)/$(PROG_NAME)

# check if user has root privileges.
define checkifroot
  if [ `id -u` -ne 0 ]; then \
    echo 'Need root privileges!'; \
    exit 1; \
  fi
endef

define checkbin
	if [ ! -e $(1) ]; then \
		echo $(1)" not exist, try \"make\"."; \
		exit 1; \
	fi
endef


install:
	@$(call checkifroot)
	@$(call checkbin, $(BIN)/$(PROG_NAME))
	@ install -d -m 755 $(PATH_INSTALL)
	@ install --strip $(BIN)/$(PROG_NAME) $(PATH_INSTALL); \
	echo "Binary instaled in "$(PATH_INSTALL)"/"$(PROG_NAME)
	@install -d -m 755 $(PATH_DOC_INSTALL)
	@install -g 0 -o 0 -m 0644 $(DOC)/$(PROG_NAME).8 $(PATH_DOC_INSTALL); \
	gzip -9 $(PATH_DOC_INSTALL)/$(PROG_NAME).8 ; \
	echo "Man page instaled in "$(PATH_DOC_INSTALL)/$(PROG_NAME)".8.gz"

uninstall:
	@$(call checkifroot)
	@ find $(PATH_DOC_INSTALL) $(PATH_INSTALL) -type f -name "$(PROG_NAME)*" \
	-delete -exec echo "Removed "{} \;

format:
	@ echo "Formating code"
	@ clang-format -i $(SRC)/*.[ch]
	@ clang-format -i $(SRC)/resolver/*.[ch]

man:
	txt2man -t netproc -v "netproc man" -s 8 $(DOC)/netproc.8.txt > $(DOC)/netproc.8
