# Generator Test Results (Member 2)

| Test ID | Input   | Expected        | Actual          | Scanner app check              | Result |
|---------|---------|-----------------|-----------------|--------------------------------|--------|
| G-01    | 123456  | PNG created     | PNG created     | CODE-128, decoded 123456       | PASS   |
| G-02    | 000123  | PNG, zeros safe | PNG, zeros safe | CODE-128, decoded 000123       | PASS   |
| G-03a   | 12345   | Rejected        | Rejected        | -                              | PASS   |
| G-03b   | 1234567 | Rejected        | Rejected        | -                              | PASS   |
| G-04    | 12A456  | Rejected        | Rejected        | -                              | PASS   |
| G-05    | (empty) | Rejected        | Rejected        | -                              | PASS   |
| G-06    | 12 456  | Rejected        | Rejected        | -                              | PASS   |

Environment: Windows 64-bit, Visual Studio 2026 (MSVC), Zint 2.16.0 (with PNG), C++17

Notes:
- Format: Code 128.
- Decoded using a barcode scanner app on a phone (bars only).
- Zint must be installed with the png feature:
  vcpkg install "zint[png]:x64-windows" --recurse