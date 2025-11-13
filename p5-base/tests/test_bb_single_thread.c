#include "sync_utils.h"
#include <assert.h>
#include <stdio.h>

/* Single-threaded bounded buffer test - no concurrency */

static bb_t test_queue;

int main() {
  LOG("=== Test: Single Thread Bounded Buffer (No Concurrency) ===");
  
  // Initialize buffer with capacity 5
  if (bb_init(&test_queue, 5) != 0) {
    LOG("FAIL: Buffer initialization failed");
    return 1;
  }
  
  LOG("Buffer initialized (capacity=5)");
  LOG("");
  
  // Single thread produces 3 items
  LOG("Phase 1: Producing 3 items");
  for (int i = 0; i < 3; i++) {
    food_tray_t *tray = create_food_tray(i, "Sandwich", 0);
    bb_put(&test_queue, tray);
    LOG("  Produced tray %d", i);
  }
  LOG("");
  
  // Single thread consumes 2 items
  LOG("Phase 2: Consuming 2 items");
  for (int i = 0; i < 2; i++) {
    food_tray_t *tray = bb_take(&test_queue);
    LOG("  Consumed tray %d (expected %d)", tray->tray_id, i);
    if (tray->tray_id != i) {
      LOG("FAIL: Expected tray_id=%d, got %d", i, tray->tray_id);
      free_food_tray(tray);
      bb_destroy(&test_queue);
      return 1;
    }
    free_food_tray(tray);
  }
  LOG("");
  
  // Single thread produces 4 more items (should have 1+4=5 total, filling buffer)
  LOG("Phase 3: Producing 4 more items (filling buffer to capacity)");
  for (int i = 3; i < 7; i++) {
    food_tray_t *tray = create_food_tray(i, "Pizza", 0);
    bb_put(&test_queue, tray);
    LOG("  Produced tray %d", i);
  }
  LOG("");
  
  // Single thread consumes all remaining 5 items
  LOG("Phase 4: Consuming all 5 remaining items");
  for (int i = 2; i < 7; i++) {
    food_tray_t *tray = bb_take(&test_queue);
    LOG("  Consumed tray %d (expected %d)", tray->tray_id, i);
    if (tray->tray_id != i) {
      LOG("FAIL: Expected tray_id=%d, got %d", i, tray->tray_id);
      free_food_tray(tray);
      bb_destroy(&test_queue);
      return 1;
    }
    free_food_tray(tray);
  }
  LOG("");
  
  LOG("PASS: Single-threaded test completed successfully");
  LOG("  All items produced and consumed in FIFO order");
  LOG("  No concurrency issues (only 1 thread)");
  
  bb_destroy(&test_queue);
  return 0;
}


