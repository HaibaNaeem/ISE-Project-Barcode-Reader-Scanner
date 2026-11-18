# Manual test table: order screen (Member 1)

Tester: ________   Computer: ________   Date: ________

| # | Test | Steps | Expected | Pass |
|---|------|-------|----------|------|
| 1 | Scan | Hold a sample barcode to the camera (or type it + Add) | Product in cart, qty 1, totals update, green banner | [ ] |
| 2 | Repeat scan | Scan the same product twice | One cart row, qty 2 | [ ] |
| 3 | Held in view | Hold one barcode steadily for 3 s | Added only once | [ ] |
| 4 | Unknown barcode | Type 999999 + Add | Red banner, cart unchanged | [ ] |
| 5 | Too much quantity | Set quantity above stock, Add | Red banner, cart unchanged | [ ] |
| 6 | Exactly the stock | Add exactly the available stock | Accepted | [ ] |
| 7 | Zero stock product | Add the zero-stock product | Rejected with message | [ ] |
| 8 | Weight product | Add 1.250 | Shows 1.250 in qty column | [ ] |
| 9 | +/- and Remove | Select row, press +, -, Remove | Qty changes by step; row removed | [ ] |
| 10 | Description | Select a row, press Description | Shows that product; disabled when cart empty | [ ] |
| 11 | Totals | Price 100, tax 15, discount 10, qty 3 | 300 / 45 / 30 / payable 315 | [ ] |
| 12 | Hold | Press Hold | Cart clears, banner ok | [ ] |
| 13 | Resume | Held Orders, pick the order | Cart comes back | [ ] |
| 14 | Pay cash | Pay, choose cash | Bill shown, cart clears, stock lower | [ ] |
| 15 | Pay card | Pay, card, merchant name | Same as above | [ ] |
| 16 | Cancel | Cancel, confirm | Cart clears, stock unchanged | [ ] |
| 17 | Restart | Close and reopen the app | Held orders, bills, stock unchanged | [ ] |
| 18 | Camera unplugged | Start app with no camera | Message shown, app stays open, manual entry works | [ ] |
| 19 | No freeze | Use the app for 2 minutes with camera on | Window never freezes | [ ] |
