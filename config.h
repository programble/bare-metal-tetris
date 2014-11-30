/* About data, shown on boot and when paused */
#define TETRIS_NAME    "Bare Metal Tetris"
#define TETRIS_VERSION "1.0.0"
#define TETRIS_URL     "https://github.com/programble/bare-metal-tetris"

/* Tetris well dimensions */
#define WELL_WIDTH  (10)
#define WELL_HEIGHT (22)

/* Initial interval in milliseconds at which to apply gravity */
#define INITIAL_SPEED (1000)

/* Delay in milliseconds before rows are cleared */
#define CLEAR_DELAY (100)

/* Scoring: score is increased by the product of the current level and a factor
 * corresponding to the number of rows cleared. */
#define SCORE_FACTOR_1 (100)
#define SCORE_FACTOR_2 (300)
#define SCORE_FACTOR_3 (500)
#define SCORE_FACTOR_4 (800)

/* Amount to increase the score for a soft drop */
#define SOFT_DROP_SCORE (1)

/* Factor by which to multiply the number of rows dropped to increase the score
 * for a hard drop */
#define HARD_DROP_SCORE_FACTOR (2)

/* Number of rows that need to be cleared to increase level */
#define ROWS_PER_LEVEL (10)
