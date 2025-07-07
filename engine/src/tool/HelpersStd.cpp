#include "tool/HelpersStd.hpp"

namespace mve {
    std::vector<const char*> deduplicateVectorCstring(const std::vector<const char*>& content) {
        std::vector<const char*> result;
        result.reserve(content.size());
        std::unordered_set<std::string_view> seen;

        for (const char* str : content) {
            if (!str) continue;                      // skip nulls
            std::string_view sv{ str };               // view into the C-string
            if (seen.insert(sv).second) {           // if this content wasn’t seen before
                result.push_back(str);              // keep the pointer
            }
        }

        return result;
    }

    void insertUniqueCStrings(std::vector<const char*>& dest, const std::vector<const char*>& src) {
        // Track what’s already in dest
        std::unordered_set<std::string_view> seen;
        seen.reserve(dest.size() + src.size());

        for (const char* s : dest) {
            if (s) seen.insert(std::string_view{ s });
        }

        // Insert only new content
        for (const char* s : src) {
            if (!s) continue;
            std::string_view sv{ s };
            if (seen.insert(sv).second) {
                dest.push_back(s);
            }
        }
    }
} // namespace mve