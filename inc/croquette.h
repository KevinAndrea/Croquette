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

/** @file croquette.h
 * @brief This is an Integer-Only implementation of a generic Dictionary in C
 *
 * @author Kevin Andrea (kandrea)
 */

#ifndef CROQUETTE_H
#define CROQUETTE_H

// Default Values
#define CROQUETTE_DEFAULT_INITIAL_SIZE 11
#define MAX_KEY_SIZE 255    // Max characters per Key

typedef enum croquette_action {
  C_Insert = 0,
  C_Remove = 1
} Croquette_Action_e;

enum croquette_returns {
  C_Error = -1,
  C_Success = 0,
};

enum croquette_defaults {
  C_Default_Capacity = 0
};

enum croquette_dofree {
  C_No_Free = 0,
  C_Do_Free = 1,
};

typedef enum croquette_error_codes {
  C_No_Error = 0,
  C_General_Error,
  C_Uninitialized,
  C_Unknown_Error,
  C_Invalid_Key,
  C_Invalid_Value,
  C_Entry_NULL,
  C_Invalid_Capacity,
  C_Invalid_Index,
  C_No_Value,
  C_Insufficient_Memory,
  C_FreeValue_Missing,
  C_ValueCompare_Missing,
  C_Exists,
  C_No_Such_Error,
  C_Num_Errors
} Croquette_Error_Code_e;


// Shared Prototypes
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
 * @return C_Success on Success
 * @return C_Error on Error (Error String Available)
 */
int croquette_create(int initial_capacity, 
                    int do_free, 
                    void (*free_value)(void *value),
                    int (*value_compare)(const void *value1, const void *value2));
/**
 * @brief Checks if Croquette is Empty
 *
 * @return 1 if Empty
 * @return 0 if Not-Empty
 * @return C_Error on Error (Error String Available)
 */
int croquette_isEmpty();
/**
 * @brief Gets the number of K,V entries in Croquette
 *
 * @return Size
 * @return C_Error on Error (Error String Available)
 */
int croquette_size();
/**
 * @brief Gets the current number of Indices in Croquette
 *
 * @return Capacity 
 * @return C_Error on Error (Error String Available)
 */
int croquette_capacity();
/**
 * @brief Checks if Croquette contains a Key
 *
 * @param key String based key to check
 * @return True if Key Exists
 * @return False if No Such Key
 * @return C_Error on Error (Error String Available)
 */
int croquette_containsKey(const char *key);
/**
 * @brief Checks if Croquette contains a Value
 *
 * @param key Value to check for.
 * @return True if Value Exists
 * @return False if No Such Value
 * @return C_Error on Error (Error String Available)
 */
int croquette_containsValue(void *value);
/**
 * @brief Gets the value for a given key. Will not Free the Value Returned.
 *
 * @param key String based key to get the value of.
 * @return void *value if Key Exists
 * @return NULL if No Such Key or any Errors (Error String Available)
 */
void *croquette_get(const char *key);
/**
 * @brief Gets the value for a given key. Will not Free the Value Returned.
 *
 * @param key String based key to get the value of.
 * @return void *value if Key Exists
 * @return default_value if No Such Key
 * @return NULL on any Errors (Error String Available)
 */
void *croquette_getOrDefault(const char *key, void *default_value);
/**
 * @brief Add a new Value to Croquette by Key
 *
 * @param key String based key to add to the croquette.
 * @param value Generic value to put in to the croquette at the key.
 * @return C_Success on Successful Update/Add
 * @return C_Error on Error (Error String Available)
 */
int croquette_put(const char *key, void *value);
/**
 * @brief Add a new Value to Croquette by Key only if Key has no Value
 *
 * @param key String based key to add to the croquette.
 * @param value Generic value to put in to the croquette at the key.
 * @return NULL if key did not exist and value was added. 
 * @return value if key did exist, existing value is returned.
 */
void *croquette_putIfAbsent(const char *key, void *value);
/**
 * @brief Assess for a ReHash and ReHash if needed
 *
 * @return C_Success if ReHash not needed or ReHash succeeded.
 * @return C_Error if ReHash was needed and Failed (Error string set).
 */
void croquette_destroy();
/**
 * @brief Resets Croquette to Initial State (Empty)
 *
 * Clears all entries and resets sizes to initial.
 *
 * @return C_Success on Success
 * @return C_Error on any Failure (Error string set).
 */
int croquette_clear();
/**
 * @brief Removes an Entry in Croquette, will Rehash if needed after.
 *
 * Will Remove a given Entry based on its Key
 * - Will only Free the Value if the do_free is set in configuration.
 *
 * @param key String based Key to identify which entry to remove.
 * @return C_Success on Successful Removal (or if no Entry was Present)
 * @return C_Error on any Failure (Error string set).
 */
int croquette_remove(const char *key);
/**
 * @brief [Convenience Function] Prints all Keys (and their Indices)
 */
void croquette_print_keys();

/**
 * @brief [Convenience Function] Prints a Description for the given Croquette Error.
 */
void croquette_print_error();
/** 
 * @brief Sets a croquette error as a convenience for testing.
 *
 * If the error code is not valid, the code will be set to C_No_Such_Error
 *
 * @param error Provides an error code to set the croquette_error to.
 */
void croquette_set_error(Croquette_Error_Code_e error);
/**
 * @brief Returns the integer value of the current Croquette Error State
 *
 * @return The current Croquette error code.
 */
Croquette_Error_Code_e croquette_get_error();

#endif
