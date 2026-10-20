#ifndef GEYI_HISTORY_H
#define GEYI_HISTORY_H
#include <stdint.h>
#include "geyi_game.h"

/* In-memory match history (REQ-315). No dynamic memory, no LVGL, no flash. */

#define GY_HISTORY_CAPACITY 20
#define GY_HISTORY_VERSION  1u

/* How the match ended. Knockout means one side lost all soldiers or drew. */
enum { GY_END_KNOCKOUT, GY_END_RED_RESIGN, GY_END_BLUE_RESIGN };

typedef struct {
    uint16_t version;      /* GY_HISTORY_VERSION, reserved for format changes */
    uint8_t  outcome;      /* GY_RED_WIN / GY_BLUE_WIN / GY_DRAW from geyi_game.h */
    uint8_t  ended_by;     /* GY_END_* */
    uint16_t moves;        /* committed turns of this match */
    uint32_t seed;         /* rng seed, kept for a later replay feature */
    uint32_t duration_ms;  /* from entering the board to the final action */
} GyRecord;

void gy_hist_reset(void);
/* Copy one record; index 0 is always the newest. Returns slot or -1 if invalid. */
int  gy_hist_add(const GyRecord *record);
int  gy_hist_count(void);
const GyRecord *gy_hist_at(int index);
void gy_hist_format_duration(uint32_t ms, char *out, int size);
int  gy_hist_selftest(void);
#endif
