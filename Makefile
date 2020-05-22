
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

PATH_INSTALL=/usr/local/sbin
SRC=./src
BIN=./bin

CC=gcc

# enable macro DEBUG for pre-processor
# CPPFLAGS=-DDEBUG2

CFLAGS=
ifdef DEBUG
	CFLAGS+=-Wall -Wextra -Werror -pedantic -pedantic-errors -O0 -g
else
	CFLAGS+= -O2 -march=native -Wall -Wextra
endif

# biblioteca terminfo
LDLIBS=-ltinfo

#.c files
C_SOURCE=$(wildcard $(SRC)/*.c)
#
# .h files
#H_SOURCE=$(wildcard *.h)
#
# Object files
# todos arquivos .c trocado a extensão para .o
OBJECTS=$(C_SOURCE:.c=.o)

# alvos fake, não são arquivos
.PHONY: all clean distclean run install uninstall format

all: $(BIN)/$(PROG_NAME)

# linka os arquivos ojetos para o executavel
# regra ja é implitica dessa forma,
# adicionado apenas para 'clareza'
$(BIN)/$(PROG_NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# caso especial do main.c que não possui .h
$(SRC)/main.o: $(SRC)/main.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# mascara generica para compilar arquivos .o
# quando usa regra implicita não compila quando o .h é alterado
# não sei porque...
%.o: %.c %.h
	 $(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	@ find . -type f -name '*.o' -delete
	@ echo "Object files removed"

distclean: clean
	@ find $(BIN) -name $(PROG_NAME) -delete
	@ echo "Removed "$(BIN)"/"$(PROG_NAME)

run:
	sudo $(BIN)/$(PROG_NAME)

# Shortcut to check if user has root privileges.
# from https://gitlab.com/fredericopissarra/t50/-/blob/master/Makefile
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
	@cp $(BIN)/$(PROG_NAME) $(PATH_INSTALL); \
	echo "Instaled in "$(PATH_INSTALL)"/"$(PROG_NAME)

uninstall:
	@$(call checkifroot)
	@ find $(PATH_INSTALL) -type f -name $(PROG_NAME) -delete; \
	echo "Removed "$(PATH_INSTALL)"/"$(PROG_NAME)


format:
	@ echo "Formating code"
	@ clang-format -i $(SRC)/*.[ch]
