#include "sync_utils.h"
#include <assert.h>
#include <stdio.h>
#include <pthread.h>

/* Sequential bounded buffer tests with ordered producer-consumer actions */
int usleep(unsigned int usec);

// Shared test data
static bb_t test_queue;
static int test_passed = 1;

// Thread action types
typedef enum {
  ACTION_PRODUCE,
  ACTION_CONSUME
} action_type_t;

// Thread action description
typedef struct {
  action_type_t type;
  int thread_id;
  int action_index;    // Order in sequence (0, 1, 2, ...)
  int value;           // For produce: tray_id; For consume: expected tray_id
  const char *food_name; // Food name for the tray
} action_t;

// Note: We don't check buffer counts in sequential tests because
// semaphores handle correctness internally. We only verify FIFO ordering.

// Step-based synchronization for sequential execution
static int current_action = 0;
static pthread_mutex_t seq_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t seq_cond = PTHREAD_COND_INITIALIZER;

static void wait_for_my_turn(int my_index) {
  pthread_mutex_lock(&seq_mutex);
  while (current_action < my_index) {
    pthread_cond_wait(&seq_cond, &seq_mutex);
  }
  pthread_mutex_unlock(&seq_mutex);
}

static void signal_next_thread(void) {
  pthread_mutex_lock(&seq_mutex);
  current_action++;
  pthread_cond_broadcast(&seq_cond);
  pthread_mutex_unlock(&seq_mutex);
}

// Generic thread function
static void* thread_func(void* arg) {
  action_t *action = (action_t*)arg;
  char label[100];
  
  // Wait for my turn in the sequence
  wait_for_my_turn(action->action_index);
  
  if (action->type == ACTION_PRODUCE) {
    snprintf(label, sizeof(label), "P%d producing", action->thread_id);
    LOG("Producer%d: Starting (step %d), putting tray #%d with %s", 
        action->thread_id, action->action_index, action->value, action->food_name);
    
    food_tray_t *tray = create_food_tray(action->value, action->food_name, action->thread_id);
    bb_put(&test_queue, tray);
    LOG("Producer%d: Put tray #%d with %s into buffer", 
        action->thread_id, action->value, action->food_name);
    LOG("PASS [P%d]: Successfully produced tray #%d", 
        action->thread_id, action->value);
    
  } else { // ACTION_CONSUME
    snprintf(label, sizeof(label), "C%d consuming", action->thread_id);
    LOG("Consumer%d: Starting (step %d)", action->thread_id, action->action_index);
    
    food_tray_t *tray = bb_take(&test_queue);
    LOG("Consumer%d: Took tray #%d with %s from buffer", 
        action->thread_id, tray->tray_id, tray->food_name);
    
    // Check tray_id (FIFO ordering)
    if (tray->tray_id != action->value) {
      LOG("FAIL [C%d tray_id]: Expected tray_id=%d, got %d", 
          action->thread_id, action->value, tray->tray_id);
      test_passed = 0;
    } else {
      LOG("PASS [C%d]: Got correct tray #%d", 
          action->thread_id, tray->tray_id);
    }
    
    // IMPORTANT: Free the tray after consuming
    free_food_tray(tray);
  }
  
  // Signal next thread in sequence
  signal_next_thread();
  
  return NULL;
}

// Run a sequence of actions
static int run_sequence(const char *name, action_t *actions, int num_actions, int buffer_size) {
  LOG("\n=== Test: %s ===", name);
  test_passed = 1;
  
  // Reset synchronization
  pthread_mutex_lock(&seq_mutex);
  current_action = 0;
  pthread_mutex_unlock(&seq_mutex);
  
  bb_init(&test_queue, buffer_size);
  
  pthread_t threads[num_actions];
  
  // Create all threads (they'll wait for their turn)
  for (int i = 0; i < num_actions; i++) {
    pthread_create(&threads[i], NULL, thread_func, &actions[i]);
  }
  
  // Wait for all threads to complete
  for (int i = 0; i < num_actions; i++) {
    pthread_join(threads[i], NULL);
  }
  
  bb_destroy(&test_queue);
  
  if (test_passed) {
    LOG("=== %s: PASSED ===\n", name);
    return 0;
  } else {
    LOG("=== %s: FAILED ===\n", name);
    return 1;
  }
}

int main(void) {
  int total_passed = 0;
  int total_tests = 0;
  
  // Test 1: Simple sequence P1 -> C1
  {
    action_t sequence[] = {
      {ACTION_PRODUCE, 1, 0, 100, "Pizza"},  // P1 produces tray #100 (step 0)
      {ACTION_CONSUME, 1, 1, 100, NULL}      // C1 consumes tray #100 (step 1)
    };
    total_tests++;
    if (run_sequence("P1->C1", sequence, 2, 5) == 0) total_passed++;
  }
  
  // Test 2: Multiple producers then consumers P1->P2->C1->C2
  {
    action_t sequence[] = {
      {ACTION_PRODUCE, 1, 0, 10, "Sandwich"},   // P1 produces tray #10 (step 0)
      {ACTION_PRODUCE, 2, 1, 20, "Salad"},      // P2 produces tray #20 (step 1)
      {ACTION_CONSUME, 1, 2, 10, NULL},         // C1 consumes tray #10 (step 2) - FIFO
      {ACTION_CONSUME, 2, 3, 20, NULL}          // C2 consumes tray #20 (step 3)
    };
    total_tests++;
    if (run_sequence("P1->P2->C1->C2", sequence, 4, 5) == 0) total_passed++;
  }
  
  // Test 3: Interleaved P1->C1->P2->C2->P3->C3
  {
    action_t sequence[] = {
      {ACTION_PRODUCE, 1, 0, 111, "Burger"},   // P1 produces tray #111
      {ACTION_CONSUME, 1, 1, 111, NULL},       // C1 consumes tray #111
      {ACTION_PRODUCE, 2, 2, 222, "Pasta"},    // P2 produces tray #222
      {ACTION_CONSUME, 2, 3, 222, NULL},       // C2 consumes tray #222
      {ACTION_PRODUCE, 3, 4, 333, "Wrap"},     // P3 produces tray #333
      {ACTION_CONSUME, 3, 5, 333, NULL}        // C3 consumes tray #333
    };
    total_tests++;
    if (run_sequence("P1->C1->P2->C2->P3->C3", sequence, 6, 5) == 0) total_passed++;
  }
  
  // Test 4: Fill buffer then drain P1->P2->P3->C1->C2->C3
  {
    action_t sequence[] = {
      {ACTION_PRODUCE, 1, 0, 1, "Sushi"},    // P1 produces tray #1
      {ACTION_PRODUCE, 2, 1, 2, "Taco"},     // P2 produces tray #2
      {ACTION_PRODUCE, 3, 2, 3, "Ramen"},    // P3 produces tray #3
      {ACTION_CONSUME, 1, 3, 1, NULL},       // C1 consumes tray #1 (FIFO)
      {ACTION_CONSUME, 2, 4, 2, NULL},       // C2 consumes tray #2
      {ACTION_CONSUME, 3, 5, 3, NULL}        // C3 consumes tray #3
    };
    total_tests++;
    if (run_sequence("P1->P2->P3->C1->C2->C3", sequence, 6, 3) == 0) total_passed++;
  }
  
  // Test 5: Stress FIFO ordering with multiple items
  {
    action_t sequence[] = {
      {ACTION_PRODUCE, 1, 0, 100, "Curry"},     // P1
      {ACTION_PRODUCE, 2, 1, 200, "Steak"},     // P2
      {ACTION_PRODUCE, 3, 2, 300, "Soup"},      // P3
      {ACTION_PRODUCE, 4, 3, 400, "Noodles"},   // P4
      {ACTION_CONSUME, 1, 4, 100, NULL},        // C1 gets first (100)
      {ACTION_CONSUME, 2, 5, 200, NULL},        // C2 gets second (200)
      {ACTION_CONSUME, 3, 6, 300, NULL},        // C3 gets third (300)
      {ACTION_CONSUME, 4, 7, 400, NULL}         // C4 gets fourth (400)
    };
    total_tests++;
    if (run_sequence("FIFO-P1->P2->P3->P4->C1->C2->C3->C4", sequence, 8, 5) == 0) total_passed++;
  }
  
  LOG("\n=== SUMMARY ===");
  LOG("Passed: %d/%d tests", total_passed, total_tests);
  
  return (total_passed == total_tests) ? 0 : 1;
}

