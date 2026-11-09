#pragma once
#include <QString>

namespace Theme {

    // Modern color constants
    inline const QString PrimaryColor        = "#2563EB"; // Vibrant Blue
    inline const QString PrimaryHover        = "#1D4ED8"; // Darker Blue
    inline const QString PrimaryPressed      = "#1E40AF"; 
    inline const QString BackgroundColor     = "#F8FAFC"; // Slate 50
    inline const QString CardBackground      = "#FFFFFF"; // Pure White
    inline const QString CardBorder          = "#E2E8F0"; // Slate 200
    inline const QString TextPrimary         = "#0F172A"; // Slate 900
    inline const QString TextSecondary       = "#64748B"; // Slate 500
    inline const QString TextMuted           = "#94A3B8"; // Slate 400

    inline const QString SuccessBg           = "#ECFDF5"; // Emerald 50
    inline const QString SuccessBorder       = "#A7F3D0"; // Emerald 200
    inline const QString SuccessText         = "#065F46"; // Emerald 800

    inline const QString ErrorBg             = "#FEF2F2"; // Red 50
    inline const QString ErrorBorder         = "#FECACA"; // Red 200
    inline const QString ErrorText           = "#991B1B"; // Red 800

    inline const QString InfoBg              = "#EFF6FF"; // Blue 50
    inline const QString InfoBorder          = "#BFDBFE"; // Blue 200
    inline const QString InfoText            = "#1E40AF"; // Blue 800

    // Full application stylesheet
    inline QString getApplicationStyle() {
        return QString(R"(
            QWidget {
                font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, 'Inter', sans-serif;
                font-size: 13px;
                color: %1;
                background-color: %2;
            }

            QMainWindow {
                background-color: %2;
            }

            /* Navigation Sidebar */
            #navBar {
                background-color: #0F172A;
                border-right: 1px solid #1E293B;
                min-width: 220px;
                max-width: 220px;
            }

            #appTitle {
                font-size: 16px;
                font-weight: 700;
                color: #FFFFFF;
                padding: 12px 0 2px 0;
            }

            #appSubtitle {
                font-size: 11px;
                font-weight: 500;
                color: #94A3B8;
                padding-bottom: 12px;
            }

            QPushButton.navButton {
                text-align: left;
                padding: 12px 18px;
                font-size: 13px;
                font-weight: 600;
                color: #94A3B8;
                background-color: transparent;
                border: none;
                border-radius: 8px;
                margin: 4px 10px;
            }

            QPushButton.navButton:hover {
                background-color: #1E293B;
                color: #F8FAFC;
            }

            QPushButton.navButton:checked {
                background-color: #2563EB;
                color: #FFFFFF;
            }

            /* Content Cards */
            .card {
                background-color: #FFFFFF;
                border: 1px solid #E2E8F0;
                border-radius: 12px;
                padding: 18px;
            }

            /* Headings */
            QLabel.pageHeading {
                font-size: 22px;
                font-weight: 700;
                color: #0F172A;
            }

            QLabel.pageSubtitle {
                font-size: 13px;
                color: #64748B;
            }

            QLabel.sectionTitle {
                font-size: 14px;
                font-weight: 600;
                color: #1E293B;
                margin-top: 6px;
                margin-bottom: 4px;
            }

            /* Line Edit */
            QLineEdit {
                background-color: #FFFFFF;
                border: 1.5px solid #CBD5E1;
                border-radius: 8px;
                padding: 10px 14px;
                font-size: 15px;
                font-weight: 600;
                color: #0F172A;
                selection-background-color: #2563EB;
            }

            QLineEdit:focus {
                border: 1.5px solid #2563EB;
                background-color: #FFFFFF;
            }

            QLineEdit:disabled {
                background-color: #F1F5F9;
                color: #94A3B8;
                border-color: #E2E8F0;
            }

            /* Action Buttons */
            QPushButton.primaryBtn {
                background-color: #2563EB;
                color: #FFFFFF;
                font-size: 13px;
                font-weight: 600;
                border: none;
                border-radius: 8px;
                padding: 10px 18px;
                min-height: 20px;
            }

            QPushButton.primaryBtn:hover {
                background-color: #1D4ED8;
            }

            QPushButton.primaryBtn:pressed {
                background-color: #1E40AF;
            }

            QPushButton.primaryBtn:disabled {
                background-color: #94A3B8;
                color: #E2E8F0;
            }

            QPushButton.secondaryBtn {
                background-color: #FFFFFF;
                color: #334155;
                font-size: 13px;
                font-weight: 600;
                border: 1.5px solid #CBD5E1;
                border-radius: 8px;
                padding: 9px 16px;
                min-height: 20px;
            }

            QPushButton.secondaryBtn:hover {
                background-color: #F8FAFC;
                border-color: #94A3B8;
                color: #0F172A;
            }

            QPushButton.secondaryBtn:pressed {
                background-color: #F1F5F9;
            }

            QPushButton.secondaryBtn:disabled {
                background-color: #F8FAFC;
                color: #CBD5E1;
                border-color: #E2E8F0;
            }

            QPushButton.dangerBtn {
                background-color: #DC2626;
                color: #FFFFFF;
                font-size: 13px;
                font-weight: 600;
                border: none;
                border-radius: 8px;
                padding: 10px 18px;
            }

            QPushButton.dangerBtn:hover {
                background-color: #B91C1C;
            }

            QPushButton.dangerBtn:disabled {
                background-color: #FCA5A5;
            }

            QPushButton.successBtn {
                background-color: #059669;
                color: #FFFFFF;
                font-size: 13px;
                font-weight: 600;
                border: none;
                border-radius: 8px;
                padding: 10px 18px;
            }

            QPushButton.successBtn:hover {
                background-color: #047857;
            }

            /* Preview Boxes */
            #previewBox {
                background-color: #F8FAFC;
                border: 2px dashed #CBD5E1;
                border-radius: 12px;
                color: #64748B;
                font-weight: 500;
            }

            /* Status Banner */
            #statusBanner {
                border-radius: 8px;
                padding: 12px 16px;
                font-weight: 500;
                font-size: 13px;
            }

            /* Big Result Card */
            #resultCodeLabel {
                font-size: 32px;
                font-weight: 800;
                letter-spacing: 4px;
                color: #0F172A;
                padding: 4px 0;
            }
        )")
        .arg(TextPrimary, BackgroundColor);
    }

    inline QString formatStatusHtml(const QString& text, const QString& type) {
        QString bg, border, color, icon;
        if (type == "success") {
            bg = SuccessBg; border = SuccessBorder; color = SuccessText; icon = "&#x2714; ";
        } else if (type == "error") {
            bg = ErrorBg; border = ErrorBorder; color = ErrorText; icon = "&#x26A0; ";
        } else {
            bg = InfoBg; border = InfoBorder; color = InfoText; icon = "&#x2139; ";
        }
        return QString(
            "<div style='background-color:%1; border:1px solid %2; color:%3; "
            "padding:10px 14px; border-radius:8px; font-weight:600;'>"
            "%4 %5</div>"
        ).arg(bg, border, color, icon, text.toHtmlEscaped());
    }

} // namespace Theme
