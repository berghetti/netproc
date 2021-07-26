
# specific unity, only append this variables
C_SOURCE= ./unity/unity.c
INC_DIRS= -Iunity

TEST1= test_hashtable

INC_DIRS += -I../src
C_SOURCE += ../src/hashtable.c

.PHONY: all clean

all: $(TEST1) clean

test_hashtable: $(TEST1).o
	$(CC) $(LDFLAGS) $(C_SOURCE) $^ $(LDLIBS) -o $@
	- ./$(TEST1)

%.o : %.c
	$(CC) $(CFLAGS) $(INC_DIRS) $(CPPFLAGS) -c $< -o $@

clean:
	@ rm -r *.o
