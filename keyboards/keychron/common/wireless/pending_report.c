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

#include "quantum.h"
#include "report_buffer.h"
#include "pending_report.h"

/* The key that wakes the keyboard is pressed while the wireless link is down, and
 * so are the ones typed while it reconnects. Their reports are kept here instead of
 * being dropped, and handed to the report buffer once the link is up again. Reports
 * older than PENDING_REPORT_TIMEOUT_MS by then are dropped, so a slow or failed
 * reconnection does not type stale keys later.
 */
#ifndef PENDING_REPORT_QUEUE_SIZE
#    define PENDING_REPORT_QUEUE_SIZE 16
#endif

#ifndef PENDING_REPORT_TIMEOUT_MS
#    define PENDING_REPORT_TIMEOUT_MS 3000
#endif

typedef struct {
    report_buffer_t report;
    uint32_t        time; // timer_read32() when the report was queued
} pending_report_t;

static pending_report_t queue[PENDING_REPORT_QUEUE_SIZE];
static uint8_t          write_idx = 0;
static uint8_t          read_idx  = 0;

void pending_report_add(report_buffer_t *report) {
    uint8_t next = (write_idx + 1) % PENDING_REPORT_QUEUE_SIZE;

    if (next == read_idx) {
        /* Full: overwrite the newest entry. A report holds every key down at that
         * moment, so the last one is the current state and must not be lost. */
        uint8_t last       = (write_idx + PENDING_REPORT_QUEUE_SIZE - 1) % PENDING_REPORT_QUEUE_SIZE;
        queue[last].report = *report;
        queue[last].time   = timer_read32();
        return;
    }

    queue[write_idx].report = *report;
    queue[write_idx].time   = timer_read32();
    write_idx               = next;
}

void pending_report_flush(void) {
    /* Reports are in time order, so dropping the old ones never keeps a press
     * without the release that follows it. */
    while (read_idx != write_idx) {
        if (timer_elapsed32(queue[read_idx].time) <= PENDING_REPORT_TIMEOUT_MS) {
            report_buffer_enqueue(&queue[read_idx].report);
        }
        read_idx = (read_idx + 1) % PENDING_REPORT_QUEUE_SIZE;
    }
}

void pending_report_clear(void) {
    read_idx  = 0;
    write_idx = 0;
}
