# HARFBUZZ-TODO.md — HarfBuzz: open questions & investigation log

> Captures the 2026-09-06 discussion about how harfbuzz should (or shouldn't)
> get wired into mulle-nanovg. Status: **further investigation required** —
> no code changed as a result of this doc.

## TL;DR

We disagree on one factual point and agreed to settle it with proof:

- **Claim A (verified in code):** freetype's `FT_CONFIG_OPTION_USE_HARFBUZZ`
  is *not* a text shaper — per freetype's own `ftoption.h` it only lets the
  **auto-hinter** consult harfbuzz for glyphs that aren't reachable via the
  font's character map (i.e. glyphs a shaper would have produced). And
  mulle-nanovg contains **zero harfbuzz callers**, so no freetype/harfbuzz
  configuration can make its text come out shaped.
- **Claim B (user's memory, no proof yet):** kerning differences were visible
  when harfbuzz was in play. This is *consistent with* real shaping having
  happened in whatever setup produced that observation (GPOS kerning only
  appears in shaped output), so it deserves an explanation, not a dismissal.

The two claims can both be true if the remembered setup had a shaping layer
that the current mulle-nanovg tree does not. Finding that layer is the task.

## Verified facts (2026-09-06, this tree)

- freetype 2.13.3 `include/freetype/config/ftoption.h` (comment at the
  `FT_CONFIG_OPTION_USE_HARFBUZZ` define, lines ~279–293):

  > FreeType uses the HarfBuzz library to improve auto-hinting of OpenType
  > fonts. If available, many glyphs not directly addressable by a font's
  > character map will be hinted also.

- `mulle-nanovg/fontstash.h` + `mulle-nanovg/nanovg.c` (the only TUs that
  touch fonts): the only `FONS_USE_*` defines are `FONS_USE_FREETYPE` /
  `FONS_USE_STB_TRUETYPE` (rasterizer backend selection). **No** harfbuzz
  references, no `hb_*` calls, no `FONS_USE_HARFBUZZ`-style switch.
- Kerning today is legacy FT pair kerning: `FT_Get_Kerning(...,
  FT_KERNING_DEFAULT, ...)` at `fontstash.h:721`. That is *not* GPOS
  positioning; it misses ligatures, mark positioning, and most real-world
  kerning in modern fonts (which often ship GPOS-only kerning).
- mulle-sde sourcetree declares harfbuzz as an **optional, unused**
  dependency: `cmake/reflect/_Libraries.cmake` (find_library only) and
  `dependency/etc/craftorder` (`stash/harfbuzz;...no-load...` on Linux
  desktop). No C code consumes it.
- Android (this repo): freetype built with `FT_DISABLE_HARFBUZZ=TRUE`;
  harfbuzz 14.4.0 cross-compiled statically but linked-unused (linker drops
  it). See `android/build-libs.sh` and `ANDROID-MATCHPLAN.md` Phase 4.5.

## The reframe (two mechanisms, often conflated)

| Mechanism | What it does | Who calls it | Result |
|---|---|---|---|
| **Shaping** (`hb_shape`) | unicode string → positioned glyph IDs via GSUB/GPOS | must be a caller in the text stack (none today) | ligatures, real kerning, complex scripts, ZWJ emoji |
| **freetype + hb** (`FT_CONFIG_OPTION_USE_HARFBUZZ`) | auto-hinter hints substituted glyphs better | freetype itself, at glyph load | only matters *after* shaping exists |

Consequence: enabling harfbuzz anywhere in the build (desktop or Android)
cannot by itself change rendered text in mulle-nanovg, because nothing shapes.

## The user's counter-observation (to be explained, not dismissed)

"I remember seeing kerning differences when using harfbuzz."

If that happened, the most likely explanations, in order:

1. **A shaping layer was active in that setup** — e.g. a different app /
   branch of nanovg that called `hb_shape`, or text rendered through a stack
   that shapes (CoreText/DirectWrite/Pango) rather than mulle-nanovg's
   fontstash. GPOS kerning differences only appear in shaped output.
2. **A different fontstash copy was in use** — one that shaped or that read
   GPOS kerning; the vendored `fontstash.h` here may not match the one from
   the observed session.
3. freetype's hb auto-hinting changed *hinting* (not kerning), and the
   difference was misattributed.

## Investigation plan (proof, not vibes)

- [ ] **Find the shaping layer in the mulle desktop stack.** Grep the whole
      mulle dependency universe (mulle-freetype, mulle-sde stashes, any other
      MulleUI text consumer), not just mulle-nanovg, for `hb_shape` /
      `hb_buffer_*` callers. `dependency/etc/craftorder` says harfbuzz is
      `no-load` on Linux desktop — confirm whether it is loaded on any
      platform mulle-nanovg actually runs on.
- [ ] **Inspect `MulleUI/mulle-freetype`** (the wrapper whose craftinfos set
      defines). What exactly does it configure? Does it only enable
      `FT_CONFIG_OPTION_USE_HARFBUZZ`, or does it ship any shaping helper?
      Compare its vendored freetype/fontstash to this repo's.
- [ ] **A/B the kerning question directly.** Build two freetype binaries
      (with/without harfbuzz + `FT_CONFIG_OPTION_USE_HARFBUZZ`) and compare
      `FT_Get_Kerning` + rasterized output for a GPOS-kerning font (e.g.
      Roboto "AV", "To", "fi"). Expectation per claim A: identical, because
      no shaper runs. If they differ → claim A is wrong for this code path
      and we have a real bug to chase.
- [ ] **A/B with a real shaper** to establish what "kerning differences when
      using harfbuzz" look like: shape the same strings via `hb_shape`
      (hb-ft over the same FT face) and compare advances vs the fontstash
      path. This pins the memory to mechanism #1 and quantifies the delta
      (ligatures, GPOS kerning) that a define-gated shaping path would add.
- [ ] **Check desktop craft defines actually applied** — is
      `FT_CONFIG_OPTION_USE_HARFBUZZ` even set by mulle-freetype, or is the
      freetype/hb link the only thing happening (Ubuntu/Debian build
      freetype *with* hb linked by default)? Note: linking hb into freetype
      is not the same as defining USE_HARFBUZZ.

## Where this could land (decisions deferred until investigation)

- **Parity-only (small):** build Android freetype *with* harfbuzz linked
  (drop `FT_DISABLE_HARFBUZZ`, link the static hb we already cross-compile)
  so Android freetype is config-identical to the desktop/mulle-freetype
  build. Changes nothing visible; removes divergence. Cheap to do now or
  later.
- **Real shaping (the only thing that changes output):** add an internal,
  compile-define-gated path in fontstash (`FONS_USE_HARFBUZZ`) that shapes
  glyph runs with hb before rasterization. **Constraint from the user: no
  new public API.** Scope notes: fontstash's text path is per-codepoint with
  multi-font fallback + ClearType dual contexts; kerning/ligatures on the
  active font is the tractable first slice; emoji ZWJ sequences are the
  bigger slice already sketched in `EMOJI-ZWJ-PLAN.md` (run buffering +
  glyph-by-ID loading + color-emoji machinery).
- **Do nothing:** keep harfbuzz built-but-unused on Android; revisit when
  color-emoji/ZWJ or attributed-text (`nvgAttributedText-proposal.md`) work
  starts.

## What the user explicitly does NOT want

- No new public API for shaping (`nvgShapeText` etc.). If an implementation
  turns out to require public API, stop and revisit rather than adding it.
- No "vibecoding" the shaping path in: the freetype/hb relationship must be
  understood and proven first (this doc exists for that reason).
