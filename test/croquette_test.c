/** @file croquette_test.c
 * @brief Unit Tester for the Croquette Library
 * - Basic Functionality Testing of All Functions
 * - This file also provides examples (via unit tests) of the use of Croquette
 *
 * @author Kevin Andrea (kandrea)
 * - Copyright Kevin Andrea - 2023
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "croquette.h"

// Testing Data
static int test_number = 0; // Simple tracker of Test Number
enum test_results { Test_Success = 0, Test_Failure };

#define MAX_NAME_LEN 50

// Test Support Functions
static void test_start(const char *);
static void test_comment(const char *message);
static void test_end(int success);

// Testing Prototypes
static int test_print_errors();
static int test_uninitialized_croquette();
static int test_invalid_key();
static int test_croquette_create();
static int test_empty_croquette();
static int test_croquette_put();
static int test_croquette_remove_dofree();
static int test_croquette_remove_nofree();
static int test_croquette_clear();

// Testing Struct Definitions
/**
 * @struct Element_s
 *
 * @brief Sample Data Struct for Testing
 *
 * This provides a simple struct showing what can be stored in the Dictionary.
 */
typedef struct element {
  char name[MAX_NAME_LEN];
  int value;
} Element_s;

// Testing Function Definitions
/**
 * @brief Function to free the element; pass into Croquette via create.
 *
 * @return void
 */
static void free_elem(void *elem) {
  if(elem) {
    free(elem);
  }
}

/**
 * @brief Function to compare two elements; pass into Croquette via create.
 *
 * @return int (<0 if e1 < e2, 0 if e1 == e2, >0 if e1 > e2)
 */
static int compare_elem(const void *elem1, const void *elem2) {
  Element_s *e1 = (Element_s *)elem1;
  Element_s *e2 = (Element_s *)elem2;
  return(e1->value - e2->value);
}

/**
 * @brief Function to create an element for testing purposes.
 * - Element uses dynamic memory, must be freed.
 *
 * @return Element_s pointer.
 */
static Element_s *create_elem(char *name, int val) {
  Element_s *elem = calloc(1, sizeof(Element_s));
  if(elem == NULL) {
    return NULL;
  }
  strncpy(elem->name, name, MAX_NAME_LEN);
  elem->value = val;
  return elem;
}

/**
 * @brief main Function to run the Unit Tests on Croquette
 *
 * @return EXIT_SUCCESS on successful execution.
 */
int main() {
  int ret = 0;

  printf("Beginning Croquette Tests...\n");
  //------[TESTING BEGIN]---------------------------------
  test_start("Printing All Error Codes");
  ret = test_print_errors();
  test_end(ret);

  test_start("Testing Uninitialized Croquette Checks");
  ret = test_uninitialized_croquette();
  test_end(ret);

  test_start("Testing Invalid Key");
  ret = test_invalid_key();
  test_end(ret);

  test_start("Testing Croquette Creation");
  ret = test_croquette_create();
  test_end(ret);
  
  test_start("Testing Empty Croquette");
  ret = test_empty_croquette();
  test_end(ret);

  test_start("Testing Puts and Verification with Gets");
  ret = test_croquette_put();
  test_end(ret);

  test_start("Testing Removes (do_free set) and Verification with Gets");
  ret = test_croquette_remove_dofree();
  test_end(ret);

  test_start("Testing Removes (no_free set) and Verification with Gets");
  ret = test_croquette_remove_nofree();
  test_end(ret);

  test_start("Testing Clear");
  ret = test_croquette_clear();
  test_end(ret);

  return EXIT_SUCCESS;
}

/**
 * @brief Function to start a unit test with a message.
 * - Increments the test_number to start a new test.  First test will be 1.
 *
 * @return void
 */
static void test_start(const char *message) {
  test_number++; // Increment to allow for next test number to be loaded.
  printf("[Test %2d] %s\n", test_number, message);
  printf(".======================\n");
}

/**
 * @brief Function to add a comment in line during a test.
 *
 * @return void
 */
static void test_comment(const char *message) {
  printf("| - %s\n", message);
}

/**
 * @brief Function to end a test with a comment based on success status.
 *
 * @return void
 */
static void test_end(int success) {
  printf("|-----------------------\n");
  if(success == Test_Success) {
    printf("| All Checks Passed\n");
  } else {
    printf("| Failure\n");
  }
  printf("\\______________________\n\n");
}

/**
 * @brief Function to Print out all Error Messages
 *
 * @return Test_Success (always succeeds)
 */
static int test_print_errors() {
  // Test Setup
  int i =  0;

  // Testing
  for(i = 0; i < C_Num_Errors; i++) {
    croquette_set_error(i);
    printf("| ");
    croquette_print_error();
  }

  // Test Teardown
  return Test_Success;
}

/**
 * @brief Function to Test Functions with Uninitialized Croquette
 *
 * @return Test_Success or Test_Failure
 */
static int test_uninitialized_croquette() {
  // Test Setup
  int ret = 0;
  Element_s *elem = NULL;
  Element_s *test_elem = create_elem("a", 42);

  // Testing
  test_comment("Checking all functions without Creating a Croquette");
  ret = croquette_isEmpty();
  assert(ret == C_Error && croquette_get_error() == C_Uninitialized);
  ret = croquette_size();
  assert(ret == C_Error && croquette_get_error() == C_Uninitialized);
  ret = croquette_capacity();
  assert(ret == C_Error && croquette_get_error() == C_Uninitialized);
  ret = croquette_containsKey("aaa");
  assert(ret == C_Error && croquette_get_error() == C_Uninitialized);
  elem = croquette_get("aaa");
  assert(elem == NULL && ret == C_Error && croquette_get_error() == C_Uninitialized);
  ret = croquette_put("aaa", test_elem);
  assert(ret == C_Error && croquette_get_error() == C_Uninitialized);
  croquette_destroy(); // Will succeed, but adding this for coverage testing
  ret = croquette_clear();
  assert(ret == C_Error && croquette_get_error() == C_Uninitialized);
  ret = croquette_remove("aaa");
  assert(ret == C_Error && croquette_get_error() == C_Uninitialized);

  assert(croquette_is_error() != 0);
  croquette_clear_error();
  assert(croquette_is_error() == 0);

  // Test Teardown
  free_elem(test_elem);
  return Test_Success;
}


/**
 * @brief Function to Test Functions with an Invalid Key
 *
 * @return Test_Success or Test_Failure
 */
static int test_invalid_key() {
  // Test Setup
  int ret = 0;
  Element_s *elem = NULL;
  Element_s *test_elem = create_elem("a", 42);

  croquette_create(0, C_Do_Free, free_elem, compare_elem);

  // Testing
  test_comment("Testing all relevant functions with an empty key");
  ret = croquette_containsKey("");
  assert(ret == C_Error && croquette_get_error() == C_Invalid_Key);
  elem = croquette_get("");
  assert(elem == NULL && ret == C_Error && croquette_get_error() == C_Invalid_Key);
  ret = croquette_put("", test_elem);
  assert(ret == C_Error && croquette_get_error() == C_Invalid_Key);
  ret = croquette_remove("");
  assert(ret == C_Error && croquette_get_error() == C_Invalid_Key);
  test_comment("Testing all relevant functions with a NULL key");
  ret = croquette_containsKey(NULL);
  assert(ret == C_Error && croquette_get_error() == C_Invalid_Key);
  elem = croquette_get(NULL);
  assert(elem == NULL && ret == C_Error && croquette_get_error() == C_Invalid_Key);
  ret = croquette_put(NULL, test_elem);
  assert(ret == C_Error && croquette_get_error() == C_Invalid_Key);
  ret = croquette_remove(NULL);
  assert(ret == C_Error && croquette_get_error() == C_Invalid_Key);

  // Test Teardown
  croquette_destroy();
  free_elem(test_elem);
  return Test_Success;
}

/**
 * @brief Function to Test croquette_create()
 *
 * @return Test_Success or Test_Failure
 */
static int test_croquette_create() {
  // Test Setup
  int ret = 0;
  int cap = 1;

  // Testing
  test_comment("Creating a Croquette with Null Free Value and Do_Free (Error)");
  ret = croquette_create(C_Default_Capacity, C_Do_Free, NULL, compare_elem);
  assert(ret == C_Error && croquette_get_error() == C_FreeValue_Missing);
  assert(croquette_is_error() != 0);

  test_comment("Creating a Croquette with Null Free Value and No_Free");
  ret = croquette_create(C_Default_Capacity, C_No_Free, NULL, compare_elem);
  assert(ret == C_Success);
  assert(croquette_is_error() == 0);
  croquette_destroy();

  test_comment("Creating a Croquette with NULL Compare Value (Error)");
  ret = croquette_create(C_Default_Capacity, C_Do_Free, free_elem, NULL);
  assert(ret == C_Error && croquette_get_error() == C_ValueCompare_Missing);

  test_comment("Creating a Croquette with 0 Initial Size (Default Test)");
  ret = croquette_create(C_Default_Capacity, C_Do_Free, free_elem, compare_elem);
  assert(ret == C_Success && croquette_capacity() == CROQUETTE_DEFAULT_INITIAL_SIZE);

  test_comment("Creating a Croquette with Existing Croquette");
  ret = croquette_create(C_Default_Capacity, C_Do_Free, free_elem, compare_elem);
  assert(ret == C_Error && croquette_get_error() == C_Exists);
  croquette_destroy();

  test_comment("Creating a Croquette with Proper Free Value and Do_Free");
  ret = croquette_create(cap, C_Do_Free, free_elem, compare_elem);
  assert(ret == C_Success);

  test_comment("Verifying Size and Capacity");
  ret = croquette_size();
  assert(ret == 0);
  ret = croquette_capacity();
  assert(ret == cap);

  // Test Teardown
  croquette_destroy();
  return Test_Success;
}

/**
 * @brief Function to Test Croquette Functions when No Keys Stored
 *
 * @return Test_Success or Test_Failure
 */
static int test_empty_croquette() {
  // Test Setup
  int ret = 0;
  Element_s *elem = NULL;
  Element_s *def = create_elem("Default", 42);

  ret = croquette_create(1, C_Do_Free, free_elem, compare_elem);

  // Testing
  test_comment("Checking isEmpty on Empty Croquette");
  ret = croquette_isEmpty();
  assert(ret == 1);

  test_comment("Checking For a Key on Empty Croquette");
  ret = croquette_containsKey("aaa");
  assert(ret == 0 && croquette_get_error() == C_No_Error && !croquette_is_error());

  test_comment("Checking For a Key (get) on Empty Croquette");
  elem = croquette_get("aaa");
  assert(elem == NULL && croquette_get_error() == C_No_Error);

  test_comment("Checking For a Default Value (getOrDefault) on Empty Croquette");
  elem = croquette_getOrDefault("aaa", def);
  assert(elem == def && croquette_get_error() == C_No_Error);

  // Test Teardown
  croquette_destroy();
  free(def);
  return Test_Success;
}

/**
 * @brief Function to Test croquette_put()
 *
 * @return Test_Success or Test_Failure
 */
static int test_croquette_put() {
  // Test Setup
  Element_s *a = create_elem("aaa", 21);
  Element_s *b = create_elem("bee", 22);
  Element_s *c = create_elem("cee", 23);
  Element_s *d = create_elem("dee", 24);
  Element_s *e = create_elem("eee", 25);
  Element_s *f = create_elem("eff", 26);
  Element_s *g = create_elem("gee", 27);
  Element_s *u_b = create_elem("bee", 1337);

  Element_s *elem = NULL;
  int success = 0;

  // Create Croquette with Capacity of 1 (will force doubling)
  croquette_create(1, C_Do_Free, free_elem, compare_elem);

  // Testing
  test_comment("Single Value Find (aaa)");
  success = croquette_containsValue(a);
  assert(success == 0);

  test_comment("Single Key Put (aaa) -- Should Double Cap from 1 -> 2");
  croquette_put(a->name, a);
  assert(croquette_size() == 1);
  assert(croquette_capacity() == 2);

  test_comment("Single Key Put (bee) -- Should Double Cap from 2 -> 4");
  croquette_put(b->name, b);
  assert(croquette_size() == 2);
  assert(croquette_capacity() == 4);

  test_comment("Duplicate Key Put (bee) -- Should Do Nothing");
  croquette_put(b->name, b);
  assert(croquette_size() == 2);
  assert(croquette_capacity() == 4);

  test_comment("Single Key Put (cee) -- Should Not Double Cap (only when greater than 75%)");
  croquette_put(c->name, c);
  assert(croquette_size() == 3);
  assert(croquette_capacity() == 4);

  test_comment("Single Key Put (dee) -- Should Double Cap from 4 -> 8");
  croquette_put(d->name, d);
  assert(croquette_size() == 4);
  assert(croquette_capacity() == 8);

  test_comment("Single Key Put (eee) -- Should Be Cap at 8");
  croquette_put(e->name, e);
  assert(croquette_size() == 5);
  assert(croquette_capacity() == 8);

  test_comment("Single Key Put (eff) -- Should Be Cap at 8");
  croquette_put(f->name, f);
  assert(croquette_size() == 6);
  assert(croquette_capacity() == 8);

  test_comment("Single Key PutIfAbsent (gee) -- Should Double Cap to 16");
  elem = croquette_putIfAbsent(g->name, g);
  assert(elem == NULL);
  assert(croquette_size() == 7);
  assert(croquette_capacity() == 16);

  test_comment("Single Key PutIfAbsent (gee) -- Should do Nothing (key exists)");
  elem = croquette_putIfAbsent(g->name, g);
  assert(elem == g);
  assert(croquette_size() == 7);
  assert(croquette_capacity() == 16);

  test_comment("Single Key Verify (eee)");
  elem = croquette_get("eee");
  assert(elem == e);
  assert(elem->value == 25);

  test_comment("Single Key Verify (aaa)");
  elem = croquette_get("aaa");
  assert(elem == a);
  assert(elem->value == 21);
  
  test_comment("Single Value Find (aaa)");
  success = croquette_containsValue(a);
  assert(success > 0);

  test_comment("Checking Key to for Original Value (bee)");
  elem = croquette_get("bee");
  assert(elem == b);
  assert(elem->value == 22);

  test_comment("Updating Key to new Value (bee)");
  croquette_put("bee", u_b);
  assert(croquette_size() == 7);
  assert(croquette_capacity() == 16);

  test_comment("Checking Key to for Updated Value (bee)");
  elem = croquette_get("bee");
  assert(elem == u_b);
  assert(elem->value == 1337);

  test_comment("Testing Print Keys");
  croquette_print_keys();

  // Test Teardown
  croquette_destroy();

  return Test_Success;
}

/**
 * @brief Function to Test croquette_remove_dofree()
 *
 * @return Test_Success or Test_Failure
 */
static int test_croquette_remove_dofree() {
  // Test Setup
  Element_s *a = create_elem("aaa", 21);
  Element_s *b = create_elem("bee", 22);
  Element_s *c = create_elem("cee", 23);
  Element_s *d = create_elem("dee", 24);
  Element_s *e = create_elem("eee", 25);
  Element_s *u_b = create_elem("bee", 26);

  int ret = 0;

  // Create Initital Croquette with keys: [2] bee->cee, [5] dee->eee, [7] aaa
  croquette_create(1, C_Do_Free, free_elem, compare_elem);
  assert(ret == C_Success);
  croquette_put(a->name, a);
  croquette_put(b->name, b);
  croquette_put(c->name, c);
  croquette_put(d->name, d);
  croquette_put(e->name, e);

  // Testing
  test_comment("Adding keys a, b, c, d, e.  Checking Size and Capacity");
  assert(croquette_size() == 5);
  assert(croquette_capacity() == 8);

  test_comment("Single Key Remove - Only (aaa)");
  ret = croquette_remove("aaa");
  assert(!croquette_containsKey("aaa"));
  assert(croquette_size() == 4);
  assert(croquette_capacity() == 8);
  assert(ret == C_Success);

  test_comment("Remove of Already Removed Key - (aaa)");
  ret = croquette_remove("aaa");
  assert(!croquette_containsKey("aaa"));
  assert(croquette_size() == 4);
  assert(croquette_capacity() == 8);
  assert(ret == C_Success);

  test_comment("Single Key Remove - First (bee)");
  ret = croquette_remove("bee");
  assert(!croquette_containsKey("bee"));
  assert(croquette_containsKey("cee"));
  assert(croquette_size() == 3);
  assert(croquette_capacity() == 4);
  assert(ret == C_Success);

  test_comment("Single Key Remove - Last (eee)");
  ret = croquette_remove("eee");
  assert(!croquette_containsKey("eee"));
  assert(croquette_containsKey("dee"));
  assert(croquette_size() == 2);
  assert(croquette_capacity() == 4);
  assert(ret == C_Success);

  test_comment("Adding in a new bee");
  croquette_put(u_b->name, u_b);
  assert(croquette_containsKey("bee"));
  assert(croquette_containsKey("cee"));
  assert(croquette_size() == 3);
  assert(croquette_capacity() == 4);

  croquette_print_keys();

  // Test Teardown
  croquette_destroy();
  return Test_Success;
}

/**
 * @brief Function to Test croquette_remove_nofree()
 *
 * @return Test_Success or Test_Failure
 */
static int test_croquette_remove_nofree() {
  // Test Setup
  Element_s *a = create_elem("aaa", 21);
  Element_s *b = create_elem("bee", 22);
  Element_s *c = create_elem("cee", 23);
  Element_s *d = create_elem("dee", 24);
  Element_s *e = create_elem("eee", 25);
  Element_s *u_b = create_elem("bee", 26);

  int ret = 0;

  // Create Initital Croquette with keys: [2] bee->cee, [5] dee->eee, [7] aaa
  ret = croquette_create(1, C_No_Free, NULL, compare_elem);
  assert(ret == C_Success);
  croquette_put(a->name, a);
  croquette_put(b->name, b);
  croquette_put(c->name, c);
  croquette_put(d->name, d);
  croquette_put(e->name, e);

  // Testing
  test_comment("Adding keys a, b, c, d, e.  Checking Size and Capacity");
  assert(croquette_size() == 5);
  assert(croquette_capacity() == 8);

  test_comment("Single Key Remove - Only (aaa)");
  ret = croquette_remove("aaa");
  assert(!croquette_containsKey("aaa"));
  assert(croquette_size() == 4);
  assert(croquette_capacity() == 8);
  assert(ret == C_Success);

  test_comment("Remove of Already Removed Key - (aaa)");
  ret = croquette_remove("aaa");
  assert(!croquette_containsKey("aaa"));
  assert(croquette_size() == 4);
  assert(croquette_capacity() == 8);
  assert(ret == C_Success);

  test_comment("Single Key Remove - First (bee)");
  ret = croquette_remove("bee");
  assert(!croquette_containsKey("bee"));
  assert(croquette_containsKey("cee"));
  assert(croquette_size() == 3);
  assert(croquette_capacity() == 4);
  assert(ret == C_Success);

  test_comment("Single Key Remove - Last (eee)");
  ret = croquette_remove("eee");
  assert(!croquette_containsKey("eee"));
  assert(croquette_containsKey("dee"));
  assert(croquette_size() == 2);
  assert(croquette_capacity() == 4);
  assert(ret == C_Success);

  test_comment("Adding in a new bee");
  croquette_put(u_b->name, u_b);
  assert(croquette_containsKey("bee"));
  assert(croquette_containsKey("cee"));
  assert(croquette_size() == 3);
  assert(croquette_capacity() == 4);

  croquette_print_keys();

  // Test Teardown
  croquette_destroy();
  free(a);
  free(b);
  free(c);
  free(d);
  free(e);
  free(u_b);
  
  return Test_Success;
}

/**
 * @brief Function to Test croquette_clear()
 *
 * @return Test_Success or Test_Failure
 */
static int test_croquette_clear() {
  // Test Setup
  Element_s *a = create_elem("aaa", 42);
  Element_s *b = create_elem("bee", 24);
  Element_s *c = create_elem("cee", 22);
  Element_s *d = create_elem("dee", 24);
  Element_s *e = create_elem("eee", 25);
  Element_s *u_b = create_elem("bee", 26);

  int ret = 0;

  // Create Initital Croquette with keys: [2] bee->cee, [5] dee->eee, [7] aaa
  ret = croquette_create(1, C_Do_Free, free_elem, compare_elem);
  assert(ret == C_Success);

  croquette_put(a->name, a);
  croquette_put(b->name, b);
  croquette_put(c->name, c);
  croquette_put(d->name, d);
  croquette_put(e->name, e);

  // Testing
  test_comment("Testing Clear");
  ret = croquette_clear();
  assert(ret == C_Success);

  assert(!croquette_containsKey("aaa"));
  assert(!croquette_containsKey("bee"));
  assert(!croquette_containsKey("cee"));
  assert(!croquette_containsKey("dee"));
  assert(!croquette_containsKey("eee"));

  assert(croquette_size() == 0);
  assert(croquette_capacity() == 1);

  test_comment("Testing Put Following Clear");
  croquette_put(u_b->name, u_b);
  assert(croquette_size() == 1);
  assert(croquette_capacity() == 2);

  // Test Teardown
  croquette_destroy();
  return Test_Success;
}

