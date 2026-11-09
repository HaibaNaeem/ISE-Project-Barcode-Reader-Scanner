# Member 1 — GUI & Integration Test Report

## 1. Test Matrix & Results

### Category A: Barcode Generation (`GeneratePage`)

| Test ID | Input | Type | Expected Behavior | Actual Behavior | Result |
|---|---|---|---|---|---|
| **GEN-01** | `123456` | Valid 6-digit | Barcode image generated, preview rendered, Save enabled | BMP created, rendered on GUI, Save enabled | **PASS** |
| **GEN-02** | `001234` | Leading zero | Handled gracefully: rejected by generator rules (first digit non-zero) | Error banner: "Enter exactly six digits (0-9). The first digit must not be 0." | **PASS** |
| **GEN-03** | `1234` | Shorter (< 6 digits) | Rejected at UI validation | Error banner: "Code must be exactly 6 digits. You entered 4 digit(s)." | **PASS** |
| **GEN-04** | `1234567` | Longer (> 6 digits) | Rejected: UI field caps at 6 chars | Cannot type 7th character (max length 6) | **PASS** |
| **GEN-05** | `12A456` / Symbols | Letters & Special Chars | Rejected by regex validator `^[0-9]{0,6}$` | Keystroke ignored by QRegularExpressionValidator | **PASS** |
| **GEN-06** | `999999` | Boundary value | Barcode generated and rendered | BMP created, rendered on GUI | **PASS** |
| **GEN-07** | `Clear` button | UI reset | Input cleared, preview reset to placeholder, Save disabled | Form cleanly reset | **PASS** |
| **GEN-08** | `Save Barcode` | File export | Opens `QFileDialog`, saves PNG/BMP/JPG | User file saved to chosen destination | **PASS** |

---

### Category B: Image File Scanning (`ScanPage`)

| Test ID | Input File | Expected Behavior | Actual Behavior | Result |
|---|---|---|---|---|
| **SCAN-01** | `output/generated_barcodes/123456.bmp` | Barcode decoded successfully, 6-digit result displayed | Result card displays `123456`, green banner shown | **PASS** |
| **SCAN-02** | `output/generated_barcodes/100230.bmp` | Decoded successfully, 6-digit result displayed | Result card displays `100230`, green banner shown | **PASS** |
| **SCAN-03** | Plain photo (no barcode) | Handled gracefully, no crash, clear error | Error banner: "No barcode detected." | **PASS** |
| **SCAN-04** | Non-image / corrupt file | Handled gracefully, clear error | Error banner: "Failed to load image. The file may be corrupt..." | **PASS** |
| **SCAN-05** | Non-existent file path | Handled gracefully | Error banner: "File does not exist" | **PASS** |
| **SCAN-06** | Copy Code button | Decoded code copied to system clipboard | Clipboard updated with 6-digit string | **PASS** |

---

### Category C: Live Webcam Scanning (`ScanPage`)

| Test ID | Scenario | Expected Behavior | Actual Behavior | Result |
|---|---|---|---|---|
| **CAM-01** | Camera Available | Hardware camera initialized, smooth preview rendered at ~33 FPS | DirectShow/VideoCapture opened, frames displayed | **PASS** |
| **CAM-02** | Camera Unavailable / Busy | Error banner displayed, app stays responsive, no crash | Error banner: "Camera unavailable or busy..." | **PASS** |
| **CAM-03** | Start / Stop functionality | "Start" opens camera, "Stop" releases hardware | Video stream stops immediately, resources freed | **PASS** |
| **CAM-04** | Detection & Debounce | Barcode detected, result displayed, decoding paused | Preview continues streaming; "Scan Again" button shown | **PASS** |
| **CAM-05** | Scan Again button | Resumes decoding for subsequent barcodes | Resume scanning active without restarting camera | **PASS** |
| **CAM-06** | Navigation / Window Close | Switching page or closing window releases camera | `stopCameraIfRunning()` safely called | **PASS** |
