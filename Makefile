.PHONY: all
all: test_assign1

test_assign1: test_assign1_1.c storage_mgr.c dberror.c
	gcc -std=c99 -o test_assign1 test_assign1_1.c storage_mgr.c dberror.c

.PHONY: clean
clean:
	rm test_assign1