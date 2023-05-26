/*  Croquette: A non-floating point library providing a C implementation of a Dictionary.
    Copyright (C) 2023 Kevin Andrea

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

/** @file croquette.c
 * @brief A non-FP based, C Implementation of a Dictionary 
 * - Key: String, Value: Anything
 * - Supports Removal with or without Freeing the Value.
 * - Only a single instance of Croquette is supported.
 * - An optional function to free the value is passed in on creation of the croquette.
 * - The Value will NOT be freed or removed from Croquette on GET.  (Pointer to Value)
 *
 * @author Kevin Andrea (kandrea)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "croquette.h"

/** 
 * @enum croquette_clear_options
 *
 * @brief enum to declare whether or not to free the value on removal
 *
 * This is stored in Croquette definition.
 */
enum croquette_clear_options {
  Croquette_NoFree_Value = 0,     ///< Do not free the value on removal from Croquette.
  Croquette_Free_Value = 1        ///< Free the value on removal from Croquette.
};


/**
 * @struct Carrier_s
 *
 * @brief Structure used for Separate Chaining 
 *
 * This is a node in a doubly-linked list for separate chaining.
 * The key is a String and value is void * to accept any generic usage.
 */
typedef struct carrier_struct {
  char *key;                      ///< Key for Croquette.
  void *value;                    ///< Value for Croquette to Store.
  struct carrier_struct *next;    ///< Next pointer for Separate Chaining.
  struct carrier_struct *prev;    ///< Previous pointer for Separate Chaining.
} Carrier_s;

/**
 * @struct Croquette_s
 *
 * @brief Main Structure for Croquette
 *
 * This provides the definitions needed for Croquette, along with all of the functions
 *   needed to work with the chosen key and value.
 */
typedef struct croquette_struct {
  int do_free;                                      ///< Boolean: Free nodes on removal?
  int size;                                         ///< Number of Keys in Croquette
  int capacity;                                     ///< Number of Indices in Croquette
  int base_capacity;                                ///< Base Number of Indices in Croquette 
  struct carrier_struct **table;                    ///< Vector of Carrier Pointers 
  void (*free_value)(void *value);                  ///< Function to call to free the Value
  int (*value_compare)(void *value1, void *value2); ///< Function to compare two values (v1 vs. v2)
} Croquette_s;

// Macro 'Functions'
#define min(x,y) (x) < (y)?(x):(y)

// Private Globals (Private to this Source File Only)
static Croquette_s *croquette = NULL;
static int croquette_error = 0;

// Strings for the Errors 
static const char *error_str[D_Num_Errors + 1] = {
  [D_No_Error] = "No Croquette Errors Encountered",
  [D_General_Error] = "An Unspecified Croquette Error was Encountered",
  [D_Uninitialized] = "The Croquette was not Initialized Properly",
  [D_Unknown_Error] = "An Unknown Error was Encoutered",
  [D_Invalid_Key] = "The Key is not Valid",
  [D_Entry_NULL] = "The Entry passed in was Null",
  [D_Invalid_Capacity] = "The Capacity Given is not Valid",
  [D_Invalid_Index] = "The Index Given is not Valid",
  [D_No_Value] = "The Value Given is not Valid",
  [D_Insufficient_Memory] = "There was a Memory Error (Insuffient Memory)",
  [D_FreeValue_Missing] = "No Function was Given to Free a Value",
  [D_ValueCompare_Missing] = "No Function was Given to Compare two Values",
  [D_Exists] = "The Croquette Already Exists",
  [D_No_Such_Error] = "No Such Error Exists",
  [D_Num_Errors] = "This is a Code to Hold the Number of Errors"
};

// Internal Prototypes - (Private to this Source File Only)
static Carrier_s *croquette_find(const char *key);
static int perform_rehash(int new_capacity);
static int rehash();
static long hash_code(const char *key);
static Carrier_s *carrier_create(const char *key, void *value);
static int insert_at_index(int index, Carrier_s *entry);
static long get_index(const char *key);
static int is_key(Carrier_s *entry, const char *key);
static int remove_entry(Carrier_s *entry);
static void free_entry(Carrier_s *entry);

/**
 * @brief Initialize a new Croquette
 *
 * Creates a new Croquette to store generic Values with String based Keys.
 * If do_free is True, then free_value is needed.  If not, it should be set to NULL.
 * Rules for Croquette
 * - Doubles when size > (initial_capacity>>1 + initial_capacity>>2)
 * - Halves when size < (initial_capcity>>2);
 * - Resets to initial_capacity on clear()
 *
 * @param initial_capacity Initial Capacity or 0 for Default Capacity for Croquette.
 * @param do_free Croquette_NoFree_Value or Croquette_Free_Value to select if it should free on removal.
 * @param free_value Function to free the value if @p do_free is Croquette_Free_Value.
 * @param value_compare Function to compare values: Returns 0 if equal, <0 if v1 < v2, >0 is v1 > v2
 * @return D_Success on Success
 * @return D_Error on Error (Error String Available)
 */
int croquette_create(int initial_capacity, 
                    int do_free, 
                    void (*free_value)(void *value),
                    int (*value_compare)(void *value1, void *value2)) {
  croquette_set_error(D_No_Error); // Reset Internal error tracker.
  // Only create if it doesn't already exist.
  if(croquette != NULL) {
    croquette_set_error(D_Exists);
    return D_Error;
  }

  // Option to enter 0 (or < 0) to use a default size
  if(initial_capacity <= 0) {
    initial_capacity = CROQUETTE_DEFAULT_INITIAL_SIZE;
  }
  
  // Verify the functions exist as needed.
  if(do_free == Croquette_Free_Value && free_value == NULL) {
    croquette_set_error(D_FreeValue_Missing);
    return D_Error;
  }
  if(value_compare == NULL) {
    croquette_set_error(D_ValueCompare_Missing);
    return D_Error;
  }

  // Allocate and Verify Memory for Symbol Table
  croquette = calloc(1, sizeof(Croquette_s));
  if(croquette == NULL) {
    croquette_set_error(D_Insufficient_Memory);
    return D_Error;
  }

  // Initialize the Memory for the Symbol Table
  // - This is a 1D array of Pointers to Carrier_s objects.
  croquette->table = calloc(initial_capacity, sizeof(Carrier_s *));
  if(croquette->table == NULL) {
    croquette_set_error(D_Insufficient_Memory);
    free(croquette);
    return D_Error;
  }

  // Initialize the remaining Values 
  croquette->do_free = do_free;
  croquette->capacity = initial_capacity;       // Capacity of Indices for Use
  croquette->size = 0;                          // Currently Used Indices
  croquette->base_capacity = initial_capacity;  // Base Capacity of Indices for Use (post Clear)

  // Initialize the remaining Functions
  croquette->free_value = free_value;           // Function to free if do_free is True
  croquette->value_compare = value_compare;     // Function to compare two Values

  return D_Success;
}

/**
 * @brief Checks if Croquette is Empty
 *
 * @return 1 if Empty
 * @return 0 if Not-Empty
 * @return D_Error on Error (Error String Available)
 */
int croquette_isEmpty() {
  croquette_set_error(D_No_Error); // Reset Internal error tracker.
  if(croquette == NULL) {
    croquette_set_error(D_Uninitialized);
    return D_Error;
  }

  return croquette->size==0;
}

/**
 * @brief Gets the number of K,V entries in Croquette
 *
 * @return Size
 * @return D_Error on Error (Error String Available)
 */
int croquette_size() {
  croquette_set_error(D_No_Error);
  if(croquette == NULL) {
    croquette_set_error(D_Uninitialized);
    return D_Error;
  }
  return croquette->size;
}

/**
 * @brief Gets the current number of Indices in Croquette
 *
 * @return Capacity 
 * @return D_Error on Error (Error String Available)
 */
int croquette_capacity() {
  croquette_set_error(D_No_Error);
  if(croquette == NULL) {
    croquette_set_error(D_Uninitialized);
    return D_Error;
  }
  return croquette->capacity;
}

/**
 * @brief Checks if Croquette contains a Key
 *
 * @param key String based key to check
 * @return True if Key Exists
 * @return False if No Such Key
 * @return D_Error on Error (Error String Available)
 */
int croquette_containsKey(const char *key) {
  croquette_set_error(D_No_Error);
  if(croquette == NULL) {
    croquette_set_error(D_Uninitialized);
    return D_Error;
  }
  if(key == NULL || strlen(key) == 0) {
    croquette_set_error(D_Invalid_Key);
    return D_Error;
  }

  return croquette_find(key)!=NULL;
}

/**
 * @brief Gets the value for a given key. Will not Free the Value Returned.
 *
 * @param key String based key to get the value of.
 * @return void *value if Key Exists
 * @return NULL if No Such Key or any Errors (Error String Available)
 */
void *croquette_get(const char *key) {
  croquette_set_error(D_No_Error);
  if(croquette == NULL) {
    croquette_set_error(D_Uninitialized);
    return NULL;
  }
  if(key == NULL || strlen(key) == 0) {
    croquette_set_error(D_Invalid_Key);
    return NULL;
  }

  Carrier_s *entry = croquette_find(key);
  return (entry!=NULL)?entry->value:NULL;
}

/**
 * @brief Finds an entry for a given Key
 *
 * @param key String based key to find.
 * @return Carrier_s *entry if Key Exists
 * @return NULL if No Such Key or any Errors (Error String Available)
 */
static Carrier_s *croquette_find(const char *key) {
  croquette_set_error(D_No_Error);
  if(croquette == NULL) {
    croquette_set_error(D_Uninitialized);
    return NULL;
  }
  if(key == NULL || strlen(key) == 0) {
    croquette_set_error(D_Invalid_Key);
    return NULL;
  }

  int index = get_index(key);
  if(index == D_Error) {
    // croquette_error propagates
    return NULL;
  }

  Carrier_s *walker = croquette->table[index];

  // Uses separate chaining, so no tombstones needed. 
  if(walker == NULL) {
    // No Entries isn't an error. 
    return NULL;
  }

  /* Walk the linked list looking for a match variable name */
  while(walker != NULL) {
    /* If a match is found, report it */
    if(is_key(walker, key)) {
      return walker;
    }
    walker = walker->next;
  }
  return NULL;
}

/**
 * @brief Add a new Value to Croquette by Key
 *
 * @param key String based key to add to the croquette.
 * @param value Generic value to put in to the croquette at the key.
 * @return D_Success on Successful Update/Add
 * @return D_Error on Error (Error String Available)
 */
int croquette_put(const char *key, void *value) {
  croquette_set_error(D_No_Error);
  if(croquette == NULL) {
    croquette_set_error(D_Uninitialized);
    return D_Error;
  }
  if(key == NULL || strlen(key) == 0) {
    croquette_set_error(D_Invalid_Key);
    return D_Error;
  }
  
  /* Try and update the existing value */
  Carrier_s *entry = croquette_find(key);
  if(entry != NULL) {
    /* Check to see if this is a different value (update) */
    if(croquette->value_compare(entry->value, value)) {
      croquette->free_value(entry->value);
      entry->value = value;
    }
    return D_Success;
  }

  /* Entry NULL, so we Need to create a new entry */
  entry = carrier_create(key, value);
  if(entry == NULL) {
    croquette_set_error(D_Insufficient_Memory);
    return D_Error;
  }

  /* Get the hash code and then insert Symbol at the index */
  int index = get_index(entry->key);
  if(index == D_Error) {
    return D_Error;
  }

  insert_at_index(index, entry);

  /* Assess and ReHash if needed */
  int rehash_success = rehash(D_Insert);
  if(rehash_success == D_Error) {
    // Error string will propagate.
    return D_Error;
  }

  return D_Success;
}

/**
 * @brief Assess for a ReHash and ReHash if needed
 *
 * @return D_Success if ReHash not needed or ReHash succeeded.
 * @return D_Error if ReHash was needed and Failed (Error string set).
 */
static int rehash(Croquette_Action_e operation) {
  croquette_set_error(D_No_Error);
  /* Calculate the load and see if a rehash is needed before insert */
  /* - Doubles when new size > (initial_capacity>>1 + initial_capacity>>2) */
  /* - Special Case to handle int division, if new size is capacity (input on capacity = 1), then double */
  int new_capacity = 0;
  switch(operation) {
    case D_Insert:
      if(((croquette->size) > ((croquette->capacity>>1) + (croquette->capacity>>2)) || 
                        (croquette->size) >= croquette->capacity)) {
        new_capacity = croquette->capacity << 1;
      }
      else { // Nothing to do.
        return D_Success;
      }
      break;
    case D_Remove:
      if(croquette->size < (croquette->capacity>>1)) {
         new_capacity = croquette->capacity >> 1;
      }
      else { // Nothing to do.
        return D_Success;
      }
      break;
    default:
      return D_Success;
  }

  int success = perform_rehash(new_capacity);
  if(success == D_Error) {
    return D_Error;
  }
  return D_Success;
}

/**
 * @brief Clears and Frees all Entries in Croquette, Removes Croquette
 * - Always Succeeds (no return)
 */
void croquette_destroy() {
  if(croquette == NULL) {
    return;
  }

  int ret = croquette_clear();
  if(ret == D_Error) {
    return;
  }

  if(croquette != NULL) {
    free(croquette->table);
    free(croquette);
    croquette = NULL;
  }
}

/**
 * @brief Resets Croquette to Initial State (Empty)
 *
 * Clears all entries and resets sizes to initial.
 *
 * @return D_Success on Success
 * @return D_Error on any Failure (Error string set).
 */
int croquette_clear() {
  croquette_set_error(D_No_Error);
  if(croquette == NULL) {
    croquette_set_error(D_Uninitialized);
    return D_Error;
  }

  /* Iterate all Keys and Free Them */
  Carrier_s *walker = NULL;
  Carrier_s *reaper = NULL;
  int i = 0;
  for(i = 0; i < croquette->capacity; i++) {
    walker = croquette->table[i];
    while(walker != NULL) {
      reaper = walker;
      walker = walker->next;
      remove_entry(reaper);
    }
    croquette->table[i] = NULL;
  }

  /* Reset to Base Hash Capacity */
  int ret = perform_rehash(croquette->base_capacity);
  return ret;
}

/**
 * @brief Removes an Entry in Croquette, will Rehash if needed after.
 *
 * Will Remove a given Entry based on its Key
 * - Will only Free the Value if the do_free is set in configuration.
 *
 * @param key String based Key to identify which entry to remove.
 * @return D_Success on Successful Removal (or if no Entry was Present)
 * @return D_Error on any Failure (Error string set).
 */
int croquette_remove(const char *key) {
  croquette_set_error(D_No_Error);
  if(croquette == NULL) {
    croquette_set_error(D_Uninitialized);
    return D_Error;
  }
  if(key == NULL || strlen(key) == 0) {
    croquette_set_error(D_Invalid_Key);
    return D_Error;
  }

  /* If there's no such key, mission accomplished. */
  Carrier_s *entry = croquette_find(key);
  if(entry == NULL) {
    return D_Success;
  } else {
    remove_entry(entry);
  }

  /* Calculate the load and see if a rehash is needed before insert */
  /* - Halves when size < (initial_capacity>>1) */
  int rehash_success = rehash(D_Remove);
  if(rehash_success == D_Error) {
    // Error string will propagate.
    return D_Error;
  }

  return D_Success;
}

/**
 * @brief [Convenience Function] Prints all Keys (and their Indices)
 */
void croquette_print_keys() {
  printf("Keys: \n");
  if(croquette == NULL) {
    return;
  }

  /* Iterate all Indices and Keys, Printing Them */
  int i = 0;
  Carrier_s *walker = NULL;
  for(i = 0; i < croquette->capacity; i++) {
    if(croquette->table[i] != NULL) {
      for(walker = croquette->table[i]; walker != NULL; walker = walker->next) {
        printf("[%2d] %s\n", i, walker->key);
      }  
    }
  }
}

/**
 * @brief Rehashes Croquette to the new Capacity (Larger or Smaller)
 * 
 * @param new_capacity The new capacity for the hash table
 * @return D_Success on Successful Rehash
 * @return D_Error on any Failure (Error string set).
 */
static int perform_rehash(int new_capacity) {
  croquette_set_error(D_No_Error);
  if(croquette == NULL) {
    croquette_set_error(D_Uninitialized);
    return D_Error;
  }
  if(new_capacity <= 0) {
    croquette_set_error(D_Invalid_Capacity);
    return D_Error;
  }

  Carrier_s **new_sable = calloc(new_capacity, sizeof(Carrier_s *));
  if(new_sable == NULL) {
    croquette_set_error(D_Insufficient_Memory);
    return D_Error;
  }

  /* Move Croquette to the new Table, but hold the Old Table */
  Carrier_s **old_sable = croquette->table;
  croquette->table = new_sable;

  int old_capacity = croquette->capacity;
  croquette->capacity = new_capacity;
  croquette->size = 0; // Will all be added back in properly below


  /* Iterate the old table and hash all the values into the new table.
   * Free the old symbols
   */
  Carrier_s *walker = NULL;
  Carrier_s *reaper = NULL;
  int i;
  for(i = 0; i < old_capacity; i++) {
    if(old_sable[i] != NULL) {
      walker = old_sable[i];
      while(walker != NULL) {
        reaper = walker;
        croquette_put(walker->key, walker->value);
        walker = walker->next;
        // The object was moved to the next table, so cut the pointer here first.
        reaper->value = NULL;
        free_entry(reaper);
      }
    }
  }
  free(old_sable);
  return D_Success;
}

/**
 * @brief Computes the Hash Code from a String
 *
 * @param key The String key to compute a Hash Code from
 * @return The Hash Code from the Key
 */
static long hash_code(const char *key) {
  long code = 0;
  int i = 0;
  int size = strlen(key);

  for(i = 0; i < size; i++) {
    code += key[i];
    if(size == 1 || i < (size - 1)) {
      code <<= 7;
    }
  }

  return code;
}

/**
 * @brief Creates a new Carrier entry object
 * 
 * @param key The String Key to add to the new Entry
 * @param value The generic Value to add to the new Entry
 * @return Carrier entry object on Success
 * @return NULL on errors (error string available)
 */
static Carrier_s *carrier_create(const char *key, void *value) {
  croquette_set_error(D_No_Error);
  Carrier_s *entry = calloc(1, sizeof(Carrier_s));
  if(entry == NULL) {
    croquette_set_error(D_Insufficient_Memory);
    return NULL;
  }

  int key_size = min(MAX_KEY_SIZE, strlen(key) + 1);
  entry->key = calloc(1, key_size);
  strncpy((char *)entry->key, key, key_size);
  entry->value = value;
  entry->next = NULL;
  entry->prev = NULL;
  return entry;
}

/**
 * @brief Inserts an Entry at the given Index
 * 
 * @param index The index to insert the Entry into
 * @param entry The entry to insert at that index
 * @return D_Success on Success
 * @return D_Error on Error Condition (Error string available)
 */
static int insert_at_index(int index, Carrier_s *entry) {
  croquette_set_error(D_No_Error);
  if(croquette == NULL) {
    croquette_set_error(D_Uninitialized);
    return D_Error;
  }
  if(index < 0 || index > croquette->capacity) {
    croquette_set_error(D_Invalid_Index);
    return D_Error;
  }

  /* Simple Case, nothing at index, so insert it */
  if(croquette->table[index] == NULL) {
    croquette->table[index] = entry;
  }
  /* Else, iterate to find the tail and insert there */
  else {
    Carrier_s *walker = croquette->table[index];
    while(walker->next != NULL) {
      walker = walker->next;
    }
    walker->next = entry;
    entry->prev = walker;
  }
  croquette->size++;
  return D_Success;
}

/**
 * @brief Gets the index for a Key
 *
 * @param key The String key to generate the Index from
 * @return Hashed Index from the Key
 * @return D_General_Error (Error string available)
 */
static long get_index(const char *key) {
  croquette_set_error(D_No_Error);
  if(croquette == NULL) {
    croquette_set_error(D_Uninitialized);
    return D_General_Error;
  }
    
  if(key == NULL || strlen(key) == 0) {
    croquette_set_error(D_Invalid_Key);
    return D_General_Error;
  }
  
  long code = hash_code(key);

  return code % croquette->capacity;
}

/**
 * @brief Check if the Key matches the Entry's Key
 *
 * The comparison is done via a string comparison up to MAX_KEY_SIZE
 *
 * @param entry The Entry to compare keys against.
 * @param key The String key to compare against the Entry's key.
 * @return True if the Key matches the Entry's Key
 * @return False if the Key does not match the Entry's Key
 */
static int is_key(Carrier_s *entry, const char *key) {
  return !(strncmp(entry->key, key, MAX_KEY_SIZE));
}

/*
 * @brief Removes a Carrier Entry in Croquette and Optionally Frees the Value
 *
 * This function will only free the Value if do_free was configured on Initialization.
 *
 * @param entry The Entry to remove from Croquette
 * @return D_Success on Success
 * @return D_Error on any Error (Error string available)
 */
static int remove_entry(Carrier_s *entry) {
  croquette_set_error(D_No_Error);
  if(entry == NULL) {
    croquette_set_error(D_Entry_NULL);
    return D_Error;
  }

  int index = get_index(entry->key);

  // Case where this is the first item in the Index, simply update the table around it.
  if(entry->prev == NULL) {
    croquette->table[index] = entry->next;
  } 
  // Otherwise, bridge around it forward.
  else {
    entry->prev->next = entry->next;
  }
  // Either way, bridge around it backwards
  if(entry->next) {
    entry->next->prev = entry->prev;
  }
  // Now, free the entry.  (Also frees value if configured to do_free)
  entry->next = NULL;
  free_entry(entry);
  
  // And adjust the croquette size
  croquette->size--;

  return D_Success;
}

/**
 * @brief Free the memory for an Entry
 *
 * Will only free the memory if do_free was configured during Initialization.
 *
 * @param entry The Entry to free
 */
static void free_entry(Carrier_s *entry) {
  if(croquette != NULL && entry != NULL && croquette->do_free == D_Do_Free) {
    croquette->free_value(entry->value);
  }
  if(entry->key) {
    free(entry->key);
  }
  free(entry);
}

/**
 * @brief [Convenience Function] Prints a Description for the given Croquette Error.
 */
void croquette_print_error() {
  if((croquette_error <= D_Num_Errors) && (croquette_error >= 0)) {
    printf("[Croquette Error %2d] %s\n", croquette_error, error_str[croquette_error]);
  }
  else {
    printf("[Croquette] No Such Error\n");
  }
}

/** 
 * @brief Sets a croquette error as a convenience for testing.
 *
 * If the error code is not valid, the code will be set to D_No_Such_Error
 *
 * @param error Provides an error code to set the croquette_error to.
 */
void croquette_set_error(Croquette_Error_Code_e error) {
  if(error < 0 || error > D_Num_Errors) {
    croquette_error = D_No_Such_Error;
  }
  croquette_error = error;
}

/**
 * @brief Returns the integer value of the current Croquette Error State
 *
 * @return The current Croquette error code.
 */
Croquette_Error_Code_e croquette_get_error() {
  return croquette_error;
}

