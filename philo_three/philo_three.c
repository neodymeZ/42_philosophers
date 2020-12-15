/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   philo_three.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: larosale <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/12/15 16:44:32 by larosale          #+#    #+#             */
/*   Updated: 2020/12/15 20:17:12 by larosale         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "philo.h"

sem_t		*g_forks;
sem_t		*g_write_lock;
sem_t		*g_ok_to_take;

/*
** Worker function for the philosopher thread.
** First, it creates a monitoring thread to check for starvation and report
** death.
** Then it starts the working loop of changing the philosopher's states.
*/

static void	philo_worker(t_philos *phil)
{
	pthread_t	monitor;

		printf("Process started\n");
	open_sems_child(phil);
	if (pthread_create(&monitor, NULL, philo_monitor, phil))
		exit(EXIT_ERR);	
	if (pthread_detach(monitor))
		exit(EXIT_ERR);
	while (1)
	{
		printf("Entering loop\n");
		philo_take_forks(phil);
		philo_eat(phil);
		philo_sleep(phil);
		philo_think(phil);
	}
}

/*
** Runs all philosophers' processes.
*/

static int	start_processes(t_philos *philos, t_params *params)
{
	int			i;
	int			stat_loc;
	int			ret;

	i = -1;
	ret = 0;
	printf("STARTING\n");
	while (++i < params->proc_num)
	{
		if (((philos + i)->pid = fork()) < 0)
			return (cleanup(philos, params, ERR_SYS));
		else if ((philos + i)->pid == 0)
		{
			philo_worker(philos + i);
			exit(0);
		}
	}
	while (waitpid(0, &stat_loc, WUNTRACED) > 0)
	{
		if (WIFEXITED(stat_loc) && (ret = WEXITSTATUS(stat_loc))
			&& ret == EXIT_DEAD)
			break ;
		if (ret == EXIT_ATE)
			params->num_eats--;
	}
	return (0);
}

/*
** Validates the command-line arguments and fills the "par"
** structure with values.
*/

static int	check_args(int argc, char **argv, t_params **par)
{
	t_params	*params;

	if (!(params = ft_calloc(1, sizeof(t_params))))
		return (cleanup(NULL, params, ERR_SYS));
	if (argc != 5 && argc != 6)
		return (cleanup(NULL, params, ERR_ARGNUM));
	if ((params->proc_num = ft_atoi(*(argv + 1))) < 2)
		return (cleanup(NULL, params, ERR_ARGPHL));
	else if (params->proc_num > MAX_PROCESSES)
		return (cleanup(NULL, params, ERR_ARGPHH));
	if ((params->t_die = ft_atoi(*(argv + 2))) < MIN_TIME
		|| (params->t_eat = ft_atoi(*(argv + 3))) < MIN_TIME
		|| (params->t_sleep = ft_atoi(*(argv + 4))) < MIN_TIME)
		return (cleanup(NULL, params, ERR_ARGTIM));
	if (argc == 6 && (params->num_eats = ft_atoi(*(argv + 5))) < 1)
		return (cleanup(NULL, params, ERR_ARGEAT));
	*par = params;
	return (0);
}

/*
** Main function of philo_two.
** Checks command-line arguments, then creates data structures for
** threads (philosophers) and resource semaphors.
** Runs philosophers' threads and starts monitor to detect
** deaths from starvation and reaching maximum eats limit.
** Finally, it cleans up the state (frees memory, closes semaphors).
*/

int			main(int argc, char **argv)
{
	t_params	*params;
	t_philos	*philos;

	params = NULL;
	philos = NULL;
	if (check_args(argc, argv, &params)
		|| !(philos = create_philos(params))
		|| create_sems(params))
		return (1);
	get_time(params);
	if (start_processes(philos, params))
		return (1);
	if (params->num_eats == 0)
		print_status(philos->num, ATE, get_time(params));
	cleanup(philos, params, OK);
	return (0);
}
