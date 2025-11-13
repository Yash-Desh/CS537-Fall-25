#include "sync_utils.h"
#include <assert.h>
#include <stdio.h>

extern int schedule_run(void);
extern int get_final_schedule_version(void); // Needs to be exposed for testing
extern int get_violation_count(void); // New: to access catch of bad overlaps

static int pass(const char* name, int rc) {
  LOG("TEST %s -> %s", name, rc==0 ? "PASS" : "FAIL");
  return rc;
}

int main(void) {
  int ok = 1;
  int rc = schedule_run();
  ok &= (pass("schedule (readers-writers run completes)", rc) == 0);
  int expected_version = 2 * 3; // 2 writers * 3 updates per writer
  int actual_version = get_final_schedule_version();
  if (actual_version != expected_version) {
    LOG("FAIL: Expected final schedule_version=%d, got %d", expected_version, actual_version);
    ok = 0;
  } else {
    LOG("PASS: Final schedule_version correct (%d)", actual_version);
  }
  int violations = get_violation_count();
  if (violations > 0) {
    LOG("FAIL: Detected %d synchronization invariant violations.", violations);
    ok = 0;
  } else {
    LOG("PASS: No critical section violations detected.");
  }
  return ok ? 0 : 1;
}
