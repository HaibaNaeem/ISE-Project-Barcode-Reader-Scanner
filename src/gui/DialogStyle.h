#pragma once
// OWNER: Member 4.
// Central styling constants and stylesheet for Member 4 dialogs (PaymentDialog, BillDialog).
// Adheres to Open/Closed Principle: appearance changes are isolated here without touching dialog logic.
#include <QString>

namespace DialogStyle {

// Light, high-contrast cashier theme palette
inline const char* const kBg          = "#F5F7FA"; // Clean light grey dialog background
inline const char* const kCardBg      = "#FFFFFF"; // Pure white card/panel background
inline const char* const kBorder      = "#D5DBE3"; // Crisp border
inline const char* const kText        = "#1F2937"; // Dark grey text (high readability)
inline const char* const kMutedText   = "#6B7280"; // Secondary muted labels
inline const char* const kPrimary     = "#0F766E"; // Deep teal for table headers & titles
inline const char* const kSuccess     = "#16A34A"; // Vibrant green for Confirm and Paid status
inline const char* const kDanger      = "#DC2626"; // Red for Cancel and validation errors
inline const char* const kWarning     = "#D97706"; // Amber accent
inline const char* const kTableAltRow = "#F0F5F4"; // Soft teal-tinted alternating table rows

inline QString dialogStyleSheet() {
    return QString(
        "QDialog {"
        "    background-color: %1;"
        "    color: %2;"
        "    font-family: 'Segoe UI', -apple-system, sans-serif;"
        "    font-size: 11pt;"
        "}"
        "QGroupBox, QFrame.card {"
        "    background-color: %3;"
        "    border: 1px solid %4;"
        "    border-radius: 6px;"
        "    margin-top: 8px;"
        "    padding: 12px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top left;"
        "    padding: 0 4px;"
        "    color: %5;"
        "    font-weight: bold;"
        "}"
        "QLabel {"
        "    color: %2;"
        "    font-size: 11pt;"
        "}"
        "QLineEdit {"
        "    background-color: #FFFFFF;"
        "    border: 1px solid %4;"
        "    border-radius: 4px;"
        "    padding: 8px;"
        "    font-size: 11pt;"
        "    color: %2;"
        "}"
        "QLineEdit:focus {"
        "    border: 1px solid %5;"
        "}"
        "QLineEdit:disabled {"
        "    background-color: #E5E7EB;"
        "    color: %6;"
        "}"
        "QRadioButton {"
        "    font-size: 11pt;"
        "    color: %2;"
        "    spacing: 8px;"
        "}"
        "QTableWidget {"
        "    background-color: #FFFFFF;"
        "    alternate-background-color: %7;"
        "    border: 1px solid %4;"
        "    border-radius: 4px;"
        "    gridline-color: %4;"
        "    font-size: 11pt;"
        "    selection-background-color: %5;"
        "    selection-color: #FFFFFF;"
        "}"
        "QHeaderView::section {"
        "    background-color: %5;"
        "    color: #FFFFFF;"
        "    padding: 8px;"
        "    font-weight: bold;"
        "    font-size: 11pt;"
        "    border: none;"
        "}"
        "QPushButton {"
        "    min-height: 40px;"
        "    border-radius: 6px;"
        "    padding: 8px 18px;"
        "    font-size: 11pt;"
        "    font-weight: bold;"
        "}"
        "QPushButton#confirmBtn {"
        "    background-color: %8;"
        "    color: #FFFFFF;"
        "    border: none;"
        "}"
        "QPushButton#confirmBtn:hover {"
        "    background-color: #15803D;"
        "}"
        "QPushButton#confirmBtn:disabled {"
        "    background-color: #9CA3AF;"
        "    color: #E5E7EB;"
        "}"
        "QPushButton#cancelBtn {"
        "    background-color: %9;"
        "    color: #FFFFFF;"
        "    border: none;"
        "}"
        "QPushButton#cancelBtn:hover {"
        "    background-color: #B91C1C;"
        "}"
        "QPushButton#closeBtn {"
        "    background-color: %5;"
        "    color: #FFFFFF;"
        "    border: none;"
        "}"
        "QPushButton#closeBtn:hover {"
        "    background-color: #115E59;"
        "}"
    ).arg(kBg, kText, kCardBg, kBorder, kPrimary, kMutedText, kTableAltRow, kSuccess, kDanger);
}

} // namespace DialogStyle

