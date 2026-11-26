/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abait-el <abait-el@student.1337.ma>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/09/10 02:27:08 by abait-el          #+#    #+#             */
/*   Updated: 2026/09/10 03:44:28 by abait-el         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# include <pthread.h>
# include <stdbool.h>
# include <stddef.h>
# include <time.h>

/**
 * @file codexion.h
 * @brief Shared types and function declarations for Codexion.
 */

typedef long				t_ms;
typedef unsigned long		t_order;

/** @brief Scheduling policy selected from the command line. */
typedef enum e_scheduler_policy
{
	POLICY_FIFO,
	POLICY_EDF
}							t_scheduler_policy;

/** @brief Immutable command-line configuration. */
typedef struct s_config
{
	int						coder_count;
	t_ms					burnout_ms;
	t_ms					compile_ms;
	t_ms					debug_ms;
	t_ms					refactor_ms;
	long					required_compiles;
	t_ms					cooldown_ms;
	t_scheduler_policy		scheduler;
}							t_config;

# define MAX_DONGLE_WAITERS 2

typedef struct s_coder		t_coder;
typedef struct s_simulation	t_simulation;

/** @brief One coder request stored in both required dongle heaps. */
typedef struct s_request
{
	t_coder					*owner;
	t_order					arrival_order;
	t_ms					burnout_deadline_ms;
}							t_request;

/** @brief Compares two requests according to FIFO or EDF priority. */
typedef int					(*t_request_compare)(const t_request *,
						const t_request *);

/**
 * @brief Small binary min-heap of requests waiting for one dongle.
 *
 * A dongle has only two neighboring coders, therefore two slots are enough.
 */
typedef struct s_request_heap
{
	t_request				items[MAX_DONGLE_WAITERS];
	size_t					count;
	t_request_compare		compare;
}							t_request_heap;

/** @brief Shared dongle state, protected by its own mutex. */
typedef struct s_dongle
{
	int						id;
	pthread_mutex_t			mutex;
	bool					is_held;
	t_ms					available_at_ms;
	t_request_heap			waiters;
}							t_dongle;

/* Core state */

/** @brief Runtime state owned by one coder thread. */
struct						s_coder
{
	int						id;
	t_dongle				*left_dongle;
	t_dongle				*right_dongle;
	bool					has_dongles;
	bool					has_pending_request;
	t_request				pending_request;
	long					compile_count;
	t_ms					burnout_deadline_ms;
	bool					is_finished;
	bool					has_burned_out;
	pthread_t				thread;
	t_simulation			*simulation;
};

/** @brief Shared state and synchronization for one simulation run. */
struct						s_simulation
{
	t_config				config;
	t_dongle				*dongles;
	t_coder					*coders;
	t_ms					started_at_ms;
	t_order					next_request_order;
	bool					is_stopped;
	bool					start_released;
	pthread_mutex_t			log_mutex;
	pthread_mutex_t			state_mutex;
	pthread_cond_t			state_changed;
	pthread_cond_t			sleep_condition;
	pthread_mutex_t			scheduler_mutex;
	pthread_cond_t			scheduler_changed;
	pthread_t				monitor_thread;
};

/* Configuration */

/**
 * @brief Parse and validate all mandatory command-line arguments.
 * @return 0 on success, -1 when an argument is invalid.
 */
int							config_parse(t_config *config, int argc,
								char **argv);

/* Simulation lifecycle */

/** @brief Initialize shared state, resources, and worker threads. */
int							simulation_init(t_simulation *simulation,
								const t_config *config);
/** @brief Wait for the monitor and every coder thread to finish. */
void						simulation_run(t_simulation *simulation);
/** @brief Release all memory and synchronization objects. */
void						simulation_destroy(t_simulation *simulation);
/** @brief Wait until every thread is created and the start time is set. */
void						simulation_wait_for_start(t_simulation *simulation);
/** @brief Return true after the simulation has been stopped. */
bool						simulation_should_stop(t_simulation *simulation);
/** @brief Sleep for a work phase, waking early when the simulation stops. */
bool						simulation_sleep(t_simulation *simulation,
								t_ms duration_ms);

/** @brief Initialize shared mutexes and condition variables. */
int							simulation_init_sync(t_simulation *simulation);
/** @brief Allocate and initialize coder and dongle arrays. */
int							simulation_init_data(t_simulation *simulation);
/** @brief Destroy only the shared synchronization objects. */
void						simulation_destroy_sync(t_simulation *simulation);

/* Worker threads */

/** @brief Thread routine that executes one coder's work cycles. */
void						*coder_thread(void *argument);
/** @brief Thread routine that detects burnout and global completion. */
void						*monitor_thread(void *argument);

/* Scheduler */

/** @brief Wait until both dongles can be granted to the coder together. */
bool						coder_acquire_dongles(t_coder *coder);
/** @brief Release both owned dongles and begin their cooldown. */
void						coder_release_dongles(t_coder *coder);
/** @brief Queue one request in both dongle heaps. */
bool						scheduler_queue_request(t_coder *coder);
/** @brief Return whether both dongles of a request are available now. */
bool						scheduler_pair_is_available(t_request *request,
								t_ms now_ms);
/** @brief Decide whether one dongle may be granted to this coder. */
bool						scheduler_dongle_allows(t_dongle *dongle,
								t_coder *coder, t_ms now_ms);
/** @brief Sleep until a release, request, or cooldown changes scheduling. */
void						scheduler_wait_for_event(t_simulation *simulation);
/** @brief Wake every coder currently waiting in the scheduler. */
void						scheduler_wake_all(t_simulation *simulation);
/** @brief Handle the one-coder case where a pair cannot exist. */
bool						scheduler_wait_for_single(t_coder *coder);

/* Request heap and policy */

/** @brief Initialize an empty two-entry request heap. */
void						request_heap_init(t_request_heap *heap,
								t_request_compare compare);
/** @brief Insert a request while preserving min-heap order. */
bool						request_heap_push(t_request_heap *heap,
								t_request request);
/** @brief Copy the highest-priority request without removing it. */
bool						request_heap_peek(t_request_heap *heap,
								t_request *request);
/** @brief Remove one coder's request from the heap. */
void						request_heap_remove(t_request_heap *heap,
								t_coder *coder);
/** @brief Return the comparator implementing the selected policy. */
t_request_compare			scheduler_comparator(t_scheduler_policy policy);

/* Dongles */

/** @brief Return true when a dongle is free and its cooldown has ended. */
bool						dongle_is_available(t_dongle *dongle, t_ms now_ms);
/** @brief Mark a granted dongle as held. */
void						dongle_acquire(t_dongle *dongle);
/** @brief Release a dongle and set its next available time. */
void						dongle_release(t_dongle *dongle, t_ms cooldown_ms);

/* Utilities */

/** @brief Return the current wall-clock time in milliseconds. */
t_ms						clock_now_ms(void);
/** @brief Return milliseconds elapsed since the simulation started. */
t_ms						simulation_elapsed_ms(t_simulation *simulation);
/** @brief Convert an absolute millisecond value to struct timespec. */
void						timespec_from_ms(struct timespec *timespec,
								t_ms absolute_ms);
/** @brief Print one serialized coder state line while the run is active. */
void						simulation_log(t_simulation *simulation,
								int coder_id, const char *state);
/** @brief Print the final serialized burnout line. */
void						simulation_log_burnout(t_simulation *simulation,
								int coder_id);

#endif
