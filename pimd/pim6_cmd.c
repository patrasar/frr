/*
 * PIM for IPv6 FRR
 * Copyright (C) 2022  Vmware, Inc.
 *		       Mobashshera Rasool <mrasool@vmware.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; see the file COPYING; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <zebra.h>

#include "lib/json.h"
#include "command.h"
#include "if.h"
#include "prefix.h"
#include "zclient.h"
#include "plist.h"
#include "hash.h"
#include "nexthop.h"
#include "vrf.h"
#include "ferr.h"

#include "pimd.h"
#include "pim6_cmd.h"
#include "pim_vty.h"
#include "lib/northbound_cli.h"
#include "pim_errors.h"
#include "pim_nb.h"

#ifndef VTYSH_EXTRACT_PL
#include "pimd/pim6_cmd_clippy.c"
#endif

DEFUN (interface_ipv6_pim,
       interface_ipv6_pim_cmd,
       "ipv6 pim",
       IPV6_STR
       PIM_STR)
{
	nb_cli_enqueue_change(vty, "./pim-enable", NB_OP_MODIFY, "true");

	return nb_cli_apply_changes(vty,
			FRR_PIM_INTERFACE_XPATH,
			"frr-routing:ipv6");

}

DEFUN (interface_no_ipv6_pim,
       interface_no_ipv6_pim_cmd,
       "no ipv6 pim",
       NO_STR
       IPV6_STR
       PIM_STR)
{
	const struct lyd_node *mld_enable_dnode;
	char mld_if_xpath[XPATH_MAXLEN + 20];

	snprintf(mld_if_xpath, sizeof(mld_if_xpath),
		 "%s/frr-gmp:gmp/address-family[address-family='%s']",
		 VTY_CURR_XPATH, "frr-routing:ipv6");
	mld_enable_dnode =
		yang_dnode_getf(vty->candidate_config->dnode,
				FRR_GMP_ENABLE_XPATH, VTY_CURR_XPATH,
				"frr-routing:ipv6");

	if (!mld_enable_dnode) {
		nb_cli_enqueue_change(vty, mld_if_xpath, NB_OP_DESTROY, NULL);
		nb_cli_enqueue_change(vty, ".", NB_OP_DESTROY, NULL);
	} else {
		if (!yang_dnode_get_bool(mld_enable_dnode, ".")) {
			nb_cli_enqueue_change(vty, mld_if_xpath, NB_OP_DESTROY,
					NULL);
			nb_cli_enqueue_change(vty, ".", NB_OP_DESTROY, NULL);
		} else
			nb_cli_enqueue_change(vty, "./pim-enable", NB_OP_MODIFY,
					"false");
	}

	return nb_cli_apply_changes(vty,
			FRR_PIM_INTERFACE_XPATH,
			"frr-routing:ipv6");
}

void pim_cmd_init(void)
{
	if_cmd_init(pim6_interface_config_write);

	install_element(INTERFACE_NODE, &interface_ipv6_pim_cmd);
	install_element(INTERFACE_NODE, &interface_no_ipv6_pim_cmd);

}
