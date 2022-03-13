#include <stdio.h>
#include "supergraph.h"
#include "supergraph.c"
#include "supergraph_helper.h"

/*
Create a simple graph with 100 posts and 10 users
*/
users_posts* create_simple_graph(int num_users, int num_posts) {

	post* posts = (post*)malloc(sizeof(post)*num_posts);
	user* users = (user*)malloc(sizeof(user)*num_users);

	// Create an array of user ids
	{
		int user_ids[num_users];
		for (int i = 0; i < num_users; i++) {
			user_ids[i] = i+1;
		}

		// Randomise it
		for (int i = 0; i < num_users; i++) {
			int r = rand()%num_users;
			int tmp = user_ids[i];
			user_ids[i] = user_ids[r];
			user_ids[r] = tmp;
		}

		// Put them in the array of users
		for (int i = 0; i < num_users; i++) {
			users[i].user_id = user_ids[i];
		}

		// Since we are hard coding, we can define the numbers of following here
		users[0].n_followers = 1;
		users[0].n_following = 1;

		users[1].n_followers = 1;
		users[1].n_following = 1;

		users[2].n_followers = 2;
		users[2].n_following = 1;

		users[3].n_followers = 2;
		users[3].n_following = 3;

		users[4].n_followers = 1;
		users[4].n_following = 1;

		users[5].n_followers = 1;
		users[5].n_following = 0;

		users[6].n_followers = 1;
		users[6].n_following = 3;

		users[7].n_followers = 0;
		users[7].n_following = 0;

		users[8].n_followers = 2;
		users[8].n_following = 2;

		users[9].n_followers = 1;
		users[9].n_following = 0;

		// Allocate memory in each user
		for (int i = 0; i < num_users; i++) {
			(users+i)->follower_idxs = (size_t*)calloc(users[i].n_followers, sizeof(size_t));
			(users+i)->following_idxs = (size_t*)calloc(users[i].n_following, sizeof(size_t));

			(users+i)->free_follower = 0;
			(users+i)->free_following = 0;
		}

		// User 3 follows users, 0, 5 and 8
		add_follower(users, 0, 3);
		add_follower(users, 5, 3);
		add_follower(users, 8, 3);

		// User 6 follows users 2 and 8
		add_follower(users, 2, 6);
		add_follower(users, 8, 6);

		// User 8 follows user 9 and user 3
		add_follower(users, 3, 8);
		add_follower(users, 9, 8);

		// User 4 follows 6 and 6 follows 4
		add_follower(users, 6, 4);
		add_follower(users, 4, 6);

		// Create the chain of followers 0 -> 1 -> 2 -> 3
		add_follower(users, 1, 0);
		add_follower(users, 2, 1);
		add_follower(users, 3, 2);
	}

	// Create our posts
	{
		int post_ids[num_posts];
		for (int i = 0; i < num_posts; i++) {
			post_ids[i] = i+1;
		}

		// Randomise it
		for (int i = 0; i < num_posts; i++) {
			int r = rand()%num_posts;
			int tmp = post_ids[i];
			post_ids[i]= post_ids[r];
			post_ids[r] = tmp;
		}

		// Put them in the array of posts
		for (int i = 0; i < num_posts; i++) {
			posts[i] = make_post(post_ids[i]);
			posts[i].reposted_idxs = (size_t*)malloc(sizeof(size_t) * num_posts);
		}

		// Make every 10th post a repost of post 0, except for post 1
		posts[0].n_reposted = 10;
		int count = 0;
		for (int i = 1; i < num_posts; i+=10) {
			if (i == 1) continue;
			posts[0].reposted_idxs[count] = i;
			count++;
		}
		posts[0].repost_avail = count;
	}

	// Associate 10 posts with each user
	{
		for (int i = 0; i < num_users; i++) {
			users[i].n_posts = num_posts/num_users;
			users[i].post_idxs = malloc(sizeof(size_t)*users[i].n_posts);
			for (int j = 0; j < users[i].n_posts; j++) {
				users[i].post_idxs[j] = i+j;
			}
		}
	}

	// Set up reposts
	{
		// Post 23 is a repost of 20
		make_repost(23, 20, posts);

		// Make 32 and 45 reposts of 31
		make_repost(32, 31, posts);
		make_repost(45, 31, posts);

		// Make 47 a repost of 45
		make_repost(47, 45, posts);

		//Make 55 a repost of 47
		make_repost(55, 47, posts);

		// Make 25 a repost of 64
		make_repost(25, 64, posts);

		// Make 79 a repost of 62
		make_repost(79, 62, posts);

		// Make posts 92, 93, ..., 99 reposts of 46
		for (int i = 92; i < 100; i++) {
			make_repost(i, 46, posts);
		}
	}

	users_posts *u_p = malloc(sizeof(users_posts)*1);
	u_p->posts = posts;
	u_p->users = users;

	return u_p;
}

void add_follower(user* users, int i1, int i2) {

	user* u1 = users + i1;
	user* u2 = users + i2;

	u1->follower_idxs[u1->free_follower] = i2;
	u2->following_idxs[u2->free_following] = i1;

	u1->free_follower += 1;
	u2->free_following += 1;
}

post make_post(uint64_t post_id) {
	post (p) = {
		.pst_id = post_id,
		.n_reposted = 0,
		.reposted_idxs = NULL,
	};
	return p;
}

void make_repost(int pst_index, int original_index, post* posts) {
	post* ori_post = posts + original_index;
	ori_post->reposted_idxs[ori_post->repost_avail++] = pst_index;
}

int main() {
	printf("Running Tests\n");

	query_helper* helper = engine_setup(2);


	// Create an initial graph
	users_posts* up = create_simple_graph(10, 100);

	for(int i = 0 ; i < 100; i ++){
		printf("indx %d post id: %lu n_repost %zu\n",i, up->posts[i].pst_id, up->posts[i].n_reposted);
	}

	result* check = find_all_reposts(up->posts, 100, up->posts[23].pst_id, helper);
	printf("expected id %lu\n", up->posts[23].pst_id);

	printf("CHECKING FIND_ALL_REPOST:\nNO. ELEMENTS: %zu\n", check->n_elements);
	if(check->n_elements == 0){
		printf("NULL element\n");
	}else{
		for(int i = 0; i < check->n_elements; i++){
			printf("Repost id found: %lu\n", ((post*)(check->elements[i]))->pst_id);
		}
	}
	free(check->elements);
	free(check);

	// Now we run our tests
	test_find_all_reposts_trivial(up, helper);	// When no reposts exist
	test_find_all_reposts_simple(up, helper);		// Return an array of one value
	test_find_all_reposts_long(up, helper);			// Return an array of many values
	test_find_all_reposts_complex(up, helper);	// Return an array of many values of a complex repost
	test_find_all_reposts_invalid(up, helper);	// Find an invalid result

	test_find_original_trivial(up, helper);			// Find original post if given the original
	test_find_original_simple(up, helper);			// Find the original post if only one level deep
	test_find_original_complex(up, helper);			// Find the original post if multiple levels deep
	test_find_original_invalid(up, helper);			// Find an inavlid result

	test_find_shortest_link_trivial(up, helper);// Find the shortest link between two adjacent
	test_find_shortest_link_simple(up, helper); // Find a short link between two
	test_find_shortest_link_long(up, helper);		// Find a long link between two
	test_find_shortest_link_invalid(up, helper);// Find a link that does not exist

	test_find_bots_trivial(up, helper);					// Find bots when bots do not exist
	test_find_bots_simple_reposts(up, helper);	// Find bots when only one exists based on repuation
	test_find_bots_simple_acc_rep(up, helper);	// Find bots when one exists on account repuation
	test_find_bots_long(up, helper);						// Find bots when multiple exist
	test_find_bots_complex(up, helper);					// Find bots and discrete bots
	test_find_bots_invalid(up, helper);					// Find bots with invalid inputs

	// Free the original graph
	for (int i = 0; i < 10; i++) {
		free((up->users + i)->follower_idxs);
		free((up->users + i)->following_idxs);
	}
	for (int i = 0; i < 100; i++) {
		free((up->posts + i)->reposted_idxs);
	}
	free(up->posts);
	free(up->users);
	free(up);

	return 0;
}

void test_find_all_reposts_trivial(users_posts* up, query_helper* helper) {
	char* testname = "test_find_all_reposts_trivial";
	bool passed = false;
	int pst_ind = 1;
	post* posts = up->posts;
	result* res = find_all_reposts(posts, 100, posts[pst_ind].pst_id, helper);
	if (res->n_elements == 1 && (post*)res->elements[0] == posts + pst_ind) {
		passed = true;
	}
	print_test_status(testname, passed);
}

void test_find_all_reposts_simple(users_posts* up, query_helper* helper) {
	char* testname = "test_find_all_reposts_simple";
	post* posts = up->posts;
	int pst_ind = 20;
	result* res = find_all_reposts(posts, 100, posts[pst_ind].pst_id, helper);

	post** expected = (post**)malloc(sizeof(post*)*1);
	expected[0] = &posts[23];
	int expected_nelements = 1;

	post** actual = (post**)res->elements;
	int actual_nelements = res->n_elements;
	printf("actual %d\n", actual_nelements);
	printf("actual id %lu\n", actual[0]->pst_id);
	printf("expected id %lu\n", posts[23].pst_id);

	bool passed = compare_post_arrays(expected, expected_nelements, actual, actual_nelements);
	print_test_status(testname, passed);
	if (!passed) print_expected_actual((void**)expected, (void**)actual, expected_nelements, actual_nelements, "post");
}
void test_find_all_reposts_long(users_posts* up, query_helper* helper) {
	char* testname = "test_find_all_reposts_long";
	post* posts = up->posts;
	int pst_ind = 46;
	result* res = find_all_reposts(posts, 100, posts[pst_ind].pst_id, helper);

	post** expected = (post**)malloc(sizeof(post*)*8);
	for (int i = 92; i <= 99; i++) {
		expected[i-92] = &posts[i];
	}
	int expected_nelements = 8;

	post** actual = (post**)res->elements;
	int actual_nelements = res->n_elements;
	printf("actual %d\n", actual_nelements);
	bool passed = compare_post_arrays(expected, expected_nelements, actual, actual_nelements);
	print_test_status(testname, passed);
	if (!passed) print_expected_actual((void**)expected, (void**)actual, expected_nelements, actual_nelements, "post");
}
void test_find_all_reposts_complex(users_posts* up, query_helper* helper) {
	char* testname = "test_find_all_reposts_complex";
	post* posts = up->posts;
	int pst_ind = 31;
	result* res = find_all_reposts(posts, 100, posts[pst_ind].pst_id, helper);

	int expected_nelements = 4;
	post** expected = (post**)malloc(sizeof(post*)*expected_nelements);
	expected[0] = &posts[32];
	expected[1] = &posts[45];
	expected[2] = &posts[47];
	expected[3] = &posts[55];

	post** actual = (post**)res->elements;

	int actual_nelements = res->n_elements;
	printf("actual %d\n", actual_nelements);
	bool passed = compare_post_arrays(expected, expected_nelements, actual, actual_nelements);
	print_test_status(testname, passed);
	if (!passed) print_expected_actual((void**)expected, (void**)actual, expected_nelements, actual_nelements, "post");
}
void test_find_all_reposts_invalid(users_posts* up, query_helper* helper) {
	char* testname = "test_find_all_reposts_invalid";
	bool passed = false;
	post* posts = up->posts;
	result* result = find_all_reposts(posts, 100, 101, helper);
	if (result == NULL) {
		passed = true;
	}
	print_test_status(testname, passed);
}

void test_find_original_trivial(users_posts* up, query_helper* helper) {
	char* testname = "test_find_original_trivial";
	bool passed = false;
	post* posts = up->posts;
	int pst_ind = 46;
	result* res = find_original(posts, 100, posts[pst_ind].pst_id, helper);

	post** expected = (post**)malloc(sizeof(post*));
	expected[0] = &posts[pst_ind];
	int expected_nelements = 1;

	passed = compare_post_arrays((post**)res->elements, res->n_elements, expected, expected_nelements);
	print_test_status(testname, passed);
	if (!passed) print_expected_actual((void**)expected, (void**)res->elements, expected_nelements, res->n_elements, "post");
}
void test_find_original_simple(users_posts* up, query_helper* helper) {
	char* testname = "test_find_original_simple";
	bool passed = false;
	post* posts = up->posts;

	int pst_ind = 11;
	post** expected = (post**)malloc(sizeof(post*));
	expected[0] = &posts[0];
	int expected_nelements = 1;

	result* res = find_original(posts, 100, posts[pst_ind].pst_id, helper);

	passed = compare_post_arrays(expected, expected_nelements, (post**)res->elements, res->n_elements);
	print_test_status(testname, passed);
	if (!passed) print_expected_actual((void**)expected, (void**)res->elements, expected_nelements, res->n_elements, "post");
}
void test_find_original_complex(users_posts* up, query_helper* helper) {
	bool passed;
	char* testname = "test_find_original_complex";
	post* posts = up->posts;
	int pst_ind = 55;
	result* res = find_original(posts, 100, posts[pst_ind].pst_id, helper);

	int expected_nelements = 1;
	post** expected = (post**)malloc(sizeof(post*)*expected_nelements);
	expected[0] = &posts[31];

	post** actual = (post**)res->elements;
	int actual_nelements = res->n_elements;

	passed = compare_post_arrays(expected, expected_nelements, actual, actual_nelements);
	print_test_status(testname, passed);
	if (!passed) print_expected_actual((void**)expected, (void**)actual, expected_nelements, actual_nelements, "post");
}
void test_find_original_invalid(users_posts* up, query_helper* helper) {
	char* testname = "test_find_original_invalid";
	bool passed = false;
	post* posts = up->posts;
	result* result = find_original(posts, 100, 101, helper);
	if (result == NULL) {
		passed = true;
	}
	print_test_status(testname, passed);
}

// Find shortest path between two users linked by 1
void test_find_shortest_link_trivial(users_posts* up, query_helper* helper) {
	char* testname = "test_find_shortest_link_trivial";
	bool passed = false;
	user* users = up->users;

	int i1 = 0;
	int i2 = 1;
	int user_id1 = users[i1].user_id;
	int user_id2 = users[i2].user_id;

	size_t expected_nelements = 2;
	user** expected = (user**)malloc(sizeof(user*)*expected_nelements);
	expected[0] = &users[i1];
	expected[1] = &users[i2];

	result* result = shortest_user_link(users, 10, user_id1, user_id2, helper);

	if (result == NULL || result->elements == NULL) {
		print_test_status(testname, false);
		printf(ANSI_COLOR_MAGENTA	"\tResult was NULL" ANSI_COLOR_RESET	"\n");
		return;
	}

	user** actual = (user**)result->elements;
	size_t actual_nelements = result->n_elements;

	passed = compare_user_arrays(expected, expected_nelements, actual, actual_nelements);
	print_test_status(testname, passed);
	if (!passed) print_expected_actual((void**)expected, (void**)actual, expected_nelements, actual_nelements, "user");

	free(expected);
}
void test_find_shortest_link_simple(users_posts* up, query_helper* helper) {
	char* testname = "test_find_shortest_link_simple";
	bool passed = false;
	user* users = up->users;

	int i1 = 1;
	int i2 = 3;
	int user_id1 = users[i1].user_id;
	int user_id2 = users[i2].user_id;

	size_t expected_nelements = 3;
	user** expected = (user**)malloc(sizeof(user*)*expected_nelements);
	expected[0] = &users[i1];
	expected[1] = &users[2];
	expected[expected_nelements-1] = &users[i2];

	result* result = shortest_user_link(users, 10, user_id1, user_id2, helper);

	if (result == NULL || result->elements == NULL) {
		print_test_status(testname, false);
		printf(ANSI_COLOR_MAGENTA	"\tResult was NULL" ANSI_COLOR_RESET	"\n");
		return;
	}

	user** actual = (user**)result->elements;
	size_t actual_nelements = result->n_elements;

	passed = compare_user_arrays(expected, expected_nelements, actual, actual_nelements);
	print_test_status(testname, passed);
	if (!passed) print_expected_actual((void**)expected, (void**)actual, expected_nelements, actual_nelements, "user");

	free(expected);
}
void test_find_shortest_link_long(users_posts* up, query_helper* helper) {
	char* testname = "test_find_shortest_link_long";
	bool passed = false;
	user* users = up->users;

	int i1 = 1;
	int i2 = 8;
	int user_id1 = users[i1].user_id;
	int user_id2 = users[i2].user_id;

	size_t expected_nelements = 5;
	user** expected = (user**)malloc(sizeof(user*)*expected_nelements);
	expected[0] = &users[i1];
	expected[1] = &users[2];
	expected[2] = &users[3];
	expected[3] = &users[6];
	expected[expected_nelements-1] = &users[i2];

	result* result = shortest_user_link(users, 10, user_id1, user_id2, helper);

	if (result == NULL || result->elements == NULL) {
		print_test_status(testname, false);
		printf(ANSI_COLOR_MAGENTA	"\tResult was NULL" ANSI_COLOR_RESET	"\n");
		return;
	}

	user** actual = (user**)result->elements;
	size_t actual_nelements = result->n_elements;

	passed = compare_user_arrays(expected, expected_nelements, actual, actual_nelements);
	print_test_status(testname, passed);
	if (!passed) print_expected_actual((void**)expected, (void**)actual, expected_nelements, actual_nelements, "user");

	free(expected);
}
void test_find_shortest_link_invalid(users_posts* up, query_helper* helper) {
	char* testname = "test_find_shortest_link_invalid";
	bool passed = false;
	user* users = up->users;
	result* result1 = shortest_user_link(users, 10, 101, 8, helper);
	result* result2 = shortest_user_link(users, 10, 8, 102, helper);
	result* result3 = shortest_user_link(users, 10, 105, 102, helper);
	result* result4 = shortest_user_link(users, 10, 4, 4, helper);
	if (result1 == NULL && result2 == NULL && result3 == NULL && result4 == NULL) {
		passed = true;
		print_test_status(testname, passed);
		return;
	} else {
		print_test_status(testname, passed);
		if (result1 != NULL) printf(ANSI_COLOR_CYAN	"\tResult 1 invalid" ANSI_COLOR_RESET "\n");
		if (result2 != NULL) printf(ANSI_COLOR_CYAN	"\tResult 2 invalid" ANSI_COLOR_RESET "\n");
		if (result3 != NULL) printf(ANSI_COLOR_CYAN	"\tResult 3 invalid" ANSI_COLOR_RESET "\n");
		if (result4 != NULL) printf(ANSI_COLOR_CYAN	"\tResult 4 invalid" ANSI_COLOR_RESET "\n");
	}
}

void test_find_bots_trivial(users_posts* up, query_helper* helper) {
	char* testname = "test_find_bots_trivial";
	bool passed = false;
	criteria (crit) = {
		.oc_threshold=1,
		.acc_rep_threshold=0,
		.bot_net_threshold=1,
	};

	user* users = up->users;
	post* posts = up->posts;
	result* res = find_bots(users, 10, posts, 100, &crit, helper);
	if (res == NULL) {
		print_test_status(testname, false);
		printf(ANSI_COLOR_MAGENTA	"\tResult was NULL" ANSI_COLOR_RESET	"\n");
		return;
	}

	if (res->n_elements == 0 && res->elements == NULL) {
		passed = true;
	}

	print_test_status(testname, passed);
}
void test_find_bots_simple_reposts(users_posts* up, query_helper* helper) {
	char* testname = "test_find_bots_simple_reposts";
	bool passed = false;
	criteria (crit) = {
		.oc_threshold=0.8,
		.acc_rep_threshold=0,
		.bot_net_threshold=1,
	};

	user* users = up->users;
	post* posts = up->posts;
	result* res = find_bots(users, 10, posts, 100, &crit, helper);
	if (res == NULL) {
		print_test_status(testname, false);
		printf(ANSI_COLOR_MAGENTA	"\tResult was NULL" ANSI_COLOR_RESET	"\n");
		return;
	}

	int expected_nelements = 1;
	user** expected = (user**)malloc(sizeof(user*)*expected_nelements);
	expected[0] = &users[9];

	user** actual = (user**)res->elements;
	int actual_nelements = res->n_elements;

	passed = compare_user_arrays(expected, expected_nelements, actual, actual_nelements);

	print_test_status(testname, passed);
	if (!passed) print_expected_actual((void**)expected, (void**)actual, expected_nelements, actual_nelements, "user");
}
void test_find_bots_simple_acc_rep(users_posts* up, query_helper* helper) {
	char* testname = "test_find_bots_simple_acc_rep";
	bool passed = false;
	criteria (crit) = {
		.oc_threshold=1,
		.acc_rep_threshold=0.26,
		.bot_net_threshold=1,
	};

	user* users = up->users;
	post* posts = up->posts;
	result* res = find_bots(users, 10, posts, 100, &crit, helper);
	if (res == NULL) {
		print_test_status(testname, false);
		printf(ANSI_COLOR_MAGENTA	"\tResult was NULL" ANSI_COLOR_RESET	"\n");
		return;
	}

	int expected_nelements = 1;
	user** expected = (user**)malloc(sizeof(user*)*expected_nelements);
	expected[0] = &users[6];

	user** actual = (user**)res->elements;
	int actual_nelements = res->n_elements;

	passed = compare_user_arrays(expected, expected_nelements, actual, actual_nelements);

	print_test_status(testname, passed);
	if (!passed) print_expected_actual((void**)expected, (void**)actual, expected_nelements, actual_nelements, "user");
}
void test_find_bots_long(users_posts* up, query_helper* helper) {
	char* testname = "test_find_bots_long";
	bool passed = false;
	criteria (crit) = {
		.oc_threshold=0.9,
		.acc_rep_threshold=0.6,
		.bot_net_threshold=1,
	};

	user* users = up->users;
	post* posts = up->posts;
	result* res = find_bots(users, 10, posts, 100, &crit, helper);
	if (res == NULL) {
		print_test_status(testname, false);
		printf(ANSI_COLOR_MAGENTA	"\tResult was NULL" ANSI_COLOR_RESET	"\n");
		return;
	}

	int expected_nelements = 5;
	user** expected = (user**)malloc(sizeof(user*)*expected_nelements);
	expected[0] = &users[0];
	expected[1] = &users[1];
	expected[2] = &users[4];
	expected[3] = &users[6];
	expected[4] = &users[9];

	user** actual = (user**)res->elements;
	int actual_nelements = res->n_elements;

	passed = compare_user_arrays(expected, expected_nelements, actual, actual_nelements);

	print_test_status(testname, passed);
	if (!passed) print_expected_actual((void**)expected, (void**)actual, expected_nelements, actual_nelements, "user");
}
void test_find_bots_complex(users_posts* up, query_helper* helper) {
	char* testname = "test_find_bots_complex";
	bool passed = false;
	criteria (crit) = {
		.oc_threshold=0.4,
		.acc_rep_threshold=0.2,
		.bot_net_threshold=0.19,
	};

	user* users = up->users;
	post* posts = up->posts;
	result* res = find_bots(users, 10, posts, 100, &crit, helper);
	if (res == NULL) {
		print_test_status(testname, false);
		printf(ANSI_COLOR_MAGENTA	"\tResult was NULL" ANSI_COLOR_RESET	"\n");
		return;
	}

	int expected_nelements = 4;
	user** expected = (user**)malloc(sizeof(user*)*expected_nelements);
	expected[0] = &users[2];
	expected[1] = &users[4];
	expected[2] = &users[6];	// Discrete Bot
	expected[3] = &users[9];

	user** actual = (user**)res->elements;
	int actual_nelements = res->n_elements;

	passed = compare_user_arrays(expected, expected_nelements, actual, actual_nelements);

	print_test_status(testname, passed);
	if (!passed) print_expected_actual((void**)expected, (void**)actual, expected_nelements, actual_nelements, "user");
}

void test_find_bots_invalid(users_posts* up, query_helper* helper) {
	char* testname = "test_find_bots_invalid";
	post* posts = up->posts;
	user* users = up->users;

	// Test first that each of the values is inside the value [0,1]
	criteria (crit) = {
		.oc_threshold=-1,
		.acc_rep_threshold=0.2,
		.bot_net_threshold=0.19,
	};

	result* r1 = find_bots(users, 10, posts, 100, &crit, helper);

	crit.oc_threshold = 0.5;
	crit.acc_rep_threshold = 3;
	result* r2 = find_bots(users, 10, posts, 100, &crit, helper);

	crit.acc_rep_threshold = 0.2;
	crit.bot_net_threshold = -0.666;
	result* r3 = find_bots(users, 10, posts, 100, &crit, helper);

	if (r1 == NULL || r2 == NULL || r3 == NULL) {
		print_test_status(testname, false);
		printf(ANSI_COLOR_MAGENTA	"\tResult was NULL" ANSI_COLOR_RESET "\n");
		return;
	}
	if (r1->elements == NULL && r2->elements == NULL && r3->elements == NULL) {
		print_test_status(testname, true);
	} else {
		print_test_status(testname, false);
		if (r1->elements != NULL) printf(ANSI_COLOR_CYAN	"\tResult 1 should return null elements."	ANSI_COLOR_RESET	"\n");
		if (r2->elements != NULL) printf(ANSI_COLOR_CYAN	"\tResult 2 should return null elements."	ANSI_COLOR_RESET	"\n");
		if (r3->elements != NULL) printf(ANSI_COLOR_CYAN	"\tResult 3 should return null elements."	ANSI_COLOR_RESET	"\n");
	}
}

void print_test_status(char* testname, bool passed) {
	printf("%s: ", testname);
	if (passed) {
		printf(ANSI_COLOR_GREEN 	"PASSED"	ANSI_COLOR_RESET "\n");
	} else {
		printf(ANSI_COLOR_RED			"FAILED" 	ANSI_COLOR_RESET "\n");
	}
}

void print_expected_actual(void** expected, void** actual, size_t expected_nelements, size_t actual_nelements, char* specifier) {
	printf("Arrays do not match:\n");
	printf("\tExpected %s Ids: [ ", specifier);
	if (strcmp(specifier, "post") == 0) {
		post** ex_posts = (post**)expected;
		post** ac_posts = (post**)actual;
		for (size_t i = 0; i < expected_nelements; i++) {
			printf("%lu ", ex_posts[i]->pst_id);
		}
		printf(" ]\n");
		printf("\tActual %s Ids: [ ", specifier);
		for (size_t i = 0; i < actual_nelements; i++) {
			printf("%lu ", ac_posts[i]->pst_id);
		}
	} else {
		user** ex_users = (user**)expected;
		user** ac_users = (user**)actual;
		for (size_t i = 0; i < expected_nelements; i++) {
			printf("%lu ", ex_users[i]->user_id);
		}
		printf(" ]\n");
		printf("\tActual %s Ids: [ ", specifier);
		for (size_t i = 0; i < actual_nelements; i++) {
			printf("%lu ", ac_users[i]->user_id);
		}
	}
	printf("]\n");
}

bool compare_user_arrays(user** expected, size_t expected_nelements, user** actual, size_t actual_nelements) {
	if (expected_nelements != actual_nelements) {
		return false;
	}
	qsort(expected, expected_nelements, sizeof(user*), user_comp);
	qsort(actual, actual_nelements, sizeof(user*), user_comp);

	return memcmp(expected, actual, sizeof(user*) * actual_nelements) == 0;
}

bool compare_post_arrays(post** expected, size_t expected_nelements, post** actual, size_t actual_nelements) {
	if (expected_nelements != actual_nelements) {
		return false;
	}
	qsort(expected, expected_nelements, sizeof(post*), post_comp);
	qsort(actual, actual_nelements, sizeof(post*), post_comp);

	return memcmp(expected, actual, sizeof(post*) * actual_nelements) == 0;
}

int user_comp(const void* elem1, const void* elem2) {
	user *u1 = ((user*)elem1);
	user *u2 = ((user*)elem2);
	if (u1->user_id > u2->user_id) return 1;
	if (u1->user_id < u2->user_id) return -1;
	return 0;
}

int post_comp(const void* elem1, const void* elem2) {
	post *p1 = ((post*)elem1);
	post *p2 = ((post*)elem2);
	if (p1->pst_id > p2->pst_id) return 1;
	if (p1->pst_id < p2->pst_id) return -1;
	return 0;
}
