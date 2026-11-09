# Member 1 — Complete Viva Preparation Guide

This guide is prepared specifically for **Member 1 (Desktop GUI - Qt 6)**. It explains everything in **very simple English** and **easy Roman Urdu** so you can confidently explain your project to your teacher / examiner and get full marks in your viva!

---

## 1. What was your role in this project?
- **English:** "I was assigned Member 1. My responsibility was the Desktop GUI developed using C++ and Qt 6. I built the main application window with sidebar navigation, the Generate Barcode page, and the Scan Barcode page. I also connected our GUI with Member 2's Barcode Generator, Member 3's Image Decoder, and Member 4's Webcam Scanner, ensuring that the application remains smooth, responsive, and never freezes."
- **Roman Urdu:** "Mera role Member 1 ka tha — Desktop GUI (Qt 6). Maine complete user interface design kiya hai: MainWindow, sidebar navigation, Generate page aur Scan page. Saath hi maine baqi team members ke services (Member 2 ka generator, Member 3 ka decoder, aur Member 4 ka camera scanner) ko GUI ke sath connect kiya hai taake application user-friendly rahe aur hang na ho."

---

## 2. What does each file do? (Files ki wazahat)

### 1. `src/gui/MainWindow.h` & `src/gui/MainWindow.cpp`
- **English:** This is the top-level window of our app. It has a modern dark sidebar navigation panel with two buttons: "Generate Barcode" and "Scan Barcode". It uses a `QStackedWidget` to seamlessly switch between the two pages. When switching pages or closing the window, it safely shuts down the webcam so the camera is never locked.
- **Roman Urdu:** Yeh hamari main desktop window hai. Isme left side par dark navigation bar hai jisme Generate aur Scan ke buttons hain. Yeh `QStackedWidget` use karke pages ko switch karta hai. Window band hote waqt camera ko safely release karta hai taake webcam busy na rahe.

### 2. `src/gui/GeneratePage.h` & `src/gui/GeneratePage.cpp`
- **English:** This is the Generate Barcode page. It contains a 6-digit input box (`QLineEdit`), "Generate Barcode" button, "Clear" button, a barcode preview area, a "Save Barcode Image" button, and status banners. It uses a `QRegularExpressionValidator` so only digits 0-9 can be entered. When clicked, it calls Member 2's `generateBarcode()`, displays the image, and lets the user save it as PNG/BMP/JPG via a file dialog.
- **Roman Urdu:** Yeh Generate page hai. Isme 6-digit ka input box hai jisme regex validator laga hai taake sirf numbers enter ho sakein. "Generate Barcode" click karne par Member 2 ka function call hota hai, barcode image preview mein show hoti hai, aur "Save" button se user image save kar sakta hai.

### 3. `src/gui/ScanPage.h` & `src/gui/ScanPage.cpp`
- **English:** This is the Scan Barcode page. It has two options: upload an image file or scan using a live webcam. It calls Member 3's `ImageDecoder` for files and Member 4's `CameraScanner` for live camera frames. It renders live frames at ~33 FPS using a `QTimer` without freezing the main thread. It shows the decoded 6-digit code in a big bold card with a "Copy to Clipboard" button.
- **Roman Urdu:** Yeh Scan page hai. Isme user image upload bhi kar sakta hai aur webcam se live scan bhi kar sakta hai. Image decoding ke liye Member 3 ka service aur camera ke liye Member 4 ka service use hota hai. Live video ke liye `QTimer` use kiya hai taake application freeze na ho. Jab code scan hota hai tou bara result card show hota hai aur "Copy" button se user code copy kar sakta hai.

### 4. `src/gui/Theme.h`
- **English:** Contains the professional stylesheet (QSS) and modern color palette (Slate, Blue, Emerald, Red). It gives the application rounded corners, clean fonts, consistent padding, and styled success/error notification banners.
- **Roman Urdu:** Yeh styling file hai jisme CSS/QSS code hai. Isse buttons, input fields, cards, aur alert messages professional aur clean dikhte hain.

### 5. `src/services/ImageDecoder.h` & `src/services/ImageDecoder.cpp`
- **English:** Member 3's integration interface. It loads image files with OpenCV (`cv::imread`) and uses ZXing to extract the 6-digit barcode value.
- **Roman Urdu:** Member 3 ka adapter/service hai jo OpenCV se image read karke ZXing se barcode decode karta hai.

---

## 3. Top Viva Questions & Answers (Examiner ke sawalaat)

### Q1: Why did you use Qt 6 Widgets instead of Qt Quick/QML?
- **English:** "Qt 6 Widgets is the native C++ desktop UI framework. It integrates directly with C++ classes, standard pointers, OpenCV Mat objects, and signals/slots without needing a separate JavaScript runtime engine."
- **Roman Urdu:** "Qt 6 Widgets direct C++ framework hai jo desktop ke liye best aur fast hai. Isme OpenCV aur C++ classes direct bind ho jati hain baghair kisi extra engine ke."

### Q2: How did you prevent the GUI from freezing during camera scanning?
- **English:** "We avoided blocking loops like `while(true)`. Instead, we used a `QTimer` configured at a 30 ms interval. On each timer tick, `m_cameraScanner.scanNextFrame()` fetches and renders only one frame. Since each tick takes less than 5 milliseconds, the Qt event loop remains completely responsive."
- **Roman Urdu:** "Maine `while(true)` ka loop GUI thread par nahi chalaya kyunke us se application hang ho jati hai. Maine `QTimer` use kiya jo har 30 millisecond baad sirf aik frame fetch karke screen par dikhata hai. Is wajah se application bilkul smooth rehti hai."

### Q3: How do you validate the 6-digit code?
- **English:** "At the GUI level, we use `QRegularExpressionValidator(QRegularExpression(\"^[0-9]{0,6}$\"))` on the `QLineEdit`, which rejects alphabets and special characters immediately. When the user clicks Generate, we also verify that `text.length() == 6`. Finally, Member 2's `isValidSixDigitCode()` validates the business logic."
- **Roman Urdu:** "Input field par `QRegularExpressionValidator` lagaya hai jo alphabets aur special symbols ko type hi nahi hone deta. Generate click karne par check karte hain ke length poori 6 digits ho. Phir Member 2 ka validator function call hota hai."

### Q4: How do you prevent repeated scans from flooding the GUI?
- **English:** "In `CameraScanner`, once a barcode is successfully decoded, scanning automatically pauses (`paused_ = true`). The live video feed continues displaying, but decoding stops. A 'Scan Again' button appears in the GUI, which calls `resumeScanning()` when the user wants to scan another barcode."
- **Roman Urdu:** "Jab pehli martaba barcode detect hota hai, tou scanning pause ho jati hai lekin camera chalta rehta hai. Is se GUI bar bar beep ya alert nahi karta. User jab 'Scan Again' par click karta hai tou dubara decoding shuru hoti hai."

### Q5: How do OpenCV Mat frames get displayed in Qt?
- **English:** "OpenCV stores images in BGR format. We use `cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB)` to convert BGR to RGB. Then we wrap the buffer in `QImage` with `QImage::Format_RGB888`, call `.copy()` to ensure safe memory ownership, convert to `QPixmap`, and display it on `QLabel`."
- **Roman Urdu:** "OpenCV frames BGR format mein hote hain jabke Qt RGB expect karta hai. Hum `cv::cvtColor` se RGB mein convert karte hain, `QImage` banate hain aur `QPixmap` ke zariye `QLabel` par show karte hain."

### Q6: What happens if the webcam is not connected or in use by another app?
- **English:** "`CameraScanner::start(0)` checks `capture_.isOpened()`. If false, it returns `success = false` with a clear message: 'Camera unavailable or busy. Close other apps using the camera and try again.' The GUI catches this and displays a red error banner to the user."
- **Roman Urdu:** "Hamara service check karta hai ke camera open hua ya nahi. Agar camera kisi aur app mein use ho raha ho ya attach na ho, tou GUI red error message display kar deta hai aur crash nahi hota."

### Q7: How does navigation between pages work?
- **English:** "We used `QStackedWidget`. The Generate page is at index 0 and the Scan page is at index 1. Clicking the sidebar buttons changes `currentIndex()`. When switching away from the Scan page, we automatically call `stopCameraIfRunning()` to release camera resources."
- **Roman Urdu:** "`QStackedWidget` use kiya hai jisme Generate page index 0 par hai aur Scan page index 1 par. Button click par index change hota hai. Aur jab user Scan page se hat-ta hai tou camera foran stop ho jata hai taake hardware release ho jaye."
