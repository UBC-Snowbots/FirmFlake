# docs/: architecture diagrams

`architecture.drawio` holds four pages. Who maintains each page matters:

| Page (`<diagram id>`) | Maintained by | Claude may edit? |
|---|---|---|
| 1. Overview (`overview`) | humans, in draw.io | **No**, unless explicitly asked. Then make a minimal in-place edit. |
| 2. Build (`build`) | humans, in draw.io | **No**, unless explicitly asked. |
| 3. Interfaces vs implementations (`iface`) | humans, in draw.io | **No**, unless explicitly asked. |
| 4. Block diagram (`blocks`) | Claude, via `gen_block_diagram.py` | **Yes**. Regenerate it, don't hand-edit. |

## Rules

- **Never regenerate or rewrite the whole `.drawio` file.** Pages 1–3 contain human edits that exist nowhere else.
  `gen_block_diagram.py` replaces only `<diagram id="blocks">`. It aborts if anything else would change. Keep that check.
- If you must touch pages 1–3 (only when asked), edit the specific attributes or cells in place with a script.
  Back up the file to the scratchpad first, then check that nothing else changed.
- The file is often **open in draw.io** while you work. After writing it, tell the user to reload it before
  saving. Otherwise draw.io's stale copy overwrites your change. Check the file's mtime before you start, too.
- Tooltips use HTML with a 15px font: `<div style="font-size:15px;line-height:1.45;max-width:520px;padding:4px">…</div>`.
  Plain-text tooltips render too small. Use the same wrapper for any new tooltip on any page.
- `*.bkp` files are draw.io's autosave backups. They are gitignored, so leave them alone.

## Updating the block diagram (page 4)

Do this when modules are added, removed, renamed or re-wired in `common_libraries/` or `platform/`, when a
new platform appears, or when a board starts or stops using a module.

1. **Gather the facts from the code.** Don't rely on memory or on the current diagram.
   ```bash
   # modules and their non-std #includes (don't filter with broad patterns like <c*>: that hides comms.h/crc.h/can_interface.h)
   git ls-files common_libraries platform | grep -v HAL_Drivers | grep -E '\.(h|hpp|c|cpp)$' | grep -v /test/ \
     | xargs grep -H -E '^\s*#include'
   # which modules each board compiles in (target_sources) and includes
   grep -n -E 'COMMON_LIBS_PATH|PLATFORM_PATH' */CMakeLists.txt
   # external users (RoverFlake2 has FirmFlake as a submodule; it's at ~/RoverFlake2 on the main dev laptop)
   grep -rn --include=CMakeLists.txt -i 'FirmFlake/common_libraries' ~/RoverFlake2/src 2>/dev/null
   ```
2. **Edit the data tables** at the top of `gen_block_diagram.py`:
   - `MODULES`: one block per module folder. Platform modules use `name=` (e.g. `can_stm32g4xx`) so they
     aren't confused with the common folder of the same name.
     - `what` starts with `Interface:`, `Generic:`, `Impl:`, `Template:` or `Vendor:`.
     - `used_by` lists the boards or external packages that compile it in, or `via <module>` when it's only
       reached through another module. An empty list draws the block dashed ("not used yet").
     - `needs` lists headers the board project must supply (e.g. `platform_defs.h`).
     - `tests=True` if `<module>/test/` exists.
   - `EDGES`: `(A, B)` means A `#include`s / depends on B. Only add edges between blocks, and keep them
     simple: one arrow per module pair, all the same arrow style. Don't draw board → module arrows. That
     information goes in `used_by`.
   - Layout is a grid: `col` 0–5, `row` from `ROW_Y` (0 = top). **Hardware layers go at the very bottom**
     and software sits on top: common_libraries → platform modules → HAL bar → "Peripheral registers
     (memory-mapped I/O)" bar in the `silicon` group. Full-width layers use `span=6, bar=True`. Line each impl
     block up under its interface (same `col`) so the arrows are straight.
   - **New platform** (e.g. `platform/esp32/`): add it to `GROUPS` and `GROUP_ORDER`, add rows to
     `ROW_Y`/`ROW_H` (shift the rows below it down), and give it its own HAL/vendor bar if it has one.
3. **Run it** from the repo root: `python3 docs/gen_block_diagram.py`. It stamps the commit and date on the page.
4. **Check it visually**. Arrows through blocks and clipped text are the usual problems:
   ```bash
   cp docs/architecture.drawio ~/arch_tmp.drawio   # drawio is a snap: it can't read /tmp
   drawio -x -f png -p 4 -s 1.2 -o ~/arch_p4.png ~/arch_tmp.drawio --no-sandbox   # -p is 1-based
   ```
   Read the PNG, then move it to the scratchpad and delete `~/arch_tmp.drawio`.
5. Tell the user what changed on the diagram and remind them to reload draw.io.

## Known facts worth keeping (verify they still hold)

- Boards compile common `.cpp` files directly in their own `CMakeLists.txt`. They don't link
  `common_libraries/CMakeLists.txt`, which only builds the PC-side Catch2 tests (`make test`, run in CI).
- In RoverFlake2, only `arm_hardware_interface` uses common_libraries (it compiles `comms/comms.c`).
  `rover_hmi` does **not**. It only matches a grep for "common" because of `rover_arm_common`.
