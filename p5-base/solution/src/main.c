#include "modules.h"
#include "sync_utils.h"
#include "bounded_buffer.h"
/* Prototypes from modules */
int schedule_run(void);
int snacks_run(void);

int main(int argc, char** argv) {
  (void)argc; (void)argv;
  LOG("Conference Simulation start");

  /* Run both simulations */
  LOG("=== Readers-Writers (Schedule Board) ===");
  schedule_run();       /* readers-writers */

  LOG("=== Producer-Consumer (Snacks) ===");
  snacks_run();         /* bounded buffer */

  LOG("Conference Simulation complete");
  return 0;
}
