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
