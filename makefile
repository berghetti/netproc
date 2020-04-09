
# https://www.gnu.org/software/make/manual/make.html#Pattern-Examples
# http://wiki.inf.ufpr.br/maziero/doku.php?id=prog2:o_sistema_make

# CC				compilador a ser usado
# CPPFLAGS	opções para o preprocessador (-DDEBUG ou -I $(DIR_INCLUDES))
# CFLAGS		opções para o compilador (-std=c99)
# LDFLAGS		opções para o ligador (-static -L/opt/opencv/lib)
# LDLIBS		bibliotecas a ligar ao código gerado (-lpthreads -lm)


# $@ - representa o alvo
# $^ - representa a lista de dependencia
# $< - representa a primeira dependencia

# alvo: dependencia
# <tab>regra

PROGNAME=netproc

CC=gcc
CFLAGS=-Wall -Wextra -pedantic -pedantic-errors -O2 -g


#.c files
C_SOURCE=$(wildcard *.c)
#
# .h files
#H_SOURCE=$(wildcard *.h)
#
# Object files
# todos arquivos .c trocado a extensão para .o
OBJECTS=$(C_SOURCE:.c=.o)

# alvos fake, não são arquivos
.PHONY: all clean run

all: $(PROGNAME)

# regra ja é implitica dessa forma,
# adicionado apenas para 'clareza'
$(PROGNAME): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# caso especial main que não possui .h
main.o: main.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# mascara generica para compilar arquivos .o
%.o: %.c %.h
	 $(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@


clean:
	@ echo "apagando arquivos .o"
	@ -find . -type f -name '*.o' -delete

run:
	sudo ./$(PROGNAME)
