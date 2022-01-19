/*
 * PIM for Quagga
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
#include "if.h"
#include "linklist.h"
#include "prefix.h"
#include "vty.h"
#include "vrf.h"
#include "plist.h"
#include "pimd.h"
#include "pim6_vty.h"
#include "pim_iface.h"
#include "pim6_cmd.h"
#include "pim_str.h"
#include "pim_ssmpingd.h"
#include "pim_pim.h"
#include "pim_oil.h"
#include "pim_static.h"
#include "pim_rp.h"
#include "pim_msdp.h"
#include "pim_ssm.h"
#include "pim_bfd.h"
#include "pim_bsm.h"
#include "pim_vxlan.h"
int pim6_interface_config_write(struct vty *vty)
{
	struct pim_instance *pim;
	struct interface *ifp;
	struct vrf *vrf;
	int writes = 0;

	RB_FOREACH (vrf, vrf_name_head, &vrfs_by_name) {
		pim = vrf->info;
		if (!pim)
			continue;
		FOR_ALL_INTERFACES (pim->vrf, ifp) {
			/* pim is enabled internally/implicitly on the vxlan
			 * termination device ipmr-lo. skip displaying that
			 * config to avoid confusion
			 */
			if (pim_vxlan_is_term_dev_cfg(pim, ifp))
				continue;
			/* IF name */
			if (vrf->vrf_id == VRF_DEFAULT)
				vty_frame(vty, "interface %s\n", ifp->name);
			else
				vty_frame(vty, "interface %s vrf %s\n",
						ifp->name, vrf->name);
			++writes;
			if (ifp->desc) {
				vty_out(vty, " description %s\n", ifp->desc);
				++writes;
			}
			/* TBD Depends on MLD data structure changes */
			vty_endframe(vty, "exit\n!\n");
			++writes;
		}
	}
	return writes;
}
