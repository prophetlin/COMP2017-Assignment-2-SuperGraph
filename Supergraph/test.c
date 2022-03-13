#include "test.h"

int len;

// Returns the max length of a bunch of strings
int max_len(char** strings, int len) {
    int max = 0;
    for (int i = 0; i < len; i++) {
        int len = strlen(strings[i]);
        if (len > max)
            max = len;
    }
    return max;
}

// Initialise the library
void test_init(char** test_names, size_t num_tests) {
    printf(":: running %zu tests\n", num_tests);
    len = max_len(test_names, num_tests);
}

// Test a function (*func)() with the name `name`
int test(int (*func)(), char* name) {
    printf(" - %s ", name);
    for (int i = 0; i < len - strlen(name); i++)
        printf(".");
    printf("... ");
    int res = func();
    // printf("  -> ");
    switch (res) {
        case 0:
            printf("✓\n");
            passed++;
            num_tests++;
            break;
        case 1:
            printf("✗\n");
            num_tests++;
            break;
        case 2:
            printf("○\n");
            break;
    }
    return res;
}

// Print summary
void test_end() {
    printf(":: %d/%d passed \n", passed, num_tests);
}
