## Project 5 -- Concurrency 

## Project Administration and Policies

**Due date:** Tuesday, November 18th, 2025, 11:59 PM

Follow all CS 537 [project policies](Admin.md).

## Project Workflow

Your solution code will be tested in the [CS 537 Docker container](https://git.doit.wisc.edu/cdis/cs/courses/cs537/useful-resources/cs537-docker). Follow the instructions in that repository to create the proper environment.  After following those instructions you should have a `cs537-projects/` directory where you can clone all of the projects for this course.

Follow the suggested [workflow](Workflow.md) for development of your solution.

## Project Overview
Imagine a **Tech Summit 2025**, hosted by the CDIS at the University of Wisconsin–Madison.

Hundreds of researchers, engineers, and students gather for a week-long conference on cutting-edge topics in distributed systems and synchronization.

To keep everyone updated, the organizers maintain a shared digital schedule board that displays the list of ongoing talks, workshops, and lunch specials. This board, however, has recently suffered race conditions leading to corrupted data (e.g., talks starting before attendees are notified). Ensuring safe concurrent access—without deadlocks—is your mission.

This project simulates a **tech conference** with two classic concurrency problems:

1. **Readers–Writers** (shared schedule board) 
2. **Producer-Consumer** (snacks distribution with bounded buffer)

Good luck with your implementation! 🚀

## Part 1: Implement Reader-Writer Lock (YOUR TASK)

**YOU MUST IMPLEMENT** a **Writer-Priority Reader-Writer Lock** that allows:
- **Multiple readers** to access shared data simultaneously
- **Only one writer** to access shared data at a time
- **Writers get priority** over readers (prevents writer starvation)

### The Scenario
In the conference simulation, the **schedule board** is shared between:
- **Readers (Attendees):** Many people reading the schedule simultaneously
- **Writers (Organizers):** Few people updating the schedule (need exclusive access)

## Implementation Requirements (YOU MUST COMPLETE THESE)

### 1. Complete the `rwlock_t` Structure 
⚠️ REQUIRED
**File:** `solution/include/sync_utils.h`

**YOU MUST ADD** the necessary fields to this structure:
```c
typedef struct {
  /* TODO: add semaphores/mutexes and counters */
  // You need to add the required fields here
} rwlock_t;
```

**Required fields to add:**
- `pthread_mutex_t m` - Protects shared variables
- `sem_t wlock` - Writer lock semaphore (binary: 0 or 1)
- `int readers` - Count of active readers
- `int writers_waiting` - Count of waiting writers
- `boolean writer_active` - Flag indicating if a writer is active


### 2. Implement Reader-Writer Lock Functions 
⚠️ REQUIRED
**File:** `solution/src/readers_writers.c`

**YOU MUST IMPLEMENT** these four functions.

#### `rw_rlock()` - Reader Lock (YOU IMPLEMENT)
**What you must do:**
- Block new readers while a writer is waiting or active (writer-priority)
- Increment reader count
- Use proper mutex locking/unlocking
- Handle writer priority correctly

#### `rw_runlock()` - Reader Unlock (YOU IMPLEMENT)
**What you must do:**
- Decrement reader count
- If this is the last reader AND writers are waiting, signal them
- Use proper mutex locking/unlocking

#### `rw_wlock()` - Writer Lock (YOU IMPLEMENT)
**What you must do:**
- Increment `writers_waiting` to block new readers (writer-priority)
- Wait in a loop while `writer_active` is true OR `readers > 0`
- When you get access, decrement `writers_waiting` and set `writer_active = true`
- Use proper mutex locking/unlocking and semaphore signaling

#### `rw_wunlock()` - Writer Unlock (YOU IMPLEMENT)
**What you must do:**
- Set `writer_active = false`
- If there are waiting writers, signal one using `sem_post(&wlock)`
- Use proper mutex locking/unlocking

### 3. Initialize the Lock Structure 
⚠️ REQUIRED
**File:** `solution/src/sync_utils.c`

**YOU MUST IMPLEMENT** `rw_init()` and `rw_destroy()` to:
- Initialize all mutexes and semaphores
- Set initial counter values
- Clean up resources on destroy

---

## Part 2: Bounded Buffer (Producer-Consumer) 
⚠️ YOUR TASK


### The Scenario
Conference snacks are managed using a bounded buffer:
- **Producers (Cooks):** Prepare food trays and put them in a queue
- **Consumers (Attendees):** Take food trays from the queue and **free the memory**
- **Bounded Buffer:** Fixed-size queue (capacity 8) holding food tray pointers

### 1. Complete the `bb_t` Structure 
⚠️ REQUIRED
**File:** `solution/include/sync_utils.h`

**First, understand the food tray structure** (already defined for you):

```c
typedef struct {
  int tray_id;           // Unique tray identifier
  char *food_name;       // Name of food on tray (dynamically allocated)
  int prepared_by;       // Cook who prepared it
} food_tray_t;
```

**YOU MUST ADD** the necessary fields to the buffer structure:

```c
typedef struct {
  /* TODO: add buffer array, semaphores, mutex, and indices */
  // You need to add the required fields here
} bb_t;
```

**Required fields to add:**
- `food_tray_t **buf` - Dynamic buffer array to store food tray **pointers**
- `int cap` - Capacity of the buffer
- `int head, tail` - Circular queue pointers (head for consumers, tail for producers)
- `sem_t empty` - Semaphore counting empty slots (initialized to capacity)
- `sem_t full` - Semaphore counting full slots (initialized to 0)
- `pthread_mutex_t m` - Mutex to protect head/tail updates

### 2. Implement Bounded Buffer Operations 
⚠️ REQUIRED
**File:** `solution/src/sync_utils.c`

**YOU MUST IMPLEMENT** these functions:

#### `bb_init()` - Initialize the buffer (YOU IMPLEMENT)
**What you must do:**
- Allocate memory for the buffer array using `calloc(capacity, sizeof(food_tray_t*))`
- Initialize `head` and `tail` to 0
- Initialize `empty` semaphore to `capacity` (all slots empty initially)
- Initialize `full` semaphore to `0` (no slots full initially)
- Initialize the mutex `m`
- Return 0 on success, -1 on failure

#### `bb_destroy()` - Clean up resources (YOU IMPLEMENT)
**What you must do:**
- Destroy the mutex
- Destroy both semaphores (`empty` and `full`)
- Free the buffer array

#### `bb_put()` - Producer adds food tray (YOU IMPLEMENT)
**Signature:** `void bb_put(bb_t *q, food_tray_t *tray)`

**What you must do:**
1. Wait on `empty` semaphore (blocks if buffer is full)
2. Lock the mutex
3. Place `tray` pointer at `buf[tail]`
4. Update `tail = (tail + 1) % cap` (circular buffer)
5. Unlock the mutex
6. Post to `full` semaphore (signal one more item available)


#### `bb_take()` - Consumer removes food tray (YOU IMPLEMENT)
**Signature:** `food_tray_t* bb_take(bb_t *q)`

**What you must do:**
1. Wait on `full` semaphore (blocks if buffer is empty)
2. Lock the mutex
3. Retrieve `tray` pointer from `buf[head]`
4. Update `head = (head + 1) % cap` (circular buffer)
5. Unlock the mutex
6. Post to `empty` semaphore (signal one more slot available)
7. Return the tray pointer

### 3. Understand the Snacks Scenario (PROVIDED)
**File:** `solution/src/bounded_buffer.c`

The snacks scenario is PROVIDED as a complete implementation showing how to use your bounded buffer. You should study this to understand:
- How producers (cooks) create food trays and use `bb_put()`
- How consumers (attendees) use `bb_take()` and **free the trays**
- How threads coordinate without explicit signaling
- Proper memory management with dynamic allocation

**Example from the provided code:**
```c
// Producer (cook) creates a tray
food_tray_t *tray = create_food_tray(tray_id, "Pizza", cook_id);
bb_put(&snack_queue, tray);  // Put pointer in buffer

// Consumer (attendee) takes and frees the tray
food_tray_t *tray = bb_take(&snack_queue);  // Get pointer from buffer
LOG("Attendee took %s", tray->food_name);
free_food_tray(tray);  // ⚠️ MUST FREE to avoid memory leak!
```

## Testing Your Implementation

### Build the Project
```bash
cd solution
make clean
make
```

### Run the Test Suite
After building, run the automated tests:
```bash
cd ../tests
./run_tests.sh -d tests
```
