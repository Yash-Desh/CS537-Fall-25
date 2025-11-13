#include "sync_utils.h"
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

/* Stress test for bounded buffer - tests for race conditions */
int usleep(unsigned int usec);

// Shared test data
static bb_t test_queue;
static int test_failed = 0;
static int total_produced = 0;
static int total_consumed = 0;
static int items_checksum_produced = 0;
static int items_checksum_consumed = 0;

// Track violations
static int overflow_detected = 0;
static int underflow_detected = 0;
static int data_corruption_detected = 0;
static int ordering_violation_detected = 0;
static pthread_mutex_t violation_mutex = PTHREAD_MUTEX_INITIALIZER;

// Configuration
#define NUM_PRODUCERS 40
#define NUM_CONSUMERS 60
#define ITEMS_PER_PRODUCER 20
#define BUFFER_CAPACITY 10
#define MAX_SLEEP_US 3000
#define TOTAL_ITEMS (NUM_PRODUCERS * ITEMS_PER_PRODUCER)

// Food names for variety
static const char* food_names[] = {
  "Pizza", "Sandwich", "Salad", "Pasta", "Burger", "Wrap", "Sushi", "Taco"
};
static const int num_food_types = 8;

// Producer thread
static void* producer_thread(void* arg) {
  int producer_id = *(int*)arg;
  
  for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
    // Random delay before producing
    usleep(rand() % MAX_SLEEP_US);
    
    // Create unique tray_id: producer_id * 1000 + item_number
    int tray_id = producer_id * 1000 + i;
    
    // Check buffer state before putting
    int count_before = 0;
    pthread_mutex_lock(&test_queue.m);
    count_before = (test_queue.tail - test_queue.head + test_queue.cap) % test_queue.cap;
    pthread_mutex_unlock(&test_queue.m);
    
    if (count_before >= test_queue.cap) {
      pthread_mutex_lock(&violation_mutex);
      LOG("OVERFLOW WARNING: Producer%d sees buffer full before put (count=%d, cap=%d)", 
          producer_id, count_before, test_queue.cap);
      overflow_detected = 1;
      pthread_mutex_unlock(&violation_mutex);
    }
    
    // Create a food tray
    const char* food = food_names[rand() % num_food_types];
    food_tray_t *tray = create_food_tray(tray_id, food, producer_id);
    
    bb_put(&test_queue, tray);
    
    pthread_mutex_lock(&violation_mutex);
    total_produced++;
    items_checksum_produced += tray_id;
    pthread_mutex_unlock(&violation_mutex);
    
    LOG("Producer%d: Put tray #%d with %s (item %d/%d)", 
        producer_id, tray_id, food, i+1, ITEMS_PER_PRODUCER);
    
    // Small delay while holding conceptual "producer slot"
    usleep(rand() % 500);
  }
  
  return NULL;
}

// Consumer thread
static void* consumer_thread(void* arg) {
  int consumer_id = *(int*)arg;
  int items_to_consume = TOTAL_ITEMS / NUM_CONSUMERS;
  
  // Last consumer takes any remaining items
  if (consumer_id == NUM_CONSUMERS - 1) {
    items_to_consume += TOTAL_ITEMS % NUM_CONSUMERS;
  }
  
  for (int i = 0; i < items_to_consume; i++) {
    // Random delay before consuming
    usleep(rand() % MAX_SLEEP_US);
    
    // Check buffer state before taking
    int count_before = 0;
    pthread_mutex_lock(&test_queue.m);
    count_before = (test_queue.tail - test_queue.head + test_queue.cap) % test_queue.cap;
    pthread_mutex_unlock(&test_queue.m);
    
    if (count_before < 0) {
      pthread_mutex_lock(&violation_mutex);
      LOG("UNDERFLOW WARNING: Consumer%d sees negative buffer count before take (count=%d)", 
          consumer_id, count_before);
      underflow_detected = 1;
      pthread_mutex_unlock(&violation_mutex);
    }
    
    food_tray_t *tray = bb_take(&test_queue);
    
    pthread_mutex_lock(&violation_mutex);
    total_consumed++;
    items_checksum_consumed += tray->tray_id;
    pthread_mutex_unlock(&violation_mutex);
    
    LOG("Consumer%d: Took tray #%d with %s (item %d/%d)", 
        consumer_id, tray->tray_id, tray->food_name, i+1, items_to_consume);
    
    // Small delay while processing
    usleep(rand() % 500);
    
    // IMPORTANT: Free the tray after consuming
    free_food_tray(tray);
  }
  
  return NULL;
}

int main(void) {
  LOG("=== Bounded Buffer Stress Test ===");
  LOG("Configuration:");
  LOG("  Producers: %d (each produces %d items)", NUM_PRODUCERS, ITEMS_PER_PRODUCER);
  LOG("  Consumers: %d (total consume %d items)", NUM_CONSUMERS, TOTAL_ITEMS);
  LOG("  Buffer capacity: %d", BUFFER_CAPACITY);
  LOG("  Total items: %d", TOTAL_ITEMS);
  LOG("");
  
  // Seed random number generator
  srand(time(NULL));
  
  // Initialize
  bb_init(&test_queue, BUFFER_CAPACITY);
  test_failed = 0;
  overflow_detected = 0;
  underflow_detected = 0;
  data_corruption_detected = 0;
  ordering_violation_detected = 0;
  total_produced = 0;
  total_consumed = 0;
  items_checksum_produced = 0;
  items_checksum_consumed = 0;
  
  pthread_t producers[NUM_PRODUCERS];
  pthread_t consumers[NUM_CONSUMERS];
  int producer_ids[NUM_PRODUCERS];
  int consumer_ids[NUM_CONSUMERS];
  
  LOG("Starting threads...");
  
  // Create all producer threads
  for (int i = 0; i < NUM_PRODUCERS; i++) {
    producer_ids[i] = i + 1;
    pthread_create(&producers[i], NULL, producer_thread, &producer_ids[i]);
  }
  
  // Create all consumer threads
  for (int i = 0; i < NUM_CONSUMERS; i++) {
    consumer_ids[i] = i + 1;
    pthread_create(&consumers[i], NULL, consumer_thread, &consumer_ids[i]);
  }
  
  LOG("All threads created. Waiting for completion...");
  
  // Wait for all producers
  for (int i = 0; i < NUM_PRODUCERS; i++) {
    pthread_join(producers[i], NULL);
  }
  
  LOG("All producers completed.");
  
  // Wait for all consumers
  for (int i = 0; i < NUM_CONSUMERS; i++) {
    pthread_join(consumers[i], NULL);
  }
  
  LOG("All consumers completed.");
  LOG("");
  
  // Final validation
  LOG("=== Results ===");
  LOG("Total items produced: %d (expected %d)", total_produced, TOTAL_ITEMS);
  LOG("Total items consumed: %d (expected %d)", total_consumed, TOTAL_ITEMS);
  LOG("Production checksum: %d", items_checksum_produced);
  LOG("Consumption checksum: %d", items_checksum_consumed);
  LOG("");
  
  // Check for violations
  if (overflow_detected) {
    LOG("FAIL: Buffer overflow detected!");
    test_failed = 1;
  } else {
    LOG("PASS: No buffer overflow detected");
  }
  
  if (underflow_detected) {
    LOG("FAIL: Buffer underflow detected!");
    test_failed = 1;
  } else {
    LOG("PASS: No buffer underflow detected");
  }
  
  if (total_produced != TOTAL_ITEMS) {
    LOG("FAIL: Not all items were produced (%d != %d)", total_produced, TOTAL_ITEMS);
    test_failed = 1;
  } else {
    LOG("PASS: All items produced (%d)", total_produced);
  }
  
  if (total_consumed != TOTAL_ITEMS) {
    LOG("FAIL: Not all items were consumed (%d != %d)", total_consumed, TOTAL_ITEMS);
    test_failed = 1;
  } else {
    LOG("PASS: All items consumed (%d)", total_consumed);
  }
  
  if (items_checksum_produced != items_checksum_consumed) {
    LOG("FAIL: Checksum mismatch - data corruption detected!");
    LOG("      Produced checksum: %d", items_checksum_produced);
    LOG("      Consumed checksum: %d", items_checksum_consumed);
    data_corruption_detected = 1;
    test_failed = 1;
  } else {
    LOG("PASS: Checksums match - no data corruption (%d)", items_checksum_produced);
  }
  
  // Check buffer is empty at the end
  int final_count = 0;
  pthread_mutex_lock(&test_queue.m);
  final_count = (test_queue.tail - test_queue.head + test_queue.cap) % test_queue.cap;
  pthread_mutex_unlock(&test_queue.m);
  
  if (final_count != 0) {
    LOG("FAIL: Buffer not empty at end (count=%d)", final_count);
    test_failed = 1;
  } else {
    LOG("PASS: Buffer empty at end");
  }
  
  bb_destroy(&test_queue);
  
  LOG("");
  if (test_failed) {
    LOG("=== STRESS TEST: FAILED ===");
    return 1;
  } else {
    LOG("=== STRESS TEST: PASSED ===");
    return 0;
  }
}

