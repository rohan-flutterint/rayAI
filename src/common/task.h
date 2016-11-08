#ifndef TASK_H
#define TASK_H

/* This API specifies the task data structures. It is in C so we can
 * easily construct tasks from other languages like Python. The datastructures
 * are also defined in such a way that memory is contiguous and all pointers
 * are relative, so that we can memcpy the datastructure and ship it over the
 * network without serialization and deserialization. */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "common.h"
#include "utstring.h"

typedef unique_id function_id;

/** The task ID is a deterministic hash of the function ID that the task
 *  executes and the argument IDs or argument values. */
typedef unique_id task_id;

/** The task instance ID is a globally unique ID generated which identifies this
 *  particular execution of the task. */
typedef unique_id task_iid;

/** The node id is an identifier for the node the task is scheduled on. */
typedef unique_id node_id;

/*
 * ==== Task specifications ====
 * Contain all the information neccessary to execute the
 * task (function id, arguments, return object ids).
 */

typedef struct task_spec_impl task_spec;

/** If argument is passed by value or reference. */
enum arg_type { ARG_BY_REF, ARG_BY_VAL };

/**
 * Compare two task IDs.
 *
 * @param first_id The first task ID to compare.
 * @param second_id The first task ID to compare.
 * @return True if the task IDs are the same and false otherwise.
 */
bool task_ids_equal(task_id first_id, task_id second_id);

/**
 * Compare two function IDs.
 *
 * @param first_id The first function ID to compare.
 * @param second_id The first function ID to compare.
 * @return True if the function IDs are the same and false otherwise.
 */
bool function_ids_equal(function_id first_id, function_id second_id);

/* Construct and modify task specifications. */

/**
 * Begin constructing a task_spec. After this is called, the arguments must be
 * added to the task_spec and then finish_construct_task_spec must be called.
 *
 * @param parent_task_id The task ID of the task that submitted this task.
 * @param parent_counter A counter indicating how many tasks were submitted by
 *        the parent task prior to this one.
 * @param function_id The function ID of the function to execute in this task.
 * @param num_args The number of arguments that this task has.
 * @param num_returns The number of return values that this task has.
 * @param args_value_size The total size in bytes of the arguments to this task
          ignoring object ID arguments.
 * @return The partially constructed task_spec.
 */
task_spec *start_construct_task_spec(task_id parent_task_id,
                                     int64_t parent_counter,
                                     function_id function_id,
                                     int64_t num_args,
                                     int64_t num_returns,
                                     int64_t args_value_size);

/**
 * Finish constructing a task_spec. This computes the task ID and the object IDs
 * of the task return values. This must be called after all of the arguments
 * have been added to the task.
 *
 * @param spec The task spec whose ID and return object IDs should be computed.
 * @return Void.
 */
void finish_construct_task_spec(task_spec *spec);

/**
 * The size of the task in bytes.
 *
 * @param spec The task_spec in question.
 * @return The size of the task_spec in bytes.
 */
int64_t task_size(task_spec *spec);

/**
 * Return the function ID of the task.
 *
 * @param spec The task_spec in question.
 * @return The function ID of the function to execute in this task.
 */
function_id task_function(task_spec *spec);

/**
 * Return the task ID of the task.
 *
 * @param spec The task_spec in question.
 * @return The task ID of the task.
 */
task_id task_task_id(task_spec *spec);

/**
 * Get the number of arguments to this task.
 *
 * @param spec The task_spec in question.
 * @return The number of arguments to this task.
 */
int64_t task_num_args(task_spec *spec);

/**
 * Get the number of return values expected from this task.
 *
 * @param spec The task_spec in question.
 * @return The number of return values expected from this task.
 */
int64_t task_num_returns(task_spec *spec);

/**
 * Get the type of an argument to this task. It should be either ARG_BY_REF or
 * ARG_BY_VAL.
 *
 * @param spec The task_spec in question.
 * @param arg_index The index of the argument in question.
 * @return The type of the argument.
 */
int8_t task_arg_type(task_spec *spec, int64_t arg_index);

/**
 * Get a particular argument to this task. This assumes the argument is an
 * object ID.
 *
 * @param spec The task_spec in question.
 * @param arg_index The index of the argument in question.
 * @return The argument at that index.
 */
object_id task_arg_id(task_spec *spec, int64_t arg_index);

/**
 * Get a particular argument to this task. This assumes the argument is a value.
 *
 * @param spec The task_spec in question.
 * @param arg_index The index of the argument in question.
 * @return The argument at that index.
 */
uint8_t *task_arg_val(task_spec *spec, int64_t arg_index);

/**
 * Get the number of bytes in a particular argument to this task. This assumes
 * the argument is a value.
 *
 * @param spec The task_spec in question.
 * @param arg_index The index of the argument in question.
 * @return The number of bytes in the argument.
 */
int64_t task_arg_length(task_spec *spec, int64_t arg_index);

/**
 * Set the next task argument. Note that this API only allows you to set the
 * arguments in their order of appearance.
 *
 * @param spec The task_spec in question.
 * @param The object ID to set the argument to.
 * @return The number of task arguments that have been set before this one. This
 *         is only used for testing.
 */
int64_t task_args_add_ref(task_spec *spec, object_id obj_id);

/**
 * Set the next task argument. Note that this API only allows you to set the
 * arguments in their order of appearance.
 *
 * @param spec The task_spec in question.
 * @param The value to set the argument to.
 * @param The length of the value to set the argument to.
 * @return The number of task arguments that have been set before this one. This
 *         is only used for testing.
 */
int64_t task_args_add_val(task_spec *spec, uint8_t *data, int64_t length);

/**
 * Get a particular return object ID of a task.
 *
 * @param spec The task_spec in question.
 * @param return_index The index of the return object ID in question.
 * @return The relevant return object ID.
 */
object_id task_return(task_spec *spec, int64_t return_index);

/**
 * Free a task_spec.
 *
 * @param The task_spec in question.
 * @return Void.
 */
void free_task_spec(task_spec *spec);

/**
 * Print the task as a humanly readable string.
 *
 * @param spec The task_spec in question.
 * @param output The buffer to write the string to.
 * @return Void.
 */
void print_task(task_spec *spec, UT_string *output);

/*
 * ==== Task instance ====
 * Contains information about a scheduled task: The task iid,
 * the task specification and the task status (WAITING, SCHEDULED,
 * RUNNING, DONE) and which node the task is scheduled on.
 */

/** The scheduling_state can be used as a flag when we are listening
 *  for an event, for example TASK_WAITING | TASK_SCHEDULED. */
enum scheduling_state {
  TASK_STATUS_WAITING = 1,
  TASK_STATUS_SCHEDULED = 2,
  TASK_STATUS_RUNNING = 4,
  TASK_STATUS_DONE = 8
};

/** A task instance is one execution of a task specification. It has a unique
 *  instance id, a state of execution (see scheduling_state) and a node it is
 *  scheduled on or running on. */
typedef struct task_instance_impl task_instance;

/* Allocate and initialize a new task instance. Must be freed with
 * scheduled_task_free after use. */
task_instance *make_task_instance(task_iid task_iid,
                                  task_spec *task,
                                  int32_t state,
                                  node_id node);

/* Size of task instance structure in bytes. */
int64_t task_instance_size(task_instance *instance);

/* Instance ID of the task instance. */
task_iid *task_instance_id(task_instance *instance);

/* The scheduling state of the task instance. */
int32_t *task_instance_state(task_instance *instance);

/* Node this task instance has been assigned to or is running on. */
node_id *task_instance_node(task_instance *instance);

/* Task specification of this task instance. */
task_spec *task_instance_task_spec(task_instance *instance);

/* Free this task instance datastructure. */
void task_instance_free(task_instance *instance);

/*
 * ==== Task update ====
 * Contains the information necessary to update a task in the task log.
 */

typedef struct {
  int32_t state;
  node_id node;
} task_update;

#endif
