
# specific unity, only append this variables
C_SOURCE= ./unity/unity.c
INC_DIRS= -Iunity

CFLAGS=-Wall -Wextra -pedantic -ggdb -O0

INC_DIRS += -I../src -I ../src/resolver

C_TMP= $(wildcard ./test*.c)
OBJS= $(C_TMP:.c=.o)

TEST = tests_all

# include here source module to test
C_SOURCE += ../src/hashtable.c \
 						../src/full_read.c \
						../src/resolver/queue.c \
						../src/str.c

.PHONY: all clean run

all: $(TEST) run clean

$(TEST): $(OBJS)
	$(CC) $(LDFLAGS) $(C_SOURCE) $^ $(LDLIBS) -o $@

%.o : %.c
	$(CC) $(CFLAGS) $(INC_DIRS) $(CPPFLAGS) -c $< -o $@

run:
	- ./$(TEST)

clean:
	@ find . -type f -name '*.o' -delete
#	@ find . -executable -type f -delete