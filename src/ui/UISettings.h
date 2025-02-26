/**
* @file UISettings.h
 * @brief Defines global UI settings for the GranularPlunderphonics plugin
 */

#pragma once

namespace GranularPlunderphonics {

    /**
     * @struct UISettings
     * @brief Stores global UI settings for the plugin
     */
    struct UISettings {
        bool darkMode = true;          // Dark mode state
        float scaleFactor = 1.0f;      // UI scaling factor (0.5 to 2.0)
        int fontSizePt = 14;           // Base font size in points
        bool showTooltips = true;      // Whether to show tooltips
        bool compactMode = false;      // Compact UI mode for smaller screens
        bool highContrastMode = false; // High contrast mode for accessibility
    };

} // namespace GranularPlunderphonics