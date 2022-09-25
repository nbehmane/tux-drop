/*
 * @file main-loop.h 
 * @brief A Glib wrapper library for main loop and event code. 
 * @brief New location will be in /shared/.
 * @author Nima Behmanesh
 */
#ifndef _MAIN_LOOP_H 
#define _MAIN_LOOP_H

/**** INCLUDES ****/
#include "main-loop-internal.h"

/**** MACROS ****/
/**** STRUCTS ****/
/*
 * @brief An opaque data structure.
 */
typedef struct main_loop_ Loop;


/**** FUNCTIONS ****/
/*
 * @brief Initializes a new main loop with it's own context.
 * @returns A pointer to the new loop.
 */
extern Loop* loop_init(void);

/*
 * @brief Runs the loop. This is a blocking call. Thread safe.
 * @param loop A pointer to a Loop object.
 */
extern gint loop_run(Loop *loop);

/*
 * @brief Quits the loop. 
 * @param loop A pointer to a Loop object.
 */
extern gint loop_quit(Loop *loop);

/*
 * @brief Frees loop resources.
 * @param loop A pointer to a Loop object.
 */
extern void loop_destroy(Loop *loop);
#endif // _MAIN_LOOP_H