/*
 * Almus Mercatus — platform engine
 * --------------------------------
 * All custom logic for the site is written in C11 (ISO/IEC 9899:2011),
 * freestanding (no libc), and compiled to WebAssembly:
 *
 *   clang --target=wasm32 -std=c11 -O2 -nostdlib \
 *         -Wl,--no-entry -Wl,--export-all -o app.wasm src/app.c
 *
 * The page carries only a small loader: it starts this module and moves
 * data between the DOM and the functions below. Every rule — the auction
 * clock, bid validation, the fair-close extension, the preview bids, the
 * search scoring, even assembling the contact address — lives here in C.
 */

#define RESULT_SZ 8192

/* The compiler may emit calls to these even in freestanding code. */
void *memset(void *d, int v, unsigned long n) {
    unsigned char *p = (unsigned char *)d;
    while (n--) *p++ = (unsigned char)v;
    return d;
}
void *memcpy(void *d, const void *s, unsigned long n) {
    unsigned char *p = (unsigned char *)d;
    const unsigned char *q = (const unsigned char *)s;
    while (n--) *p++ = *q++;
    return d;
}

static char result_buf[RESULT_SZ];  /* auction log lives here          */
static char scratch_q[256];         /* JS writes the search query here */
static char scratch_t[2048];        /* JS writes the text to score     */
static char contact_buf[64];        /* assembled contact address       */

/* ---------- tiny freestanding helpers ---------- */

static int s_len(const char *s) { int n = 0; while (s[n]) n++; return n; }

static void s_copy(char *dst, const char *src) {
    int i = 0; while (src[i]) { dst[i] = src[i]; i++; } dst[i] = 0;
}

static void s_cat(char *dst, const char *src) { s_copy(dst + s_len(dst), src); }

static char lower_of(char c) {
    if (c >= 'A' && c <= 'Z') return (char)(c + 32);
    return c;
}

/* Append an unsigned integer as decimal text. */
static void s_cat_uint(char *dst, unsigned v) {
    char tmp[12]; int n = 0;
    if (v == 0) tmp[n++] = '0';
    while (v) { tmp[n++] = (char)('0' + (v % 10)); v /= 10; }
    char *d = dst + s_len(dst);
    while (n) *d++ = tmp[--n];
    *d = 0;
}

/* Append a money amount held in cents, e.g. 1250 -> "$12.50". */
static void s_cat_money(char *dst, unsigned cents) {
    s_cat(dst, "$");
    s_cat_uint(dst, cents / 100);
    s_cat(dst, ".");
    unsigned r = cents % 100;
    char *d = dst + s_len(dst);
    *d++ = (char)('0' + (r / 10));
    *d++ = (char)('0' + (r % 10));
    *d = 0;
}

/* Deterministic PRNG (xorshift32) so the preview behaves the same
   on every machine — no hidden state, nothing collected. */
static unsigned rng_state = 0x2F6E2B1u;
static unsigned rng_next(void) {
    unsigned x = rng_state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    rng_state = x;
    return x;
}

/* ---------- the preview auction ----------
 * One lot, a fair-close clock: any bid inside the final 10 seconds
 * extends the round back to 10 seconds, so nobody can snipe a lot
 * at the last instant. Preview bids from "the room" are simulated
 * here in C so a visitor can feel a round before the first real
 * seller goes live.
 */

static unsigned lot_price;      /* cents */
static int      lot_seconds;    /* seconds left in the round */
static int      lot_bids;       /* total bids this round */
static int      lot_state;      /* 0 = running, 1 = ended */
static int      you_lead;       /* 1 = the visitor holds the top bid */

static void log_line(const char *msg) {
    /* Keep the log bounded: drop the oldest half when nearly full. */
    if (s_len(result_buf) > RESULT_SZ - 512) {
        int half = RESULT_SZ / 2, i = 0;
        while (result_buf[half + i] && i < RESULT_SZ - half - 1) {
            result_buf[i] = result_buf[half + i]; i++;
        }
        result_buf[i] = 0;
    }
    s_cat(result_buf, msg);
    s_cat(result_buf, "\n");
}

void am_auction_reset(unsigned start_cents) {
    lot_price   = start_cents ? start_cents : 100;
    lot_seconds = 120;
    lot_bids    = 0;
    lot_state   = 0;
    you_lead    = 0;
    rng_state   = 0x2F6E2B1u;
    result_buf[0] = 0;
    log_line("The preview room is open. Every bid below is a demonstration.");
}

int am_place_bid(unsigned cents) {
    if (lot_state) return 2;                       /* round over */
    unsigned min_next = lot_price + (lot_price >= 10000 ? 500 :
                                     lot_price >= 2500  ? 200 : 100);
    if (cents < min_next) return 1;                /* below the next step */
    lot_price = cents;
    lot_bids++;
    you_lead = 1;
    if (lot_seconds < 10) lot_seconds = 10;        /* fair-close extension */
    char line[96]; line[0] = 0;
    s_cat(line, "Your bid: ");
    s_cat_money(line, cents);
    s_cat(line, " — you hold the lot.");
    log_line(line);
    return 0;
}

void am_auction_tick(void) {
    if (lot_state) return;
    if (lot_seconds > 0) lot_seconds--;
    if (lot_seconds == 0) {
        lot_state = 1;
        log_line(you_lead ? "Round closed. The lot would be yours — giving stated plainly, as promised."
                          : "Round closed. The next preview round can be started any time.");
        return;
    }
    /* A simulated room bid roughly one tick in five, while the round
       is young and the price is still modest. */
    if (lot_seconds > 12 && lot_bids < 60 && (rng_next() % 5) == 0) {
        unsigned step = lot_price >= 10000 ? 500 : lot_price >= 2500 ? 200 : 100;
        lot_price += step + (rng_next() % 3) * 100;
        lot_bids++;
        if (you_lead) {
            you_lead = 0;
            char line[96]; line[0] = 0;
            s_cat(line, "Preview bid: ");
            s_cat_money(line, lot_price);
            s_cat(line, " — the room outbid you.");
            log_line(line);
        } else {
            char line[96]; line[0] = 0;
            s_cat(line, "Preview bid: ");
            s_cat_money(line, lot_price);
            s_cat(line, ".");
            log_line(line);
        }
        if (lot_seconds < 10) lot_seconds = 10;    /* fair-close applies to all */
    }
}

unsigned am_price(void)       { return lot_price; }
int      am_seconds_left(void){ return lot_seconds; }
int      am_bid_count(void)   { return lot_bids; }
int      am_state(void)       { return lot_state; }
int      am_you_lead(void)    { return you_lead; }
unsigned am_next_step(void) {
    return lot_price + (lot_price >= 10000 ? 500 : lot_price >= 2500 ? 200 : 100);
}

/* ---------- shared buffers (JS <-> C) ---------- */

const char *am_result_ptr(void) { return result_buf; }
int am_result_len(void)         { return s_len(result_buf); }
char *am_scratch_q(void)        { return scratch_q; }
char *am_scratch_t(void)        { return scratch_t; }

/* ---------- search scoring ----------
 * Case-insensitive score of the query (scratch Q) against a text
 * (scratch T): whole-word and substring hits, with a bonus when the
 * text opens with the query. Used by the Explore rail so filtering
 * is decided here, not in page script.
 */
int am_search_score(int qlen, int tlen) {
    const char *q = scratch_q;
    const char *t = scratch_t;
    if (qlen > 255)  qlen = 255;
    if (tlen > 2047) tlen = 2047;
    if (qlen <= 0) return 1;                       /* empty query matches all */
    if (tlen <= 0) return 0;
    int score = 0;
    for (int i = 0; i + qlen <= tlen; i++) {
        int hit = 1;
        for (int j = 0; j < qlen; j++)
            if (lower_of(t[i + j]) != lower_of(q[j])) { hit = 0; break; }
        if (hit) {
            score += 2;
            if (i == 0 || t[i - 1] == ' ' || t[i - 1] == '\n') score += 3;
            i += qlen - 1;
        }
    }
    return score;
}

/* ---------- contact address, assembled in C ----------
 * The address never sits in the page source for scrapers to lift;
 * it is built here and handed to the loader to render as a link.
 */
const char *am_contact(void) {
    char *w = contact_buf;
    w[0] = 0;
    s_cat(w, "joshua");
    s_cat(w, ".");
    s_cat(w, "foraker");
    s_cat(w, "@");
    s_cat(w, "almsmercatus");
    s_cat(w, ".");
    s_cat(w, "com");
    return w;
}
