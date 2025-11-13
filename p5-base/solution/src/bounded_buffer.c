#define _XOPEN_SOURCE 700
#include <unistd.h>
#include "sync_utils.h"
#include <stdlib.h>

int usleep(unsigned int usec);

/* Config */
enum { NUM_ATTENDEES = 40, NUM_COOKS = 2, BUF_C = 8, SNACKS_PER_ATTENDEE = 1 };

/* Food names for variety */
static const char* food_names[] = {
  "Pizza Slice", "Sandwich", "Salad Bowl", "Fruit Plate", 
  "Pasta Bowl", "Burger", "Wrap", "Sushi Roll"
};
static const int num_food_types = 8;

static bb_t snack_queue;
static int tray_counter = 0;
static pthread_mutex_t tray_counter_mutex = PTHREAD_MUTEX_INITIALIZER;

static void* cook(void* arg) {
  long id = (long)arg;
  for (;;) {
    jitter_us(500, 5000);
    
    // Get unique tray ID
    pthread_mutex_lock(&tray_counter_mutex);
    int tray_id = tray_counter++;
    pthread_mutex_unlock(&tray_counter_mutex);
    
    // Create a food tray with random food
    const char* food = food_names[rand() % num_food_types];
    food_tray_t *tray = create_food_tray(tray_id, food, (int)id);
    
    bb_put(&snack_queue, tray);
    LOG("Kitchen#%ld produced tray #%d with %s", id, tray_id, food);
  }
  return NULL;
}

static void* attendee(void* arg) {
  long id = (long)arg;
  for (int k = 0; k < SNACKS_PER_ATTENDEE; ++k) {
    food_tray_t *tray = bb_take(&snack_queue);
    LOG("Attendee#%ld took tray #%d with %s (prepared by Cook#%d)", 
        id, tray->tray_id, tray->food_name, tray->prepared_by);
    
    jitter_us(500, 3000);
    
    // IMPORTANT: Student must free the tray after consuming
    free_food_tray(tray);
  }
  return NULL;
}

/* Public entry for main/test */
int snacks_run(void) {
  srand((unsigned)time(NULL));
  if (bb_init(&snack_queue, BUF_C)) DIE("bb_init");

  pthread_t people[NUM_ATTENDEES];
  for (long i=0;i<NUM_COOKS;i++)   spawn(cook, (void*)i, "cook");
  for (long i=0;i<NUM_ATTENDEES;i++) people[i] = spawn(attendee, (void*)i, "attendee");

  for (int i=0;i<NUM_ATTENDEES;i++) join(people[i]);

  /* In a full sim, cooks run forever; in the module demo we don't join them. */
  LOG("Snacks module complete (all attendees served once).");
  bb_destroy(&snack_queue);
  return 0;
}

