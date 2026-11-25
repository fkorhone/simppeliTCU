#!/usr/bin/env python3
"""
Generate maintainable SVG wiring diagrams for simppeliTCU documentation.

Usage:
    python docs/generate_diagrams.py

Outputs:
    - docs/wiring_lilygo_adapter.svg    (Option A: Recommended 16-pin adapter + LilyGo T-2CAN, IT-CAN + optional AV-CAN)
    - docs/wiring_waveshare_adapter.svg (Alternative controller: Waveshare ESP32-S3-RS485-CAN)
    - docs/wiring_alternatives.svg      (Option B: CAN-only unpin + tap, Option C: TCU unplugged)
    - docs/wiring_can_loopback.svg      (Optional CAN loopback for passive monitoring alongside original TCU)

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
            f'fill="{fill}" stroke="{stroke}" stroke_width="{stroke_width}"{dash_attr}/>'
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
        highlights: dict[int, tuple[str, str, str]],
    ) -> dict[int, tuple[float, float]]:
        """
        Draw a 16-pin Sumitomo NH .025" connector in Wire-Side View (latch at top).
        - Male Wire-Side View: top row 8..1 (left->right), bottom row 16..9 (left->right)
        - Female Wire-Side View: top row 1..8 (left->right), bottom row 9..16 (left->right)
        highlights: {pin_number: (fill_col, txt_col, optional_dash)}
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
                    entry = highlights[pin]
                    fill_col, txt_col = entry[0], entry[1]
                    dash = entry[2] if len(entry) > 2 else ""
                    self.rect(
                        px + 1.5,
                        py + 1.5,
                        cell_w - 3,
                        cell_h - 3,
                        fill=fill_col,
                        stroke="#0f172a",
                        stroke_width=1.6,
                        rx=3,
                        dash=dash,
                    )
                    self.text(cx, cy + 4, str(pin), size=11.5, fill=txt_col, weight="bold", anchor="middle")
                else:
                    self.rect(px + 1.5, py + 1.5, cell_w - 3, cell_h - 3, fill="#ffffff", stroke="#94a3b8", stroke_width=1.1, rx=3)
                    self.text(cx, cy + 4, str(pin), size=10.5, fill="#64748b", anchor="middle")

        return coords

    def draw_40pin_tcu_connector(
        self,
        x: float,
        y: float,
        highlights: dict[int, tuple[str, str, str, str]],
    ) -> dict[int, tuple[float, float]]:
        """
        Draw Nissan Leaf ZE1 40-pin TCU Female Plug in Wire-Side View (latch at top).
        Matches docs/tcu_pins.jpg:
        - Top row (left -> right): 20 .. 1  (Pin 1 is TOP-RIGHT)
        - Bottom row (left -> right): 40 .. 21 (Pin 26 below 6, 27 below 7, 28 below 8)
        highlights: {pin: (fill, text_color, stroke_dash, stroke_color)}
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
                    fill_col, txt_col, dash, stroke_col = highlights[pin]
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
COL_CANH_DEV = "#eab308"  # Yellow/Gold IT-CAN H on device side
COL_CANL_DEV = "#16a34a"  # Green IT-CAN L on device side
COL_CANH_OEM = "#0ea5e9"  # Sky Blue (SB) factory CAN-H (#6 IT-CAN H & #26 M CAN-H)
COL_CANL_OEM = "#8b5cf6"  # Violet (V) factory IT-CAN L (#7)
COL_MCANH_DEV = "#0284c7" # Sky Blue / Cyan M CAN-H (AV-CAN H, #26 SB / Pin 5)
COL_MCANL_OEM = "#65a30d" # Light Green (LG) factory M CAN-L (AV-CAN L, #27 LG / Pin 6)


def generate_lilygo_adapter() -> None:
    s = SvgBuilder(
        1440,
        850,
        "Option A: Non-Destructive 16-Pin Sumitomo Adapter Wiring (LilyGo T-2CAN, IT-CAN + Optional AV-CAN)",
    )

    # Header banner
    s.rect(16, 14, 1408, 54, fill="#0f172a", stroke="#0f172a", rx=8)
    s.text(
        34,
        39,
        "Option A (Recommended): Non-Destructive 16-Pin Adapter — LilyGo T-2CAN (IT-CAN + Optional AV-CAN)",
        size=18.5,
        fill="#ffffff",
        weight="bold",
    )
    s.text(
        34,
        58,
        "All connector pinouts shown from WIRE-SIDE VIEW (looking into wire-entry back of connector, lock tab on top — matches tcu_pins.jpg). Dashed lines = optional AV-CAN.",
        size=12,
        fill="#cbd5e1",
    )

    # Left & Right main cards with generous center gutter
    s.rect(16, 80, 646, 754, fill="#f8fafc", stroke="#cbd5e1", stroke_width=1.5, rx=10)
    s.text(34, 106, "1. DEVICE SIDE (Build & Pre-Flash on Bench)", size=15, fill="#0f172a", weight="bold")

    s.rect(778, 80, 646, 754, fill="#f8fafc", stroke="#cbd5e1", stroke_width=1.5, rx=10)
    s.text(796, 106, "2. CAR SIDE (Behind Glovebox — No Factory Wires Cut)", size=15, fill="#0f172a", weight="bold")

    # =========================================================================
    # LEFT: LilyGo T-2CAN Board (Dual CAN: Port B = IT-CAN, Port A = Optional AV-CAN)
    # =========================================================================
    bx, by, bw, bh = 54, 134, 270, 256
    s.rect(bx, by, bw, bh, fill="#1e293b", stroke="#0f172a", stroke_width=2.2, rx=8)
    s.text(bx + 16, by + 24, "LilyGo T-2CAN Board", size=15, fill="#ffffff", weight="bold")
    s.text(bx + 16, by + 42, "ESP32-S3 (Dual CAN: TWAI + MCP2515)", size=11, fill="#94a3b8")

    # ESP32-S3 shield illustration
    s.rect(bx + 20, by + 60, 98, 84, fill="#475569", stroke="#94a3b8", stroke_width=1.4, rx=4)
    s.text(bx + 69, by + 99, "ESP32-S3", size=11.5, fill="#f8fafc", weight="bold", anchor="middle")
    s.text(bx + 69, by + 116, "Wi-Fi / MCU", size=10, fill="#cbd5e1", anchor="middle")

    # USB-C port on left edge of board
    s.rect(bx - 12, by + 88, 18, 34, fill="#cbd5e1", stroke="#334155", stroke_width=1.6, rx=3)
    s.text(bx + 16, by + 170, "USB-C Port:", size=10.5, fill="#f8fafc", weight="bold")
    s.text(bx + 16, by + 185, "5V Power +", size=10.5, fill="#cbd5e1")
    s.text(bx + 16, by + 200, "Serial CLI", size=10.5, fill="#cbd5e1")

    # Port B screw terminal block (top right of board - REQUIRED IT-CAN)
    pb_x, pb_y = bx + 150, by + 52  # y=186..278
    s.rect(pb_x, pb_y, 112, 92, fill="#14532d", stroke="#4ade80", stroke_width=1.8, rx=4)
    s.text(pb_x + 8, pb_y + 15, "PORT B (IT-CAN)", size=10.5, fill="#bbf7d0", weight="bold")
    s.text(pb_x + 8, pb_y + 28, "TWAI GPIO 7 / 6", size=9.5, fill="#86efac")

    term_coords: dict[str, tuple[float, float]] = {}
    for t_name, ty in [("CANHB", pb_y + 43), ("CANLB", pb_y + 62), ("SGNDB", pb_y + 81)]:
        s.text(pb_x + 70, ty + 4, t_name, size=10.2, fill="#ffffff", weight="bold", anchor="end")
        s.circle(pb_x + 90, ty, 6.2, fill="#cbd5e1", stroke="#0f172a", stroke_width=1.3)
        s.line(pb_x + 86, ty - 3, pb_x + 94, ty + 3, stroke="#334155", stroke_width=1.3)
        term_coords[t_name] = (pb_x + 112, ty)

    # Port A screw terminal block (bottom right of board - OPTIONAL AV-CAN / M-CAN)
    pa_x, pa_y = bx + 150, by + 152  # y=286..378
    s.rect(pa_x, pa_y, 112, 92, fill="#1e3a8a", stroke="#60a5fa", stroke_width=1.6, rx=4, dash="5,3")
    s.text(pa_x + 8, pa_y + 15, "PORT A (Opt. AV)", size=10.2, fill="#dbeafe", weight="bold")
    s.text(pa_x + 8, pa_y + 28, "MCP2515 SPI", size=9.5, fill="#93c5fd")

    for t_name, ty in [("CANHA", pa_y + 43), ("CANLA", pa_y + 62), ("SGNDA", pa_y + 81)]:
        s.text(pa_x + 70, ty + 4, t_name, size=10.2, fill="#ffffff", weight="bold", anchor="end")
        s.circle(pa_x + 90, ty, 6.2, fill="#cbd5e1", stroke="#0f172a", stroke_width=1.3)
        s.line(pb_x + 86, ty - 3, pb_x + 94, ty + 3, stroke="#334155", stroke_width=1.3)
        term_coords[t_name] = (pa_x + 112, ty)

    # Note about SGNDB/SGNDA right under LilyGo
    s.rect(54, 400, 290, 54, fill="#eff6ff", stroke="#93c5fd", stroke_width=1.2, rx=6)
    s.text(64, 418, "Note on SGNDB / SGNDA (Signal GND):", size=10.8, fill="#1e3a8a", weight="bold")
    s.text(64, 433, "Connected by default (recommended for isolated", size=10.2, fill="#1e40af")
    s.text(64, 447, "transceivers; usually works without them too).", size=10.2, fill="#1e40af")

    # =========================================================================
    # LEFT BOTTOM: Automotive 12V -> 5V USB Converter + 2A Inline Fuse
    # =========================================================================
    s.rect(54, 474, 232, 136, fill="#ffffff", stroke="#475569", stroke_width=1.8, rx=8)
    s.text(170, 496, "Automotive 12V -> 5V USB", size=13, fill="#0f172a", weight="bold", anchor="middle")
    s.text(170, 512, "Step-Down Socket / Converter", size=11, fill="#475569", anchor="middle")

    s.rect(68, 532, 54, 52, fill="#e2e8f0", stroke="#334155", stroke_width=1.5, rx=6)
    s.text(95, 555, "5V USB", size=10.5, fill="#0f172a", weight="bold", anchor="middle")
    s.text(95, 571, "OUT", size=10, fill="#475569", anchor="middle")

    s.circle(264, 535, 7, fill="#e2e8f0", stroke="#0f172a", stroke_width=1.4)
    s.text(248, 539, "- GND", size=11.5, fill="#0f172a", weight="bold", anchor="end")
    s.circle(264, 579, 7, fill="#fee2e2", stroke="#dc2626", stroke_width=1.4)
    s.text(248, 583, "+ 12V", size=11.5, fill="#dc2626", weight="bold", anchor="end")

    # USB-A to USB-C power cable from Converter to LilyGo USB-C
    s.wire([(68, 558), (28, 558), (28, by + 105), (bx - 12, by + 105)], "#ca8a04", width=5.0)
    s.text(36, 466, "5V USB Cable", size=11, fill="#854d0e", weight="bold")

    # =========================================================================
    # CENTER-LEFT: 16-Pin Sumitomo NH .025" MALE Connector (Wire-Side View)
    # Placed at y=296..376; title cleanly at y=148..165 above all wires!
    # =========================================================================
    male_x, male_y = 408, 296
    s.text(male_x + 114, 148, "16-Pin NH .025\" MALE", size=14, fill="#0f172a", weight="bold", anchor="middle")
    s.text(male_x + 114, 165, "Wire-Side View (6 std + 2 opt. AV pins)", size=11, fill="#475569", anchor="middle")

    male_highlights = {
        1: ("#fee2e2", "#991b1b", ""),     # +12V in
        2: ("#1e293b", "#ffffff", ""),     # GND in
        3: ("#fef08a", "#713f12", ""),     # IT-CAN H
        4: ("#bbf7d0", "#14532d", ""),     # IT-CAN L
        5: ("#bae6fd", "#0c4a6e", "3,2"),  # Opt. M CAN-H (AV-CAN H)
        6: ("#d9f99d", "#365314", "3,2"),  # Opt. M CAN-L (AV-CAN L)
        9: ("#fecaca", "#991b1b", ""),     # +12V loop out
        10: ("#334155", "#ffffff", ""),    # GND loop out
    }
    m_pins = s.draw_16pin_connector(male_x, male_y, is_male=True, highlights=male_highlights)

    # Non-crossing nested staircase for CAN + Power wires on Device Side:
    # Vertical GND trunk at x=352:
    gnd_bus_x = 352
    gnd_junc = (gnd_bus_x, 535)

    p1x, p1y = m_pins[1]
    p2x, p2y = m_pins[2]
    p3x, p3y = m_pins[3]
    p4x, p4y = m_pins[4]
    p5x, p5y = m_pins[5]
    p6x, p6y = m_pins[6]
    p9x, p9y = m_pins[9]
    p10x, p10y = m_pins[10]

    # 1) IT-CAN H: CANHB (y=229) -> p3x -> Pin 3
    th_x, th_y = term_coords["CANHB"]
    s.wire([(th_x, th_y), (p3x, th_y), (p3x, p3y - 13)], COL_CANH_DEV, width=3.2)
    s.text(448, th_y - 5, "IT-CAN H (Pin 3)", size=10.2, fill="#854d0e", weight="bold")

    # 2) IT-CAN L: CANLB (y=248) -> p4x -> Pin 4
    tl_x, tl_y = term_coords["CANLB"]
    s.wire([(tl_x, tl_y), (p4x, tl_y), (p4x, p4y - 13)], COL_CANL_DEV, width=3.2)
    s.text(440, tl_y - 5, "IT-CAN L (Pin 4)", size=10.2, fill="#166534", weight="bold")

    # 3) Optional AV-CAN H (M CAN-H): CANHA (y=329) -> x=374 -> up to y=262 -> p5x -> Pin 5
    tah_x, tah_y = term_coords["CANHA"]
    s.wire([(tah_x, tah_y), (374, tah_y), (374, 262), (p5x, 262), (p5x, p5y - 13)], COL_MCANH_DEV, width=2.8, dash="6,4")

    # 4) Optional AV-CAN L (M CAN-L): CANLA (y=348) -> x=388 -> up to y=276 -> p6x -> Pin 6
    tal_x, tal_y = term_coords["CANLA"]
    s.wire([(tal_x, tal_y), (388, tal_y), (388, 276), (p6x, 276), (p6x, p6y - 13)], COL_MCANL_OEM, width=2.8, dash="6,4")
    s.text(404, 405, "Dashed Pin 5 & 6 = Opt. AV-CAN", size=10.2, fill="#1e40af", weight="bold")
    s.text(404, 419, "(Nag Dismiss & GPS Tracking)", size=10, fill="#3b82f6")

    # 5) GND splice: Pin 2 + Pin 10 + Converter (-) + SGNDB (+ optional SGNDA)
    tsb_x, tsb_y = term_coords["SGNDB"]
    tsa_x, tsa_y = term_coords["SGNDA"]
    s.wire([(p2x, p2y - 13), (p2x, 204), (gnd_bus_x, 204), gnd_junc], COL_GND, width=3.0)
    s.wire([(p10x, p10y + 13), (p10x, 438), (gnd_bus_x, 438)], COL_GND, width=3.0)
    s.wire([(tsb_x, tsb_y), (gnd_bus_x, tsb_y)], COL_GND, width=3.0)
    s.wire([(tsa_x, tsa_y), (gnd_bus_x, tsa_y)], COL_GND, width=2.6, dash="5,4")
    s.wire([(264, 535), gnd_junc], COL_GND, width=3.0)

    s.circle(gnd_bus_x, tsb_y, 4.2, fill=COL_GND)
    s.circle(gnd_bus_x, tsa_y, 4.2, fill=COL_GND)
    s.circle(gnd_bus_x, 438, 4.2, fill=COL_GND)
    s.circle(*gnd_junc, 4.5, fill=COL_GND)
    s.text(362, 531, "GND splice (4 ends, or 5 w/ SGNDA)", size=10.5, fill="#0f172a", weight="bold")

    # 6) +12V 3-way split: Male Pin 1 + Male Pin 9 + 2A Inline Fuse -> Converter (+)
    v12_bus_x = 648
    v12_junc = (v12_bus_x, 579)
    s.wire([(p1x, p1y - 13), (p1x, 188), (v12_bus_x, 188), v12_junc], COL_12V, width=3.2)
    s.wire([(p9x, p9y + 13), (p9x, 456), (v12_bus_x, 456)], COL_12V, width=3.2)
    s.circle(v12_bus_x, 456, 4.5, fill=COL_12V)

    fuse_cx, fuse_cy = 468, 579
    s.wire([v12_junc, (fuse_cx + 29, fuse_cy)], COL_12V, width=3.2)
    s.wire([(fuse_cx - 29, fuse_cy), (264, 579)], COL_12V, width=3.2)
    s.fuse(fuse_cx, fuse_cy, "2A Inline Fuse")
    s.text(576, 570, "+12V splice (3 ends)", size=10.5, fill="#991b1b", weight="bold", anchor="middle")

    # Device-side summary box at bottom-left
    s.rect(54, 626, 594, 192, fill="#ffffff", stroke="#cbd5e1", stroke_width=1.3, rx=6)
    s.text(70, 648, "Device-Side Harness Summary (Step 2 on Bench):", size=12.5, fill="#0f172a", weight="bold")
    s.text(70, 668, "• +12V (Red, 3 ends): Male Pin 1  +  loop to Male Pin 9  +  via 2A Fuse to 12V->USB (+)", size=11, fill="#334155")
    s.text(70, 687, "• GND (Black, 4 or 5 ends): Male Pin 2 + loop to Pin 10 + 12V->USB (-) + SGNDB (+ opt. SGNDA)", size=11, fill="#334155")
    s.text(70, 706, "• IT-CAN (Required): Male Pin 3 -> Port B CANHB   |   Male Pin 4 -> Port B CANLB", size=11, fill="#334155")
    s.text(70, 725, "• AV-CAN / M-CAN (Optional): Male Pin 5 -> Port A CANHA   |   Male Pin 6 -> Port A CANLA", size=11, fill="#1e40af", weight="bold")
    s.text(80, 742, "(Enables automatic infotainment nag-screen dismissal & GPS location tracking)", size=10.5, fill="#334155")
    s.text(70, 764, "• Why loop Pins 1->9 & 2->10? Routes +12V & GND back to the factory TCU so the cabin", size=11, fill="#0f172a", weight="bold")
    s.text(80, 781, "hands-free microphone keeps working, without cutting any factory wires!", size=11, fill="#0f172a", weight="bold")
    s.text(70, 803, "• Passive-monitoring option: See wiring_can_loopback.svg to loop CAN back via Pins 11–14.", size=10.5, fill="#475569", italic=True)

    # =========================================================================
    # MATING INDICATOR IN CENTER GUTTER
    # =========================================================================
    s.line(638, 336, 800, 336, stroke="#0284c7", stroke_width=2.5, dash="5,4")
    s.badge(720, 336, "PLUGS TOGETHER", bg="#0284c7", fg="#ffffff", border="#0369a1", w=122, h=24, size=10.5)

    # =========================================================================
    # RIGHT: 16-Pin Sumitomo NH .025" FEMALE Connector (Wire-Side View)
    # =========================================================================
    fem_x, fem_y = 802, 296
    s.text(fem_x + 114, 148, "16-Pin NH .025\" FEMALE", size=14, fill="#0f172a", weight="bold", anchor="middle")
    s.text(fem_x + 114, 165, "Wire-Side View (4–6 OEM + 2 return pins)", size=11, fill="#475569", anchor="middle")

    fem_highlights = {
        1: ("#ffffff", "#0f172a", ""),     # OEM #1 White +12V
        2: ("#1e293b", "#ffffff", ""),     # OEM #28 Black GND
        3: ("#bae6fd", "#0c4a6e", ""),     # OEM #6 Sky Blue IT-CAN H
        4: ("#ddd6fe", "#4c1d95", ""),     # OEM #7 Violet IT-CAN L
        5: ("#e0f2fe", "#0369a1", "3,2"),  # Opt. OEM #26 Sky Blue M CAN-H
        6: ("#ecfccb", "#365314", "3,2"),  # Opt. OEM #27 Light Green M CAN-L
        9: ("#fecaca", "#991b1b", ""),     # +12V return to TCU #1
        10: ("#334155", "#ffffff", ""),    # GND return to TCU #28
    }
    f_pins = s.draw_16pin_connector(fem_x, fem_y, is_male=False, highlights=fem_highlights)

    # =========================================================================
    # TOP-RIGHT: Car Wiring Harness Bundle (Source of the 4 + 2 unpinned OEM wires)
    # =========================================================================
    loom_x, loom_y = 1102, 126
    s.rect(loom_x, loom_y, 304, 166, fill="#e2e8f0", stroke="#475569", stroke_width=1.8, rx=8)
    s.text(loom_x + 14, loom_y + 21, "Car Wiring Harness Loom", size=13.5, fill="#0f172a", weight="bold")
    s.text(loom_x + 14, loom_y + 38, "Unpinned from 40-pin TCU -> 16-pin Female:", size=10.8, fill="#334155")
    s.text(loom_x + 14, loom_y + 57, "• OEM #1 (W, White): +12V -> Fem Pin 1", size=10.4, fill="#0f172a", weight="bold")
    s.text(loom_x + 14, loom_y + 73, "• OEM #28 (B, Black): GND -> Fem Pin 2", size=10.4, fill="#0f172a", weight="bold")
    s.text(loom_x + 14, loom_y + 89, "• OEM #6 (SB, Sky Blue): IT-CAN H -> Fem Pin 3", size=10.4, fill="#0369a1", weight="bold")
    s.text(loom_x + 14, loom_y + 105, "• OEM #7 (V, Violet): IT-CAN L -> Fem Pin 4", size=10.4, fill="#6d28d9", weight="bold")
    s.line(loom_x + 14, loom_y + 114, loom_x + 290, loom_y + 114, stroke="#cbd5e1", stroke_width=1.2)
    s.text(loom_x + 14, loom_y + 130, "Optional AV-CAN (Nag Dismiss & GPS):", size=10.2, fill="#1e40af", weight="bold")
    s.text(loom_x + 14, loom_y + 145, "• OEM #26 (SB, Sky Blue): M CAN-H -> Fem Pin 5", size=10.2, fill="#0284c7", weight="bold")
    s.text(loom_x + 14, loom_y + 160, "• OEM #27 (LG, Lt Green): M CAN-L -> Fem Pin 6", size=10.2, fill="#4d7c0f", weight="bold")

    # Route the 6 unpinned OEM wires at y=184, 198, 212, 226, 240, 254 (all strictly > 165 and < 284!)
    fp1x, fp1y = f_pins[1]
    fp2x, fp2y = f_pins[2]
    fp3x, fp3y = f_pins[3]
    fp4x, fp4y = f_pins[4]
    fp5x, fp5y = f_pins[5]
    fp6x, fp6y = f_pins[6]

    s.wire([(loom_x, 184), (fp1x, 184), (fp1x, fp1y - 13)], COL_12V_OEM, width=3.2, outline="#334155")
    s.wire([(loom_x, 198), (fp2x, 198), (fp2x, fp2y - 13)], COL_GND, width=3.2)
    s.wire([(loom_x, 212), (fp3x, 212), (fp3x, fp3y - 13)], COL_CANH_OEM, width=3.2)
    s.wire([(loom_x, 226), (fp4x, 226), (fp4x, fp4y - 13)], COL_CANL_OEM, width=3.2)
    s.wire([(loom_x, 240), (fp5x, 240), (fp5x, fp5y - 13)], COL_MCANH_DEV, width=2.8, dash="6,4")
    s.wire([(loom_x, 254), (fp6x, 254), (fp6x, fp6y - 13)], COL_MCANL_OEM, width=2.8, dash="6,4")

    # Remaining untouched OEM wires going down the far right to 40-pin TCU connector
    s.wire([(1384, loom_y + 166), (1384, 514), (1358, 514)], "#94a3b8", width=6.0)
    s.text(1374, 355, "Remaining OEM wires", size=10.5, fill="#475569", anchor="end", italic=True)
    s.text(1374, 370, "(stay in 40-pin plug)", size=10.5, fill="#475569", anchor="end", italic=True)

    # =========================================================================
    # RIGHT LOWER: Original 40-Pin Nissan TCU Connector (Wire-Side View)
    # =========================================================================
    tcu_x, tcu_y = 922, 498
    s.text(tcu_x + 218, tcu_y - 30, "Original 40-Pin Nissan TCU Connector — Wire-Side View", size=13, fill="#0f172a", weight="bold", anchor="middle")
    s.text(tcu_x + 218, tcu_y - 15, "(Plugs back into original TCU so hands-free microphone stays powered)", size=10.8, fill="#475569", anchor="middle")

    tcu_highlights = {
        1: ("#fecaca", "#991b1b", "", "#ffffff"),        # +12V return from Fem Pin 9
        6: ("#0f172a", "#fca5a5", "3,2", "#ef4444"),     # EMPTY (unpinned IT-CAN H)
        7: ("#0f172a", "#fca5a5", "3,2", "#ef4444"),     # EMPTY (unpinned IT-CAN L)
        26: ("#0f172a", "#93c5fd", "3,2", "#38bdf8"),    # EMPTY if AV-CAN unpinned (M CAN-H)
        27: ("#0f172a", "#bef264", "3,2", "#84cc16"),    # EMPTY if AV-CAN unpinned (M CAN-L)
        28: ("#e2e8f0", "#0f172a", "", "#ffffff"),       # GND return from Fem Pin 10
    }
    t_pins = s.draw_40pin_tcu_connector(tcu_x, tcu_y, highlights=tcu_highlights)

    # Callout pointing to empty pins 6/7 (and 26/27 right below them!)
    t6x, t6y = t_pins[6]
    t7x, t7y = t_pins[7]
    mid_67 = (t6x + t7x) / 2
    s.line(mid_67, t6y - 12, mid_67, tcu_y - 48, stroke="#dc2626", stroke_width=1.6, dash="3,2")
    s.badge(mid_67, tcu_y - 58, "#6, #7 EMPTY (+ #26, #27 if AV-CAN used)", bg="#fef2f2", fg="#991b1b", border="#f87171", w=248, h=20, size=10.2)

    # Route the 2 short return wires cleanly without touching any text:
    fp9x, fp9y = f_pins[9]
    fp10x, fp10y = f_pins[10]
    t1x, t1y = t_pins[1]
    t28x, t28y = t_pins[28]

    s.wire(
        [(fp9x, fp9y + 13), (fp9x, 404), (1368, 404), (1368, t1y), (t1x + 10, t1y)],
        COL_12V,
        width=3.2,
    )
    s.text(868, 397, "+12V return wire (Fem Pin 9 -> TCU #1)", size=10.8, fill="#991b1b", weight="bold")

    s.wire(
        [(fp10x, fp10y + 13), (fp10x, 592), (t28x, 592), (t28x, t28y + 12)],
        COL_GND,
        width=3.2,
    )
    s.text(868, 608, "GND return wire (Fem Pin 10 -> TCU #28)", size=10.8, fill="#0f172a", weight="bold")

    # 40-pin TCU Pinout Reference Table at bottom-right
    s.rect(796, 626, 610, 192, fill="#ffffff", stroke="#cbd5e1", stroke_width=1.3, rx=6)
    s.text(812, 648, "Nissan Leaf ZE1 40-Pin TCU Reference & Car-Side Pin Mapping:", size=12.5, fill="#0f172a", weight="bold")
    s.text(812, 668, "• TCU #1  (W, White — +12V BAT)  : Unpin OEM -> Fem #1   |  Return wire: Fem #9 -> TCU #1", size=10.8, fill="#334155")
    s.text(812, 687, "• TCU #28 (B, Black — GND)       : Unpin OEM -> Fem #2   |  Return wire: Fem #10 -> TCU #28", size=10.8, fill="#334155")
    s.text(812, 706, "• TCU #6  (SB, Sky Blue — IT-CAN H): Unpin OEM -> Fem #3 |  Leave TCU #6 EMPTY*", size=10.8, fill="#334155")
    s.text(812, 725, "• TCU #7  (V, Violet — IT-CAN L)   : Unpin OEM -> Fem #4 |  Leave TCU #7 EMPTY*", size=10.8, fill="#334155")
    s.text(812, 744, "• TCU #26 (SB, Sky Blue — M CAN-H) : [Optional AV-CAN] Unpin OEM -> Fem #5 | Leave #26 EMPTY*", size=10.8, fill="#1e40af", weight="bold")
    s.text(812, 763, "• TCU #27 (LG, Lt Green — M CAN-L) : [Optional AV-CAN] Unpin OEM -> Fem #6 | Leave #27 EMPTY*", size=10.8, fill="#1e40af", weight="bold")
    s.text(812, 784, "*Note: Two Sky Blue (SB) wires exist (#6 top row vs #26 bottom row)—tag them before unpinning!", size=10.5, fill="#9a3412", weight="bold")
    s.text(812, 803, " NH .025\" female pins from the 16-pin kit fit directly into the 40-pin TCU connector.", size=10.5, fill="#0f172a")

    s.save(DOCS_DIR / "wiring_lilygo_adapter.svg")


def generate_waveshare_adapter() -> None:
    s = SvgBuilder(
        980,
        520,
        "Alternative Controller Wiring: Waveshare ESP32-S3-RS485-CAN to 16-Pin Adapter",
    )

    s.rect(16, 14, 948, 52, fill="#0f172a", stroke="#0f172a", rx=8)
    s.text(
        34,
        38,
        "Alternative Controller: Waveshare ESP32-S3-RS485-CAN (Single CAN — IT-CAN or AV-CAN)",
        size=17.5,
        fill="#ffffff",
        weight="bold",
    )
    s.text(
        34,
        56,
        "Uses onboard 7–36V DC regulator (no external 12V->5V USB converter needed). Wire its single CAN port to IT-CAN (#6/#7) OR AV-CAN (#26/#27).",
        size=11.8,
        fill="#cbd5e1",
    )

    s.rect(16, 78, 948, 426, fill="#f8fafc", stroke="#cbd5e1", stroke_width=1.5, rx=10)

    wx, wy, ww, wh = 68, 144, 340, 165
    s.rect(wx, wy, ww, wh, fill="#1e293b", stroke="#0f172a", stroke_width=2.2, rx=8)
    s.text(wx + ww / 2, wy + 28, "Waveshare ESP32-S3-RS485-CAN", size=15, fill="#ffffff", weight="bold", anchor="middle")
    s.text(wx + ww / 2, wy + 47, "Built-in DC 7–36V Power & Single CAN (GPIO 15/16)", size=11, fill="#94a3b8", anchor="middle")

    s.rect(wx + 12, wy + 72, 92, 68, fill="#14532d", stroke="#4ade80", stroke_width=1.6, rx=4)
    s.text(wx + 58, wy + 90, "DC 7~36V", size=10.5, fill="#bbf7d0", weight="bold", anchor="middle")
    ws_vpos = (wx + 12, wy + 108)
    ws_vneg = (wx + 12, wy + 128)
    s.circle(wx + 28, wy + 108, 6, fill="#fecaca", stroke="#dc2626", stroke_width=1.3)
    s.text(wx + 42, wy + 112, "+ (12V)", size=10.5, fill="#ffffff", weight="bold")
    s.circle(wx + 28, wy + 128, 6, fill="#cbd5e1", stroke="#0f172a", stroke_width=1.3)
    s.text(wx + 42, wy + 132, "- (GND)", size=10.5, fill="#ffffff", weight="bold")

    s.rect(wx + 135, wy + wh - 8, 54, 14, fill="#cbd5e1", stroke="#334155", stroke_width=1.4, rx=3)
    s.text(wx + 162, wy + wh + 22, "USB-C (Flashing / CLI)", size=10.5, fill="#334155", weight="bold", anchor="middle")

    s.rect(wx + ww - 112, wy + 64, 100, 88, fill="#14532d", stroke="#4ade80", stroke_width=1.6, rx=4)
    s.text(wx + ww - 62, wy + 80, "CAN / RS485", size=10.5, fill="#bbf7d0", weight="bold", anchor="middle")

    ws_canh = (wx + ww, wy + 98)
    ws_canl = (wx + ww, wy + 118)
    s.text(wx + ww - 36, wy + 102, "CAN-H", size=10.5, fill="#ffffff", weight="bold", anchor="end")
    s.circle(wx + ww - 22, wy + 98, 6, fill="#fef08a", stroke="#0f172a", stroke_width=1.3)
    s.text(wx + ww - 36, wy + 122, "CAN-L", size=10.5, fill="#ffffff", weight="bold", anchor="end")
    s.circle(wx + ww - 22, wy + 118, 6, fill="#bbf7d0", stroke="#0f172a", stroke_width=1.3)
    s.text(wx + ww - 62, wy + 142, "(RS485 A/B unused)", size=9.5, fill="#86efac", anchor="middle")

    male_x, male_y = 655, 214
    s.text(male_x + 114, 104, "16-Pin NH .025\" MALE", size=14, fill="#0f172a", weight="bold", anchor="middle")
    s.text(male_x + 114, 121, "Wire-Side View (mates with Car Side)", size=11, fill="#475569", anchor="middle")

    male_highlights = {
        1: ("#fee2e2", "#991b1b", ""),
        2: ("#1e293b", "#ffffff", ""),
        3: ("#fef08a", "#713f12", ""),
        4: ("#bbf7d0", "#14532d", ""),
        5: ("#bae6fd", "#0c4a6e", "3,2"),
        6: ("#d9f99d", "#365314", "3,2"),
        9: ("#fecaca", "#991b1b", ""),
        10: ("#334155", "#ffffff", ""),
    }
    m_pins = s.draw_16pin_connector(male_x, male_y, is_male=True, highlights=male_highlights)

    p3x, p3y = m_pins[3]
    p4x, p4y = m_pins[4]
    p5x, p5y = m_pins[5]
    p6x, p6y = m_pins[6]
    s.wire([ws_canh, (505, ws_canh[1]), (505, 168), (p3x, 168), (p3x, p3y - 13)], COL_CANH_DEV, width=3.2)
    s.wire([(505, 168), (p5x, 168), (p5x, p5y - 13)], COL_MCANH_DEV, width=2.4, dash="5,3")
    s.text(552, 160, "CAN-H -> Pin 3 (IT-CAN) or Pin 5 (AV-CAN)", size=10.5, fill="#854d0e", weight="bold", anchor="middle")

    s.wire([ws_canl, (538, ws_canl[1]), (538, 186), (p4x, 186), (p4x, p4y - 13)], COL_CANL_DEV, width=3.2)
    s.wire([(538, 186), (p6x, 186), (p6x, p6y - 13)], COL_MCANL_OEM, width=2.4, dash="5,3")
    s.text(634, 201, "CAN-L -> Pin 4 or Pin 6", size=10.3, fill="#166534", weight="bold", anchor="middle")

    p2x, p2y = m_pins[2]
    p10x, p10y = m_pins[10]
    s.wire([(p2x, p2y - 13), (p2x, 146), (912, 146), (912, 354), (40, 354), (40, ws_vneg[1]), ws_vneg], COL_GND, width=3.0)
    s.wire([(p10x, p10y + 13), (p10x, 354)], COL_GND, width=3.0)
    s.circle(p10x, 354, 4.5, fill=COL_GND)
    s.text(630, 346, "GND (Pin 2 looped to Pin 10 -> Waveshare DC -)", size=11, fill="#0f172a", weight="bold", anchor="middle")

    p1x, p1y = m_pins[1]
    p9x, p9y = m_pins[9]
    fuse_cx, fuse_cy = 340, 392
    s.wire([(p1x, p1y - 13), (p1x, 134), (932, 134), (932, fuse_cy), (fuse_cx + 29, fuse_cy)], COL_12V, width=3.2)
    s.wire([(p9x, p9y + 13), (p9x, fuse_cy)], COL_12V, width=3.2)
    s.circle(p9x, fuse_cy, 4.5, fill=COL_12V)
    s.wire([(fuse_cx - 29, fuse_cy), (24, fuse_cy), (24, ws_vpos[1]), ws_vpos], COL_12V, width=3.2)
    s.fuse(fuse_cx, fuse_cy, "2A Inline Fuse")
    s.text(630, 384, "+12V (Pin 1 looped to Pin 9 -> 2A Fuse -> Waveshare DC +)", size=11, fill="#991b1b", weight="bold", anchor="middle")

    # Single-CAN bus choice box at bottom
    s.rect(40, 418, 898, 72, fill="#ffffff", stroke="#cbd5e1", stroke_width=1.3, rx=6)
    s.text(56, 439, "Single-CAN Bus Choice (Connect ONLY ONE bus — never bridge IT-CAN and AV-CAN together!):", size=11.5, fill="#0f172a", weight="bold")
    s.text(56, 458, "• Standard TCU Replacement (IT-CAN): Wire CAN-H/L to Pins 3 & 4 (unpin TCU #6/#7) -> Climate, Charging, Locks, SOC, Temp, Odometer.", size=10.8, fill="#334155")
    s.text(56, 476, "• Standalone AV-CAN Mode: Wire CAN-H/L to Pins 5 & 6 (unpin TCU #26/#27; leave #6/#7 in TCU!) -> Nag-Screen Dismiss & GPS Tracking only.", size=10.8, fill="#1e40af", weight="bold")

    s.save(DOCS_DIR / "wiring_waveshare_adapter.svg")


def generate_can_loopback() -> None:
    """
    Generate optional CAN loopback schematic (Pins 1..6 -> 9..14) for using simppeliTCU
    in passive-monitoring mode (or partial bus isolation) while keeping the original TCU connected.
    """
    s = SvgBuilder(
        1420,
        560,
        "Optional CAN Loopback Wiring: Monitoring Alongside an Active Original TCU",
    )

    s.rect(16, 14, 1388, 54, fill="#0f172a", stroke="#0f172a", rx=8)
    s.text(
        34,
        39,
        "Optional Adapter Variation: Looping CAN Buses Back to Original TCU (Passive Monitoring Mode)",
        size=18,
        fill="#ffffff",
        weight="bold",
    )
    s.text(
        34,
        58,
        "Routes IT-CAN (3->11, 4->12) and/or M-CAN / AV-CAN (5->13, 6->14) back to the factory TCU so the original TCU stays connected.",
        size=12,
        fill="#cbd5e1",
    )

    # Left: Device Side | Right: Car Side
    s.rect(16, 80, 646, 378, fill="#f8fafc", stroke="#cbd5e1", stroke_width=1.5, rx=10)
    s.text(34, 106, "1. DEVICE SIDE (16-Pin Male — Add Loopback Jumpers)", size=14.5, fill="#0f172a", weight="bold")

    s.rect(758, 80, 646, 378, fill="#f8fafc", stroke="#cbd5e1", stroke_width=1.5, rx=10)
    s.text(776, 106, "2. CAR SIDE (16-Pin Female -> Returns to 40-Pin TCU)", size=14.5, fill="#0f172a", weight="bold")

    # Compact LilyGo + Power block on far left
    s.rect(42, 134, 240, 224, fill="#1e293b", stroke="#0f172a", stroke_width=2.0, rx=8)
    s.text(162, 158, "simppeliTCU (LilyGo T-2CAN)", size=13.5, fill="#ffffff", weight="bold", anchor="middle")
    s.text(162, 175, "(+ 2A Fuse & 12V->5V USB Supply)", size=10.5, fill="#94a3b8", anchor="middle")

    # Terminal exit labels on right edge of LilyGo summary block
    dev_ports = [
        ("+12V (via 2A Fuse)", 202, COL_12V),
        ("GND (+ SGNDB/SGNDA)", 224, COL_GND),
        ("Port B CANHB (IT-H)", 252, COL_CANH_DEV),
        ("Port B CANLB (IT-L)", 274, COL_CANL_DEV),
        ("Port A CANHA (M-H)", 304, COL_MCANH_DEV),
        ("Port A CANLA (M-L)", 326, COL_MCANL_OEM),
    ]
    for label, py, _ in dev_ports:
        s.text(270, py + 4, label, size=10.2, fill="#f8fafc", weight="bold", anchor="end")
        s.circle(278, py, 4, fill="#cbd5e1")

    # 16-pin Male Connector at x=410, y=236
    male_x, male_y = 410, 236
    s.text(male_x + 114, 132, "16-Pin NH .025\" MALE (Wire-Side)", size=13, fill="#0f172a", weight="bold", anchor="middle")
    s.text(male_x + 114, 148, "Top row (1–6) loops to Bottom row (9–14)", size=10.8, fill="#475569", anchor="middle")

    male_hl = {
        1: ("#fee2e2", "#991b1b", ""),
        2: ("#1e293b", "#ffffff", ""),
        3: ("#fef08a", "#713f12", ""),
        4: ("#bbf7d0", "#14532d", ""),
        5: ("#bae6fd", "#0c4a6e", ""),
        6: ("#d9f99d", "#365314", ""),
        9: ("#fecaca", "#991b1b", ""),
        10: ("#334155", "#ffffff", ""),
        11: ("#fef9c3", "#713f12", "3,2"),
        12: ("#dcfce7", "#14532d", "3,2"),
        13: ("#e0f2fe", "#0c4a6e", "3,2"),
        14: ("#ecfccb", "#365314", "3,2"),
    }
    mp = s.draw_16pin_connector(male_x, male_y, is_male=True, highlights=male_hl)

    # Connect all 6 signals cleanly: top row (1..6) at y=164..224, bottom row (9..14) loops down at y=340..415!
    loop_pairs = [
        (1, 9, 202, 164, 424, COL_12V, ""),
        (2, 10, 224, 174, 412, COL_GND, ""),
        (3, 11, 252, 184, 400, COL_CANH_DEV, "5,3"),
        (4, 12, 274, 194, 388, COL_CANL_DEV, "5,3"),
        (5, 13, 304, 204, 376, COL_MCANH_DEV, "5,3"),
        (6, 14, 326, 214, 364, COL_MCANL_OEM, "5,3"),
    ]
    for idx, (top_p, bot_p, dev_y, top_bus_y, bot_bus_y, col, dsh) in enumerate(loop_pairs):
        tx, ty = mp[top_p]
        bx_p, by_p = mp[bot_p]
        trunk_x = 312 + idx * 14  # 312, 326, 340, 354, 368, 382 — 6 parallel vertical trunks, zero overlap!
        s.wire([(282, dev_y), (trunk_x, dev_y)], col, width=2.6, dash=dsh)
        s.wire([(tx, ty - 13), (tx, top_bus_y), (trunk_x, top_bus_y), (trunk_x, bot_bus_y), (bx_p, bot_bus_y), (bx_p, by_p + 13)], col, width=2.6, dash=dsh)
        s.circle(trunk_x, dev_y, 3.8, fill=col)

    s.text(339, 444, "Dashed loops (11–14): populate for whichever bus(es) should also reach Original TCU", size=10.3, fill="#1e40af", weight="bold", anchor="middle")

    # Mating badge
    s.line(640, 276, 780, 276, stroke="#0284c7", stroke_width=2.5, dash="5,4")
    s.badge(710, 276, "PLUGS TOGETHER", bg="#0284c7", fg="#ffffff", border="#0369a1", w=116, h=22, size=10)

    # Right: 16-pin Female Connector at x=782, y=236
    fem_x, fem_y = 782, 236
    s.text(fem_x + 114, 132, "16-Pin NH .025\" FEMALE (Wire-Side)", size=13, fill="#0f172a", weight="bold", anchor="middle")
    s.text(fem_x + 114, 148, "Pins 1–6: Unpinned OEM | Pins 9–14: Return to TCU", size=10.8, fill="#475569", anchor="middle")

    fem_hl = {
        1: ("#ffffff", "#0f172a", ""),
        2: ("#1e293b", "#ffffff", ""),
        3: ("#bae6fd", "#0c4a6e", ""),
        4: ("#ddd6fe", "#4c1d95", ""),
        5: ("#e0f2fe", "#0369a1", ""),
        6: ("#ecfccb", "#365314", ""),
        9: ("#fecaca", "#991b1b", ""),
        10: ("#334155", "#ffffff", ""),
        11: ("#bae6fd", "#0c4a6e", "3,2"),
        12: ("#ddd6fe", "#4c1d95", "3,2"),
        13: ("#e0f2fe", "#0369a1", "3,2"),
        14: ("#ecfccb", "#365314", "3,2"),
    }
    fp = s.draw_16pin_connector(fem_x, fem_y, is_male=False, highlights=fem_hl)

    # Top-right OEM Loom box + Bottom-right TCU 40-pin return box
    s.rect(1096, 126, 290, 124, fill="#e2e8f0", stroke="#475569", stroke_width=1.6, rx=6)
    s.text(1110, 146, "Car Loom (Unpinned OEM -> Fem 1–6):", size=11.5, fill="#0f172a", weight="bold")
    s.text(1110, 165, "#1 (W +12V) -> 1   |   #28 (B GND) -> 2", size=10.5, fill="#334155", weight="bold")
    s.text(1110, 184, "#6 (SB IT-H) -> 3  |   #7  (V IT-L) -> 4", size=10.5, fill="#0369a1", weight="bold")
    s.text(1110, 203, "#26 (SB M-H) -> 5  |   #27 (LG M-L) -> 6", size=10.5, fill="#4d7c0f", weight="bold")
    s.text(1110, 224, "(Uses 6 existing factory .025\" female pins)", size=10, fill="#475569", italic=True)

    for idx, (pin_n, col, outl) in enumerate([
        (1, COL_12V_OEM, "#334155"),
        (2, COL_GND, ""),
        (3, COL_CANH_OEM, ""),
        (4, COL_CANL_OEM, ""),
        (5, COL_MCANH_DEV, ""),
        (6, COL_MCANL_OEM, ""),
    ]):
        px, py = fp[pin_n]
        wy = 164 + idx * 11
        s.wire([(1096, wy), (px, wy), (px, py - 13)], col, width=2.6, outline=outl)

    s.rect(1096, 298, 290, 144, fill="#1e293b", stroke="#0f172a", stroke_width=1.8, rx=6)
    s.text(1110, 320, "40-Pin Original TCU (Return Jumpers):", size=11.5, fill="#ffffff", weight="bold")
    s.text(1110, 341, "• Fem #9  -> TCU #1  (+12V)", size=10.3, fill="#fecaca", weight="bold")
    s.text(1110, 358, "• Fem #10 -> TCU #28 (GND)", size=10.3, fill="#e2e8f0", weight="bold")
    s.text(1110, 375, "• Fem #11 -> TCU #6  (IT-CAN H, opt.)", size=10.3, fill="#bae6fd")
    s.text(1110, 392, "• Fem #12 -> TCU #7  (IT-CAN L, opt.)", size=10.3, fill="#ddd6fe")
    s.text(1110, 409, "• Fem #13 -> TCU #26 (M CAN-H, opt.)", size=10.3, fill="#7dd3fc")
    s.text(1110, 426, "• Fem #14 -> TCU #27 (M CAN-L, opt.)", size=10.3, fill="#bef264")

    for idx, (pin_n, col, dsh) in enumerate([
        (9, COL_12V, ""),
        (10, COL_GND, ""),
        (11, COL_CANH_OEM, "5,3"),
        (12, COL_CANL_OEM, "5,3"),
        (13, COL_MCANH_DEV, "5,3"),
        (14, COL_MCANL_OEM, "5,3"),
    ]):
        px, py = fp[pin_n]
        wy = 418 - idx * 14
        s.wire([(px, py + 13), (px, wy), (1096, wy)], col, width=2.6, dash=dsh)

    # Warning banner along full width at bottom
    s.rect(16, 470, 1388, 74, fill="#fffbeb", stroke="#f59e0b", stroke_width=1.5, rx=8)
    s.text(34, 493, "Important — Avoid Active CAN Transmit Collisions When Looping CAN Back to the Original TCU:", size=12.5, fill="#92400e", weight="bold")
    s.text(34, 513, "• If IT-CAN (#6/#7 via Pins 11–12) is looped back to an active original TCU, use simppeliTCU for PASSIVE MONITORING ONLY (do not trigger climate, charge,", size=11.2, fill="#78350f")
    s.text(34, 531, "  or lock commands, as both TCUs transmitting 0x56E frames will collide). You can also loop one bus while isolating the other simply by adding/removing jumpers!", size=11.2, fill="#78350f")

    s.save(DOCS_DIR / "wiring_can_loopback.svg")


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

    s.rect(592, 152, 62, 214, fill="#e2e8f0", stroke="#475569", stroke_width=1.6, rx=6)
    s.text(623, 248, "Car", size=11.5, fill="#0f172a", weight="bold", anchor="middle")
    s.text(623, 264, "Loom", size=11.5, fill="#0f172a", weight="bold", anchor="middle")

    s.wire([(592, 166), (576, 166), (576, 140), (350, 140), (350, 192), (208, 192)], COL_CANH_OEM, width=3.0)
    s.wire([(592, 182), (584, 182), (584, 132), (336, 132), (336, 210), (208, 210)], COL_CANL_OEM, width=3.0)
    s.badge(276, 170, "Unpinned #6 & #7", bg="#e0f2fe", fg="#075985", border="#7dd3fc", w=120, h=20, size=10)

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

    s.rect(36, 398, 618, 70, fill="#fffbeb", stroke="#f59e0b", stroke_width=1.3, rx=6)
    s.text(50, 418, "Splice Tip:", size=11.5, fill="#92400e", weight="bold")
    s.text(118, 418, "Tap #1 (+12V) & #28 (GND) on the harness behind the 40-pin plug (Posi-Tap / Wago / solder).", size=11, fill="#78350f")
    s.text(50, 437, "Do NOT use insulation-displacement clips ('rosvoliitin' / Scotchlocks) on CAN data wires;", size=11, fill="#92400e", weight="bold")
    s.text(50, 454, "for optional AV-CAN, also unpin #26 (SB, M CAN-H) & #27 (LG, M CAN-L) to LilyGo Port A.", size=11, fill="#78350f")

    # =========================================================================
    # RIGHT PANEL: Option C — Original TCU Completely Disconnected
    # =========================================================================
    s.rect(692, 78, 652, 406, fill="#f8fafc", stroke="#cbd5e1", stroke_width=1.5, rx=10)
    s.text(710, 106, "Option C: Original TCU Completely Unplugged (4 or 6 Wires)", size=15, fill="#0f172a", weight="bold")
    s.text(710, 124, "Hands-free microphone DISABLED • Simplest wiring (no power loopback needed)", size=11.5, fill="#b45309", weight="bold")

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

    s.rect(1136, 152, 190, 214, fill="#334155", stroke="#475569", stroke_width=2.0, rx=6, dash="6,4")
    s.text(1231, 175, "40-Pin Harness Plug", size=13, fill="#ffffff", weight="bold", anchor="middle")
    s.text(1231, 192, "(Left UNPLUGGED from TCU)", size=10.5, fill="#fcd34d", weight="bold", anchor="middle")

    s.text(1150, 220, "Pin #6  (SB IT-CAN H)", size=10.5, fill="#bae6fd", weight="bold")
    s.text(1150, 245, "Pin #7  (V IT-CAN L)", size=10.5, fill="#ddd6fe", weight="bold")
    s.text(1150, 300, "Pin #28 (B GND)", size=10.5, fill="#f8fafc", weight="bold")
    s.text(1150, 346, "Pin #1  (W +12V)", size=10.5, fill="#fecaca", weight="bold")

    s.wire([(897, 192), (1136, 216)], COL_CANH_OEM, width=3.2)
    s.wire([(897, 210), (1136, 241)], COL_CANL_OEM, width=3.2)

    s.wire([(1136, 296), (948, 296), (948, 326), (897, 326)], COL_GND, width=3.0)
    s.wire([(948, 296), (948, 228), (897, 228)], COL_GND, width=3.0)
    s.circle(948, 296, 4.5, fill=COL_GND)

    fuse_cx, fuse_cy = 1020, 346
    s.wire([(1136, 346), (fuse_cx + 29, fuse_cy)], COL_12V, width=3.2)
    s.wire([(fuse_cx - 29, fuse_cy), (897, 346)], COL_12V, width=3.2)
    s.fuse(fuse_cx, fuse_cy, "2A Fuse")

    s.rect(712, 398, 614, 70, fill="#f1f5f9", stroke="#94a3b8", stroke_width=1.3, rx=6)
    s.text(726, 419, "When to use Option C:", size=11.5, fill="#0f172a", weight="bold")
    s.text(726, 437, "• Original TCU is removed or left unplugged (+ optional #26/#27 to LilyGo Port A for AV-CAN).", size=11, fill="#334155")
    s.text(726, 454, "• No power loopback needed, but the cabin hands-free phone microphone will not function.", size=11, fill="#334155")

    s.save(DOCS_DIR / "wiring_alternatives.svg")


if __name__ == "__main__":
    generate_lilygo_adapter()
    generate_waveshare_adapter()
    generate_can_loopback()
    generate_alternatives()
    print("Generated 4 SVG wiring diagrams in", DOCS_DIR)
