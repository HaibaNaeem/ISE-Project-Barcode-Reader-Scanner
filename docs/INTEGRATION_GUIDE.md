# Member 1 — Desktop GUI Integration Guide & Architecture

## 1. Overview & Architecture

The application follows a modular, decoupled architecture using **Qt 6 (Widgets)** for the presentation layer and C++17 services for core logic:

```
+--------------------------------------------------------------------------+
|                                MainWindow                                |
|  - Branded Sidebar Navigation (Generate Barcode / Scan Barcode)          |
|  - Central QStackedWidget container                                      |
+------------------------------------+-------------------------------------+
                                     |
         +---------------------------+---------------------------+
         |                                                       |
         v                                                       v
+----------------------------------+   +-----------------------------------+
|          GeneratePage            |   |             ScanPage              |
|  - 6-digit QLineEdit with regex  |   |  - Upload Image File button       |
|  - Generate Barcode button       |   |  - Live Webcam Start/Stop buttons |
|  - QPixmap image preview         |   |  - Real-time video preview label  |
|  - Save Barcode dialog           |   |  - 6-digit decoded result card    |
|  - Status error/success banners  |   |  - "Scan Again" debounce button   |
+----------------------------------+   +-----------------------------------+
                 |                                      |            |
                 v                                      v            v
+----------------------------------+   +----------------+ +----------------+
|       Member 2: Generator        |   |   Member 3:    | |   Member 4:    |
|   generateBarcode(code, path)    |   |  ImageDecoder  | | CameraScanner  |
+----------------------------------+   +----------------+ +----------------+
```

---

## 2. Module Interfaces & Communication

### Member 2 — Barcode Generator (`src/services/BarcodeGenerator.h`)
- **Function Called:**
  ```cpp
  OperationResult generateBarcode(const std::string& code, const std::string& outputBmpPath);
  ```
- **How GUI Integrates:**
  - `GeneratePage::onGenerateClicked()` validates that the input has exactly 6 digits using `QRegularExpression("^[0-9]{6}$")`.
  - It creates output directory `output/generated_barcodes/`.
  - It invokes `generateBarcode(code.toStdString(), outPath.toStdString())`.
  - If `result.success == true`, GUI loads the image into `QPixmap`, scales it smoothly preserving aspect ratio, and enables the "Save Barcode Image" button.
  - If `result.success == false`, GUI presents `result.message` in the red error banner.

### Member 3 — Uploaded Image Decoder (`src/services/ImageDecoder.h`)
- **Function Called:**
  ```cpp
  OperationResult ImageDecoder::decodeFile(const std::string& filePath);
  ```
- **How GUI Integrates:**
  - `ScanPage::onUploadImageClicked()` triggers `QFileDialog::getOpenFileName(...)` for PNG, JPG, JPEG, and BMP formats.
  - The image is loaded into `QPixmap` and previewed.
  - `ImageDecoder::decodeFile(filePath)` decodes the barcode using OpenCV and ZXing-C++.
  - If `result.success == true`, the 6-digit code is prominently rendered in the result card, and the user can click "Copy Code to Clipboard".
  - If no barcode is found or file is corrupt, a clear error banner explains the issue.

### Member 4 — Camera Scanner (`src/services/CameraScanner.h`)
- **Class & Methods Used:**
  ```cpp
  CameraScanner scanner;
  scanner.start(0);                   // Opens webcam index 0
  scanner.stop();                     // Safely releases hardware
  scanner.scanNextFrame(cv::Mat& f);  // Grabs frame and decodes every Nth frame
  scanner.resumeScanning();           // Resumes after a successful detection
  ```
- **How GUI Integrates (Asynchronous & Non-Blocking):**
  - Live scanning runs on a 30 ms `QTimer` (`~33 FPS`).
  - The UI thread is **never blocked** because each timer tick processes only a single frame.
  - OpenCV `cv::Mat` frames are converted to `QImage` (RGB888) and displayed smoothly on `ScanPage`.
  - When a barcode is detected, decoding automatically pauses so the GUI is not flooded with duplicate alerts, while the live video stream continues smoothly.
  - User can click "Scan Again" to resume scanning, or "Stop Camera" to release the camera.
  - When navigating away from `ScanPage` or closing the application, `stopCameraIfRunning()` ensures the webcam device is released immediately.

---

## 3. Discrepancies & Resolutions Documented

1. **Barcode Symbology (Code 128 vs Code 39):**
   - Project prompt initially specified Code 128.
   - Member 2 committed a custom Code 39 generator in `aa3edcd`.
   - `CameraScanner.cpp` was originally configured for `Code128` only.
   - **Fix:** In `CameraScanner.cpp`, enabled `ZXing::BarcodeFormat::Code128 | ZXing::BarcodeFormat::Code39` so both symbologies decode transparently.

2. **Member 3 Service Missing:**
   - Member 3 (`dev-member3`) had not committed their implementation.
   - **Resolution:** Provided `src/services/ImageDecoder.h` and `src/services/ImageDecoder.cpp` as a clean adapter delegating to the shared ZXing engine, preventing duplicate logic and allowing Member 3 to seamlessly connect or expand their module.

3. **Leading Zero Handling:**
   - Member 2 added a restriction in `CodeValidator.cpp` requiring the first digit to be 1-9.
   - The GUI input accepts any 6 digits (0-9) so users can type freely. If a code starting with `0` is passed to Member 2's generator, the service returns `"Enter exactly six digits (0-9). The first digit must not be 0."`, which the GUI displays cleanly as a validation error.
