#pragma once
// OWNER: Member 2. Used by M3 (OrderStore) and M4 (BillStore). Shared contract (frozen).
#include <string>
#include <vector>
#include "CommonTypes.h"

// ---- One line of a data file ----
// Split a line like:  101 123456 MILKP "Milk Powder 400g" count 450.00 #
// into fields (quotes removed, trailing '#' removed). Empty quoted "" gives an empty field.
// On error: success=false and message contains the line number, e.g. "line 7: missing closing quote".
OperationResult splitRecord(const std::string& line, int lineNumber, std::vector<std::string>& fields);

// Wrap text in quotes: Milk Powder -> "Milk Powder"; empty text -> "". Use it for every text field.
std::string quoteField(const std::string& text);

// Join already-prepared fields with single spaces and add the final " #".
// Example: writeRecord({"101", quoteField("Milk Powder")}) -> 101 "Milk Powder" #
std::string writeRecord(const std::vector<std::string>& fields);

// ---- Whole-file helpers ----
// Missing file -> success=false. Empty file -> success=true with no lines. Blank lines are skipped.
OperationResult readLines(const std::string& path, std::vector<std::string>& lines);

// ---- All-or-nothing writing of several files ----
class FileTransaction {
public:
    void add(const std::string& targetPath, const std::string& newContent);

    // 1) write every file as <target>.tmp  2) rename all over the targets.
    // If anything fails: delete the .tmp files, restore any file already replaced, return success=false.
    OperationResult commit();

    // For tests only: make commit() fail on purpose (after the temp files are written)
    // so "nothing changes" can be checked.
    void simulateFailure(bool enable) { simulateFailure_ = enable; }

private:
    struct Entry { std::string target, content; };
    std::vector<Entry> entries_;
    bool simulateFailure_ = false;
};
