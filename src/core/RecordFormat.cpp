// OWNER: Member 2. STUB written by Member 5 so the project builds. Replace with the real code.
#include "RecordFormat.h"

OperationResult splitRecord(const std::string&, int, std::vector<std::string>&) { return {false, "TODO (M2): splitRecord"}; }
std::string quoteField(const std::string& text) { return "\"" + text + "\""; }
std::string writeRecord(const std::vector<std::string>&) { return "TODO #"; }
OperationResult readLines(const std::string&, std::vector<std::string>&) { return {false, "TODO (M2): readLines"}; }

void FileTransaction::add(const std::string& targetPath, const std::string& newContent) { entries_.push_back({targetPath, newContent}); }
OperationResult FileTransaction::commit() { return {false, "TODO (M2): FileTransaction::commit"}; }
