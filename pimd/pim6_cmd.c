/*
 * PIM for IPv6 FRR
 * Copyright (C) 2022 VMWARE
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
#include "pim6_vty.h"
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

DEFUN (interface_ipv6_pim_drprio,
       interface_ipv6_pim_drprio_cmd,
       "ipv6 pim drpriority (1-4294967295)",
       IPV6_STR
       PIM_STR
       "Set the Designated Router Election Priority\n"
       "Value of the new DR Priority\n")
{
	int idx_number = 3;

	nb_cli_enqueue_change(vty, "./dr-priority", NB_OP_MODIFY,
			argv[idx_number]->arg);

	return nb_cli_apply_changes(vty, FRR_PIM_INTERFACE_XPATH,
			"frr-routing:ipv6");
}

DEFUN (interface_no_ipv6_pim_drprio,
       interface_no_ipv6_pim_drprio_cmd,
       "no ip pim drpriority [(1-4294967295)]",
       NO_STR
       IPV6_STR
       PIM_STR
       "Revert the Designated Router Priority to default\n"
       "Old Value of the Priority\n")
{
	nb_cli_enqueue_change(vty, "./dr-priority", NB_OP_DESTROY, NULL);

	return nb_cli_apply_changes(vty, FRR_PIM_INTERFACE_XPATH,
				    "frr-routing:ipv6");
}

DEFUN (interface_ipv6_pim_hello,
       interface_ipv6_pim_hello_cmd,
       "ipv6 pim hello (1-65535) [(1-65535)]",
       IPV6_STR
       PIM_STR
       IFACE_PIM_HELLO_STR
       IFACE_PIM_HELLO_TIME_STR
       IFACE_PIM_HELLO_HOLD_STR)
{
        int idx_time = 3;
        int idx_hold = 4;
        const struct lyd_node *mld_enable_dnode;

	mld_enable_dnode =
		yang_dnode_getf(vty->candidate_config->dnode,
				FRR_GMP_ENABLE_XPATH, VTY_CURR_XPATH,
				"frr-routing:ipv6");
	if (!mld_enable_dnode) {
		nb_cli_enqueue_change(vty, "./pim-enable", NB_OP_MODIFY,
				"true");
	} else {
		if (!yang_dnode_get_bool(mld_enable_dnode, "."))
			nb_cli_enqueue_change(vty, "./pim-enable", NB_OP_MODIFY,
					"true");
	}

	nb_cli_enqueue_change(vty, "./hello-interval", NB_OP_MODIFY,
			argv[idx_time]->arg);

	if (argc == idx_hold + 1)
		nb_cli_enqueue_change(vty, "./hello-holdtime", NB_OP_MODIFY,
				argv[idx_hold]->arg);

	return nb_cli_apply_changes(vty,
			FRR_PIM_INTERFACE_XPATH,
			"frr-routing:ipv6");
}

DEFUN (interface_no_ipv6_pim_hello,
       interface_no_ipv6_pim_hello_cmd,
       "no ipv6 pim hello [(1-65535) [(1-65535)]]",
       NO_STR
       IPV6_STR
       PIM_STR
       IFACE_PIM_HELLO_STR
       IGNORED_IN_NO_STR
       IGNORED_IN_NO_STR)
{
	nb_cli_enqueue_change(vty, "./hello-interval", NB_OP_DESTROY, NULL);
	nb_cli_enqueue_change(vty, "./hello-holdtime", NB_OP_DESTROY, NULL);

	return nb_cli_apply_changes(vty,
			FRR_PIM_INTERFACE_XPATH,
			"frr-routing:ipv6");
}

DEFPY (interface_ipv6_pim_activeactive,
       interface_ipv6_pim_activeactive_cmd,
       "[no$no] ipv6 pim active-active",
       NO_STR
       IPV6_STR
       PIM_STR
       "Mark interface as Active-Active for MLAG operations, Hidden because not finished yet\n")
{
	if (no)
		nb_cli_enqueue_change(vty, "./active-active", NB_OP_MODIFY,
				"false");
	else {
		nb_cli_enqueue_change(vty, "./pim-enable", NB_OP_MODIFY,
				"true");

		nb_cli_enqueue_change(vty, "./active-active", NB_OP_MODIFY,
				"true");
	}

	return nb_cli_apply_changes(vty,
			FRR_PIM_INTERFACE_XPATH,
			"frr-routing:ipv6");
}

DEFUN_HIDDEN (interface_ipv6_pim_ssm,
              interface_ipv6_pim_ssm_cmd,
              "ipv6 pim ssm",
              IPV6_STR
              PIM_STR
              IFACE_PIM_STR)
{
	int ret;

	nb_cli_enqueue_change(vty, "./pim-enable", NB_OP_MODIFY, "true");

	ret = nb_cli_apply_changes(vty,
			FRR_PIM_INTERFACE_XPATH,
			"frr-routing:ipv6");

	if (ret != NB_OK)
		return ret;

	vty_out(vty,
		"WARN: Enabled PIM SM on interface; configure PIM SSM range if needed\n");

	return NB_OK;
}

DEFUN_HIDDEN (interface_no_ipv6_pim_ssm,
              interface_no_ipv6_pim_ssm_cmd,
              "no ipv6 pim ssm",
              NO_STR
              IPV6_STR
              PIM_STR
              IFACE_PIM_STR)
{
	const struct lyd_node *mld_enable_dnode;
	char mld_if_xpath[XPATH_MAXLEN + 20];

	snprintf(mld_if_xpath, sizeof(mld_if_xpath),
		 "%s/frr-gmp:gmp/address-family[address-family='%s']",
		 VTY_CURR_XPATH, "frr-routing:ipv6");
	mld_enable_dnode = yang_dnode_getf(vty->candidate_config->dnode,
					    FRR_GMP_ENABLE_XPATH,
					    VTY_CURR_XPATH,
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

	return nb_cli_apply_changes(vty, FRR_PIM_INTERFACE_XPATH,
			"frr-routing:ipv6");
}

DEFUN_HIDDEN (interface_ipv6_pim_sm,
	      interface_ipv6_pim_sm_cmd,
	      "ipv6 pim sm",
	      IPV6_STR
	      PIM_STR
	      IFACE_PIM_SM_STR)
{
	nb_cli_enqueue_change(vty, "./pim-enable", NB_OP_MODIFY, "true");

	return nb_cli_apply_changes(vty,
			FRR_PIM_INTERFACE_XPATH,
			"frr-routing:ipv6");
}

DEFUN_HIDDEN (interface_no_ipv6_pim_sm,
	      interface_no_ipv6_pim_sm_cmd,
	      "no ipv6 pim sm",
	      NO_STR
	      IPV6_STR
	      PIM_STR
	      IFACE_PIM_SM_STR)
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
	install_element(INTERFACE_NODE, &interface_ipv6_pim_drprio_cmd);
	install_element(INTERFACE_NODE, &interface_no_ipv6_pim_drprio_cmd);
	install_element(INTERFACE_NODE, &interface_ipv6_pim_hello_cmd);
	install_element(INTERFACE_NODE, &interface_no_ipv6_pim_hello_cmd);
	install_element(INTERFACE_NODE, &interface_ipv6_pim_activeactive_cmd);
	install_element(INTERFACE_NODE, &interface_ipv6_pim_ssm_cmd);
	install_element(INTERFACE_NODE, &interface_no_ipv6_pim_ssm_cmd)
        install_element(INTERFACE_NODE, &interface_ip_pim_sm_cmd);
        install_element(INTERFACE_NODE, &interface_no_ip_pim_sm_cmd);
}

