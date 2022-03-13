#ifndef TEST_H
#define TEST_H
#include <stdio.h>
#include <string.h>

int passed;
int num_tests;

void test_init();
void test_end();
int test(int (*func)(), char* name);

#endif
