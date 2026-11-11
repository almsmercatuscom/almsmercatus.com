# Almus Mercatus — The Altruistic Mart

almsmercatus.com · Live at https://www.almsmercatus.com

From Latin: *almus* (kind, nourishing, benevolent) + *mercatus* (market, mart).

A live-shopping marketplace in the making, presented with Apple-style
navigation and restraint: one idea per section, benefits before features,
nothing on the page that cannot be said plainly.

## How it is built

- `index.html` — the whole front door: global nav, hero, a working preview
  of the live room, Explore, How it works, Giving, About, footer.
- `src/app.c` — **all custom logic is C11** (ISO/IEC 9899:2011), freestanding,
  compiled to WebAssembly and embedded in the page: the auction clock, bid
  validation, the fair-close extension, preview bids, search scoring, and
  assembling the contact address. The inline script in `index.html` is only
  a loader that starts the module and moves data between the page and C.
  Rebuild with clang:
  `clang --target=wasm32 -std=c11 -O2 -nostdlib -Wl,--no-entry -Wl,--export-all -o app.wasm src/app.c`
  (or zig: `zig cc -target wasm32-freestanding -std=c11 -O2 -nostdlib -c src/app.c` then
  `zig wasm-ld --no-entry --export-all -o app.wasm`), then base64-embed the module
  where the base64 string sits in `index.html`.
- `robots.txt` — declines all crawling, including named AI/data bots.
- `labyrinth.html` — a honeypot of fabricated content for scrapers that
  ignore robots.txt. Nothing on it is real.
- `CNAME` — GitHub Pages custom domain: www.almsmercatus.com.

## Open TODOs for Joshua to decide

- Concrete giving model (e.g. % of proceeds, featured causes, seller pledge)
- Real call to action / shop link once the first shows are scheduled
- Newsletter signup (contact email contact@almsmercatus.com is live,
  assembled by the C engine so scrapers cannot lift it from the source)

## Trades edition (2026-11-10)

The platform is tailored to the skilled trades — the research behind it is in
the goal workspace (trades-research.md). On the page:

- **The Trades Rail** (Explore): power tools (from $40), hand tools ($10),
  test & measurement ($35), welding ($60), surplus material ($15), and whole
  kits & buyouts (appraisal free). Every lot is Tested Live, serials shown.
- **Services**: Tested Live certification $15/lot; serial-check included;
  kit appraisal free; apprentice starter kits from $199; crew outfitting per
  lot; calibration dates stated on test gear.
- **Field notes**: altruistic tips and free-education pointers woven through
  the site (a rotating note in the hero plus cards between sections) — help
  given away free, on the house.
- **Payments**: purchases and donations will run through Stripe (card,
  Apple Pay, bank); the Support section is live and the payment links are
  wired as soon as the owner's Stripe account is connected. The mart never
  sees or stores card numbers.

## Original mechanisms (written from scratch, 2026-11-10)

Studied the platforms tradespeople frequent; borrowed none of their
features. Built our own, modeled on the physical supply house:

- **The Rail** — the live room. **The Bench** — where lots are tested and
  graded. **The Counter** — free straight answers. **The Yard** — surplus.
- **The Two-Minute Truth** — every lot: run under load, serial shown,
  flaws named, before the first bid.
- **The condition rubric** — Work-Ready / Bench-Worn / Project, plus the
  rule: no grade without the bench.
- **Chain of Hands** — a provenance card (origin, hands, work) plus a
  bill of sale with the serial, traveling with every serialized tool.
- **The handover** — a retiring seller's two-minute live handover is
  recorded and travels with a whole kit to its next owner.

## Principles folded in from the platform study (2026-11-10)

From platform-principles-research.md — principles, not features:
public bench protocol + the fixed Bench Card on every lot (fault list
never blank); one-number pricing stated in the open; the post-sale job
loop "from sold to settled" (closes only on buyer confirmation); The
Mart's Word — a house-backed remedy keyed to the recorded bench test;
the Apprentice Hour (masters teaching live before featured shows);
a named, accountable human at The Counter.

## Aesthetic evidence pass (2026-11-10)

Applied from aesthetic-research.md (sourced readability/perception
studies): split the gold token — decorative gold stays #a5813c, but
small text now uses --gold-ink #7d5f22 (5.3:1 on parchment, AA+);
--faint darkened #96835f -> #7a6a48 (4.7:1); foil gradient deepened;
step numerals moved off the low-contrast soft gold; long prose bumped
to 18px and capped at ~38rem measure everywhere. Kept, on evidence:
Palatino/Georgia serif direction, parchment ground, ink and burgundy
(both AAA), 17px/1.65 base settings, light theme with dark accents.
