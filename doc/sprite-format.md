# Vega Strike sprite (`.spr`) format

A `.spr` file is a small text file describing a 2D sprite: the main menu, the HUD, the base screens and the
cockpit all draw from them. The engine reads one line at a time in `VSSprite`
(`engine/src/gfx/sprite.cpp`).

## The lines

```
<image, or a path ending in .ani>  <alphamap, or 0 for none>
<width> <height>
<centre x> <centre y>              — or, when the sprite is animated, an animation header
<imagefile> <flag> [options]       — the first frame (animated sprites only)
```

A still sprite stops after line three.

- **Line 1, first token** is the image to draw — *or* a path ending in `.ani`, which marks the sprite as
  animated. Only the suffix matters; see below.
- **Line 1, second token** is the alphamap to load, or `0` for none. Animated sprites do not use it: their
  frames carry their own.
- **Line 2** is the sprite's size. The loader halves the width and negates the height internally.
- **Line 3** is the sprite's own centre — unless the sprite is animated *and* writes an animation header
  there instead.
- **Line 4 onwards** are frames.

## Animated or still: line one's `.ani` suffix

The engine decides a sprite is animated by looking at whether line one's first token ends in `.ani`:

```cpp
if (texture ends with ".ani") {
    surface = new AnimatedTexture(f, ...);   // f is the .spr itself
    isAnimation = true;
}
```

**That path is never opened.** The token is a sentinel that selects the animated code path; the animation's
header and frames are read from the rest of the `.spr` file. Naming an `.ani` that does not exist is
therefore harmless — but see the notes for authors below.

## Line three means two things

For an animated sprite, line three is either

- **the sprite's own centre** (two fractions), with the animation header on line four; or
- **the animation header**, for files written without a centre.

The header is `<frame count> <seconds per frame> [options]`, and it announces itself: its first token is an
integer of one or more. A centre is a pair of small fractions. So `1 1000 mins=…` is a header, and
`0.0315 -0.636` is a centre — the loader tells them apart by what line three says on its own.

Do not try to tell them apart by looking at the *next* line: a frame line reads like a header's own
contents, and the file reader skips blank lines, so a file with a blank line four misleads that test.

**Why a misread centre is a silent failure:** with `graphics.offset_sprites_by_pos` set, a sprite's centre
is *added* to the position the screen places it at. A header read as a centre gives a centre like
`(1, 1000)`, which puts the sprite a thousand units off the top of the screen — while its texture, its
frames and its band all measure correct.

## Frames, the flag token, and the rectangle

A frame line is `<imagefile> <flag> [options]`.

**The second token is a flag, not a filename.** `true`, `false`, `-` or nothing at all all mean "this frame
has no alphamap". Reading it as a filename makes the alphamap load fail and the frame never becomes a
usable texture, which draws nothing at all — silently.

The `options` on a frame line and on the animation header carry the frame's texture rectangle in normalised
texture coordinates — `mins=`, `mint=`, `maxs=`, `maxt=` — so several frames, or several sprites, can share
one image and each carve out its own band of it. That is how a single `menu_buttons.image` holds every menu
button, and `menu_buttons_pressed.image` every pressed state.

## The two shapes, and what shipped content uses

| shape | line 3 | line 4 |
|---|---|---|
| header form | the animation header | `<imagefile> <flag>` |
| centre form | the sprite's centre | the animation header |

Both are accepted. In the 0.11 content, all sprites that name an `.ani` write the **header form**:

```
interfaces/main_menu/menu_buttons.ani 0
1.94 1.94
1 1000 mins=0,mint=0.196,maxs=1,maxt=0.289
interfaces/main_menu/menu_buttons.image true
```

and a still sprite is simply:

```
interfaces/main_menu/menu.image 0
1.94 1.94
0 0
```

## Notes for authors

- **Name an `.ani` only when the sprite is animated.** The path is a sentinel, so a name that points nowhere
  costs nothing at run time — but it reads as a missing asset, and has been reported as one.
- **A still sprite is the simpler form**: image on line one, size, centre, and stop. No `.ani`, no frame.
- **Keep line three's tokens positional.** `1 2` is a header; `0.03 -0.64` is a centre; the loader cannot
  recover the difference from anything else on the line.

## History

Before 0.11 the loader read line three as a centre unconditionally, and opened the image line's flag token
as an alphamap filename. Header-form animated sprites were therefore drawn off-screen (their centre read as
`(1, 1000)`) and animated frames never became usable textures, so animated sprites drew nothing at all.
Both are fixed in 0.11.
