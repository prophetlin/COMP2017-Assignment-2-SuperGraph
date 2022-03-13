#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "supergraph.c"
#include "supergraph.h"
#include "supergraph_loader.h"

#define MAX_BUFFER 65535
struct timespec start, finish;
double time_elapsed;
double time_elapsed1;
double time_elapsed2;
double time_elapsed3;

// Reads given graph file and returns a book graph.
post* post_loader(size_t* count, const char* filename) {

	char buffer[MAX_BUFFER];
	size_t n_posts = 0;

	// Open graph file
	FILE* f = fopen(filename, "r");
	if (f == NULL) {
		perror("Fatal error! Unable to open graph file");
		return NULL;
	}

	// Read book count
	if (fgets(buffer, MAX_BUFFER, f) == NULL || sscanf(buffer, "%zu", &n_posts) == 0) {
		fprintf(stderr, "Fatal error! Unable to read posts.\n");
		return NULL;
	}

	post* graph = malloc(sizeof(post) * n_posts);

	// Read books
	for (size_t i = 0; i < n_posts; i++) {

		// Read user id
		if (fgets(buffer, MAX_BUFFER, f) == NULL || sscanf(buffer, "%zu", &graph[i].pst_id) == 0) {
			fprintf(stderr, "Fatal error! Unable to read post id %zu.\n", i);
			return NULL;
		}

		// Read n_following
		if (fgets(buffer, MAX_BUFFER, f) == NULL || sscanf(buffer, "%zu", &graph[i].n_reposted) == 0) {
			fprintf(stderr, "Fatal error! Unable to read reposts%zu.\n", i);
			return NULL;
		}

		size_t cap = 10;
		size_t size = 0;

		buffer[MAX_BUFFER - 1] = '\0';
		size_t length = MAX_BUFFER - 1;
		graph[i].reposted_idxs = malloc(sizeof(size_t) * cap);

		// Read following edges
		while (length == MAX_BUFFER - 1 && buffer[length] != '\n') {
			if (fgets(buffer, MAX_BUFFER, f) == NULL) {
				fprintf(stderr, "Fatal error! Unable to read following  %zu.\n", i);
			}

			length = strlen(buffer);
			for (char* s = strtok(buffer, " "); s != NULL; s = strtok(NULL, " ")) {
				if (size == cap) {
					cap = cap * 2;
					graph[i].reposted_idxs = realloc(graph[i].reposted_idxs, sizeof(size_t) * cap);
				}
				if (strcmp("\n", s) != 0) {
					size_t k = strtol(s, NULL, 10);
					graph[i].reposted_idxs[size] = k;
					size++;
				}
			}
		}


	}
	*count = n_posts;

	return graph;
}


// Reads given graph file and returns a book graph.
user* user_loader(size_t* count, const char* filename) {

	char buffer[MAX_BUFFER];
	size_t n_users = 0;

	// Open graph file
	FILE* f = fopen(filename, "r");
	if (f == NULL) {
		perror("Fatal error! Unable to open graph file");
		return NULL;
	}

	// Read book count
	if (fgets(buffer, MAX_BUFFER, f) == NULL || sscanf(buffer, "%zu", &n_users) == 0) {
		fprintf(stderr, "Fatal error! Unable to read users.\n");
		return NULL;
	}

	user* graph = malloc(sizeof(user) * n_users);

	// Read books
	for (size_t i = 0; i < n_users; i++) {

		// Read user id
		if (fgets(buffer, MAX_BUFFER, f) == NULL || sscanf(buffer, "%zu", &graph[i].user_id) == 0) {
			fprintf(stderr, "Fatal error! Unable to read usr id for user %zu.\n", i);
			return NULL;
		}

		// Read n_following
		if (fgets(buffer, MAX_BUFFER, f) == NULL || sscanf(buffer, "%zu", &graph[i].n_followers) == 0) {
			fprintf(stderr, "Fatal error! Unable to read following for user %zu.\n", i);
			return NULL;
		}

		// Read n_followers
		if (fgets(buffer, MAX_BUFFER, f) == NULL || sscanf(buffer, "%zu", &graph[i].n_following) == 0) {
			fprintf(stderr, "Fatal error! Unable to read followers for user %zu.\n", i);
			return NULL;
		}

		// Read n_posts
		if (fgets(buffer, MAX_BUFFER, f) == NULL || sscanf(buffer, "%zu", &graph[i].n_posts) == 0) {
			fprintf(stderr, "Fatal error! Unable to read followers for user %zu.\n", i);
			return NULL;
		}

		size_t cap = 10;
		size_t size = 0;

		buffer[MAX_BUFFER - 1] = '\0';
		size_t length = MAX_BUFFER - 1;
		graph[i].follower_idxs = malloc(sizeof(size_t) * cap);

		// Read following edges
		while (length == MAX_BUFFER - 1 && buffer[length] != '\n') {
			if (fgets(buffer, MAX_BUFFER, f) == NULL) {
				fprintf(stderr, "Fatal error! Unable to read following  %zu.\n", i);
			}

			length = strlen(buffer);
			for (char* s = strtok(buffer, " "); s != NULL; s = strtok(NULL, " ")) {
				if (size == cap) {
					cap = cap * 2;
					graph[i].follower_idxs = realloc(graph[i].follower_idxs, sizeof(size_t) * cap);
				}
				if (strcmp("\n", s) != 0) {
					size_t k = strtol(s, NULL, 10);
					graph[i].follower_idxs[size] = k;
					size++;
				}
			}
		}

		cap = 10;
		size = 0;

		buffer[MAX_BUFFER - 1] = '\0';
		length = MAX_BUFFER - 1;
		graph[i].following_idxs = malloc(sizeof(size_t) * cap);

		// Read followers
		while (length == MAX_BUFFER - 1 && buffer[length] != '\n') {
			if (fgets(buffer, MAX_BUFFER, f) == NULL) {
				fprintf(stderr, "Fatal error! Unable to followers for user %zu.\n", i);
			}

			length = strlen(buffer);
			for (char* s = strtok(buffer, " "); s != NULL; s = strtok(NULL, " ")) {
				if (size == cap) {
					cap = cap * 2;
					graph[i].following_idxs = realloc(graph[i].following_idxs, sizeof(size_t) * cap);
				}
				if (strcmp("\n", s) != 0) {
					size_t k = strtol(s, NULL, 10);
					graph[i].following_idxs[size] = k;
					size++;
				}
			}
		}

		cap = 10;
		size = 0;

		buffer[MAX_BUFFER - 1] = '\0';
		length = MAX_BUFFER - 1;
		graph[i].post_idxs = malloc(sizeof(size_t) * cap);

		// Read posts
		while (length == MAX_BUFFER - 1 && buffer[length] != '\n') {
			if (fgets(buffer, MAX_BUFFER, f) == NULL) {
				fprintf(stderr, "Fatal error! Unable to read posts for user %zu.\n", i);
			}

			length = strlen(buffer);
			for (char* s = strtok(buffer, " "); s != NULL; s = strtok(NULL, " ")) {
				if (size == cap) {
					cap *= 2;
					graph[i].post_idxs = realloc(graph[i].post_idxs, sizeof(size_t) * cap);
				}
				if (strcmp("\n", s) != 0) {
					size_t k = strtol(s, NULL, 10);
					graph[i].post_idxs[size] = k;
					size++;
				}
			}
		}
	}



	*count = n_users;

	return graph;
}

int main(){
	size_t post_count = 0;
	size_t user_count = 0;
	struct query_helper* query_helper = engine_setup(1);
	struct post* posts = post_loader(&post_count, "social.posts");
	struct user* users = user_loader(&user_count, "social.users");
	struct criteria* crit = malloc(sizeof(struct criteria));
	crit->oc_threshold = 0.5;
	crit->acc_rep_threshold = 0.5;
	crit->bot_net_threshold = 0.5;
	fflush(stdout);
	printf("Number of users: %zu\n",user_count);
	printf("Number of posts: %zu\n",post_count);
	printf("Starting...\n");
	printf("---------------------------------------------------------\n");
	printf("Testing find_all_reposts of user id 1495\n");
	clock_gettime(CLOCK_MONOTONIC, &start);
	struct result* result1 = find_all_reposts(posts, post_count, 1495, query_helper);
	clock_gettime(CLOCK_MONOTONIC, &finish);
	time_elapsed1 = (finish.tv_sec - start.tv_sec);
	time_elapsed1 += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	printf("Time Elapsed: %f\n", time_elapsed1);
	printf("Number of repost found: %zu\n", result1->n_elements);
	printf("---------------------------------------------------------\n");
	printf("Testing find_original of user id 182915\n");
	clock_gettime(CLOCK_MONOTONIC, &start);
	struct result* result2 = find_original(posts, post_count, 182915, query_helper);
	clock_gettime(CLOCK_MONOTONIC, &finish);
	time_elapsed2 = (finish.tv_sec - start.tv_sec);
	time_elapsed2 += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	printf("Time Elapsed: %f\n", time_elapsed2);
	printf("Number of original found: %zu\n", result2->n_elements);
	printf("---------------------------------------------------------\n");
	printf("Testing shortest_user_link of user id 0 to 99999\n");
	clock_gettime(CLOCK_MONOTONIC, &start);
	struct result* result3 = shortest_user_link(users, user_count, 0, 99999, query_helper);
	clock_gettime(CLOCK_MONOTONIC, &finish);
	time_elapsed3 = (finish.tv_sec - start.tv_sec);
	time_elapsed3 += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	printf("Time Elapsed: %f\n", time_elapsed3);
	printf("Number of elements in path: %zu\n", result3->n_elements);
	printf("---------------------------------------------------------\n");
	printf("Testing find_all_bots\n");
	clock_gettime(CLOCK_MONOTONIC, &start);
	struct result* result = find_bots(users, user_count, posts, post_count, crit, query_helper);
	clock_gettime(CLOCK_MONOTONIC, &finish);
	time_elapsed = (finish.tv_sec - start.tv_sec);
	time_elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	printf("Time Elapsed: %f\n", time_elapsed);
	printf("Number of bots found: %zu\n", result->n_elements);
	printf("Number of users: %zu\n",user_count);

	engine_cleanup(query_helper);
	free(crit);
}
