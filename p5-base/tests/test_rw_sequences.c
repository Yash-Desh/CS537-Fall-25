#include "sync_utils.h"
#include <assert.h>
#include <stdio.h>
#include <pthread.h>

/* Multiple reader-writer sequence tests with different orderings */
int usleep(unsigned int usec);

// Shared test data
static rwlock_t test_board;
static int test_schedule = 0;
static int test_passed = 1;

// Thread action types
typedef enum {
  ACTION_READ,
  ACTION_WRITE
} action_type_t;

// Thread action description
typedef struct {
  action_type_t type;
  int thread_id;
  int action_index;    // Order in sequence (0, 1, 2, ...)
  int expected_value;  // For readers: value to check; For writers: increment amount
} action_t;

// Access internal rwlock_t fields for testing
static int get_readers_count(rwlock_t *rw) {
  return rw->readers;
}

// State validation (for writers only - they should have exclusive access)
static void check_writer_exclusive(const char *label) {
  int actual_readers = get_readers_count(&test_board);
  
  if (actual_readers != 0) {
    LOG("FAIL [%s]: Writer should have exclusive access, but readers=%d", label, actual_readers);
    test_passed = 0;
  } else {
    LOG("PASS [%s]: Writer has exclusive access (readers=0)", label);
  }
}

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
  
  if (action->type == ACTION_READ) {
    snprintf(label, sizeof(label), "R%d reading", action->thread_id);
    LOG("Reader%d: Starting (step %d)", action->thread_id, action->action_index);
    
    rw_rlock(&test_board);
    int value = test_schedule;
    LOG("Reader%d: Acquired read lock, schedule=%d", action->thread_id, value);
    
    // Check value
    if (value != action->expected_value) {
      LOG("FAIL [R%d value]: Expected schedule=%d, got %d", 
          action->thread_id, action->expected_value, value);
      test_passed = 0;
    } else {
      LOG("PASS [R%d value]: Read correct schedule value (%d)", 
          action->thread_id, action->expected_value);
    }
    
    usleep(20000); // Hold lock for 20ms
    rw_runlock(&test_board);
    LOG("Reader%d: Released read lock", action->thread_id);
    
  } else { // ACTION_WRITE
    snprintf(label, sizeof(label), "W%d writing", action->thread_id);
    LOG("Writer%d: Starting (step %d)", action->thread_id, action->action_index);
    
    rw_wlock(&test_board);
    int old_val = test_schedule;
    test_schedule += action->expected_value;
    LOG("Writer%d: Acquired write lock, updating schedule %d -> %d", 
        action->thread_id, old_val, test_schedule);
    
    // Validate exclusive access
    check_writer_exclusive(label);
    
    usleep(20000); // Hold lock for 20ms
    rw_wunlock(&test_board);
    LOG("Writer%d: Released write lock, schedule=%d", action->thread_id, test_schedule);
  }
  
  // Signal next thread in sequence
  signal_next_thread();
  
  return NULL;
}

// Run a sequence of actions
static int run_sequence(const char *name, action_t *actions, int num_actions, int expected_final) {
  LOG("\n=== Test: %s ===", name);
  test_schedule = 0;
  test_passed = 1;
  
  // Reset synchronization
  pthread_mutex_lock(&seq_mutex);
  current_action = 0;
  pthread_mutex_unlock(&seq_mutex);
  
  rw_init(&test_board);
  
  pthread_t threads[num_actions];
  
  // Create all threads (they'll wait for their turn)
  for (int i = 0; i < num_actions; i++) {
    pthread_create(&threads[i], NULL, thread_func, &actions[i]);
  }
  
  // Wait for all threads to complete
  for (int i = 0; i < num_actions; i++) {
    pthread_join(threads[i], NULL);
  }
  
  rw_destroy(&test_board);
  
  // Final validation
  if (test_schedule != expected_final) {
    LOG("FAIL [Final value]: Expected final schedule=%d, got %d", expected_final, test_schedule);
    test_passed = 0;
  } else {
    LOG("PASS [Final value]: Final schedule value correct (%d)", expected_final);
  }
  
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
  
  // Test 1: Original sequence R1 -> W1 -> R2 -> W2 -> R3
  {
    action_t sequence[] = {
      {ACTION_READ,  1, 0, 0},  // R1 reads 0 (step 0)
      {ACTION_WRITE, 1, 1, 1},  // W1 writes: 0->1 (step 1)
      {ACTION_READ,  2, 2, 1},  // R2 reads 1 (step 2)
      {ACTION_WRITE, 2, 3, 1},  // W2 writes: 1->2 (step 3)
      {ACTION_READ,  3, 4, 2}   // R3 reads 2 (step 4)
    };
    total_tests++;
    if (run_sequence("R1->W1->R2->W2->R3", sequence, 5, 2) == 0) total_passed++;
  }
  
  // Test 2: All readers first, then all writers: R1 -> R2 -> R3 -> W1 -> W2
  {
    action_t sequence[] = {
      {ACTION_READ,  1, 0, 0},  // R1 reads 0 (step 0)
      {ACTION_READ,  2, 1, 0},  // R2 reads 0 (step 1)
      {ACTION_READ,  3, 2, 0},  // R3 reads 0 (step 2)
      {ACTION_WRITE, 1, 3, 1},  // W1 writes: 0->1 (step 3)
      {ACTION_WRITE, 2, 4, 1}   // W2 writes: 1->2 (step 4)
    };
    total_tests++;
    if (run_sequence("R1->R2->R3->W1->W2", sequence, 5, 2) == 0) total_passed++;
  }
  
  // Test 3: Alternating: R1 -> W1 -> R2 -> W2 -> R3 -> W3
  {
    action_t sequence[] = {
      {ACTION_READ,  1, 0, 0},  // R1 reads 0 (step 0)
      {ACTION_WRITE, 1, 1, 1},  // W1 writes: 0->1 (step 1)
      {ACTION_READ,  2, 2, 1},  // R2 reads 1 (step 2)
      {ACTION_WRITE, 2, 3, 1},  // W2 writes: 1->2 (step 3)
      {ACTION_READ,  3, 4, 2},  // R3 reads 2 (step 4)
      {ACTION_WRITE, 3, 5, 1}   // W3 writes: 2->3 (step 5)
    };
    total_tests++;
    if (run_sequence("R1->W1->R2->W2->R3->W3", sequence, 6, 3) == 0) total_passed++;
  }
  
  // Test 4: All writers first: W1 -> W2 -> W3 -> R1 -> R2
  {
    action_t sequence[] = {
      {ACTION_WRITE, 1, 0, 1},  // W1 writes: 0->1 (step 0)
      {ACTION_WRITE, 2, 1, 1},  // W2 writes: 1->2 (step 1)
      {ACTION_WRITE, 3, 2, 1},  // W3 writes: 2->3 (step 2)
      {ACTION_READ,  1, 3, 3},  // R1 reads 3 (step 3)
      {ACTION_READ,  2, 4, 3}   // R2 reads 3 (step 4)
    };
    total_tests++;
    if (run_sequence("W1->W2->W3->R1->R2", sequence, 5, 3) == 0) total_passed++;
  }
  
  // Test 5: Writer sandwich: R1 -> W1 -> W2 -> W3 -> R2
  {
    action_t sequence[] = {
      {ACTION_READ,  1, 0, 0},  // R1 reads 0 (step 0)
      {ACTION_WRITE, 1, 1, 1},  // W1 writes: 0->1 (step 1)
      {ACTION_WRITE, 2, 2, 1},  // W2 writes: 1->2 (step 2)
      {ACTION_WRITE, 3, 3, 1},  // W3 writes: 2->3 (step 3)
      {ACTION_READ,  2, 4, 3}   // R2 reads 3 (step 4)
    };
    total_tests++;
    if (run_sequence("R1->W1->W2->W3->R2", sequence, 5, 3) == 0) total_passed++;
  }
  
  // Summary
  LOG("\n======================================");
  LOG("SUMMARY: %d/%d tests passed", total_passed, total_tests);
  LOG("======================================\n");
  
  return (total_passed == total_tests) ? 0 : 1;
}

