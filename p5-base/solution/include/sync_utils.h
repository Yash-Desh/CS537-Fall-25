#ifndef SYNC_UTILS_H
#define SYNC_UTILS_H

#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define DIE(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

/* Logging (timestamped) */
uint64_t now_ms(void);
#define LOG(fmt, ...) \
  do { fprintf(stdout, "[%10llu ms] " fmt "\n", (unsigned long long)now_ms(), ##__VA_ARGS__); fflush(stdout); } while (0)

/* Thread helpers */
typedef void* (*thread_fn)(void*);
pthread_t spawn(thread_fn fn, void *arg, const char *name);
void join(pthread_t t);

/* Random jitter for schedule perturbation */
void jitter_us(int min_us, int max_us);

/* ---------- Reader-Writer Lock for Conference Schedule ---------- */

/* Reader-Writer lock (students implement in readers_writers.c) */
typedef struct {
  /* TODO: add semaphores/mutexes and counters */
  // You need to add the required fields here to implement writer-priority reader-writer lock
} rwlock_t;

int  rw_init(rwlock_t *rw);
void rw_destroy(rwlock_t *rw);
void rw_rlock(rwlock_t *rw);
void rw_runlock(rwlock_t *rw);
void rw_wlock(rwlock_t *rw);
void rw_wunlock(rwlock_t *rw);

/* ---------- Bounded Buffer for Producer-Consumer (Snacks) ---------- */

/* Food tray structure */
typedef struct {
  int tray_id;           // Unique tray identifier
  char *food_name;       // Name of food on tray (dynamically allocated)
  int prepared_by;       // Cook who prepared it
} food_tray_t;

/* Bounded buffer (students implement) */
typedef struct {
  /* TODO: add buffer array, semaphores, mutex, and indices */
  // You need to add the required fields here to implement a bounded buffer
} bb_t;

int  bb_init(bb_t *q, int capacity);
void bb_destroy(bb_t *q);
void bb_put(bb_t *q, food_tray_t *tray);   /* blocks if full */
food_tray_t* bb_take(bb_t *q);             /* blocks if empty, returns tray to consume */

/* Helper functions for food trays */
food_tray_t* create_food_tray(int tray_id, const char *food_name, int cook_id);
void free_food_tray(food_tray_t *tray);

#endif
