#include "geyi_history.h"
#include <stdio.h>
#include <string.h>

static int checks;

#define CHECK(x) do { ++checks; if (!(x)) { printf("FAIL line %d: %s\n", __LINE__, #x); return 1; } } while (0)

static GyRecord make(int outcome, int ended_by, int moves, uint32_t seed, uint32_t duration_ms)
{
    GyRecord r;
    memset(&r, 0, sizeof(r));
    r.outcome = (uint8_t)outcome;
    r.ended_by = (uint8_t)ended_by;
    r.moves = (uint16_t)moves;
    r.seed = seed;
    r.duration_ms = duration_ms;
    return r;
}

static int duration_is(uint32_t ms, const char *want)
{
    char text[8];
    gy_hist_format_duration(ms, text, (int)sizeof(text));
    return strcmp(text, want) == 0;
}

int main(void)
{
    GyRecord r;
    int i;

    CHECK(gy_hist_selftest());

    gy_hist_reset();
    CHECK(gy_hist_count() == 0);
    CHECK(gy_hist_at(0) == NULL);

    r = make(GY_RED_WIN, GY_END_KNOCKOUT, 12, 20261015u, 90000u);
    CHECK(gy_hist_add(&r) >= 0);
    CHECK(gy_hist_count() == 1);
    CHECK(gy_hist_at(0)->outcome == GY_RED_WIN);
    CHECK(gy_hist_at(0)->moves == 12);
    CHECK(gy_hist_at(0)->seed == 20261015u);
    CHECK(gy_hist_at(0)->duration_ms == 90000u);
    CHECK(gy_hist_at(0)->version == GY_HISTORY_VERSION);
    CHECK(gy_hist_at(1) == NULL);

    r = make(GY_BLUE_WIN, GY_END_RED_RESIGN, 3, 7u, 1000u);
    CHECK(gy_hist_add(&r) >= 0);
    CHECK(gy_hist_count() == 2);
    /* Newest first, previous record shifts to index 1. */
    CHECK(gy_hist_at(0)->outcome == GY_BLUE_WIN);
    CHECK(gy_hist_at(0)->ended_by == GY_END_RED_RESIGN);
    CHECK(gy_hist_at(1)->moves == 12);

    /* Invalid input is rejected and leaves the store unchanged. */
    r = make(GY_PLAYING, GY_END_KNOCKOUT, 1, 1u, 1u);
    CHECK(gy_hist_add(&r) == -1);
    CHECK(gy_hist_add(NULL) == -1);
    r = make(GY_DRAW, 7, 1, 1u, 1u);
    CHECK(gy_hist_add(&r) == -1);
    CHECK(gy_hist_count() == 2);

    /* Overflow drops the oldest record, not the newest. */
    gy_hist_reset();
    for (i = 0; i < GY_HISTORY_CAPACITY + 5; ++i) {
        r = make(i % 2 ? GY_BLUE_WIN : GY_RED_WIN, GY_END_KNOCKOUT, i, (uint32_t)i, (uint32_t)i * 1000u);
        CHECK(gy_hist_add(&r) >= 0);
        CHECK(gy_hist_count() == (i + 1 < GY_HISTORY_CAPACITY ? i + 1 : GY_HISTORY_CAPACITY));
    }
    CHECK(gy_hist_count() == GY_HISTORY_CAPACITY);
    CHECK(gy_hist_at(0)->moves == GY_HISTORY_CAPACITY + 4);
    CHECK(gy_hist_at(GY_HISTORY_CAPACITY - 1)->moves == 5);
    CHECK(gy_hist_at(GY_HISTORY_CAPACITY) == NULL);
    CHECK(gy_hist_at(-1) == NULL);

    /* Newest-to-oldest ordering must be monotonic after wrap-around. */
    for (i = 1; i < GY_HISTORY_CAPACITY; ++i)
        CHECK(gy_hist_at(i - 1)->moves == gy_hist_at(i)->moves + 1);

    gy_hist_reset();
    CHECK(gy_hist_count() == 0);
    CHECK(gy_hist_at(0) == NULL);

    CHECK(duration_is(0u, "00:00"));
    CHECK(duration_is(999u, "00:00"));
    CHECK(duration_is(1000u, "00:01"));
    CHECK(duration_is(59999u, "00:59"));
    CHECK(duration_is(60000u, "01:00"));
    CHECK(duration_is(3599000u, "59:59"));
    CHECK(duration_is(6000000u, "99:59"));
    CHECK(duration_is(UINT32_MAX, "99:59"));
    {   /* A tiny buffer must not overflow and must stay terminated. */
        char small[4];
        memset(small, 'x', sizeof(small));
        gy_hist_format_duration(60000u, small, (int)sizeof(small));
        CHECK(small[sizeof(small) - 1] == 0);
    }

    printf("PASS: %d assertions; TC-315 history store and duration format\n", checks);
    return 0;
}
