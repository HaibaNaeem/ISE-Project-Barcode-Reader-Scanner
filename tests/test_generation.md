\# Generator Test Results (Member 2)



Symbology: Code 39 (own encoder, no barcode library). Output: BMP image.



| Test ID | Input   | Expected            | Actual      | Decode check            | Result |

|---------|---------|---------------------|-------------|-------------------------|--------|

| G-01    | 123456  | BMP created         | BMP created | CODE\_39, decoded 123456 | PASS   |

| G-02    | 100230  | BMP created         | BMP created | CODE\_39, decoded 100230 | PASS   |

| G-03    | 999999  | BMP created         | BMP created | not checked             | PASS   |

| V-01    | 000123  | Rejected (starts 0) | Rejected    | -                       | PASS   |

| V-02    | 012345  | Rejected (starts 0) | Rejected    | -                       | PASS   |

| V-03a   | 12345   | Rejected            | Rejected    | -                       | PASS   |

| V-03b   | 1234567 | Rejected            | Rejected    | -                       | PASS   |

| V-04    | 12A456  | Rejected            | Rejected    | -                       | PASS   |

| V-05    | (empty) | Rejected            | Rejected    | -                       | PASS   |

| V-06    | 12 456  | Rejected            | Rejected    | -                       | PASS   |



Environment: Windows x64, Visual Studio 2026 (MSVC 19.51.36260), C++17.

Console result: Passed 10, Failed 0.

Decode check done with zxing.org (verification only, not part of the app).

