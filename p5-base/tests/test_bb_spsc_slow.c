#define _POSIX_C_SOURCE 200809L
#include "sync_utils.h"
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/* Single Producer Single Consumer with delays - tests blocking behavior */
int usleep(unsigned int usec);

static bb_t test_queue;
static int test_passed = 1;

#define NUM_ITEMS 5

static void* producer_thread(void* arg) {
  (void)arg;
  
  for (int i = 0; i < NUM_ITEMS; i++) {
    LOG("Producer: Creating tray %d", i);
    food_tray_t *tray = create_food_tray(i, "Burger", 1);
    
    LOG("Producer: Putting tray %d into buffer", i);
    bb_put(&test_queue, tray);
    LOG("Producer: Successfully put tray %d", i);
    
    // Simulate slow production (2 second delay)
    LOG("Producer: Sleeping for 2 seconds...");
    sleep(2);
  }
  
  LOG("Producer: Finished producing all %d items", NUM_ITEMS);
  return NULL;
}

static void* consumer_thread(void* arg) {
  (void)arg;
  
  // Small initial delay to let producer start
  usleep(500000); // 0.5 seconds
  
  for (int i = 0; i < NUM_ITEMS; i++) {
    LOG("Consumer: Taking tray from buffer (expecting %d)", i);
    food_tray_t *tray = bb_take(&test_queue);
    
    LOG("Consumer: Got tray %d", tray->tray_id);
    
    if (tray->tray_id != i) {
      LOG("FAIL: Expected tray_id=%d, got %d", i, tray->tray_id);
      test_passed = 0;
    }
    
    free_food_tray(tray);
    
    // Consumer is faster - no delay
    LOG("Consumer: Consumed tray (no delay, waiting for next)");
  }
  
  LOG("Consumer: Finished consuming all %d items", NUM_ITEMS);
  return NULL;
}

int main() {
  LOG("=== Test: SPSC (Single Producer, Single Consumer) with Slow Producer ===");
  LOG("Producer: Produces with 2-second delays");
  LOG("Consumer: Consumes as fast as possible");
  LOG("This tests blocking when buffer is empty");
  LOG("");
  
  if (bb_init(&test_queue, 3) != 0) {
    LOG("FAIL: Buffer initialization failed");
    return 1;
  }
  LOG("Buffer initialized (capacity=3)");
  LOG("");
  
  pthread_t producer, consumer;
  
  LOG("Starting threads...");
  pthread_create(&producer, NULL, producer_thread, NULL);
  pthread_create(&consumer, NULL, consumer_thread, NULL);
  
  pthread_join(producer, NULL);
  pthread_join(consumer, NULL);
  
  LOG("");
  if (test_passed) {
    LOG("PASS: SPSC slow producer test completed successfully");
    LOG("  Consumer correctly blocked when buffer was empty");
    LOG("  All items consumed in FIFO order");
  } else {
    LOG("FAIL: SPSC slow producer test failed");
  }
  
  bb_destroy(&test_queue);
  return test_passed ? 0 : 1;
}

