#include "supergraph.h"
#include "supergraph.c"
#include "test.h"
#include "test.c"
#define NUM_TESTS 15
#define NUM_PROCESSORS 4

query_helper* qh;

// Graph struct
typedef struct graph {
    post* posts;
    user* users;
    size_t num_users;
    size_t num_posts;
} graph;

// Takes a pointer to an array, an int, and a pointer to the size variable
//  for that array. Resizes the array, inserts the item, and updates the
//  size variable.
void list_append(size_t** array, uint64_t item, size_t* size) {
    (*size)++;
    *array = realloc(*array, sizeof(uint64_t) * (*size));
    (*array)[(*size)-1] = item;
}

// Free a user struct
void user_free(user* u) {
    if (u->n_followers > 0) free(u->follower_idxs);
    if (u->n_following > 0) free(u->following_idxs);
    if (u->n_posts > 0)     free(u->post_idxs);
}

// Free a post struct
void post_free(post* p) {
    free(p->reposted_idxs);
}

// Free a graph struct
void graph_free(graph* g) {
    for (int i = 0; i < g->num_users; i++) {
        user_free(&(g->users[i]));
    }
    for (int i = 0; i < g->num_posts; i++) {
        post_free(&(g->posts[i]));
    }
    free(g->posts);
    free(g->users);
    free(g);
}

// Free a result struct
void res_free(result* res) {
    free(res->elements);
    free(res);
}

// Create a new user. All function arguments are passed to the user struct.
user* new_user(uint64_t user_id,
        size_t* follower_idxs,
        size_t  n_followers,
        size_t* following_idxs,
        size_t  n_following,
        size_t* post_idxs,
        size_t  n_posts) {
    // Malloc a user
    user* u = (user*)malloc(sizeof(user));

    // Malloc all the arrays inside it
    if (follower_idxs == NULL) {
        u->follower_idxs =
            (size_t*)malloc((n_followers == 0) ? 1 : n_followers);
    }
    if (following_idxs == NULL) {
        u->following_idxs =
            (size_t*)malloc((n_following == 0) ? 1 : n_following);
    }
    if (post_idxs == NULL) {
        u->post_idxs = (size_t*)malloc((n_posts == 0) ? 1 : n_posts);
    }

    u->user_id = user_id;
    u->n_followers = n_followers;
    u->n_following = n_following;
    u->n_posts = n_posts;
    return u;
}

// Adds user follower_id as a follower of user followee_id
//  both follower_idx and followee_idx are indexes of the user array
//  (updates both follower.following_idxs and followee.follower_idxs)
void user_follow(graph* g, size_t follower_idx, size_t followee_idx) {
    user* follower = &g->users[follower_idx];
    user* followee = &g->users[followee_idx];

    // Check if already being followed / following

    int is_following = 0;
    int is_follower = 0;

    for (int i = 0; i < follower->n_following; i++) {
        if (follower->following_idxs[i] == followee_idx) is_following = 1;
        if (followee->follower_idxs[i] == follower_idx) is_follower = 1;
    }

    // If not, add to the follower / following lists
    if (!is_following)
        list_append(&follower->following_idxs,
                followee_idx,
                &follower->n_following);
    if (!is_follower)
        list_append(&followee->follower_idxs,
                follower_idx,
                &followee->n_followers);
}

// Designates a post post_idx as being posted by user poster_idx
//  Note that this doesn't do error checking on the post
void user_post(graph* g, size_t poster_idx, size_t post_idx) {
    user* poster = &(g->users[poster_idx]);

    // Check if already in the user's post array
    for (int i = 0; i < poster->n_posts; i++) {
        if (poster->post_idxs[i] == post_idx) return;
    }

    list_append(&poster->post_idxs, post_idx, &poster->n_posts);
}

// Make a new heap-allocated post struct
post* new_post(uint64_t pst_id,
        size_t* reposted_idxs,
        size_t n_reposted) {
    post* p = (post*)malloc(sizeof(post));
    p->pst_id = pst_id;
    p->reposted_idxs = reposted_idxs;
    p->n_reposted = n_reposted;

    // Malloc the post's reposted array
    if (reposted_idxs == NULL) {
        p->reposted_idxs = (size_t*)malloc(sizeof(uint64_t)
                * ((n_reposted == 0) ? 1 : n_reposted));
    }
    return p;
}

// Makes a new graph object containing num_users users and num_posts posts
graph* new_graph(size_t num_users, size_t num_posts) {

    graph* g = (graph*)malloc(sizeof(graph));

    g->num_users = num_users;
    g->num_posts = num_posts;
    g->users = malloc(sizeof(user) * num_users);
    g->posts = malloc(sizeof(post) * num_posts);

    // Create the users
    for (int i = 0; i < num_users; i++) {
        g->users[i] = *new_user(i, NULL, 0, NULL, 0, NULL, 0);
    }

    // Create the posts
    for (int i = 0; i < num_posts; i++) {
        g->posts[i] = *new_post(i, NULL, 0);
    }
    return g;
}

// Marks post index repost_idx as a repost of post index oc_idx
void repost_post(graph* reddit, uint64_t oc_idx, uint64_t repost_idx) {
    post* oc = &reddit->posts[oc_idx];
    post repost = reddit->posts[repost_idx];

    // Return if we already know it's a repost
    for (int i = 0; i < oc->n_reposted; i++) {
        if (repost.pst_id == oc->reposted_idxs[i])
            return;
    }

    // GENERAL REPOSTI
    //  Append to the OC's reposted list
    list_append(&oc->reposted_idxs, repost.pst_id, &oc->n_reposted);
}

// Debug prints all post objects and their reposts
void print_reposts(graph* g) {
    for (int i = 0; i < g->num_posts; i++) {
        printf(" - post %d: %zu reposts\n", i, g->posts[i].n_reposted);
        for (int j = 0; j < g->posts[i].n_reposted; j++)
            printf("   - post %zu\n", g->posts[i].reposted_idxs[j]);
    }
}

// BEGIN THE TESTS
// Test return codes:
//  0: Pass
//  1: Fail
//  2: Not implemented

////////////////////////////
// find_all_reposts tests //
////////////////////////////

// Tests when there are no reposts in a graph
//  (should return an empty result*)
int test_find_reposts_no_reposts() {
    graph* g = new_graph(5, 5);
    query_helper* qh = engine_setup(NUM_PROCESSORS);

    result* res = find_all_reposts(g->posts,
            g->num_posts,
            g->posts[0].pst_id,
            qh);

    if (res->n_elements != 1) return 1;
    if (res->elements[0] != &g->posts[0]) return 1;

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);

    return 0;
}

// Tests one repost
int test_find_reposts_one_repost() {
    graph* g = new_graph(2, 2);
    query_helper* qh = engine_setup(NUM_PROCESSORS);

    // Make post 1 a repost of post 0
    repost_post(g, 0, 1);

    result* res = find_all_reposts(g->posts,
            g->num_posts,
            0,
            qh);

    // If has one element that equals the repost
    if (res->n_elements != 2) return 1;
    if (res->elements[0] != &g->posts[0]) return 1;
    if (res->elements[1] != &g->posts[1]) return 1;

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);

    return 0;
}

// Tests a [familiar] scenario where all posts are reposts of posts[0]
int test_find_reposts_reddit() {
    graph* g = new_graph(5, 5);
    query_helper* qh = engine_setup(NUM_PROCESSORS);

    // Make each post a repost of post 0
    for (int i = 0; i < g->num_posts - 1; i++) {
        repost_post(g, 0, i + 1);
    }

    result* res = find_all_reposts(g->posts,
            g->num_posts,
            g->posts[0].pst_id,
            qh);

    // NOTE: THIS TEST MAY FAIL
    // It relies on the posts being returned in sequential order,
    //  which my implementation does. If that's not the case, you'll
    //  need to update the &g->posts indexes to be the correct order.

    // If all four elements are correct, free stuff and return 0
    if (res->n_elements != 5) return 1;
    if (res->elements[0] != &g->posts[0]) return 1;
    if (res->elements[1] != &g->posts[1]) return 1;
    if (res->elements[2] != &g->posts[2]) return 1;
    if (res->elements[3] != &g->posts[3]) return 1;
    if (res->elements[4] != &g->posts[4]) return 1;

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);

    return 0;
}

/////////////////////////
// find_original tests //
/////////////////////////

// Tests when there are no reposts
//  (should return whatever you pass into it, as all posts are their own
//   original posts)
int test_find_original_no_reposts() {
    graph* g = new_graph(5, 5);
    query_helper* qh = engine_setup(NUM_PROCESSORS);

    result* res = find_original(g->posts, g->num_posts, g->posts[0].pst_id, qh);

    // If the single element in the list is the one we passed in:
    if (res->n_elements != 1) return 1;
    if (res->elements[0] != &g->posts[0]) return 1;

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);

    return 0;
}

// Tests finding the origin of a single repost.
int test_find_original_one_repost() {
    graph* g = new_graph(2, 2);
    query_helper* qh = engine_setup(NUM_PROCESSORS);

    repost_post(g, 0, 1);

    // Find the origin of post 1
    result* res = find_original(g->posts, g->num_posts, g->posts[1].pst_id, qh);

    // If the single element in res is post 1, pass
    if (res->n_elements != 1) return 1;
    if (res->elements[0] != &g->posts[0]) return 1;

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);

    return 0;
}

// Tests a large graph
int test_find_original_reddit() {
    graph* g = new_graph(5, 5);
    query_helper* qh = engine_setup(NUM_PROCESSORS);

    // Each post is a repost of the one before it
    for (int i = 0; i < g->num_posts - 1; i++) {
        repost_post(g, i, i + 1);
    }

    result* res = find_original(g->posts, g->num_posts, g->posts[4].pst_id, qh);

    // If the origin of the fourth post is post 0, pass
    if (res->n_elements != 1) return 1;
    if (res->elements[0] != &g->posts[0]) return 1;

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);

    return 0;
}

/////////////////////////
// shortest_link tests //
/////////////////////////

// Tests the shortest link between two adjacent users
int test_shortest_link_adjacent() {
    graph* g = new_graph(2, 2);
    query_helper* qh = engine_setup(NUM_PROCESSORS);

    // User 0 is following user 1
    user_follow(g, 0, 1);

    // Find shortest link between user 0 and user 1
    result* res = shortest_user_link(g->users,
            g->num_users,
            g->users[0].user_id,
            g->users[1].user_id,
            qh);

    // If both users 0 and 1 are in the list, pass
    if (res->n_elements != 2) return 1;
    if (((user*)(res->elements[0]))->user_id != g->users[0].user_id) return 1;
    if (((user*)(res->elements[1]))->user_id != g->users[1].user_id) return 1;

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);

    return 0;
}

// Tests the shortest path between four users
int test_shortest_link_three_link() {
    graph* g = new_graph(4, 4);
    query_helper* qh = engine_setup(NUM_PROCESSORS);

    // 0 -> 1 -> 2 -> 3
    user_follow(g, 0, 1);
    user_follow(g, 1, 2);
    user_follow(g, 2, 3);

    // Distance between users 0 and 3
    result* res = shortest_user_link(g->users,
            g->num_users,
            g->users[0].user_id,
            g->users[3].user_id,
            qh);

    // Check if in the right order
    if (res->n_elements != 4) return 1;
    if (((user*)(res->elements[0]))->user_id != g->users[0].user_id) return 1;
    if (((user*)(res->elements[1]))->user_id != g->users[1].user_id) return 1;
    if (((user*)(res->elements[2]))->user_id != g->users[2].user_id) return 1;
    if (((user*)(res->elements[3]))->user_id != g->users[3].user_id) return 1;

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);

    return 0;
}

// Tests whether the algorithm picks the shorter of two paths
int test_shortest_link_three_link_two_path() {
    graph* g = new_graph(4, 4);
    query_helper* qh = engine_setup(NUM_PROCESSORS);

    // Graph structure:
    //  0
    //  |
    //  1
    //  |\
    //  | 2
    //  |/
    //  3
    //
    // (where all edges are directed from top to bottom)

    user_follow(g, 0, 1);
    user_follow(g, 1, 2);
    user_follow(g, 2, 3);
    user_follow(g, 1, 3);

    result* res = shortest_user_link(g->users,
            g->num_users,
            g->users[0].user_id,
            g->users[3].user_id,
            qh);

    // The shortest path from 0 to 3 is 0 -> 1 -> 3
    if (res->n_elements != 3) return 1;
    if (((user*)(res->elements[0]))->user_id != g->users[0].user_id) return 1;
    if (((user*)(res->elements[1]))->user_id != g->users[1].user_id) return 1;
    if (((user*)(res->elements[2]))->user_id != g->users[3].user_id) return 1;

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);

    return 0;
}

// Tests whether the algorithm looks both ways -
//  i.e. from A to B and from B to A
int test_shortest_link_both_ways() {
    graph* g = new_graph(4, 4);
    query_helper* qh = engine_setup(NUM_PROCESSORS);

    // Oh god how am I going to draw this
    //  (I gave up)
    //
    // 0 -> 1 -> 2 -> 3, but 3 -> 0

    user_follow(g, 0, 1);
    user_follow(g, 1, 2);
    user_follow(g, 2, 3);
    user_follow(g, 3, 0);

    result* res = shortest_user_link(g->users,
            g->num_users,
            g->users[0].user_id,
            g->users[3].user_id,
            qh);

    // Shortest path is from 3 to 0
    if (res->n_elements != 2) return 1;
    if (((user*)(res->elements[0]))->user_id != g->users[3].user_id) return 1;
    if (((user*)(res->elements[1]))->user_id != g->users[0].user_id) return 1;

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);

    return 0;
}

/////////////////////
// find_bots tests //
/////////////////////

// Note that these all expect these users returned in order of index.
//  This is not what the real test cases will do.

// Tests an empty graph returns nothing
int test_find_bots_empty_graph() {
    criteria* crit = malloc(sizeof(crit));
    query_helper* qh = engine_setup(NUM_PROCESSORS);
    graph* g = new_graph(10, 10);

    crit->oc_threshold      = 0.5f;
    crit->acc_rep_threshold = 0.5f;
    crit->bot_net_threshold = 0.5f;

    result* res = find_bots(
            g->users,
            g->num_users,
            g->posts,
            g->num_posts,
            crit,
            qh);

    // Just checking if nothing is returned
    if (res->n_elements != 0) return 1;

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);
    free(crit);
    return 0;
}

// Tests the first criteria: being over the repost/OC threshold
int test_find_bots_first_crit() {
    criteria* crit = malloc(sizeof(crit));
    query_helper* qh = engine_setup(NUM_PROCESSORS);
    graph* g = new_graph(10, 10);

    crit->oc_threshold      = 0.5f;
    crit->acc_rep_threshold = 0.5f;
    crit->bot_net_threshold = 0.5f;

    // User 0 has 1 post, user 1 has two posts
    user_post(g, 0, 0);
    user_post(g, 1, 1);
    user_post(g, 1, 2);

    // Both of user 1's posts are reposts
    repost_post(g, 0, 1);
    repost_post(g, 0, 2);

    result* res = find_bots(
            g->users,
            g->num_users,
            g->posts,
            g->num_posts,
            crit,
            qh);

    // User 1 is our bot here
    if (res->n_elements != 1) return 1;
    if (res->elements[0] != &(g->users[1])) return 1;

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);
    free(crit);
    return 0;
}

// Tests the second criteria: Being under the account reputation threshold
int test_find_bots_second_crit() {
    criteria* crit = malloc(sizeof(crit));
    query_helper* qh = engine_setup(NUM_PROCESSORS);
    graph* g = new_graph(10, 10);

    crit->oc_threshold      = 0.5f;
    crit->acc_rep_threshold = 0.5f;
    crit->bot_net_threshold = 1;

    // Our bot user this time will be user 3, following 3 accounts whilst
    //  only being followed by 1.
    user_follow(g, 3, 0);
    user_follow(g, 3, 1);
    user_follow(g, 3, 2);
    user_follow(g, 2, 3);

    result* res = find_bots(
            g->users,
            g->num_users,
            g->posts,
            g->num_posts,
            crit,
            qh);

    // Is user 3 our bot
    if (res->n_elements != 1) return 1;
    if (res->elements[0] != &(g->users[3])) return 1;

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);
    free(crit);
    return 0;
}

// Tests the third criteria: Being over the bot threshold.
int test_find_bots_third_crit() {
    criteria* crit = malloc(sizeof(crit));
    query_helper* qh = engine_setup(NUM_PROCESSORS);
    graph* g = new_graph(10, 10);

    // Turn off account rep checking (so it doesn't interfere with the tests)
    crit->oc_threshold      = 0.6f;
    crit->acc_rep_threshold = 0.0f;
    crit->bot_net_threshold = 0.5f;

    // This is the big one.

    // Post 0 is the only OC, posts 1, 2, 3 and 4 are all reposts of it
    repost_post(g, 0, 1);
    repost_post(g, 0, 2);
    repost_post(g, 0, 3);
    repost_post(g, 0, 4);

    // User 0 owns the only OC, whereas 1 and 2 own only reposts
    //  (this means that they need to be marked as bots)
    user_post(g, 0, 0);
    user_post(g, 1, 1);
    user_post(g, 1, 2);
    user_post(g, 2, 3);
    user_post(g, 2, 4);

    // User 3 (our discrete bot) is followed by three people (two of which are bots)
    user_follow(g, 0, 3);
    user_follow(g, 1, 3);
    user_follow(g, 2, 3);

    result* res = find_bots(
            g->users,
            g->num_users,
            g->posts,
            g->num_posts,
            crit,
            qh);

    // Users 1 and 2 are bots by OC threshold, user 3 is a discrete bot
    if (res->n_elements != 3) return 1;
    if (res->elements[0] != &(g->users[1])) return 1;
    if (res->elements[1] != &(g->users[2])) return 1;
    if (res->elements[2] != &(g->users[3])) return 1;

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);
    free(crit);
    return 0;
}

// Tests the third criteria with a discrete bot that depends on another discrete bot.
int test_find_bots_third_crit_many() {
    criteria* crit = malloc(sizeof(crit));
    query_helper* qh = engine_setup(NUM_PROCESSORS);
    graph* g = new_graph(10, 10);

    // Turn off account rep checking (so it doesn't interfere with the tests)
    crit->oc_threshold      = 0.6f;
    crit->acc_rep_threshold = 0.0f;
    crit->bot_net_threshold = 0.6f;

    // User 0 will be our discrete bot that depends on User 1 (also discrete)
    //  to be discovered to be a discrete bot
    // User 2 will be the only non-bot
    // Users 3 and 4 will be bots based on OC threshold

    /* 2   3   4
     *  \ / \ /
     *   v   v
     *   0<--1
     *
     */

    // User 2 holds OC, users 3 and 4 only hold reposts
    user_post(g, 2, 0);
    user_post(g, 3, 1);
    user_post(g, 4, 2);

    // posts 1 and 2 are reposts of post 0
    repost_post(g, 0, 1);
    repost_post(g, 0, 2);

    // User 0 is followed by a discrete bot, a non-bot, and a bot
    user_follow(g, 1, 0);
    user_follow(g, 2, 0);
    user_follow(g, 3, 0);

    // User 1 is followed by two bots (so it will be marked as a discrete bot,
    //  and trigger detection of user 0 as a discrete bot)
    user_follow(g, 3, 1);
    user_follow(g, 4, 1);

    result* res = find_bots(
            g->users,
            g->num_users,
            g->posts,
            g->num_posts,
            crit,
            qh);

    // Users 0 and 1 are discrete bots, and 3 and 4 are bots by OC threshold
    printf("\n");
    if (res->n_elements != 4){
      printf("FAILED NUMBER OF ELEMENT MISMATCH\n");
      printf("OUTPUT: %zu\n", res->n_elements);
      printf("EXPECTED: %d\n", 4);
      return 1;
    }
    if (res->elements[0] != &(g->users[0])){
      printf("FAILED USER ID MISMATCH \n");
      user* x = (user*)res->elements;
      printf("OUTPUT: %llu\n", x[0].user_id);
      printf("EXPECTED: %d\n", 0);
      return 1;
    }

    if (res->elements[1] != &(g->users[0])){
      printf("FAILED USER ID MISMATCH\n");
      user* x = (user*)res->elements;
      printf("OUTPUT: %llu\n", x[1].user_id);
      printf("EXPECTED: %d\n", 1);
      return 1;
    }
    if (res->elements[2] != &(g->users[3])){
      printf("FAILED USER ID MISMATCH\n");
      user* x = (user*)res->elements;
      printf("OUTPUT: %llu\n", x[3].user_id);
      printf("EXPECTED: %d\n", 4);
      return 1;
    }
    if (res->elements[3] != &(g->users[4])){
      printf("FAILED USER ID MISMATCH\n");
      user* x = (user*)res->elements;
      printf("OUTPUT: %llu\n", x[3].user_id);
      printf("EXPECTED: %d\n", 4);
      return 1;
    }

    graph_free(g);
    res_free(res);
    engine_cleanup(qh);
    free(crit);
    return 0;
}

int main() {
    int (*tests[NUM_TESTS])() = {
        // find_all_reposts
        test_find_reposts_no_reposts,
        test_find_reposts_one_repost,
        test_find_reposts_reddit,

        // find_original
        test_find_original_no_reposts,
        test_find_original_one_repost,
        test_find_original_reddit,

        // shortest_user_link
        test_shortest_link_adjacent,
        test_shortest_link_three_link,
        test_shortest_link_three_link_two_path,
        test_shortest_link_both_ways,

        // find_bots
        test_find_bots_empty_graph,
        test_find_bots_first_crit,
        test_find_bots_second_crit,
        test_find_bots_third_crit,
        test_find_bots_third_crit_many,
    };
    char *test_names[NUM_TESTS] = {
        "test_find_reposts_no_reposts",
        "test_find_reposts_one_repost",
        "test_find_reposts_reddit",
        "test_find_original_no_reposts",
        "test_find_original_one_repost",
        "test_find_original_reddit",
        "test_shortest_link_adjacent",
        "test_shortest_link_three_link",
        "test_shortest_link_three_link_two_path",
        "test_shortest_link_both_ways",
        "test_find_bots_empty_graph",
        "test_find_bots_first_crit",
        "test_find_bots_second_crit",
        "test_find_bots_third_crit",
        "test_find_bots_third_crit_many",
    };
    test_init(test_names, NUM_TESTS);

    for (int i = 0; i < NUM_TESTS; i++) {
        test(tests[i], test_names[i]);
    }
    test_end();
}
