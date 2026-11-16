/* Copyright 2026 Federico Bolcato
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

/*
 * Extends the ChibiOS OTGv1 USB low level driver header, the same way board.h, halconf.h and mcuconf.h in this
 * folder extend theirs (this folder comes first in the include path).
 *
 * Keychron ChibiOS fork ba10f3a8 "Fixed BOARD_OTG_NOVBUSSENS not work correctly": on OTG stepping 1 cores, which
 * includes the STM32F401, usbConnectBus() sets GCCFG_VBUSBSEN and so enables VBUS sensing on PA9 even when the board
 * declares BOARD_OTG_NOVBUSSENS. On the K3 Max PA9 is the Bluetooth mode switch (BT_MODE_SELECT_PIN), not VBUS.
 * Keep GCCFG as usb_lld_start() sets it (NOVBUSSENS) by making bus connect and disconnect no-ops, as the fork does.
 */

#include_next <hal_usb_lld.h>

/* Checked by chibios_fixes.c, so that the build fails if this header stops being picked up */
#define K3_MAX_HAL_USB_LLD_OVERRIDE

#if defined(BOARD_OTG_NOVBUSSENS) && (STM32_OTG_STEPPING == 1)
#    undef usb_lld_connect_bus
#    undef usb_lld_disconnect_bus
#    define usb_lld_connect_bus(usbp) ((void)(usbp))
#    define usb_lld_disconnect_bus(usbp) ((void)(usbp))
#endif
