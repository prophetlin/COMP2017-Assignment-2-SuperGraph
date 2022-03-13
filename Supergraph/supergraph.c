#include "supergraph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>


/*----------------------------------------------------------------------
 * SLIGHTLY MODIFIED VERSION OF QUEUE IMPLEMENTATION FROM
 * https://www.geeksforgeeks.org/queue-set-2-linked-list-implementation/
 -----------------------------------------------------------------------*/
struct QNode {
    size_t key;
    struct QNode *next;
};

// The queue, front stores the front node of LL and rear stores ths
// last node of LL
struct Queue {
    struct QNode* front;
    struct QNode* rear;
};

// A utility function to create a new linked list node.
struct QNode* new_node(size_t k) {
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->key = k;
    temp->next = NULL;
    return temp;
}

// A utility function to create an empty queue
struct Queue* create_queue() {
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}

// The function to add a key k to q
void enqueue(struct Queue *q, size_t k) {
    // Create a new LL node
    struct QNode* temp = new_node(k);

    // If queue is empty, then new node is front and rear both
    if (q->rear == NULL)
    {
        q->front = q->rear = temp;
        return;
    }

    // Add the new node at the end of queue and change rear
    q->rear->next = temp;
    q->rear = temp;
}

// Function to remove a key from given queue q
void dequeue(struct Queue *q) {
    // If queue is empty, return NULL.
    if (q->front == NULL) {

    }else {
        // Store previous front and move front one node ahead
        struct QNode *temp = q->front;
        q->front = q->front->next;

        // If front becomes NULL, then change rear also as NULL
        if (q->front == NULL) {
            q->rear = NULL;
        }
        free(temp);
    }

}

// Function to get key of front node in the queue q
size_t peek(struct Queue *q) {

    size_t index_ret = q->front->key;
    return index_ret;
}

// function to check if queue q is empty
bool is_empty(struct Queue *q) {

    if (q->front != NULL) {
        //printf("\n\nqueue is empty \n");
        return false;
    }
    return true;
}

void clear_queue(struct Queue *q) {
    while(q->front != NULL) {
        dequeue(q);
    }
}

/*--------------------------------------------------------------------
 * END OF GEEK FOR GEEK BORROWED QUEUE
 --------------------------------------------------------------------*/



bool calc_oc_threshold(float n_reposts, float n_posts, float oc_threshold) {

    if ((n_reposts/n_posts) > oc_threshold) {
        //printf("calculated threshold %f\n", n_reposts/n_posts);
        return true;
    }
    return false;
}

/* @param n_followers, number of follower the user have
 * @param n_following, number of users the user is following
 * @param acc_rep_threshold, threshold to check for bot by account reputation
 * the function returns if it passes the threshold test or not
 * return true, if pass else return false
 */

bool calc_rep(float n_followers, float n_following, float acc_rep_threshold) {

    if ((n_followers/(n_followers+n_following)) < acc_rep_threshold) {
        return true;
    }
    return false;
}

bool calc_discrete(float bots_following, float n_followers, float bot_net_threshold) {

    if ((bots_following / n_followers) > bot_net_threshold) {
        return true;
    }
    return false;
}

bool bfs(user* users, size_t count, size_t src, size_t dest, size_t *pred, size_t *dist){

    struct Queue *q = create_queue();
    bool visited[count];

    /* initialise visited array to be all false
     * initialise pred array to all SIZE_MAX
     * SIZE_MAX as placeholder
     * set dist array to all SIZE_MAX
     * as no path has been made
     */
    for(size_t i = 0; i < count; i++){
        visited[i] = false;
        dist[i] = SIZE_MAX;
        pred[i] = SIZE_MAX;
    }

    /* From src to dest
     * now source is first to be visited and
     * distance from source to itself should be 0
     */
    visited[src] = true;
    dist[src] = 0;
    enqueue(q,src);

    while(!is_empty(q)) {

        size_t ret = peek(q);
        dequeue(q);

        if(users[ret].n_following != 0) {

            for (size_t i = 0; i < users[ret].n_following; i++) {

                if (visited[users[ret].following_idxs[i]] == false) {
                    visited[users[ret].following_idxs[i]] = true;
                    dist[users[ret].following_idxs[i]] = dist[ret] + 1;
                    pred[users[ret].following_idxs[i]] = ret;
                    enqueue(q,users[ret].following_idxs[i]);

                    if(users[ret].following_idxs[i] == dest){
                        clear_queue(q);
                        free(q);
                        return true;
                    }
                }
            }
        }

    }
    clear_queue(q);
    free(q);
    return false;
}

void* bfs_thread(void* helper){

    struct Queue *q = create_queue();
    query_helper* help = (query_helper*)helper;
    bool visited[help->count];

    /* initialise visited array to be all false
     * initialise pred array to all SIZE_MAX
     * SIZE_MAX as placeholder
     * set dist array to all SIZE_MAX
     * as no path has been made
     */
    for(size_t i = 0; i < help->count; i++){
        visited[i] = false;
        help->dist[i] = SIZE_MAX;
        help->pred[i] = SIZE_MAX;
    }

    /* From src to dest
     * now source is first to be visited and
     * distance from source to itself should be 0
     */
    visited[help->src] = true;
    help->dist[help->src] = 0;
    enqueue(q,help->src);

    while(!is_empty(q)) {

        size_t ret = peek(q);
        dequeue(q);

        if(help->users[ret].n_following != 0) {

            for (size_t i = 0; i < help->users[ret].n_following; i++) {

                if (visited[help->users[ret].following_idxs[i]] == false) {
                    visited[help->users[ret].following_idxs[i]] = true;
                    help->dist[help->users[ret].following_idxs[i]] = help->dist[ret] + 1;
                    help->pred[help->users[ret].following_idxs[i]] = ret;
                    enqueue(q,help->users[ret].following_idxs[i]);

                    if(help->users[ret].following_idxs[i] == help->dst){
                        clear_queue(q);
                        free(q);
						            help->path_exist = true;
                        return NULL;
                    }
                }
            }
        }

    }
    clear_queue(q);
    free(q);
    return NULL;
}

query_helper* engine_setup(size_t n_processors) {

    return NULL;
}

result* find_all_reposts(post* posts, size_t count, uint64_t post_id, query_helper* helper) {

    //set up result structure
    struct result* reposts_list = (struct result*) malloc(sizeof(struct result));
    reposts_list->elements = malloc(sizeof(post*) * count);



    //go through the posts array
    //to find the index of the query id
    size_t index_post_id = SIZE_MAX;
    for (size_t post_index = 0; post_index < count; post_index++) {

        if(posts[post_index].pst_id == post_id) {
            index_post_id = post_index;
            break;
        }
    }

    //if no post id can be found
    if (index_post_id == SIZE_MAX) {
        free(reposts_list->elements);
        reposts_list->elements = NULL;
        reposts_list->n_elements = 0;
        return reposts_list;
    }

    //if post doesn't contain any repost
    if (posts[index_post_id].n_reposted == 0) {
        reposts_list->elements[0] = &posts[index_post_id];
        reposts_list->n_elements = 1;
        return reposts_list;
    }

    struct Queue *q = create_queue();
    // push source post to the queue
    enqueue(q,index_post_id);

    size_t count_repost = 0;
    while(!is_empty(q)) {

        size_t ret = peek(q);
        dequeue(q);
        if(posts[ret].n_reposted != 0) {

            for (size_t i = 0; i < posts[ret].n_reposted; i++) {
                // push the reposts indexes to the queue
                enqueue(q,posts[ret].reposted_idxs[i]);
            }

        }

        // add the node to the result list
        reposts_list->elements[count_repost] = (void*)&posts[ret];
        count_repost++;

    }

    reposts_list->n_elements = count_repost;
    clear_queue(q);
    free(q);
    return reposts_list;
}

result* find_original(post* posts, size_t count, uint64_t post_id, query_helper* helper) {

    struct result* original = (struct result*) malloc(sizeof(struct result));
    original->elements = malloc(sizeof(post*) * 1);

    // checks all post array for the id requested
    int index_post_id = -1;
    for (int post_index = 0; post_index < count; post_index++) {

        if(posts[post_index].pst_id == post_id) {
            index_post_id = post_index;
            break;
        }
    }

    // if no post id can be found
    if (index_post_id == -1) {
        free(original->elements);
        original->elements = NULL;
        original->n_elements = 0;
        return original;
    }

    /* go through all posts and check their reposts list
     * if the repost list contains the queried id
     * check if the post is also a repost
     * repeat and check through all posts again until
     * the post doesn't contain the id of the repost
     */

    for (int post_index = 0; post_index < count; post_index++) {
        for (int i = 0; i < posts[post_index].n_reposted; i++) {

            if (posts[post_index].reposted_idxs[i] == index_post_id) {
                index_post_id = post_index;
                post_index = -1;
                break;
            }
        }
    }

    original->elements[0] =  (void*)&posts[index_post_id];
    original->n_elements = 1;
    return original;
}

result* shortest_user_link(user* users, size_t count, uint64_t userA, uint64_t userB, query_helper* helper) {

    struct result* shortest_path = (struct result*) malloc(sizeof(struct result));
    shortest_path->elements = malloc(sizeof(user*) * count);
    shortest_path->n_elements = 0;

    if (userA == userB) {
        free(shortest_path->elements);
        shortest_path->elements = NULL;
        return shortest_path;
    }

    size_t userA_index = SIZE_MAX;
    size_t userB_index = SIZE_MAX;

    // find index of userA and userB in users array
    for(size_t user_index = 0; user_index < count; user_index++) {

        if(users[user_index].user_id == userA) {
            userA_index = user_index;

        }
        if(users[user_index].user_id == userB) {
            userB_index = user_index;
        }
    }

    // if either of them does not exist in users array
    if (userA_index == SIZE_MAX || userB_index == SIZE_MAX) {
        free(shortest_path->elements);
        shortest_path->elements = NULL;
        return shortest_path;
    }


    /* predecessor[i] array stores predecessors of
     * i and distance array stores distance of i
     * where i is any node in the graph
     * from source node
     * visited array stored whether each node is visited
     * once in BFS either false or true
     */

      size_t predA[count];
      size_t distA[count];

      size_t predB[count];
      size_t distB[count];

      size_t pathA_length = 0;
      size_t pathB_length = 0;



      pthread_t pthread[2];
      helper = malloc(sizeof(query_helper));
      helper->src = userA_index;
      helper->dst = userB_index;
      helper->users = users;
      helper->count = count;
      helper->dist = distA;
      helper->pred = predA;
	    helper->path_exist = false;
      pthread_create(&(pthread[0]),NULL,bfs_thread,helper);

	    query_helper* helper1 = malloc(sizeof(query_helper));
      helper1->src= userB_index;
      helper1->dst = userA_index;
	    helper1->users = users;
	    helper1->count = count;
      helper1->dist = distB;
      helper1->pred = predB;
	    helper1->path_exist = false;
      pthread_create(&(pthread[1]),NULL,bfs_thread,helper1);

      for (int i = 0; i < 2; i++){
          if (pthread_join(pthread[i], NULL) != 0) {
                  perror("unable to join thread");
                  return NULL;
           }
       }


       if(helper1->path_exist == false && helper->path_exist == false){
		       free(helper);
	   	     free(helper1);
           free(shortest_path->elements);
           shortest_path->elements = NULL;
           return shortest_path;
       }


       pathA_length = distA[userB_index];
       pathB_length = distB[userA_index];

	if(helper1->path_exist == true && helper->path_exist == true){
       // take the one with least path length
       if(pathA_length <= pathB_length) {
           // back track the path and construct the shortest path
           size_t pathA[count];
           size_t users_in_pathA = 0;
           size_t back_track = userB_index;
           pathA[users_in_pathA] = back_track;

           while(predA[back_track] != SIZE_MAX) {
               users_in_pathA++;
               pathA[users_in_pathA] = predA[back_track];
               back_track = predA[back_track];

           }
           //add user in path A to result
           int x = 0;
           for(int i = users_in_pathA; i >= 0; i--) {
               shortest_path->elements[x] =  &users[pathA[i]];
               x++;
           }

           shortest_path->n_elements = users_in_pathA+1;

       }else {

           size_t pathB[count];
           size_t users_in_pathB = 0;
           size_t back_track = userA_index;
           pathB[users_in_pathB] = back_track;

           while(predB[back_track] != SIZE_MAX) {
               users_in_pathB++;
               pathB[users_in_pathB] = predB[back_track];
               back_track = predB[back_track];

           }

           //add user in path B to result
           int x = 0;
           for(int i = users_in_pathB; i >= 0; i--) {
               shortest_path->elements[x] =  &users[pathB[i]];
               x++;
           }
           shortest_path->n_elements = users_in_pathB+1;
       }

	    }else if(helper->path_exist == true && helper1->path_exist == false){
		       size_t pathA[count];
           size_t users_in_pathA = 0;
           size_t back_track = userB_index;
           pathA[users_in_pathA] = back_track;

           while(predA[back_track] != SIZE_MAX) {
               users_in_pathA++;
               pathA[users_in_pathA] = predA[back_track];
               back_track = predA[back_track];

           }
           //add user in path A to result
           int x = 0;
           for(int i = users_in_pathA; i >= 0; i--) {
               shortest_path->elements[x] =  &users[pathA[i]];
               x++;
           }
           shortest_path->n_elements = users_in_pathA+1;

	        }else{
		          size_t pathB[count];
              size_t users_in_pathB = 0;
              size_t back_track = userA_index;
              pathB[users_in_pathB] = back_track;

              while(predB[back_track] != SIZE_MAX) {
                users_in_pathB++;
                pathB[users_in_pathB] = predB[back_track];
                back_track = predB[back_track];

              }

             //add user in path B to result
             int x = 0;
             for(int i = users_in_pathB; i >= 0; i--) {
                 shortest_path->elements[x] =  &users[pathB[i]];
                 x++;
             }
             shortest_path->n_elements = users_in_pathB+1;
	        }

	  free(helper);
	  free(helper1);
    return shortest_path;
}

result* find_bots(user* users, size_t user_count, post* posts, size_t post_count, criteria* crit, query_helper* helper) {

    struct result* bots = (struct result*) malloc(sizeof(struct result));
    bots->elements = malloc(sizeof(user*) * user_count);
    size_t number_of_bots = 0;
    bool bot_mark_idxs[user_count];
    bool repost_mark[post_count];

    memset(bot_mark_idxs,0, sizeof(bool)*user_count);
    memset(repost_mark,0, sizeof(bool)*post_count);

    //out of range threshold
    if(crit->oc_threshold < 0 || crit->oc_threshold >1){
        free(bots->elements);
        bots->elements = NULL;
        bots->n_elements = 0;
        return bots;
    }
    if(crit->acc_rep_threshold < 0 || crit->acc_rep_threshold >1){
        free(bots->elements);
        bots->elements = NULL;
        bots->n_elements = 0;
        return bots;
    }
    if(crit->bot_net_threshold < 0 || crit->bot_net_threshold >1){
        free(bots->elements);
        bots->elements = NULL;
        bots->n_elements = 0;
        return bots;
    }

    for(int x = 0; x < post_count; x++){
        for(int y = 0; y <  posts[x].n_reposted; y++){
            size_t index = posts[x].reposted_idxs[y];
            repost_mark[index] = true;
        }
    }
    //Reposts more than posts
    //go through each user posts and check if their post is original
    //keep track of how many are repost
    size_t number_of_reposts = 0;
    float og_threshold = crit->oc_threshold;

    for(size_t user_index= 0; user_index < user_count; user_index++) {
        size_t user_n_posts = users[user_index].n_posts;


        for(size_t post_index = 0; post_index < user_n_posts; post_index++) {
            size_t current_post = users[user_index].post_idxs[post_index];
            if(repost_mark[current_post] == true){
                number_of_reposts++;
            }

        }

        if (calc_oc_threshold(number_of_reposts,user_n_posts,og_threshold) == true){

            bots->elements[number_of_bots] = &users[user_index];
            bot_mark_idxs[user_index] = true;
            number_of_bots++;
        }

        number_of_reposts = 0;
    }

	 //account reputation
    float acc_rep_threshold = crit->acc_rep_threshold;
    for(int user_index= 0; user_index < user_count; user_index++) {
        if(bot_mark_idxs[user_index] == false){
            if(calc_rep(users[user_index].n_followers,users[user_index].n_following,acc_rep_threshold) == true) {
                // add the user to result list
                bots->elements[number_of_bots] = &users[user_index];
                bot_mark_idxs[user_index] = true;
                number_of_bots++;
            }
        }
    }

    //finding discrete bots
    //(bots_following user / n_followers) > bot_net_threshold

    float bot_net_threshold = crit->bot_net_threshold;
    size_t num_of_bot_following = 0;
    for(int user_index= 0; user_index < user_count; user_index++) {
        if(bot_mark_idxs[user_index] == false){

            for(int f = 0; f < users[user_index].n_followers; f++){
                size_t check = users[user_index].follower_idxs[f];
                if(bot_mark_idxs[check] == true ){
                    num_of_bot_following++;
                }
            }

            if(calc_discrete(num_of_bot_following,users[user_index].n_followers,bot_net_threshold) == true) {

                bots->elements[number_of_bots] = (void*)&users[user_index];
                bot_mark_idxs[user_index] = true;
                number_of_bots++;
                user_index = -1;
            }
        }
        num_of_bot_following = 0;
    }

    //bots->elements = realloc(bots->elements,sizeof(user*) * number_of_bots);
    bots->n_elements = number_of_bots;
    return bots;
}

void engine_cleanup(query_helper* helpers) {
    //Clean up your engine
}
