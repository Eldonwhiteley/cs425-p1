#include <stdlib.h>
#include <stdio.h>
#include "harness/unity.h"
#include "../src/lab.h"

void setUp(void) {
  printf("Setting up tests...\n");
}

void tearDown(void){
  printf("Tearing down tests...\n");
}

void test_check_status_code(void)
{
  char* status_msg = "220 Good stuff :)";
  char* good_status = "220";
  char* bad_status = "123";
  char* truncated_status = "220030";

  TEST_ASSERT_TRUE(check_status_code(status_msg, good_status));
  TEST_ASSERT_FALSE(check_status_code(status_msg, bad_status));
  TEST_ASSERT_TRUE(check_status_code(status_msg, truncated_status));
}

