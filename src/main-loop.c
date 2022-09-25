/*
 * @file main-loop.c 
 * @brief A Glib wrapper library for main loop and event code. 
 * @brief New location will be in /shared/.
 * @author Nima Behmanesh
 */
#include "main-loop-internal.h"

/*
 * @brief Initializes a new main loop with it's own context.
 * @returns A pointer to the new loop.
 */
extern Loop* loop_init(void)
{
   //* Create a new context.
   GMainContext *context = g_main_context_new();
   //* Create a new loop.
   GMainLoop *new_loop = g_main_loop_new(context, FALSE);
   //* Initialize the new Loop.
   Loop *new_loop_obj = (Loop *)malloc(sizeof(Loop));
   new_loop_obj->context = context;
   new_loop_obj->loop = new_loop;
   return new_loop_obj;
}

/*
 * @brief Runs the loop. This is a blocking call. Thread safe.
 * @param loop A pointer to a Loop object.
 */
extern gint loop_run(Loop *loop)
{
   g_main_loop_run(loop->loop);
}

/*
 * @brief Quits the loop. 
 * @param loop A pointer to a Loop object.
 */
extern gint loop_quit(Loop *loop)
{
   g_main_loop_quit(loop->loop);
}

/*
 * @brief Frees loop resources.
 * @param loop A pointer to a Loop object.
 */
extern void loop_destroy(Loop *loop)
{
   // Can't destroy what's not there.
   g_return_if_fail(loop != NULL);
   // Don't destroy it if we're still running. 
   g_return_if_fail(g_main_loop_is_running(loop->loop) == FALSE);

   // Free everything.
   g_main_loop_unref(loop->loop);
   g_main_context_unref(loop->context);
   free(loop);
}