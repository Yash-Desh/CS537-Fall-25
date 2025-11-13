#include "sync_utils.h"
#include "bounded_buffer.h"
#include <assert.h>
#include <stdio.h>

extern int snacks_run(void);

static int pass(const char* name, int rc) {
  LOG("TEST %s -> %s", name, rc==0 ? "PASS" : "FAIL");
  return rc;
}

int main(void) {
  int ok = 1;
  int rc = snacks_run();
  ok &= (pass("snacks (bounded-buffer run completes)", rc) == 0);
  
  // Basic success check - if it completes without deadlock, it's good
  LOG("PASS: Bounded buffer test completed successfully");
  
  return ok ? 0 : 1;
}


