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

#include "report_buffer.h"

/* Reports made while the wireless link is down, sent once it is back */
void pending_report_add(report_buffer_t *report); // the link is down
void pending_report_activity(void);               // a report went out over the link
bool pending_report_flush(void);                  // the link is up: true if it sent any report
void pending_report_cancel(void);                 // host or transport switch
void pending_report_clear(void);                  // wakeup from low power mode
