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
#ifndef PIM6_VTY_H
#define PIM6_VTY_H
#include "vty.h"
int pim_debug_config_write(struct vty *vty);
int pim_global_config_write_worker(struct pim_instance *pim, struct vty *vty);
int pim6_interface_config_write(struct vty *vty);
#endif /* PIM6_VTY_H */
