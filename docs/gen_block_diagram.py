#!/usr/bin/env python3
"""
Regenerates ONLY the "4. Block diagram" page (<diagram id="blocks">) inside docs/architecture.drawio.
Every other page is human-maintained and is left byte-for-byte untouched (checked before writing).

Usage:  python3 docs/gen_block_diagram.py            # from the repo root
To update the diagram: edit MODULES / EDGES below, then re-run. See docs/CLAUDE.md.
"""
import datetime
import os
import re
import subprocess
import sys
from xml.sax.saxutils import quoteattr

DRAWIO = os.path.join(os.path.dirname(os.path.abspath(__file__)), "architecture.drawio")
PAGE_ID = "blocks"
PAGE_NAME = "4. Block diagram (modules)"

# ---------------------------------------------------------------------------------------------
# Data: one entry per block. Keep this in sync with the repo (see docs/CLAUDE.md).
#   name  : block title (default: "<last folder of path>/")
#   group : which container the block sits in (key of GROUPS)
#   col   : grid column (0..5), row: grid row (key of ROW_Y, 0 = top), span: columns wide (default 1)
#   bar   : True for a thin full-width layer bar (HAL, registers)
#   path  : folder, relative to the repo root
#   files : main files (short)
#   what  : one line, what it is. Prefix with "Interface:", "Generic:" or "Impl:".
#   used_by : boards / external packages that compile it in (from their CMakeLists target_sources,
#             or for header-only modules, their includes). Empty list => drawn dashed ("not used yet").
#   needs : headers the BOARD must provide (e.g. platform_defs.h)
#   tests : True if it has Catch2 tests under <module>/test/
# ---------------------------------------------------------------------------------------------
GROUPS = {
    "silicon": ("STM32G4 silicon", "the chip itself", ("#f5f5f5", "#666666")),
    "stm32g4xx": ("platform/stm32g4xx/", "hardware-specific: STM32G4 implementations", ("#ffe6cc", "#d79b00")),
    "common": ("common_libraries/", "hardware-agnostic: interfaces + generic code", ("#d5e8d4", "#82b366")),
}
GROUP_ORDER = ["common", "stm32g4xx", "silicon"]  # top to bottom: software on top, hardware at the very bottom

ROW_Y = {0: 110, 1: 270, 2: 490, 3: 650, 4: 790}
ROW_H = {0: 130, 1: 130, 2: 130, 3: 50, 4: 100}
COL_X0, COL_W, COL_GAP = 100, 210, 30

MODULES = [
    # --- hardware (very bottom): not code, the chip itself ---
    dict(id="regs", name="Peripheral registers (memory-mapped I/O)", group="silicon", col=0, row=4, span=6, bar=True,
         path="platform/stm32g4xx/HAL_Drivers/CMSIS/Device/ST/STM32G4xx/Include",
         files="addresses in stm32g431xx.h, stm32g474xx.h",
         what="Hardware: GPIO, TIM, FDCAN, ADC... are driven by reading/writing fixed memory addresses, "
              "e.g. GPIOA->ODR at 0x48000014",
         used_by=["HAL_Drivers"]),
    # --- platform/stm32g4xx ---
    dict(id="hal", name="HAL_Drivers/", group="stm32g4xx", col=0, row=3, span=6, bar=True, path="platform/stm32g4xx/HAL_Drivers",
         files="STM32G4xx_HAL_Driver, CMSIS", what="Vendor: ST's HAL + ARM CMSIS (not our code)",
         used_by=["every STM32G4 board (via cmake/stm32cubemx)"]),
    dict(id="can_stm32", name="can_stm32g4xx", group="stm32g4xx", col=0, row=2, path="platform/stm32g4xx/Modules/can",
         files="can_stm32g4xx.h/.cpp", what="Impl: CanSTM32G4XX : CanInterface (FDCAN)",
         used_by=["PDB_24", "mini_rover"], needs=["platform_defs.h", "gpio_config_base.h"]),
    dict(id="clock_stm32", name="clock_stm32g4xx", group="stm32g4xx", col=1, row=2, path="platform/stm32g4xx/Modules/clock",
         files="clock_stm32g4xx.hpp/.cpp", what="Impl: ClockSTM32G4XX : ClockInterface (HAL_GetTick)",
         used_by=["PDB_24", "mini_rover"]),
    dict(id="pio_stm32", name="primitive_io_stm32g4xx", group="stm32g4xx", col=3, row=2, path="platform/stm32g4xx/Modules/primitive_io",
         files="primitive_io_stm32g4xx.hpp/.cpp",
         what="Impl: digital out + analog in (ADC) for STM32G4",
         used_by=["PDB_24", "mini_rover"]),
    dict(id="servo_stm32", name="servo_stm32g4xx", group="stm32g4xx", col=4, row=2, path="platform/stm32g4xx/Modules/servo",
         files="servo_stm32g4xx.hpp/.cpp", what="Impl: ServoSTM32G4XX : ServoInterface (timer PWM)",
         used_by=["mini_rover"], needs=["platform_defs.h"]),
    dict(id="datatypes", name="main_datatypes_stm32g4xx", group="stm32g4xx", col=5, row=2, path="platform/stm32g4xx",
         files="main_datatypes_stm32g4xx.h", what="STM32 hardware structs (port/pin/timer/ADC)",
         used_by=["servo_stm32g4xx"]),

    # --- common_libraries ---
    dict(id="can", group="common", col=0, row=1, path="common_libraries/can",
         files="can_interface.h", what="Interface: CanInterface (init / send / read)",
         used_by=["via can_stm32g4xx"]),
    dict(id="clock", group="common", col=1, row=1, path="common_libraries/clock",
         files="clock_interface.hpp, timer, stopwatch",
         what="Interface: ClockInterface. Generic: Timer, Stopwatch",
         used_by=["PDB_24", "mini_rover"]),
    dict(id="usr_leds", group="common", col=2, row=1, path="common_libraries/usr_leds",
         files="usr_led.hpp/.cpp", what="Generic: UsrLed / UsrLedControl (4 status LEDs)",
         used_by=["PDB_24", "mini_rover"]),
    dict(id="pio", group="common", col=3, row=1, path="common_libraries/primitive_io",
         files="primitive_io_base.hpp",
         what="Interface: PrimitiveDigitalOutput, PrimitiveAnalogInput",
         used_by=["via primitive_io_stm32g4xx"]),
    dict(id="servo", group="common", col=4, row=1, path="common_libraries/servo",
         files="servo_interface.hpp", what="Interface: ServoInterface (positional / continuous)",
         used_by=["via servo_stm32g4xx"]),
    dict(id="comms", group="common", col=0, row=0, path="common_libraries/comms",
         files="comms.h/.c, device_ids.h",
         what="Generic: CAN message format + device IDs, shared with ROS2",
         used_by=["PDB_24", "mini_rover", "RoverFlake2 arm_hardware_interface"], tests=True),
    dict(id="crc", group="common", col=1, row=0, path="common_libraries/crc",
         files="crc.h/.c", what="Generic: CRC16 (CCITT-FALSE)", used_by=["via comms"], tests=True),
    dict(id="lpf", group="common", col=2, row=0, path="common_libraries/low_pass_fir_filter",
         files="low_pass_fir_filter.h", what="Generic: moving-average low-pass filter (template)",
         used_by=[], tests=True),
    dict(id="sensors", group="common", col=3, row=0, path="common_libraries/sensors",
         files="sensor_interface.hpp", what="Interface: Sensor (no implementation yet)", used_by=[]),
    dict(id="example", group="common", col=4, row=0, path="common_libraries/example",
         files="some_lib.h/.cpp", what="Template: how to write + test a module", used_by=[], tests=True),
]

# (from, to) = "from depends on / #includes to". Keep it to real #include dependencies between blocks.
EDGES = [
    ("can_stm32", "can"), ("can_stm32", "hal"),
    ("clock_stm32", "clock"), ("clock_stm32", "hal"),
    ("pio_stm32", "pio"), ("pio_stm32", "hal"),
    ("servo_stm32", "servo"), ("servo_stm32", "pio"), ("servo_stm32", "datatypes"), ("servo_stm32", "hal"),
    ("datatypes", "hal"),
    ("hal", "regs"),
    ("usr_leds", "clock"), ("usr_leds", "pio"),
    ("can", "comms"),
    ("comms", "crc"),
]

# ---------------------------------------------------------------------------------------------
# Rendering
# ---------------------------------------------------------------------------------------------
BOX = ("rounded=1;whiteSpace=wrap;html=1;arcSize=6;fillColor={f};strokeColor={s};"
       "verticalAlign=top;align=left;spacing=6;spacingTop=2;fontSize=11;")
GROUP = ("rounded=0;whiteSpace=wrap;html=1;fillColor={f};strokeColor={s};opacity=35;horizontal=0;"
         "verticalAlign=top;align=center;spacingTop=4;fontSize=14;")
TEXT = "text;html=1;whiteSpace=wrap;align=left;verticalAlign=top;spacing=4;"
EDGE = ("edgeStyle=orthogonalEdgeStyle;rounded=1;html=1;endArrow=block;endFill=1;strokeWidth=1.5;"
        "strokeColor=#333333;jettySize=auto;orthogonalLoop=1;")


def esc(t):
    return t.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def geom(m):
    span = m.get("span", 1)
    x = COL_X0 + m["col"] * (COL_W + COL_GAP)
    w = span * COL_W + (span - 1) * COL_GAP
    return x, ROW_Y[m["row"]], w, ROW_H[m["row"]]


def title(m):
    return m.get("name") or m["path"].split("/")[-1] + "/"


def label(m):
    parts = [f"<b>{esc(title(m))}</b>",
             f'<font style="font-size:10px" face="Courier New">{esc(m["files"])}</font>',
             esc(m["what"])]
    if m.get("bar"):  # thin full-width bar
        return " &nbsp; ".join(parts)
    used = m.get("used_by") or []
    parts.append(f'<font color="#555555" style="font-size:10px">used by: {esc(", ".join(used)) if used else "<i>nothing yet</i>"}</font>')
    if m.get("needs"):
        parts.append(f'<font color="#a05000" style="font-size:10px">board must provide: {esc(", ".join(m["needs"]))}</font>')
    if m.get("tests"):
        parts.append('<font color="#2d7d2d" style="font-size:10px">✓ unit tests</font>')
    return "<br>".join(parts)


def tooltip(m):
    t = [f"<b>{esc(title(m))}</b>  ({esc(m['path'])}/)", esc(m["what"]), f"Files: {esc(m['files'])}",
         f"Used by: {esc(', '.join(m.get('used_by') or ['nothing yet']))}"]
    if m.get("needs"):
        t.append("The board project must provide: " + esc(", ".join(m["needs"])))
    if m.get("tests"):
        t.append(f"Catch2 tests: {esc(m['path'])}/test/")
    return ('<div style="font-size:15px;line-height:1.45;max-width:520px;padding:4px">'
            + "<br>".join(t) + "</div>")


def edge_style(a, b):
    """Straight vertical/horizontal arrow when the boxes line up; otherwise let draw.io route it."""
    ax, ay, aw, ah = a
    bx, by, bw, bh = b
    lo, hi = max(ax, bx), min(ax + aw, bx + bw)
    if hi - lo >= 20 and ay != by:  # stacked: vertical arrow at the centre of the overlap
        x = (lo + hi) / 2
        down = by > ay
        return (f"exitX={(x - ax) / aw:.4f};exitY={1 if down else 0};exitDx=0;exitDy=0;"
                f"entryX={(x - bx) / bw:.4f};entryY={0 if down else 1};entryDx=0;entryDy=0;")
    if ay == by:  # same row: horizontal arrow
        right = bx > ax
        return (f"exitX={1 if right else 0};exitY=0.5;exitDx=0;exitDy=0;"
                f"entryX={0 if right else 1};entryY=0.5;entryDx=0;entryDy=0;")
    return None  # diagonal: caller routes it through the gap between rows (see dogleg)


def dogleg(a, b):
    """Diagonal dependency: leave the bottom/top near the side facing the target, run along the gap between
    the two rows, then enter the target from the top/bottom near the side facing the source."""
    ax, ay, aw, ah = a
    bx, by, bw, bh = b
    down = by > ay
    ex = 0.2 if bx < ax else 0.8
    nx = 0.8 if bx < ax else 0.2
    gap_y = (ay + ah + by) / 2 if down else (by + bh + ay) / 2
    style = (f"exitX={ex};exitY={1 if down else 0};exitDx=0;exitDy=0;"
             f"entryX={nx};entryY={0 if down else 1};entryDx=0;entryDy=0;")
    points = [(ax + ex * aw, gap_y), (bx + nx * bw, gap_y)]
    return style, points


def build_page():
    cells = []
    ids = {m["id"] for m in MODULES}
    for a, b in EDGES:
        assert a in ids and b in ids, f"edge {a}->{b} references an unknown module"

    try:
        commit = subprocess.run(["git", "rev-parse", "--short", "HEAD"], capture_output=True, text=True,
                                cwd=os.path.dirname(DRAWIO)).stdout.strip()
    except OSError:
        commit = "?"
    cells.append(cell("b-title", '<font style="font-size:22px"><b>Block diagram: modules and how they link</b></font><br>'
                      '<font color="#666666">Arrow A → B: A #includes / depends on B. Dashed = not compiled into any board yet. '
                      f"Hover a block for details. Generated by docs/gen_block_diagram.py from commit {esc(commit)} "
                      f"on {datetime.date.today().isoformat()}. Do not hand-edit this page.</font>",
                      40, 20, 1400, 70, TEXT))

    # group containers: all the same width, height fitted to their members, title rotated on a left strip
    xs = [geom(m) for m in MODULES]
    x0, x1 = min(g[0] for g in xs) - 70, max(g[0] + g[2] for g in xs) + 25
    for gid in GROUP_ORDER:
        gtitle, sub, (f, s) = GROUPS[gid]
        mem = [geom(m) for m in MODULES if m["group"] == gid]
        if not mem:
            continue
        y0 = min(g[1] for g in mem) - 20
        y1 = max(g[1] + g[3] for g in mem) + 20
        cells.append(cell(f"g-{gid}", f"<b>{esc(gtitle)}</b><br><i style=\"font-size:12px\">{esc(sub)}</i>",
                          x0, y0, x1 - x0, y1 - y0, GROUP.format(f=f, s=s)))

    for m in MODULES:
        f, s = GROUPS[m["group"]][2]
        style = BOX.format(f=f, s=s)
        if m.get("bar"):
            style += "verticalAlign=middle;"
        if not m.get("used_by") and not m.get("bar"):
            style += "dashed=1;fillColor=#ffffff;"
        x, y, w, h = geom(m)
        cells.append(cell("b-" + m["id"], label(m), x, y, w, h, style, tip=tooltip(m)))

    g = {m["id"]: geom(m) for m in MODULES}
    for a, b in EDGES:
        style, pts = edge_style(g[a], g[b]), ""
        if style is None:
            style, points = dogleg(g[a], g[b])
            pts = '<Array as="points">' + "".join(f'<mxPoint x="{x:.0f}" y="{y:.0f}"/>' for x, y in points) + "</Array>"
        cells.append(
            f'<mxCell id={quoteattr(f"be-{a}-{b}")} value="" style={quoteattr(EDGE + style)} '
            f'edge="1" parent="1" source="b-{a}" target="b-{b}"><mxGeometry relative="1" as="geometry">{pts}</mxGeometry></mxCell>')

    body = "\n        ".join(cells)
    return (f'<diagram id="{PAGE_ID}" name={quoteattr(PAGE_NAME)}>\n'
            f'    <mxGraphModel dx="1400" dy="900" grid="1" gridSize="10" guides="1" tooltips="1" connect="1" '
            f'arrows="1" fold="1" page="1" pageScale="1" pageWidth="1500" pageHeight="850" math="0" shadow="0">\n'
            f'      <root>\n        <mxCell id="0" />\n        <mxCell id="1" parent="0" />\n        {body}\n'
            f'      </root>\n    </mxGraphModel>\n  </diagram>')


def cell(cid, value, x, y, w, h, style, tip=None):
    geo = f'<mxGeometry x="{x}" y="{y}" width="{w}" height="{h}" as="geometry" />'
    if tip:
        return (f'<UserObject label={quoteattr(value)} tooltip={quoteattr(tip)} id={quoteattr(cid)}>'
                f'<mxCell style={quoteattr(style)} vertex="1" parent="1">{geo}</mxCell></UserObject>')
    return (f'<mxCell id={quoteattr(cid)} value={quoteattr(value)} style={quoteattr(style)} '
            f'vertex="1" parent="1">{geo}</mxCell>')


def main():
    src = open(DRAWIO, encoding="utf-8").read()
    page_re = re.compile(r'[ \t]*<diagram\b[^>]*\bid="' + PAGE_ID + r'"[^>]*>.*?</diagram>\n?', re.S)
    without = page_re.sub("", src)
    assert "</mxfile>" in without, "unexpected file format"
    new_page = "  " + build_page() + "\n"
    out = without.replace("</mxfile>", new_page + "</mxfile>", 1)
    n_pages = len(re.findall(r"<diagram\b", out))
    out = re.sub(r'(<mxfile\b[^>]*\bpages=")\d+(")', rf"\g<1>{n_pages}\2", out, count=1)

    # Safety: everything except our page (and the pages= count) must be unchanged.
    norm = lambda t: re.sub(r'pages="\d+"', "", page_re.sub("", t))
    assert norm(out) == norm(src), "would modify human-maintained pages; aborting"

    open(DRAWIO, "w", encoding="utf-8").write(out)
    print(f"wrote page '{PAGE_NAME}' ({len(MODULES)} blocks, {len(EDGES)} arrows) to {DRAWIO}")


if __name__ == "__main__":
    sys.exit(main())
