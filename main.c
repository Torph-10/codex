/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abelgarh <abelgarh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/08/28 12:38:44 by ase               #+#    #+#             */
/*   Updated: 2026/09/11 00:32:13 by abelgarh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

int	main(int ac, char **av)
{
	t_config		config;
	t_simulation	sim;

	if (!parse(ac, av, &config))
		return (1);
	if (!init_simulation(&sim, &config))
		return (1);
	if (!create_monitor(&sim))
		return (1);
	if (!create_coders(&sim))
		return (1);
	if (!join_coders(&sim))
		return (1);
	if (!join_monitor(&sim))
		return (1);
	cleanup_simulation(&sim);
	return (0);
}
