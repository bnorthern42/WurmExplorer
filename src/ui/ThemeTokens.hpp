#pragma once

#include <QString>

namespace treasure {
namespace ui {
namespace theme {

    // Background & Surfaces
    constexpr const char* BG_DARK           = "#18181b"; // Main window / canvas backdrop
    constexpr const char* BG_SIDEBAR        = "#121214"; // Left Navigation sidebar
    constexpr const char* SURFACE_DARK      = "#202124"; // Top control bar, panel backgrounds
    constexpr const char* SURFACE_CARD      = "#27282d"; // Group boxes, cards, inputs, table cells
    constexpr const char* SURFACE_HOVER     = "#32343b"; // Button hover, item hover
    constexpr const char* SURFACE_ACTIVE    = "#3a3c46"; // Pressed buttons, active rows

    // Borders & Dividers
    constexpr const char* BORDER_MUTED      = "#383a42"; // Subtle slate borders
    constexpr const char* BORDER_FOCUS      = "#04b97f"; // Input focus outline / active border

    // Accents (Emerald & Mint - Zero Blue)
    constexpr const char* ACCENT_EMERALD    = "#04b97f"; // Primary accent, active tool, CTAs
    constexpr const char* ACCENT_MINT       = "#37efba"; // Vibrant hover glow, title text
    constexpr const char* ACCENT_TINT       = "#0d3829"; // Checked button / active item background
    constexpr const char* ACCENT_PRESSED    = "#02875b"; // Darker green pressed state

    // Typography & Content
    constexpr const char* TEXT_PRIMARY      = "#f4f4f5"; // High contrast headings & active labels
    constexpr const char* TEXT_SECONDARY    = "#a1a1aa"; // Muted descriptions, captions, placeholders
    constexpr const char* TEXT_ON_ACCENT    = "#121214"; // Text on vibrant green buttons
    constexpr int FONT_SIZE_GRID_PX         = 10;        // Readable fixed pixel size for 2D elevation grid
    constexpr int CELL_MIN_WIDTH_PX         = 48;        // Minimum column width to house up to 5 digits comfortably
    constexpr int CELL_MIN_HEIGHT_PX        = 26;        // Minimum row height for elevation cells

    // Status Colors
    constexpr const char* STATUS_DANGER     = "#ef4444"; // Delete / Error
    constexpr const char* STATUS_WARNING    = "#f59e0b"; // Warning / Caution
    constexpr const char* STATUS_SUCCESS    = "#10b981"; // Success / Ready

} // namespace theme
} // namespace ui
} // namespace treasure
