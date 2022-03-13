#include <stdio.h>
#include <time.h>
#include "supergraph.h"
#include <unistd.h>

int main() {
	clock_t begin = clock();
	
	//Your query here
	
	clock_t end = clock();
	double time_elapsed = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Time Elapsed: %f\n", time_elapsed);
	
	return 0;
}
