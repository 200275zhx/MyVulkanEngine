#include <vector>
#include <string_view>
#include <unordered_set>

namespace mve {
    std::vector<const char*> deduplicateVectorCstring(const std::vector<const char*>& content);
    void insertUniqueCStrings(std::vector<const char*>& dest, const std::vector<const char*>& src);
} // namespace mve