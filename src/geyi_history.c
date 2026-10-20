#include "geyi_history.h"
#include <stdio.h>
#include <string.h>

/* Ring buffer: fixed storage, oldest record overwritten when full (REQ-315). */
static GyRecord items[GY_HISTORY_CAPACITY];
static int count;
static int next;   /* slot receiving the next record */

void gy_hist_reset(void)
{
    memset(items, 0, sizeof(items));
    count = 0;
    next = 0;
}

int gy_hist_add(const GyRecord *record)
{
    int slot;
    if (!record) return -1;
    if (record->outcome < GY_RED_WIN || record->outcome > GY_DRAW) return -1;
    if (record->ended_by > GY_END_BLUE_RESIGN) return -1;
    slot = next;
    items[slot] = *record;
    items[slot].version = (uint16_t)GY_HISTORY_VERSION;
    next = (next + 1) % GY_HISTORY_CAPACITY;
    if (count < GY_HISTORY_CAPACITY) ++count;
    return slot;
}

int gy_hist_count(void)
{
    return count;
}

const GyRecord *gy_hist_at(int index)
{
    if (index < 0 || index >= count) return NULL;
    return &items[(next - 1 - index + GY_HISTORY_CAPACITY * 2) % GY_HISTORY_CAPACITY];
}

void gy_hist_format_duration(uint32_t ms, char *out, int size)
{
    uint32_t minutes, seconds;
    if (!out || size <= 0) return;
    minutes = ms / 60000u;
    seconds = (ms / 1000u) % 60u;
    if (minutes > 99u) { minutes = 99u; seconds = 59u; }
    snprintf(out, (size_t)size, "%02u:%02u", (unsigned)minutes, (unsigned)seconds);
}

static int expect_duration(uint32_t ms, const char *want)
{
    char text[8];
    gy_hist_format_duration(ms, text, (int)sizeof(text));
    return strcmp(text, want) == 0;
}

int gy_hist_selftest(void)
{
    GyRecord record;
    int i;
    memset(&record, 0, sizeof(record));
    gy_hist_reset();
    if (gy_hist_count() != 0 || gy_hist_at(0) != NULL || gy_hist_at(-1) != NULL) return 0;

    record.outcome = GY_RED_WIN;
    record.ended_by = GY_END_KNOCKOUT;
    record.moves = 7;
    record.seed = 20261015u;
    record.duration_ms = 75000u;
    if (gy_hist_add(&record) < 0 || gy_hist_count() != 1) return 0;
    if (!gy_hist_at(0) || gy_hist_at(0)->moves != 7 || gy_hist_at(1) != NULL) return 0;
    if (gy_hist_at(0)->version != GY_HISTORY_VERSION) return 0;

    /* Reject impossible records without touching the stored data. */
    record.outcome = GY_PLAYING;
    if (gy_hist_add(&record) != -1 || gy_hist_count() != 1) return 0;
    record.outcome = GY_DRAW;
    record.ended_by = 9;
    if (gy_hist_add(&record) != -1 || gy_hist_count() != 1) return 0;

    /* Overflow keeps the newest and drops the oldest. */
    gy_hist_reset();
    for (i = 0; i < GY_HISTORY_CAPACITY + 3; ++i) {
        memset(&record, 0, sizeof(record));
        record.outcome = (uint8_t)(i % 2 ? GY_BLUE_WIN : GY_RED_WIN);
        record.ended_by = GY_END_KNOCKOUT;
        record.moves = (uint16_t)i;
        if (gy_hist_add(&record) < 0) return 0;
    }
    if (gy_hist_count() != GY_HISTORY_CAPACITY) return 0;
    if (!gy_hist_at(0) || gy_hist_at(0)->moves != GY_HISTORY_CAPACITY + 2) return 0;
    if (!gy_hist_at(GY_HISTORY_CAPACITY - 1) || gy_hist_at(GY_HISTORY_CAPACITY - 1)->moves != 3) return 0;
    if (gy_hist_at(GY_HISTORY_CAPACITY) != NULL) return 0;

    if (!expect_duration(0u, "00:00")) return 0;
    if (!expect_duration(59999u, "00:59")) return 0;
    if (!expect_duration(60000u, "01:00")) return 0;
    if (!expect_duration(3599000u, "59:59")) return 0;
    if (!expect_duration(6000000u, "99:59")) return 0;
    {   /* Truncation must still terminate the string. */
        char small[4];
        gy_hist_format_duration(60000u, small, (int)sizeof(small));
        if (small[sizeof(small) - 1] != 0) return 0;
    }

    gy_hist_reset();
    return gy_hist_count() == 0;
}
