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

/*
 * The Keychron ChibiOS fork (Keychron/ChibiOS, branch 25q3) patches ChibiOS itself. This firmware builds with QMK's
 * ChibiOS, so the patches the K3 Max (STM32F401) relies on are applied from the keyboard instead:
 *
 * - dd3be323 "Fixed STM32F401 GPIOH clock": board_init() below.
 * - a568fde8 "Added write protection checking on BKP domain initialization": early_hardware_init_post() below.
 * - ba10f3a8 "Fixed BOARD_OTG_NOVBUSSENS not work correctly": hal_usb_lld.h in this folder.
 */

#include "hal.h"

#if !defined(STM32F401xx)
#    error "The K3 Max ChibiOS fixes target the STM32F401"
#endif

#if !defined(K3_MAX_HAL_USB_LLD_OVERRIDE)
#    error "keyboards/keychron/k3_max/hal_usb_lld.h must be found before the ChibiOS one"
#endif

/*
 * Called from __early_init(), once the clock tree is running and before halInit(). ChibiOS opens the backup domain
 * in hal_lld_init() and immediately configures the RTC clock source in it, without waiting for the write
 * protection to be lifted. Lift it here and wait: hal_lld_init() does not reset the PWR interface, so the domain
 * is already writable when ChibiOS gets there. The RTC drives the wireless timers (battery and indicators).
 */
void early_hardware_init_post(void) {
    rccEnablePWRInterface(true);
    PWR->CR |= PWR_CR_DBP;
    while ((PWR->CR & PWR_CR_DBP) == 0U) {
        /* Waits until the backup domain write protection is disabled. */
    }
}

/*
 * Called at the end of halInit(). The STM32F401 registry leaves GPIOH out of STM32_GPIO_EN_MASK, so the port is never
 * clocked, the board defaults for it are lost and hal_lld_init() resets it. lpm_stm32f401.c sets H0/H1 before
 * entering STOP mode, which needs the port clocked. Clock it and apply the board defaults, as the Keychron fork does
 * by adding GPIOH to the mask.
 */
void board_init(void) {
#if !(STM32_GPIO_EN_MASK & RCC_AHB1ENR_GPIOHEN)
    rccEnableAHB1(RCC_AHB1ENR_GPIOHEN, true);

    GPIOH->OTYPER  = VAL_GPIOH_OTYPER;
    GPIOH->OSPEEDR = VAL_GPIOH_OSPEEDR;
    GPIOH->PUPDR   = VAL_GPIOH_PUPDR;
    GPIOH->ODR     = VAL_GPIOH_ODR;
    GPIOH->AFRL    = VAL_GPIOH_AFRL;
    GPIOH->AFRH    = VAL_GPIOH_AFRH;
    GPIOH->MODER   = VAL_GPIOH_MODER;
#endif
}
