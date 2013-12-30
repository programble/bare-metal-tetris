typedef unsigned char      u8;
typedef signed   char      s8;
typedef unsigned short     u16;
typedef signed   short     s16;
typedef unsigned int       u32;
typedef signed   int       s32;
typedef unsigned long long u64;
typedef signed   long long s64;

#define noreturn __attribute__((noreturn)) void

typedef enum bool {
    false,
    true
} bool;

/* Port I/O */

static inline u8 inb(u16 p)
{
    u8 r;
    asm("inb %1, %0" : "=a" (r) : "dN" (p));
    return r;
}

static inline void outb(u16 p, u8 d)
{
    asm("outb %1, %0" : : "dN" (p), "a" (d));
}

/* Timing */

static inline u64 rdtsc(void)
{
    u32 hi, lo;
    asm("rdtsc" : "=a" (lo), "=d" (hi));
    return ((u64) lo) | (((u64) hi) << 32);
}

u8 rtcs(void)
{
    u8 last = 0, sec;
    do { /* until value is the same twice in a row */
        /* wait for update not in progress */
        do { outb(0x70, 0x0A); } while (inb(0x71) & 0x80);
        outb(0x70, 0x00);
        sec = inb(0x71);
    } while (sec != last && (last = sec));
    return sec;
}

u64 tps(void)
{
    static u64 ti = 0, dt = 0;
    static u8 last_sec = 0xFF;
    u8 sec = rtcs();
    if (sec != last_sec) {
        last_sec = sec;
        u64 tf = rdtsc();
        dt = tf - ti;
        ti = tf;
    }
    return dt;
}

enum timer {
    TIMER_BLINK,
    TIMER_BEEP,
    TIMER__LENGTH
};

u64 timers[TIMER__LENGTH] = {0};

bool interval(enum timer timer, u64 ticks)
{
    u64 tf = rdtsc();
    if (tf - timers[timer] >= ticks) {
        timers[timer] = tf;
        return true;
    } else return false;
}

bool wait(enum timer timer, u64 ticks)
{
    if (timers[timer]) {
        if (rdtsc() - timers[timer] >= ticks) {
            timers[timer] = 0;
            return true;
        } else return false;
    } else {
        timers[timer] = rdtsc();
        return false;
    }
}

/* Video Output */

enum color {
    BLACK,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    YELLOW,
    GRAY,
    BRIGHT
};

#define COLS (80)
#define ROWS (25)
u16 *const video = (u16*) 0xB8000;

void putc(u8 x, u8 y, enum color fg, enum color bg, char c)
{
    u16 z = (bg << 12) | (fg << 8) | c;
    video[y * COLS + x] = z;
}

void puts(u8 x, u8 y, enum color fg, enum color bg, const char *s)
{
    for (; *s; s++, x++)
        putc(x, y, fg, bg, *s);
}

void clear(enum color bg)
{
    u8 x, y;
    for (y = 0; y < ROWS; y++)
        for (x = 0; x < COLS; x++)
            putc(x, y, bg, bg, ' ');
}

/* Keyboard Input */

#define KEY_D     (0x20)
#define KEY_UP    (0x48)
#define KEY_DOWN  (0x50)
#define KEY_LEFT  (0x4B)
#define KEY_RIGHT (0x4D)
#define KEY_ENTER (0x1C)
#define KEY_SPACE (0x39)

u8 scan(void)
{
    static u8 key = 0;
    u8 scan = inb(0x60);
    if (scan != key)
        return key = scan;
    else return 0;
}

/* PC Speaker */

void pcspk_freq(u32 freq)
{
    u32 div = 1193180 / freq;
    outb(0x43, 0xB6);
    outb(0x42, (u8) div);
    outb(0x42, (u8) (div >> 8));
}

void pcspk_on(void)
{
    outb(0x61, inb(0x61) | 0x3);
}

void pcspk_off(void)
{
    outb(0x61, inb(0x61) & 0xFC);
}

/* Formatting */

char *itoa(u32 n, u8 r, u8 w)
{
    static const char d[16] = "0123456789ABCDEF";
    static char s[34];
    s[33] = 0;
    u8 i = 33;
    do {
        i--;
        s[i] = d[n % r];
        n /= r;
    } while (i > 33 - w);
    return (char *) (s + i);
}

/* Random */

u32 rand(u32 range)
{
    return (u32) rdtsc() % range;
}

noreturn main()
{
    clear(BLACK);
    pcspk_freq(200);

    bool debug = false;
    u64 tpms;
    u8 x = COLS / 2, y = ROWS / 2, color = YELLOW;
    bool beep = false;
    u32 random = 0;
loop:
    tpms = (u32) tps() / 1000;

    if (debug) {
        puts(0, 0, BRIGHT | GREEN, BLACK, "RTC sec:");
        puts(10, 0, GREEN, BLACK, itoa(rtcs(), 16, 2));
        puts(0, 1, BRIGHT | GREEN, BLACK, "ticks/ms:");
        puts(10, 1, GREEN, BLACK, itoa(tpms, 10, 10));
        puts(0, 2, BRIGHT | GREEN, BLACK, "timers:");
        u32 i;
        for (i = 0; i < TIMER__LENGTH; i++)
            puts(10 + i * 11, 2, GREEN, BLACK, itoa(timers[i], 10, 10));

        puts(0, 3, BRIGHT | GREEN, BLACK, "random:");
        puts(10, 3, GREEN, BLACK, itoa(random, 10, 1));
    }

    if (interval(TIMER_BLINK, tpms * 500))
        color ^= BRIGHT;

    if (beep && wait(TIMER_BEEP, tpms * 100)) {
        pcspk_off();
        beep = false;
    }

    u8 key;
    if ((key = scan())) {
        if (debug) {
            puts(0, 4, BRIGHT | GREEN, BLACK, "key:");
            puts(10, 4, GREEN, BLACK, itoa(key, 16, 2));
        }

        putc(x, y, BLACK, BLACK, ' ');
        switch(key) {
        case KEY_D:
            debug = !debug;
            clear(BLACK);
            break;
        case KEY_SPACE: random = rand(7); break;
        case KEY_UP: y--; break;
        case KEY_DOWN: y++; break;
        case KEY_LEFT: x--; break;
        case KEY_RIGHT: x++; break;
        }
    }

    if (x >= COLS || y >= ROWS) {
        beep = true;
        pcspk_on();
        x = COLS / 2;
        y = ROWS / 2;
    }

    putc(x, y, color, BLACK, '*');

    goto loop;
}
