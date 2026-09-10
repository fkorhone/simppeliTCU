#!/usr/bin/env python3
"""
Generate maintainable SVG wiring diagrams for simppeliTCU documentation.

Usage:
    python docs/generate_diagrams.py

Outputs:
    - docs/wiring_lilygo_adapter.svg    (Option A: Recommended 16-pin adapter + LilyGo T-2CAN)
    - docs/wiring_waveshare_adapter.svg (Alternative controller: Waveshare ESP32-S3-RS485-CAN)
    - docs/wiring_alternatives.svg      (Option B: CAN-only unpin + tap, Option C: TCU unplugged)

Pure Python 3 standard library only — no external packages required.
"""

from pathlib import Path
from xml.sax.saxutils import escape

DOCS_DIR = Path(__file__).resolve().parent


class SvgBuilder:
    def __init__(self, width: int, height: int, title: str):
        self.width = width
        self.height = height
        self.title = title
        self.elements: list[str] = []

    def add(self, raw: str) -> None:
        self.elements.append(raw)

    def rect(
        self,
        x: float,
        y: float,
        w: float,
        h: float,
        fill: str = "#ffffff",
        stroke: str = "#334155",
        stroke_width: float = 1.5,
        rx: float = 6,
        dash: str = "",
    ) -> None:
        dash_attr = f' stroke-dasharray="{dash}"' if dash else ""
        self.add(
            f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" '
            f'fill="{fill}" stroke="{stroke}" stroke-width="{stroke_width}"{dash_attr}/>'
        )

    def circle(
        self,
        cx: float,
        cy: float,
        r: float,
        fill: str = "#0f172a",
        stroke: str = "none",
        stroke_width: float = 1.5,
    ) -> None:
        self.add(
            f'<circle cx="{cx}" cy="{cy}" r="{r}" fill="{fill}" '
            f'stroke="{stroke}" stroke-width="{stroke_width}"/>'
        )

    def line(
        self,
        x1: float,
        y1: float,
        x2: float,
        y2: float,
        stroke: str = "#0f172a",
        stroke_width: float = 2.0,
        dash: str = "",
    ) -> None:
        dash_attr = f' stroke-dasharray="{dash}"' if dash else ""
        self.add(
            f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" '
            f'stroke="{stroke}" stroke-width="{stroke_width}" stroke-linecap="round"{dash_attr}/>'
        )

    def wire(
        self,
        points: list[tuple[float, float]],
        color: str,
        width: float = 3.0,
        dash: str = "",
        outline: str = "",
    ) -> None:
        pts = " ".join(f"{x},{y}" for x, y in points)
        dash_attr = f' stroke-dasharray="{dash}"' if dash else ""
        if outline:
            self.add(
                f'<polyline points="{pts}" fill="none" stroke="{outline}" '
                f'stroke-width="{width + 2.2}" stroke-linecap="round" stroke-linejoin="round"{dash_attr}/>'
            )
        self.add(
            f'<polyline points="{pts}" fill="none" stroke="{color}" '
            f'stroke-width="{width}" stroke-linecap="round" stroke-linejoin="round"{dash_attr}/>'
        )

    def text(
        self,
        x: float,
        y: float,
        content: str,
        size: float = 13,
        fill: str = "#0f172a",
        weight: str = "normal",
        anchor: str = "start",
        italic: bool = False,
    ) -> None:
        style = ' font-style="italic"' if italic else ""
        self.add(
            f'<text x="{x}" y="{y}" font-family="Segoe UI, Arial, Helvetica, sans-serif" '
            f'font-size="{size}" font-weight="{weight}" fill="{fill}" text-anchor="{anchor}"{style}>'
            f"{escape(content)}</text>"
        )

    def badge(
        self,
        x: float,
        y: float,
        label: str,
        bg: str = "#e2e8f0",
        fg: str = "#0f172a",
        border: str = "#94a3b8",
        w: float = 0,
        h: float = 22,
        size: float = 11.5,
    ) -> None:
        bw = w if w > 0 else max(46, len(label) * 7.0 + 16)
        self.rect(x - bw / 2, y - h / 2, bw, h, fill=bg, stroke=border, stroke_width=1.2, rx=4)
        self.text(x, y + 4, label, size=size, fill=fg, weight="bold", anchor="middle")

    def fuse(self, cx: float, cy: float, label: str = "2A Fuse") -> None:
        """Draw an inline automotive fuse symbol centered at (cx, cy)."""
        w, h = 58, 24
        self.rect(cx - w / 2, cy - h / 2, w, h, fill="#fff7ed", stroke="#c2410c", stroke_width=2.0, rx=4)
        self.add(
            f'<path d="M {cx - w/2} {cy} L {cx - 14} {cy} Q {cx - 7} {cy - 7}, {cx} {cy} '
            f'T {cx + 14} {cy} L {cx + w/2} {cy}" fill="none" stroke="#c2410c" stroke-width="2.2"/>'
        )
        self.circle(cx - 16, cy, 2.5, fill="#c2410c")
        self.circle(cx + 16, cy, 2.5, fill="#c2410c")
        self.text(cx, cy - 17, label, size=11.5, fill="#9a3412", weight="bold", anchor="middle")

    def draw_16pin_connector(
        self,
        x: float,
        y: float,
        is_male: bool,
        highlights: dict[int, tuple[str, str]],
    ) -> dict[int, tuple[float, float]]:
        """
        Draw a 16-pin Sumitomo NH .025" connector in Wire-Side View (latch at top).
        - Male Wire-Side View: top row 8..1 (left->right), bottom row 16..9 (left->right)
        - Female Wire-Side View: top row 1..8 (left->right), bottom row 9..16 (left->right)
        Returns center coordinates {pin_number: (cx, cy)} for every pin.
        """
        cell_w, cell_h = 26, 26
        pad = 10
        total_w = 8 * cell_w + 2 * pad  # 228 px
        total_h = 2 * cell_h + 2 * pad + 8  # 80 px

        latch_w = 54
        self.rect(
            x + (total_w - latch_w) / 2,
            y - 12,
            latch_w,
            14,
            fill="#cbd5e1",
            stroke="#475569",
            stroke_width=1.6,
            rx=2,
        )
        self.rect(
            x + (total_w - 26) / 2,
            y - 8,
            26,
            8,
            fill="#94a3b8",
            stroke="#475569",
            stroke_width=1.2,
            rx=1,
        )

        self.rect(x, y, total_w, total_h, fill="#f1f5f9", stroke="#334155", stroke_width=2.0, rx=5)

        top_pins = list(range(8, 0, -1)) if is_male else list(range(1, 9))
        bot_pins = list(range(16, 8, -1)) if is_male else list(range(9, 17))

        coords: dict[int, tuple[float, float]] = {}
        for row_idx, row_pins in enumerate([top_pins, bot_pins]):
            for col_idx, pin in enumerate(row_pins):
                px = x + pad + col_idx * cell_w
                py = y + pad + row_idx * (cell_h + 8)
                cx = px + cell_w / 2
                cy = py + cell_h / 2
                coords[pin] = (cx, cy)

                if pin in highlights:
                    fill_col, txt_col = highlights[pin]
                    self.rect(px + 1.5, py + 1.5, cell_w - 3, cell_h - 3, fill=fill_col, stroke="#0f172a", stroke_width=1.6, rx=3)
                    self.text(cx, cy + 4, str(pin), size=11.5, fill=txt_col, weight="bold", anchor="middle")
                else:
                    self.rect(px + 1.5, py + 1.5, cell_w - 3, cell_h - 3, fill="#ffffff", stroke="#94a3b8", stroke_width=1.1, rx=3)
                    self.text(cx, cy + 4, str(pin), size=10.5, fill="#64748b", anchor="middle")

        return coords

    def draw_40pin_tcu_connector(
        self,
        x: float,
        y: float,
        highlights: dict[int, tuple[str, str, str]],
    ) -> dict[int, tuple[float, float]]:
        """
        Draw Nissan Leaf ZE1 40-pin TCU Female Plug in Wire-Side View (latch at top).
        Matches docs/tcu_pins.jpg:
        - Top row (left -> right): 20 .. 1  (Pin 1 is TOP-RIGHT)
        - Bottom row (left -> right): 40 .. 21 (Pin 28 is below Pin 8)
        """
        cell_w, cell_h = 21, 24
        pad = 8
        total_w = 20 * cell_w + 2 * pad  # 436 px
        total_h = 2 * cell_h + 2 * pad + 6  # 70 px

        latch_w = 80
        self.rect(
            x + (total_w - latch_w) / 2,
            y - 12,
            latch_w,
            14,
            fill="#334155",
            stroke="#0f172a",
            stroke_width=1.6,
            rx=2,
        )
        self.text(
            x + total_w / 2,
            y - 2,
            "LOCK TAB",
            size=8.5,
            fill="#f8fafc",
            weight="bold",
            anchor="middle",
        )

        self.rect(x, y, total_w, total_h, fill="#1e293b", stroke="#0f172a", stroke_width=2.0, rx=5)

        top_pins = list(range(20, 0, -1))
        bot_pins = list(range(40, 20, -1))

        coords: dict[int, tuple[float, float]] = {}
        for row_idx, row_pins in enumerate([top_pins, bot_pins]):
            for col_idx, pin in enumerate(row_pins):
                px = x + pad + col_idx * cell_w
                py = y + pad + row_idx * (cell_h + 6)
                cx = px + cell_w / 2
                cy = py + cell_h / 2
                coords[pin] = (cx, cy)

                if pin in highlights:
                    fill_col, txt_col, dash = highlights[pin]
                    stroke_col = "#ef4444" if dash else "#ffffff"
                    self.rect(
                        px + 1,
                        py + 1,
                        cell_w - 2,
                        cell_h - 2,
                        fill=fill_col,
                        stroke=stroke_col,
                        stroke_width=1.8,
                        rx=2,
                        dash=dash,
                    )
                    self.text(cx, cy + 4, str(pin), size=10, fill=txt_col, weight="bold", anchor="middle")
                else:
                    self.rect(px + 1.5, py + 1.5, cell_w - 3, cell_h - 3, fill="#334155", stroke="#64748b", stroke_width=0.9, rx=2)
                    self.text(cx, cy + 3.5, str(pin), size=8.5, fill="#cbd5e1", anchor="middle")

        return coords

    def save(self, path: Path) -> None:
        svg = [
            f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {self.width} {self.height}" '
            f'width="{self.width}" height="{self.height}">',
            f"  <title>{escape(self.title)}</title>",
            f'  <rect x="0" y="0" width="{self.width}" height="{self.height}" fill="#ffffff"/>',
            *("  " + el for el in self.elements),
            "</svg>",
        ]
        path.write_text("\n".join(svg) + "\n", encoding="utf-8")


# Color palette for wires
COL_12V = "#dc2626"       # Red (+12V new/device wire)
COL_12V_OEM = "#f8fafc"   # White factory +12V wire (drawn with dark outline)
COL_GND = "#1e293b"       # Black/dark slate GND wire
COL_CANH_DEV = "#eab308"  # Yellow/Gold CAN-H on device side
COL_CANL_DEV = "#16a34a"  # Green CAN-L on device side
COL_CANH_OEM = "#0ea5e9"  # Sky Blue factory CAN-H (#6)
COL_CANL_OEM = "#8b5cf6"  # Violet factory CAN-L (#7)


def generate_lilygo_adapter() -> None:
    s = SvgBuilder(
        1420,
        780,
        "Option A: Non-Destructive 16-Pin Sumitomo Adapter Wiring (LilyGo T-2CAN & Nissan Leaf ZE1 TCU)",
    )

    # Header banner
    s.rect(16, 14, 1388, 54, fill="#0f172a", stroke="#0f172a", rx=8)
    s.text(
        34,
        39,
        "Option A (Recommended): Non-Destructive 16-Pin Adapter Harness — LilyGo T-2CAN",
        size=19,
        fill="#ffffff",
        weight="bold",
    )
    s.text(
        34,
        58,
        "All connector pinouts shown from WIRE-SIDE VIEW (looking into wire-entry back of connector, lock tab on top — matches tcu_pins.jpg).",
        size=12.5,
        fill="#cbd5e1",
    )

    # Left & Right main cards with generous center gutter (108 px wide) for MATES badge
    s.rect(16, 80, 640, 682, fill="#f8fafc", stroke="#cbd5e1", stroke_width=1.5, rx=10)
    s.text(34, 106, "1. DEVICE SIDE (Build & Pre-Flash on Bench)", size=15, fill="#0f172a", weight="bold")

    s.rect(764, 80, 640, 682, fill="#f8fafc", stroke="#cbd5e1", stroke_width=1.5, rx=10)
    s.text(782, 106, "2. CAR SIDE (Behind Glovebox — No Factory Wires Cut)", size=15, fill="#0f172a", weight="bold")

    # =========================================================================
    # LEFT: LilyGo T-2CAN Board
    # =========================================================================
    bx, by, bw, bh = 54, 136, 266, 218
    s.rect(bx, by, bw, bh, fill="#1e293b", stroke="#0f172a", stroke_width=2.2, rx=8)
    s.text(bx + 16, by + 24, "LilyGo T-2CAN Board", size=15, fill="#ffffff", weight="bold")
    s.text(bx + 16, by + 42, "ESP32-S3 (Flash & Config via USB-C)", size=11, fill="#94a3b8")

    # ESP32-S3 shield illustration
    s.rect(bx + 22, by + 58, 96, 82, fill="#475569", stroke="#94a3b8", stroke_width=1.4, rx=4)
    s.text(bx + 70, by + 97, "ESP32-S3", size=11.5, fill="#f8fafc", weight="bold", anchor="middle")
    s.text(bx + 70, by + 114, "Wi-Fi / MCU", size=10, fill="#cbd5e1", anchor="middle")

    # USB-C port on left edge of board (labels nicely stacked under ESP32-S3 without hitting Port A)
    s.rect(bx - 12, by + 84, 18, 34, fill="#cbd5e1", stroke="#334155", stroke_width=1.6, rx=3)
    s.text(bx + 16, by + 166, "USB-C Port:", size=10.5, fill="#f8fafc", weight="bold")
    s.text(bx + 16, by + 181, "5V Power +", size=10.5, fill="#cbd5e1")
    s.text(bx + 16, by + 195, "Serial CLI", size=10.5, fill="#cbd5e1")

    # Port B screw terminal block (top right of board - ACTIVE IT-CAN)
    pb_x, pb_y = bx + 150, by + 54
    s.rect(pb_x, pb_y, 108, 96, fill="#14532d", stroke="#4ade80", stroke_width=1.8, rx=4)
    s.text(pb_x + 8, pb_y + 16, "PORT B (IT-CAN)", size=10.5, fill="#bbf7d0", weight="bold")
    s.text(pb_x + 8, pb_y + 29, "TWAI GPIO 7 / 6", size=9.5, fill="#86efac")

    port_b_terms = [
        ("CANHB", pb_y + 45),
        ("CANLB", pb_y + 65),
        ("SGNDB", pb_y + 85),
    ]
    term_coords: dict[str, tuple[float, float]] = {}
    for t_name, ty in port_b_terms:
        s.text(pb_x + 68, ty + 4, t_name, size=10.5, fill="#ffffff", weight="bold", anchor="end")
        s.circle(pb_x + 88, ty, 6.5, fill="#cbd5e1", stroke="#0f172a", stroke_width=1.3)
        s.line(pb_x + 84, ty - 3, pb_x + 92, ty + 3, stroke="#334155", stroke_width=1.4)
        term_coords[t_name] = (pb_x + 108, ty)

    # Port A screw terminal block (bottom right of board - optional AV-CAN)
    pa_x, pa_y = bx + 150, by + 158
    s.rect(pa_x, pa_y, 108, 48, fill="#334155", stroke="#64748b", stroke_width=1.2, rx=4, dash="4,3")
    s.text(pa_x + 54, pa_y + 20, "PORT A (MCP2515)", size=9.5, fill="#cbd5e1", weight="bold", anchor="middle")
    s.text(pa_x + 54, pa_y + 35, "Optional AV-CAN", size=9.5, fill="#94a3b8", anchor="middle")

    # Note about SGNDB right under LilyGo
    s.rect(54, 366, 294, 52, fill="#eff6ff", stroke="#93c5fd", stroke_width=1.2, rx=6)
    s.text(64, 385, "Note on SGNDB (Signal Ground):", size=11, fill="#1e3a8a", weight="bold")
    s.text(64, 401, "Connected by default (recommended for isolated", size=10.5, fill="#1e40af")
    s.text(64, 414, "CAN transceiver; usually works without it too).", size=10.5, fill="#1e40af")

    # =========================================================================
    # LEFT BOTTOM: Automotive 12V -> 5V USB Converter + 2A Inline Fuse
    # =========================================================================
    s.rect(54, 444, 232, 138, fill="#ffffff", stroke="#475569", stroke_width=1.8, rx=8)
    s.text(170, 466, "Automotive 12V -> 5V USB", size=13, fill="#0f172a", weight="bold", anchor="middle")
    s.text(170, 482, "Step-Down Socket / Converter", size=11, fill="#475569", anchor="middle")

    # USB port on left of converter
    s.rect(68, 504, 54, 52, fill="#e2e8f0", stroke="#334155", stroke_width=1.5, rx=6)
    s.text(95, 527, "5V USB", size=10.5, fill="#0f172a", weight="bold", anchor="middle")
    s.text(95, 543, "OUT", size=10, fill="#475569", anchor="middle")

    # +12V and -GND terminals on right of converter
    s.circle(264, 507, 7, fill="#e2e8f0", stroke="#0f172a", stroke_width=1.4)
    s.text(248, 511, "- GND", size=11.5, fill="#0f172a", weight="bold", anchor="end")
    s.circle(264, 551, 7, fill="#fee2e2", stroke="#dc2626", stroke_width=1.4)
    s.text(248, 555, "+ 12V", size=11.5, fill="#dc2626", weight="bold", anchor="end")

    # USB-A to USB-C power cable from Converter to LilyGo USB-C
    s.wire([(68, 530), (28, 530), (28, by + 101), (bx - 12, by + 101)], "#ca8a04", width=5.0)
    s.text(36, 436, "5V USB Cable", size=11, fill="#854d0e", weight="bold")

    # =========================================================================
    # CENTER-LEFT: 16-Pin Sumitomo NH .025" MALE Connector (Wire-Side View)
    # Place connector at y=282 and put its title cleanly to the LEFT/BELOW where no wires pass!
    # Wait: if we put title at y=136..160 ( way ABOVE all wires! ), zero wires ever touch it!
    # =========================================================================
    male_x, male_y = 402, 280
    s.text(male_x + 114, 148, "16-Pin NH .025\" MALE", size=14, fill="#0f172a", weight="bold", anchor="middle")
    s.text(male_x + 114, 165, "Wire-Side View (6 male pins used)", size=11, fill="#475569", anchor="middle")

    male_highlights = {
        1: ("#fee2e2", "#991b1b"),   # +12V in
        2: ("#1e293b", "#ffffff"),   # GND in
        3: ("#fef08a", "#713f12"),   # CAN-H
        4: ("#bbf7d0", "#14532d"),   # CAN-L
        9: ("#fecaca", "#991b1b"),   # +12V loop out
        10: ("#334155", "#ffffff"),  # GND loop out
    }
    m_pins = s.draw_16pin_connector(male_x, male_y, is_male=True, highlights=male_highlights)

    # Wires on Device Side (all routed cleanly between y=185 and y=280, below the title at y=165!):
    # 1) CAN-H: Male Pin 3 -> LilyGo CANHB (y=235)
    p3x, p3y = m_pins[3]
    th_x, th_y = term_coords["CANHB"]
    s.wire([(th_x, th_y), (p3x, th_y), (p3x, p3y - 13)], COL_CANH_DEV, width=3.2)
    s.text(390, th_y - 6, "CAN-H (Pin 3)", size=10.8, fill="#854d0e", weight="bold")

    # 2) CAN-L: Male Pin 4 -> LilyGo CANLB (y=255)
    p4x, p4y = m_pins[4]
    tl_x, tl_y = term_coords["CANLB"]
    s.wire([(tl_x, tl_y), (p4x, tl_y), (p4x, p4y - 13)], COL_CANL_DEV, width=3.2)
    s.text(390, tl_y - 6, "CAN-L (Pin 4)", size=10.8, fill="#166534", weight="bold")

    # 3) GND 4-way split: Male Pin 2 + Male Pin 10 + Converter (-) + LilyGo SGNDB
    p2x, p2y = m_pins[2]
    p10x, p10y = m_pins[10]
    ts_x, ts_y = term_coords["SGNDB"]
    gnd_bus_x = 374
    gnd_junc = (gnd_bus_x, 507)

    # Pin 2 goes up to y=206 -> left to x=374 -> down to y=507
    s.wire([(p2x, p2y - 13), (p2x, 206), (gnd_bus_x, 206), gnd_junc], COL_GND, width=3.0)
    # Pin 10 goes down to y=396 -> left to x=374
    s.wire([(p10x, p10y + 13), (p10x, 396), (gnd_bus_x, 396)], COL_GND, width=3.0)
    # SGNDB goes right to x=374 (solid wire by default!)
    s.wire([(ts_x, ts_y), (gnd_bus_x, ts_y)], COL_GND, width=3.0)
    # Converter (-) goes from (264, 507) to gnd_junc
    s.wire([(264, 507), gnd_junc], COL_GND, width=3.0)

    s.circle(gnd_bus_x, ts_y, 4.5, fill=COL_GND)
    s.circle(gnd_bus_x, 396, 4.5, fill=COL_GND)
    s.circle(*gnd_junc, 4.5, fill=COL_GND)
    s.text(384, 503, "GND splice (4 ends)", size=10.8, fill="#0f172a", weight="bold")

    # 4) +12V 3-way split: Male Pin 1 + Male Pin 9 + 2A Inline Fuse -> Converter (+)
    p1x, p1y = m_pins[1]
    p9x, p9y = m_pins[9]
    v12_bus_x = 640
    v12_junc = (v12_bus_x, 551)
    s.wire([(p1x, p1y - 13), (p1x, 190), (v12_bus_x, 190), v12_junc], COL_12V, width=3.2)
    s.wire([(p9x, p9y + 13), (p9x, 414), (v12_bus_x, 414)], COL_12V, width=3.2)
    s.circle(v12_bus_x, 414, 4.5, fill=COL_12V)

    fuse_cx, fuse_cy = 468, 551
    s.wire([v12_junc, (fuse_cx + 29, fuse_cy)], COL_12V, width=3.2)
    s.wire([(fuse_cx - 29, fuse_cy), (264, 551)], COL_12V, width=3.2)
    s.fuse(fuse_cx, fuse_cy, "2A Inline Fuse")
    s.text(572, 542, "+12V splice (3 ends)", size=10.8, fill="#991b1b", weight="bold", anchor="middle")

    # Device-side summary box at bottom-left
    s.rect(54, 604, 586, 140, fill="#ffffff", stroke="#cbd5e1", stroke_width=1.3, rx=6)
    s.text(70, 626, "Device-Side Harness Summary (Step 2 on Bench):", size=12.5, fill="#0f172a", weight="bold")
    s.text(70, 646, "• +12V (Red, 3 ends): Male Pin 1  +  loop to Male Pin 9  +  via 2A Inline Fuse to 12V->USB (+)", size=11.3, fill="#334155")
    s.text(70, 665, "• GND (Black, 4 ends): Male Pin 2  +  loop to Male Pin 10  +  12V->USB (-)  +  LilyGo SGNDB", size=11.3, fill="#334155")
    s.text(70, 684, "• IT-CAN: Male Pin 3 -> LilyGo CANHB   |   Male Pin 4 -> LilyGo CANLB", size=11.3, fill="#334155")
    s.text(70, 705, "• Why loop Pins 1->9 & 2->10? Routes +12V & GND back to the factory TCU so the car's hands-free", size=11.3, fill="#0f172a", weight="bold")
    s.text(80, 722, "microphone keeps working, without cutting or splicing any original car wires!", size=11.3, fill="#0f172a", weight="bold")

    # =========================================================================
    # MATING INDICATOR IN THE 108px GUTTER BETWEEN MALE (ends 630) & FEMALE (starts 786)
    # =========================================================================
    s.line(632, 320, 784, 320, stroke="#0284c7", stroke_width=2.5, dash="5,4")
    s.badge(708, 320, "PLUGS TOGETHER", bg="#0284c7", fg="#ffffff", border="#0369a1", w=122, h=24, size=10.5)

    # =========================================================================
    # RIGHT: 16-Pin Sumitomo NH .025" FEMALE Connector (Wire-Side View)
    # Put connector at y=280 and put its title cleanly to the LEFT/ABOVE at y=148 (above all wires!)
    # =========================================================================
    fem_x, fem_y = 786, 280
    s.text(fem_x + 114, 148, "16-Pin NH .025\" FEMALE", size=14, fill="#0f172a", weight="bold", anchor="middle")
    s.text(fem_x + 114, 165, "Wire-Side View (4 OEM + 2 return pins)", size=11, fill="#475569", anchor="middle")

    fem_highlights = {
        1: ("#ffffff", "#0f172a"),   # Factory White +12V (from #1)
        2: ("#1e293b", "#ffffff"),   # Factory Black GND (from #28)
        3: ("#bae6fd", "#0c4a6e"),   # Factory Sky Blue CAN-H (from #6)
        4: ("#ddd6fe", "#4c1d95"),   # Factory Violet CAN-L (from #7)
        9: ("#fecaca", "#991b1b"),   # New +12V return to TCU #1
        10: ("#334155", "#ffffff"),  # New GND return to TCU #28
    }
    f_pins = s.draw_16pin_connector(fem_x, fem_y, is_male=False, highlights=fem_highlights)

    # =========================================================================
    # TOP-RIGHT: Car Wiring Harness Bundle (Source of the 4 unpinned OEM wires)
    # Positioned at x=1092, y=132 so wires exit left at y=190..235 cleanly BELOW Female title!
    # =========================================================================
    loom_x, loom_y = 1092, 136
    s.rect(loom_x, loom_y, 292, 134, fill="#e2e8f0", stroke="#475569", stroke_width=1.8, rx=8)
    s.text(loom_x + 14, loom_y + 22, "Car Wiring Harness Loom", size=13.5, fill="#0f172a", weight="bold")
    s.text(loom_x + 14, loom_y + 40, "4 wires unpinned from 40-pin TCU plug", size=11, fill="#334155")
    s.text(loom_x + 14, loom_y + 55, "click straight into 16-pin Female:", size=11, fill="#334155")
    s.text(loom_x + 14, loom_y + 76, "• OEM #1 (White): +12V BAT -> Fem Pin 1", size=10.5, fill="#0f172a", weight="bold")
    s.text(loom_x + 14, loom_y + 93, "• OEM #28 (Black): GND -> Fem Pin 2", size=10.5, fill="#0f172a", weight="bold")
    s.text(loom_x + 14, loom_y + 110, "• OEM #6 (Sky Blue): CAN-H -> Fem Pin 3", size=10.5, fill="#0369a1", weight="bold")
    s.text(loom_x + 14, loom_y + 126, "• OEM #7 (Violet): CAN-L -> Fem Pin 4", size=10.5, fill="#6d28d9", weight="bold")

    # Route the 4 unpinned OEM wires at y=186, 202, 218, 234 (all strictly > 165 so zero text overlap!)
    fp1x, fp1y = f_pins[1]
    fp2x, fp2y = f_pins[2]
    fp3x, fp3y = f_pins[3]
    fp4x, fp4y = f_pins[4]

    s.wire([(loom_x, 186), (fp1x, 186), (fp1x, fp1y - 13)], COL_12V_OEM, width=3.2, outline="#334155")
    s.wire([(loom_x, 202), (fp2x, 202), (fp2x, fp2y - 13)], COL_GND, width=3.2)
    s.wire([(loom_x, 218), (fp3x, 218), (fp3x, fp3y - 13)], COL_CANH_OEM, width=3.2)
    s.wire([(loom_x, 234), (fp4x, 234), (fp4x, fp4y - 13)], COL_CANL_OEM, width=3.2)

    # Remaining untouched 36-wire loom going down the far right to 40-pin TCU connector
    s.wire([(1364, loom_y + 134), (1364, 480), (1340, 480)], "#94a3b8", width=6.0)
    s.text(1354, 332, "Other 36 OEM wires", size=10.5, fill="#475569", anchor="end", italic=True)
    s.text(1354, 347, "(stay in 40-pin plug)", size=10.5, fill="#475569", anchor="end", italic=True)

    # =========================================================================
    # RIGHT LOWER: Original 40-Pin Nissan TCU Connector (Wire-Side View)
    # Put connector at x=904, y=476
    # =========================================================================
    tcu_x, tcu_y = 904, 476
    s.text(tcu_x + 218, tcu_y - 30, "Original 40-Pin Nissan TCU Connector — Wire-Side View", size=13, fill="#0f172a", weight="bold", anchor="middle")
    s.text(tcu_x + 218, tcu_y - 15, "(Plugs back into original TCU so hands-free microphone stays powered)", size=10.8, fill="#475569", anchor="middle")

    tcu_highlights = {
        1: ("#fecaca", "#991b1b", ""),        # Receives +12V return from Fem Pin 9
        6: ("#0f172a", "#fca5a5", "3,2"),     # EMPTY (unpinned CAN-H)
        7: ("#0f172a", "#fca5a5", "3,2"),     # EMPTY (unpinned CAN-L)
        28: ("#e2e8f0", "#0f172a", ""),       # Receives GND return from Fem Pin 10
    }
    t_pins = s.draw_40pin_tcu_connector(tcu_x, tcu_y, highlights=tcu_highlights)

    # Callout pointing to empty pins 6 & 7 (placed cleanly above title at y=418)
    t6x, t6y = t_pins[6]
    t7x, t7y = t_pins[7]
    mid_67 = (t6x + t7x) / 2
    s.line(mid_67, t6y - 12, mid_67, tcu_y - 48, stroke="#dc2626", stroke_width=1.6, dash="3,2")
    s.badge(mid_67, tcu_y - 58, "Pins #6 & #7 LEFT EMPTY", bg="#fef2f2", fg="#991b1b", border="#f87171", w=164, h=20, size=10.5)

    # Route the 2 short return wires (with .025" female pins on BOTH ends) WITHOUT crossing any text:
    # Fem Pin 9 -> y=392 -> x=1348 (around right of TCU title!) -> down to y=498 -> into TCU #1!
    # Wait: TCU #1 is at top-right of 40-pin plug (t1x = 904 + 8 + 19*21 + 10.5 = 1321.5, t1y = 476 + 8 + 12 = 496)!
    # Routing at y=390 over to x=1348, down to y=496, and left into t1x+10.5 completely avoids the TCU title AND the empty-pins badge!
    fp9x, fp9y = f_pins[9]
    fp10x, fp10y = f_pins[10]
    t1x, t1y = t_pins[1]
    t28x, t28y = t_pins[28]

    s.wire(
        [(fp9x, fp9y + 13), (fp9x, 388), (1350, 388), (1350, t1y), (t1x + 10, t1y)],
        COL_12V,
        width=3.2,
    )
    s.text(852, 381, "+12V return wire (Fem Pin 9 -> TCU #1)", size=10.8, fill="#991b1b", weight="bold")

    s.wire(
        [(fp10x, fp10y + 13), (fp10x, 572), (t28x, 572), (t28x, t28y + 12)],
        COL_GND,
        width=3.2,
    )
    s.text(835, 588, "GND return wire (Fem Pin 10 -> TCU #28)", size=10.8, fill="#0f172a", weight="bold")

    # 40-pin TCU Pinout Reference Table at bottom-right
    s.rect(782, 604, 604, 140, fill="#ffffff", stroke="#cbd5e1", stroke_width=1.3, rx=6)
    s.text(798, 626, "Nissan Leaf ZE1 40-Pin TCU Reference & Car-Side Pin Mapping:", size=12.5, fill="#0f172a", weight="bold")
    s.text(798, 646, "• TCU #1  (White, +12V BAT)  : Unpin OEM wire -> 16-pin Fem #1   |  Return wire: Fem #9 -> TCU #1", size=11, fill="#334155")
    s.text(798, 665, "• TCU #28 (Black, GND)       : Unpin OEM wire -> 16-pin Fem #2   |  Return wire: Fem #10 -> TCU #28", size=11, fill="#334155")
    s.text(798, 684, "• TCU #6  (Sky Blue, CAN-H)  : Unpin OEM wire -> 16-pin Fem #3   |  Leave TCU #6 socket EMPTY", size=11, fill="#334155")
    s.text(798, 703, "• TCU #7  (Violet, CAN-L)    : Unpin OEM wire -> 16-pin Fem #4   |  Leave TCU #7 socket EMPTY", size=11, fill="#334155")
    s.text(798, 724, "• Terminal compatibility: NH .025\" female pins from the 16-pin kit fit directly into TCU #1 & #28.", size=11, fill="#0f172a", weight="bold")

    s.save(DOCS_DIR / "wiring_lilygo_adapter.svg")


def generate_waveshare_adapter() -> None:
    s = SvgBuilder(
        980,
        450,
        "Alternative Controller Wiring: Waveshare ESP32-S3-RS485-CAN to 16-Pin Adapter",
    )

    s.rect(16, 14, 948, 52, fill="#0f172a", stroke="#0f172a", rx=8)
    s.text(
        34,
        38,
        "Alternative Controller: Waveshare ESP32-S3-RS485-CAN (Device Side)",
        size=18,
        fill="#ffffff",
        weight="bold",
    )
    s.text(
        34,
        56,
        "Uses onboard 7–36V DC wide-input regulator (no separate 12V->5V USB socket needed). Car Side (16-pin Female) is identical to Option A.",
        size=12,
        fill="#cbd5e1",
    )

    s.rect(16, 78, 948, 356, fill="#f8fafc", stroke="#cbd5e1", stroke_width=1.5, rx=10)

    # Waveshare industrial case drawing
    wx, wy, ww, wh = 68, 144, 340, 165
    s.rect(wx, wy, ww, wh, fill="#1e293b", stroke="#0f172a", stroke_width=2.2, rx=8)
    s.text(wx + ww / 2, wy + 28, "Waveshare ESP32-S3-RS485-CAN", size=15, fill="#ffffff", weight="bold", anchor="middle")
    s.text(wx + ww / 2, wy + 47, "Built-in DC 7–36V Power & Single CAN (GPIO 15/16)", size=11, fill="#94a3b8", anchor="middle")

    # Left green screw terminal: Power DC 7~36V
    s.rect(wx + 12, wy + 72, 92, 68, fill="#14532d", stroke="#4ade80", stroke_width=1.6, rx=4)
    s.text(wx + 58, wy + 90, "DC 7~36V", size=10.5, fill="#bbf7d0", weight="bold", anchor="middle")
    ws_vpos = (wx + 12, wy + 108)
    ws_vneg = (wx + 12, wy + 128)
    s.circle(wx + 28, wy + 108, 6, fill="#fecaca", stroke="#dc2626", stroke_width=1.3)
    s.text(wx + 42, wy + 112, "+ (12V)", size=10.5, fill="#ffffff", weight="bold")
    s.circle(wx + 28, wy + 128, 6, fill="#cbd5e1", stroke="#0f172a", stroke_width=1.3)
    s.text(wx + 42, wy + 132, "- (GND)", size=10.5, fill="#ffffff", weight="bold")

    # Bottom USB-C indicator
    s.rect(wx + 135, wy + wh - 8, 54, 14, fill="#cbd5e1", stroke="#334155", stroke_width=1.4, rx=3)
    s.text(wx + 162, wy + wh + 22, "USB-C (Flashing / CLI)", size=10.5, fill="#334155", weight="bold", anchor="middle")

    # Right green screw terminal: CAN H / CAN L (+ RS485 unused)
    s.rect(wx + ww - 112, wy + 64, 100, 88, fill="#14532d", stroke="#4ade80", stroke_width=1.6, rx=4)
    s.text(wx + ww - 62, wy + 80, "CAN / RS485", size=10.5, fill="#bbf7d0", weight="bold", anchor="middle")

    ws_canh = (wx + ww, wy + 98)
    ws_canl = (wx + ww, wy + 118)
    s.text(wx + ww - 36, wy + 102, "CAN-H", size=10.5, fill="#ffffff", weight="bold", anchor="end")
    s.circle(wx + ww - 22, wy + 98, 6, fill="#fef08a", stroke="#0f172a", stroke_width=1.3)
    s.text(wx + ww - 36, wy + 122, "CAN-L", size=10.5, fill="#ffffff", weight="bold", anchor="end")
    s.circle(wx + ww - 22, wy + 118, 6, fill="#bbf7d0", stroke="#0f172a", stroke_width=1.3)
    s.text(wx + ww - 62, wy + 142, "(RS485 A/B unused)", size=9.5, fill="#86efac", anchor="middle")

    # 16-pin Male Connector at y=214; put title cleanly at y=104..121 ABOVE all wires!
    male_x, male_y = 655, 214
    s.text(male_x + 114, 104, "16-Pin NH .025\" MALE", size=14, fill="#0f172a", weight="bold", anchor="middle")
    s.text(male_x + 114, 121, "Wire-Side View (mates with Car Side)", size=11, fill="#475569", anchor="middle")

    male_highlights = {
        1: ("#fee2e2", "#991b1b"),
        2: ("#1e293b", "#ffffff"),
        3: ("#fef08a", "#713f12"),
        4: ("#bbf7d0", "#14532d"),
        9: ("#fecaca", "#991b1b"),
        10: ("#334155", "#ffffff"),
    }
    m_pins = s.draw_16pin_connector(male_x, male_y, is_male=True, highlights=male_highlights)

    # CAN-H & CAN-L wires routed cleanly at y=168 and y=184 (strictly below title y=121!)
    p3x, p3y = m_pins[3]
    p4x, p4y = m_pins[4]
    s.wire([ws_canh, (520, ws_canh[1]), (520, 168), (p3x, 168), (p3x, p3y - 13)], COL_CANH_DEV, width=3.2)
    s.text(520, 160, "CAN-H (Pin 3)", size=11, fill="#854d0e", weight="bold", anchor="middle")

    s.wire([ws_canl, (555, ws_canl[1]), (555, 186), (p4x, 186), (p4x, p4y - 13)], COL_CANL_DEV, width=3.2)
    s.text(595, 200, "CAN-L (Pin 4)", size=11, fill="#166534", weight="bold", anchor="middle")

    # GND wire (Male Pin 2 + loop to Male Pin 10 -> Waveshare V-) routed at y=150 (below title!)
    p2x, p2y = m_pins[2]
    p10x, p10y = m_pins[10]
    s.wire([(p2x, p2y - 13), (p2x, 150), (912, 150), (912, 360), (40, 360), (40, ws_vneg[1]), ws_vneg], COL_GND, width=3.0)
    s.wire([(p10x, p10y + 13), (p10x, 360)], COL_GND, width=3.0)
    s.circle(p10x, 360, 4.5, fill=COL_GND)
    s.text(630, 352, "GND (Pin 2 looped to Pin 10 -> Waveshare DC -)", size=11, fill="#0f172a", weight="bold", anchor="middle")

    # +12V wire (Male Pin 1 + loop to Male Pin 9 -> 2A Fuse -> Waveshare V+) routed at y=136 (below title!)
    p1x, p1y = m_pins[1]
    p9x, p9y = m_pins[9]
    fuse_cx, fuse_cy = 340, 398
    s.wire([(p1x, p1y - 13), (p1x, 136), (932, 136), (932, fuse_cy), (fuse_cx + 29, fuse_cy)], COL_12V, width=3.2)
    s.wire([(p9x, p9y + 13), (p9x, fuse_cy)], COL_12V, width=3.2)
    s.circle(p9x, fuse_cy, 4.5, fill=COL_12V)
    s.wire([(fuse_cx - 29, fuse_cy), (24, fuse_cy), (24, ws_vpos[1]), ws_vpos], COL_12V, width=3.2)
    s.fuse(fuse_cx, fuse_cy, "2A Inline Fuse")
    s.text(630, 390, "+12V (Pin 1 looped to Pin 9 -> 2A Fuse -> Waveshare DC +)", size=11, fill="#991b1b", weight="bold", anchor="middle")

    s.save(DOCS_DIR / "wiring_waveshare_adapter.svg")


def generate_alternatives() -> None:
    s = SvgBuilder(
        1360,
        500,
        "Alternative Car-Side Installation Options (Option B: CAN-Only Unpin + Power Tap, Option C: TCU Unplugged)",
    )

    s.rect(16, 14, 1328, 52, fill="#0f172a", stroke="#0f172a", rx=8)
    s.text(
        34,
        38,
        "Simplified Car-Side Installation Alternatives (Without 16-Pin Sumitomo Adapter)",
        size=18,
        fill="#ffffff",
        weight="bold",
    )
    s.text(
        34,
        56,
        "Choose Option B if you want to keep the hands-free microphone without buying a 16-pin housing, or Option C if you do not need the microphone.",
        size=12,
        fill="#cbd5e1",
    )

    # =========================================================================
    # LEFT PANEL: Option B — CAN-Only Unpin + Power Tap
    # =========================================================================
    s.rect(16, 78, 652, 406, fill="#f8fafc", stroke="#cbd5e1", stroke_width=1.5, rx=10)
    s.text(34, 106, "Option B: Unpin CAN (#6, #7) Only + Tap Power (#1, #28)", size=15, fill="#0f172a", weight="bold")
    s.text(34, 124, "Hands-free microphone WORKS • 40-pin plug stays connected to TCU", size=11.5, fill="#15803d", weight="bold")

    # Left mini LilyGo + USB box
    s.rect(36, 152, 172, 92, fill="#1e293b", stroke="#0f172a", stroke_width=1.8, rx=6)
    s.text(122, 174, "LilyGo T-2CAN", size=13, fill="#ffffff", weight="bold", anchor="middle")
    s.text(198, 196, "CANHB", size=10, fill="#fef08a", weight="bold", anchor="end")
    s.text(198, 214, "CANLB", size=10, fill="#bbf7d0", weight="bold", anchor="end")
    s.text(198, 232, "SGNDB", size=10, fill="#cbd5e1", weight="bold", anchor="end")

    s.rect(36, 286, 172, 76, fill="#ffffff", stroke="#475569", stroke_width=1.6, rx=6)
    s.text(122, 308, "12V -> 5V USB", size=12, fill="#0f172a", weight="bold", anchor="middle")
    s.text(198, 330, "- GND", size=10.5, fill="#0f172a", weight="bold", anchor="end")
    s.text(198, 350, "+ 12V", size=10.5, fill="#dc2626", weight="bold", anchor="end")
    s.wire([(36, 324), (24, 324), (24, 198), (36, 198)], "#ca8a04", width=4.5)

    # Simplified TCU 40-pin block in center-right of Panel B + Car Harness Loom on far right of Panel B!
    s.rect(404, 152, 158, 214, fill="#1e293b", stroke="#0f172a", stroke_width=2.0, rx=6)
    s.text(483, 174, "40-Pin TCU Plug", size=12.5, fill="#ffffff", weight="bold", anchor="middle")
    s.text(483, 190, "(Plugged into TCU)", size=10, fill="#86efac", anchor="middle")

    s.rect(416, 202, 134, 25, fill="#0f172a", stroke="#f87171", stroke_width=1.4, rx=3, dash="3,2")
    s.text(483, 219, "#6 CAN-H (EMPTY)", size=10, fill="#fca5a5", weight="bold", anchor="middle")

    s.rect(416, 233, 134, 25, fill="#0f172a", stroke="#f87171", stroke_width=1.4, rx=3, dash="3,2")
    s.text(483, 250, "#7 CAN-L (EMPTY)", size=10, fill="#fca5a5", weight="bold", anchor="middle")

    s.rect(416, 286, 134, 26, fill="#334155", stroke="#94a3b8", stroke_width=1.2, rx=3)
    s.text(483, 303, "#28 GND (Stays in)", size=10, fill="#f8fafc", weight="bold", anchor="middle")

    s.rect(416, 326, 134, 26, fill="#334155", stroke="#94a3b8", stroke_width=1.2, rx=3)
    s.text(483, 343, "#1 +12V (Stays in)", size=10, fill="#fecaca", weight="bold", anchor="middle")

    # Car Harness Loom bar on far right of Panel B (x=590..652)
    s.rect(592, 152, 62, 214, fill="#e2e8f0", stroke="#475569", stroke_width=1.6, rx=6)
    s.text(623, 248, "Car", size=11.5, fill="#0f172a", weight="bold", anchor="middle")
    s.text(623, 264, "Loom", size=11.5, fill="#0f172a", weight="bold", anchor="middle")

    # Unpinned #6 & #7 route ABOVE the 40-pin TCU plug from Car Loom straight to LilyGo!
    s.wire([(592, 166), (576, 166), (576, 140), (350, 140), (350, 192), (208, 192)], COL_CANH_OEM, width=3.0)
    s.wire([(592, 182), (584, 182), (584, 132), (336, 132), (336, 210), (208, 210)], COL_CANL_OEM, width=3.0)
    s.badge(276, 170, "Unpinned #6 & #7", bg="#e0f2fe", fg="#075985", border="#7dd3fc", w=120, h=20, size=10)

    # OEM #28 GND passes from Car Loom -> TCU #28, tapped on the left side? Wait: wires enter the BACK of 40-pin plug (same side as Car Loom)!
    # Showing #28 and #1 wires going between Car Loom (x=592) and TCU Plug (x=550), OR showing the wire loop on the left of TCU plug:
    # Actually let's connect Car Loom -> TCU #28 (592 -> 550) AND extend left to tap point so it's crystal clear it's the same wire!
    s.wire([(592, 299), (550, 299)], COL_GND, width=3.2)
    s.wire([(416, 299), (250, 299), (250, 326), (208, 326)], COL_GND, width=2.8)
    s.wire([(250, 299), (250, 228), (208, 228)], COL_GND, width=2.8)
    s.circle(416, 299, 5, fill="#0284c7")
    s.circle(250, 299, 4, fill=COL_GND)

    s.wire([(592, 339), (550, 339)], COL_12V_OEM, width=3.2, outline="#334155")
    s.circle(416, 339, 5, fill="#0284c7")
    fuse_bx, fuse_by = 316, 346
    s.wire([(416, 339), (384, 339), (384, fuse_by), (fuse_bx + 29, fuse_by)], COL_12V, width=3.0)
    s.wire([(fuse_bx - 29, fuse_by), (208, fuse_by)], COL_12V, width=3.0)
    s.fuse(fuse_bx, fuse_by, "2A Fuse")
    s.text(326, 286, "Tap #28 & #1 (Posi-Tap / Wago)", size=10, fill="#0369a1", weight="bold", anchor="middle")

    # Warning box at bottom of Panel B
    s.rect(36, 398, 618, 70, fill="#fffbeb", stroke="#f59e0b", stroke_width=1.3, rx=6)
    s.text(50, 418, "Splice Tip:", size=11.5, fill="#92400e", weight="bold")
    s.text(118, 418, "Tap #1 (+12V) & #28 (GND) on the harness behind the 40-pin plug (Posi-Tap / Wago / solder).", size=11, fill="#78350f")
    s.text(50, 437, "Do NOT use insulation-displacement clips ('rosvoliitin' / Scotchlocks) on CAN-H / CAN-L", size=11, fill="#92400e", weight="bold")
    s.text(50, 454, "data wires—vehicle vibrations can cause intermittent CAN bus errors.", size=11, fill="#78350f")

    # =========================================================================
    # RIGHT PANEL: Option C — Original TCU Completely Disconnected
    # =========================================================================
    s.rect(692, 78, 652, 406, fill="#f8fafc", stroke="#cbd5e1", stroke_width=1.5, rx=10)
    s.text(710, 106, "Option C: Original TCU Completely Unplugged (4 Wires Only)", size=15, fill="#0f172a", weight="bold")
    s.text(710, 124, "Hands-free microphone DISABLED • Simplest wiring (no power loopback needed)", size=11.5, fill="#b45309", weight="bold")

    # Mini LilyGo + USB box
    s.rect(712, 152, 185, 92, fill="#1e293b", stroke="#0f172a", stroke_width=1.8, rx=6)
    s.text(804, 174, "LilyGo T-2CAN", size=13, fill="#ffffff", weight="bold", anchor="middle")
    s.text(886, 196, "CANHB", size=10, fill="#fef08a", weight="bold", anchor="end")
    s.text(886, 214, "CANLB", size=10, fill="#bbf7d0", weight="bold", anchor="end")
    s.text(886, 232, "SGNDB", size=10, fill="#cbd5e1", weight="bold", anchor="end")

    s.rect(712, 286, 185, 76, fill="#ffffff", stroke="#475569", stroke_width=1.6, rx=6)
    s.text(804, 308, "12V -> 5V USB", size=12, fill="#0f172a", weight="bold", anchor="middle")
    s.text(886, 330, "- GND", size=10.5, fill="#0f172a", weight="bold", anchor="end")
    s.text(886, 350, "+ 12V", size=10.5, fill="#dc2626", weight="bold", anchor="end")
    s.wire([(712, 324), (700, 324), (700, 198), (712, 198)], "#ca8a04", width=4.5)

    # Unplugged 40-pin harness on right of Panel C
    s.rect(1136, 152, 190, 214, fill="#334155", stroke="#475569", stroke_width=2.0, rx=6, dash="6,4")
    s.text(1231, 175, "40-Pin Harness Plug", size=13, fill="#ffffff", weight="bold", anchor="middle")
    s.text(1231, 192, "(Left UNPLUGGED from TCU)", size=10.5, fill="#fcd34d", weight="bold", anchor="middle")

    s.text(1150, 220, "Pin #6  (Sky Blue CAN-H)", size=10.5, fill="#bae6fd", weight="bold")
    s.text(1150, 245, "Pin #7  (Violet CAN-L)", size=10.5, fill="#ddd6fe", weight="bold")
    s.text(1150, 300, "Pin #28 (Black GND)", size=10.5, fill="#f8fafc", weight="bold")
    s.text(1150, 346, "Pin #1  (White +12V)", size=10.5, fill="#fecaca", weight="bold")

    # Direct 4 wires via any 4-pin plug (or pins 1-4 of 16-pin connector)
    s.wire([(897, 192), (1136, 216)], COL_CANH_OEM, width=3.2)
    s.wire([(897, 210), (1136, 241)], COL_CANL_OEM, width=3.2)

    # GND split (3 ends: #28 -> Converter - & SGNDB, solid line)
    s.wire([(1136, 296), (948, 296), (948, 326), (897, 326)], COL_GND, width=3.0)
    s.wire([(948, 296), (948, 228), (897, 228)], COL_GND, width=3.0)
    s.circle(948, 296, 4.5, fill=COL_GND)

    # +12V via 2A fuse
    fuse_cx, fuse_cy = 1020, 346
    s.wire([(1136, 346), (fuse_cx + 29, fuse_cy)], COL_12V, width=3.2)
    s.wire([(fuse_cx - 29, fuse_cy), (897, 346)], COL_12V, width=3.2)
    s.fuse(fuse_cx, fuse_cy, "2A Fuse")

    # Info box at bottom of Panel C
    s.rect(712, 398, 614, 70, fill="#f1f5f9", stroke="#94a3b8", stroke_width=1.3, rx=6)
    s.text(726, 419, "When to use Option C:", size=11.5, fill="#0f172a", weight="bold")
    s.text(726, 437, "• You remove the original TCU module completely or leave its 40-pin plug detached.", size=11, fill="#334155")
    s.text(726, 454, "• Uses only 4 wires (no pins 9 & 10 return loop), but hands-free phone mic will not function.", size=11, fill="#334155")

    s.save(DOCS_DIR / "wiring_alternatives.svg")


if __name__ == "__main__":
    generate_lilygo_adapter()
    generate_waveshare_adapter()
    generate_alternatives()
    print("Generated 3 SVG wiring diagrams in", DOCS_DIR)
