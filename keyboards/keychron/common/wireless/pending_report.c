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
 * being dropped, and handed to the report buffer once the link is up again.
 *
 * Only a keyboard that was idle starts collecting: when the link goes down while
 * typing (lost signal, host or transport switch) the keys are still dropped, as
 * they would reach the host late. Collecting then lasts PENDING_REPORT_WINDOW_MS:
 * if the link is not back by then, every collected report is dropped, so the host
 * never gets only the end of what was typed.
 */
#ifndef PENDING_REPORT_QUEUE_SIZE
#    define PENDING_REPORT_QUEUE_SIZE 16
#endif

/* No keyboard report for this long makes the keyboard idle */
#ifndef PENDING_REPORT_IDLE_MS
#    define PENDING_REPORT_IDLE_MS 10000
#endif

/* How long collected reports wait for the link, from the first one. The wireless
 * module gives up reconnecting after 5 seconds (reconnect_timeout). */
#ifndef PENDING_REPORT_WINDOW_MS
#    define PENDING_REPORT_WINDOW_MS 5000
#endif

static report_buffer_t queue[PENDING_REPORT_QUEUE_SIZE];
static uint8_t         write_idx = 0;
static uint8_t         read_idx  = 0;

static bool     collecting       = false;
static uint32_t collect_time     = 0;    // first report collected
static bool     idle             = true; // no report since boot or wakeup, last_report_time is meaningless
static uint32_t last_report_time = 0;    // last keyboard report, sent or not

static void drop(void) {
    read_idx   = 0;
    write_idx  = 0;
    collecting = false;
}

static bool window_expired(void) {
    return timer_elapsed32(collect_time) > PENDING_REPORT_WINDOW_MS;
}

void pending_report_activity(void) {
    idle             = false;
    last_report_time = timer_read32();
}

void pending_report_add(report_buffer_t *report) {
    bool was_idle = idle || timer_elapsed32(last_report_time) >= PENDING_REPORT_IDLE_MS;
    pending_report_activity();

    if (collecting && window_expired()) drop();

    if (!collecting) {
        if (!was_idle) return;
        collecting   = true;
        collect_time = timer_read32();
    }

    uint8_t next = (write_idx + 1) % PENDING_REPORT_QUEUE_SIZE;

    if (next == read_idx) {
        /* Full: overwrite the newest entry. A report holds every key down at that
         * moment, so the last one is the current state and must not be lost. */
        queue[(write_idx + PENDING_REPORT_QUEUE_SIZE - 1) % PENDING_REPORT_QUEUE_SIZE] = *report;
        return;
    }

    queue[write_idx] = *report;
    write_idx        = next;
}

bool pending_report_flush(void) {
    bool sent = false;

    if (collecting && !window_expired()) {
        while (read_idx != write_idx) {
            report_buffer_enqueue(&queue[read_idx]);
            read_idx = (read_idx + 1) % PENDING_REPORT_QUEUE_SIZE;
            sent     = true;
        }
    }
    drop();

    return sent;
}

void pending_report_cancel(void) {
    drop();
    /* Count the switch as typing, so the keys pressed right after it are dropped */
    pending_report_activity();
}

void pending_report_clear(void) {
    drop();
    idle = true;
}
