/*
 *  $Id: //WIFI_SOC/release/SDK_4_1_0_0/source/linux-2.6.21.x/drivers/media/video/pvrusb2/pvrusb2-debug.h#1 $
 *
 *  Copyright (C) 2005 Mike Isely <isely@pobox.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
#ifndef __PVRUSB2_DEBUG_H
#define __PVRUSB2_DEBUG_H

extern int pvrusb2_debug;

#define pvr2_trace(msk, fmt, arg...) do {if(msk & pvrusb2_debug) printk(KERN_INFO "pvrusb2: " fmt "\n", ##arg); } while (0)

/* These are listed in *rough* order of decreasing usefulness and
   increasing noise level. */
#define PVR2_TRACE_INFO       (1 <<  0) // Normal messages
#define PVR2_TRACE_ERROR_LEGS (1 <<  1) // error messages
#define PVR2_TRACE_TOLERANCE  (1 <<  2) // track tolerance-affected errors
#define PVR2_TRACE_TRAP       (1 <<  3) // Trap & report misbehavior from app
#define PVR2_TRACE_INIT       (1 <<  4) // misc initialization steps
#define PVR2_TRACE_START_STOP (1 <<  5) // Streaming start / stop
#define PVR2_TRACE_CTL        (1 <<  6) // commit of control changes
#define PVR2_TRACE_DEBUG      (1 <<  7) // Temporary debug code
#define PVR2_TRACE_EEPROM     (1 <<  8) // eeprom parsing / report
#define PVR2_TRACE_STRUCT     (1 <<  9) // internal struct creation
#define PVR2_TRACE_OPEN_CLOSE (1 << 10) // application open / close
#define PVR2_TRACE_CREG       (1 << 11) // Main critical region entry / exit
#define PVR2_TRACE_SYSFS      (1 << 12) // Sysfs driven I/O
#define PVR2_TRACE_FIRMWARE   (1 << 13) // firmware upload actions
#define PVR2_TRACE_CHIPS      (1 << 14) // chip broadcast operation
#define PVR2_TRACE_I2C        (1 << 15) // I2C related stuff
#define PVR2_TRACE_I2C_CMD    (1 << 16) // Software commands to I2C modules
#define PVR2_TRACE_I2C_CORE   (1 << 17) // I2C core debugging
#define PVR2_TRACE_I2C_TRAF   (1 << 18) // I2C traffic through the adapter
#define PVR2_TRACE_V4LIOCTL   (1 << 19) // v4l ioctl details
#define PVR2_TRACE_ENCODER    (1 << 20) // mpeg2 encoder operation
#define PVR2_TRACE_BUF_POOL   (1 << 21) // Track buffer pool management
#define PVR2_TRACE_BUF_FLOW   (1 << 22) // Track buffer flow in system
#define PVR2_TRACE_DATA_FLOW  (1 << 23) // Track data flow
#define PVR2_TRACE_DEBUGIFC   (1 << 24) // Debug interface actions
#define PVR2_TRACE_GPIO       (1 << 25) // GPIO state bit changes


#endif /* __PVRUSB2_HDW_INTERNAL_H */

/*
  Stuff for Emacs to see, in order to encourage consistent editing style:
  *** Local Variables: ***
  *** mode: c ***
  *** fill-column: 75 ***
  *** tab-width: 8 ***
  *** c-basic-offset: 8 ***
  *** End: ***
  */
