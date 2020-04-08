
# http://wiki.inf.ufpr.br/maziero/doku.php?id=prog2:o_sistema_make
# CC				compilador a ser usado
# CPPFLAGS	opções para o preprocessador
# CFLAGS		opções para o compilador
# LDFLAGS		opções para o ligador
# LDLIBS		bibliotecas a ligar ao código gerado


# $@ - representa o alvo
# $^ - representa a lista de dependencia
# $< - representa a primeira dependencia

# alvo: dependencia
# <tab>regra

PROGNAME=process

CFLAGS=-Wall -Wextra -pedantic -pedantic-errors -O2 -g

CC=gcc

#.c files
C_SOURCE=$(wildcard *.c)
#
# .h files
#H_SOURCE=$(wildcard *.h)
#
# Object files
# todos arquivos .c trocado a extensão para .o
OBJECTS=$(C_SOURCE:.c=.o)

# alvos fake, que não são arquivos
.PHONY: all clean run

all: $(PROGNAME)

# regra ja é implitica dessa forma
# adicionado apenas para 'clareza'
$(PROGNAME): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@


# # mascara generica para compilar arquivos .o
# regra implicita, make sabe a regra para executar
# https://www.gnu.org/software/make/manual/html_node/make-Deduces.html#make-Deduces
%.o: %.c %.h
#	 $(CC) $(CFLAGS) -c $< -o $@


clean:
	@ echo "apagando arquivos .o"
	@ -find . -type f -name '*.o' -delete

run:
	sudo ./$(PROGNAME)
