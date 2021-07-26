
# specific unity, only append this variables
C_SOURCE= ./unity/unity.c
INC_DIRS= -Iunity

CFLAGS=-Wall -Wextra -pedantic -ggdb -O0

INC_DIRS += -I../src

C_SOURCE += ../src/hashtable.c \
 						../src/full_read.c

TEST = exec_tests

.PHONY: all clean run

all: $(TEST) run clean

$(TEST): $(TEST).o
	$(CC) $(LDFLAGS) $(C_SOURCE) $^ $(LDLIBS) -o $@

%.o : %.c
	$(CC) $(CFLAGS) $(INC_DIRS) $(CPPFLAGS) -c $< -o $@

run:
	- ./$(TEST)

clean:
	@ find . -type f -name '*.o' -delete
	@ find . -executable -type f -delete
