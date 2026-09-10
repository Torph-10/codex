/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   codexion.h                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ase <ase@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 12:36:40 by ase               #+#    #+#             */
/*   Updated: 2026/09/08 16:08:19 by ase              ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CODEXION_H
# define CODEXION_H

# define SCHED_FIFO 1
# define SCHED_EDF 0

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <pthread.h>
# include <unistd.h>
# include <sys/time.h>

typedef struct s_simulation	t_simulation;
typedef struct s_coder		t_coder;
typedef struct s_dongle		t_dongle;
typedef struct s_config		t_config;
typedef struct s_request	t_request;
typedef struct s_heap		t_heap;

typedef struct s_config
{
	long	number_of_coders;
	long	time_to_burnout;
	long	time_to_compile;
	long	time_to_debug;
	long	time_to_refactor;
	long	number_of_compiles_required;
	long	dongle_cooldown;
	long	scheduler;
}	t_config;

typedef struct s_request
{
	long	coder_id;
	long	arrival_order;
	long	deadline;
}	t_request;

typedef struct s_heap
{
	t_request	*requests;
	int			size;
	int			capacity;
	int			scheduler;
}	t_heap;

typedef struct s_dongle
{
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;
	long			available_at;
	int				id;
	int				held_by;
	long			fifo_ticket;
	t_heap			waiters;
}	t_dongle;

typedef struct s_coder
{
	int				id;
	pthread_t		thread;
	int				compile_count;
	long			last_compile_start;
	t_dongle		*left;
	t_dongle		*right;
	t_simulation	*simulation;
}	t_coder;

typedef struct s_simulation
{
	t_config		config;
	t_coder			*coders;
	t_dongle		*dongles;
	pthread_mutex_t	log_mutex;
	pthread_mutex_t	state_mutex;
	pthread_t		monitor;
	long			start_time;
	int				stopped;
}	t_simulation;

int		parse(int ac, char **av, t_config *config);
int		init_simulation(t_simulation *sim, t_config *config);
void	*coder_routine(void *arg);
long	get_time_ms(void);
int		create_coders(t_simulation *sim);
int		join_coders(t_simulation *sim);
void	*monitor_routine(void *arg);
int		create_monitor(t_simulation *sim);
int		take_dongles(t_coder *coder);
void	release_dongles(t_coder *coder);
int		join_monitor(t_simulation *sim);
long	get_current_time(t_simulation *sim);
void	print_status(t_simulation *sim, int coder_id, char *msg);
void	smart_sleep(long duration_ms, t_simulation *sim);
int		simulation_stopped(t_simulation *sim);
void	heap_push(t_heap *heap, t_coder *coder, t_dongle *dongle);
void	heap_remove(t_heap *heap, int coder_id);
int		heap_top(t_heap *heap);
void	queue_request(t_coder *coder, t_dongle *dongle);
void	cancel_request(t_coder *coder, t_dongle *dongle);
int		try_acquire(t_coder *coder, t_dongle *dongle);
void	sift_down(t_heap *heap, int i);
void	cleanup_dongles(t_simulation *sim, int count);
void	cleanup_simulation(t_simulation *sim);
void	heap_swap(t_request *a, t_request *b);
int		heap_priority(t_heap *heap, t_request *a, t_request *b);

#endif
