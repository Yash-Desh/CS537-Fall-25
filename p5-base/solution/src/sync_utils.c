#define _XOPEN_SOURCE 700
#include <unistd.h>
#include "sync_utils.h"
#include <sys/time.h>
#include <string.h>

int usleep(unsigned int usec);
uint64_t now_ms(void) {
  struct timeval tv; gettimeofday(&tv, NULL);
  return (uint64_t)tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
}

pthread_t spawn(thread_fn fn, void *arg, const char *name) {
  (void)name; /* name useful for extended logging */
  pthread_t t;
  if (pthread_create(&t, NULL, fn, arg)) DIE("pthread_create");
  return t;
}
void join(pthread_t t) { if (pthread_join(t, NULL)) DIE("pthread_join"); }

void jitter_us(int min_us, int max_us) {
  int span = (max_us > min_us) ? (max_us - min_us) : 1;
  int d = min_us + (rand() % span);
  usleep(d);
}

/* ------- Reader-Writer Lock: initialization and cleanup ------- */
int rw_init(rwlock_t *rw) {
  /* TODO: Initialize all fields in rwlock_t structure
   * - Initialize mutex
   * - Initialize semaphores
   * - Set initial counter values
   */
  // (void)rw;  // Remove this when you implement the function
  pthread_mutex_init(&rw->m, NULL);
  sem_init(&rw->OKToRead, 0, 0);
  sem_init(&rw->OKToWrite, 0, 0);
  rw->readers_active = 0;
  rw->readers_waiting = 0;
  rw->writer_active = 0;
  rw->writers_waiting = 0;
  return 0;
}

void rw_destroy(rwlock_t *rw) {
  /* TODO: Clean up resources
   * - Destroy mutex
   * - Destroy semaphores
   */
  // (void)rw;  // Remove this when you implement the function
  pthread_mutex_destroy(&rw->m);
  sem_destroy(&rw->OKToRead);
  sem_destroy(&rw->OKToWrite);
}
/* RW lock functions are implemented in readers_writers.c */

/* ------- Food Tray Helper Functions ------- */
food_tray_t* create_food_tray(int tray_id, const char *food_name, int cook_id) {
  food_tray_t *tray = malloc(sizeof(food_tray_t));
  if (!tray) DIE("malloc food_tray");
  
  tray->tray_id = tray_id;
  tray->food_name = strdup(food_name);
  if (!tray->food_name) DIE("strdup food_name");
  tray->prepared_by = cook_id;
  
  return tray;
}

void free_food_tray(food_tray_t *tray) {
  if (tray) {
    free(tray->food_name);
    free(tray);
  }
}

/* ------- Bounded Buffer: initialization and operations ------- */
int bb_init(bb_t *q, int capacity) {
  /* TODO: Initialize bounded buffer
   * - Allocate buffer array for food_tray_t pointers
   * - Initialize head and tail to 0
   * - Initialize empty semaphore to capacity
   * - Initialize full semaphore to 0
   * - Initialize mutex
   * - Return 0 on success, -1 on failure
   */
  // (void)q;
  // (void)capacity;
  q->buf = calloc(capacity, sizeof(food_tray_t*));
  q->cap = capacity;
  q->head = 0;
  q->tail = 0;
  sem_init(&q->empty, 0, capacity);
  sem_init(&q->full, 0, 0);
  pthread_mutex_init(&q->m, NULL);
  return 0;
}

void bb_destroy(bb_t *q) {
  /* TODO: Clean up bounded buffer resources
   * - Destroy mutex
   * - Destroy semaphores
   * - Free buffer array
   */
  // (void)q;
  sem_destroy(&q->full);
  sem_destroy(&q->empty);
  pthread_mutex_destroy(&q->m);
  free(q->buf);
}

void bb_put(bb_t *q, food_tray_t *tray) {
  /* TODO: Put item in buffer (producer)
   * - Wait on empty semaphore
   * - Lock mutex
   * - Add tray to buffer at tail position
   * - Update tail (circular: (tail + 1) % capacity)
   * - Unlock mutex
   * - Post to full semaphore
   */
  // (void)q;
  // (void)tray;
  sem_wait(&q->empty);
  pthread_mutex_lock(&q->m);
  q->buf[q->tail] = tray;
  q->tail = (q->tail + 1) % q->cap;
  pthread_mutex_unlock(&q->m);
  sem_post(&q->full);
}

food_tray_t* bb_take(bb_t *q) {
  /* TODO: Take item from buffer (consumer)
   * - Wait on full semaphore
   * - Lock mutex
   * - Remove tray from buffer at head position
   * - Update head (circular: (head + 1) % capacity)
   * - Unlock mutex
   * - Post to empty semaphore
   * - Return the tray
   */
  // (void)q;
  food_tray_t *temp = NULL;
  sem_wait(&q->full);
  pthread_mutex_lock(&q->m);
  temp = q->buf[q->head];
  q->head = (q->head + 1) % q->cap;
  pthread_mutex_unlock(&q->m);
  sem_post(&q->empty);
  return temp;
}
