#include "RecordFormat.h"
#include <filesystem>
#include <fstream>

OperationResult splitRecord(const std::string& line, int lineNumber, std::vector<std::string>& fields)
{
    OperationResult r;
    auto fail = [&](const std::string& m) {
        r.success = false;
        r.message = "line " + std::to_string(lineNumber) + ": " + m;
        return r;
    };

    std::vector<std::pair<std::string, bool>> tok;   // text, quoted?
    size_t i = 0, n = line.size();
    while (i < n) {
        if (line[i] == ' ' || line[i] == '\t') { ++i; continue; }
        if (line[i] == '"') {
            size_t j = line.find('"', i + 1);
            if (j == std::string::npos) return fail("missing closing quote");
            tok.push_back({line.substr(i + 1, j - i - 1), true});
            i = j + 1;
            if (i < n && line[i] != ' ' && line[i] != '\t') return fail("text right after a closing quote");
        } else {
            size_t j = i;
            while (j < n && line[j] != ' ' && line[j] != '\t') {
                if (line[j] == '"') return fail("unexpected quote inside a word");
                ++j;
            }
            tok.push_back({line.substr(i, j - i), false});
            i = j;
        }
    }
    if (tok.empty() || tok.back().second || tok.back().first != "#")
        return fail("record must end with #");
    tok.pop_back();

    fields.clear();
    for (auto& t : tok) fields.push_back(t.first);
    r.success = true;
    return r;
}

std::string quoteField(const std::string& text) { return "\"" + text + "\""; }

std::string writeRecord(const std::vector<std::string>& fields)
{
    std::string s;
    for (auto& f : fields) s += f + " ";
    return s + "#";
}

OperationResult readLines(const std::string& path, std::vector<std::string>& lines)
{
    OperationResult r;
    lines.clear();
    std::ifstream in(path, std::ios::binary);
    if (!in) { r.message = "File not found: " + path; return r; }
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.find_first_not_of(" \t") == std::string::npos) continue;   // blank line skip
        lines.push_back(line);
    }
    r.success = true;
    return r;
}

void FileTransaction::add(const std::string& targetPath, const std::string& newContent)
{
    entries_.push_back({targetPath, newContent});
}

OperationResult FileTransaction::commit()
{
    namespace fs = std::filesystem;
    OperationResult r;
    std::error_code ec;
    auto removeTmp = [&] { for (auto& e : entries_) fs::remove(e.target + ".tmp", ec); };

    // 1. sab .tmp files likho
    for (auto& e : entries_) {
        std::ofstream out(e.target + ".tmp", std::ios::binary);
        out << e.content;
        out.close();
        if (!out) { removeTmp(); r.message = "Cannot write " + e.target; return r; }
    }

    // test ke liye jaan-boojh kar fail
    if (simulateFailure_) {
        removeTmp();
        r.message = "Saving failed (simulated). No file was changed.";
        return r;
    }

    // 2. swap: purani file .bak mein, phir .tmp ko asli naam do
    size_t done = 0;
    bool ok = true;
    for (; done < entries_.size(); ++done) {
        auto& t = entries_[done].target;
        if (fs::exists(t)) { fs::rename(t, t + ".bak", ec); if (ec) { ok = false; break; } }
        fs::rename(t + ".tmp", t, ec);
        if (ec) { ok = false; break; }
    }

    if (!ok) {   // jo replace ho chuki thin unhe wapas karo
        for (size_t k = 0; k <= done && k < entries_.size(); ++k) {
            auto& t = entries_[k].target;
            if (fs::exists(t + ".bak")) { fs::remove(t, ec); fs::rename(t + ".bak", t, ec); }
        }
        removeTmp();
        r.message = "Saving failed. No file was changed.";
        return r;
    }

    for (auto& e : entries_) fs::remove(e.target + ".bak", ec);
    r.success = true;
    r.message = "Saved.";
    return r;
}