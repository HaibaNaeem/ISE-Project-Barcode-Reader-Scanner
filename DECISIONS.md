# Decisions made while writing the skeleton (Member 5)

Frozen on Sunday 10:00. After that, changes need everyone's agreement.

| # | Topic | Decision | Why / who it affects |
|---|-------|----------|----------------------|
| 1 | C++ standard | C++17 (already set in CMake). Uses std::optional and std::filesystem. | All |
| 2 | Where Money/Quantity/MeasureUnit live | `Units.h` (M2). Everything includes it. | All |
| 3 | Bill.txt columns | From the sample row: billId, customer, orderId, acronym, unitPrice, **discountPerUnit**, qty, subtotal, tax total, discount total, payable, status. Tax per unit is not in the file (only the tax total). | M4. [ask] if the instructor wants tax per unit too |
| 4 | RecordFormat API | splitRecord / quoteField / writeRecord / readLines / FileTransaction (see header) | M2 writes, M3 + M4 use |
| 5 | Stores give text | OrderStore, BillStore, ProductCatalog each have serialize(); pay() hands the text to FileTransaction | M3, M4 |
| 6 | OrderManager constructor | OrderManager(const ProductCatalog&, OrderStore* = nullptr). nullptr = no saving (tests) | M1, M3 |
| 7 | Clock | OrderManager::setClock() so tests get predictable Order IDs | M3 |
| 8 | After payment | CheckoutService calls OrderManager::clearCurrent() | M3, M4 |
| 9 | Stock reduction | ProductCatalog::reduceStock(id, amount), fails below zero | M2, M4 |
| 10 | Change for cash | changeDue() in Pricing.h, so the GUI does no money maths | M4 |
| 11 | Test hooks | FileTransaction::simulateFailure() and CheckoutService::simulateWriteFailure() | M4 |
| 12 | Image paths | Stored relative to the .exe folder; DataPaths::resolve() turns them into real paths | M1, M2 |
| 13 | CTest | Every tests/test_*.cpp except test_camera. Runs from the repo root. test_decoder runs after test_generator. | All |
| 14 | Data folder | data/ next to the .exe; files copied from sample_data/ only when missing; never overwritten | M1 |
