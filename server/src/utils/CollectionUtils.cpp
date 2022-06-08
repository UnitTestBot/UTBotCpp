#include "CollectionUtils.h"

#include "utils/path/FileSystemPath.h"
#include <iterator>

namespace CollectionUtils {

    std::vector<int> range(int start, int end, int step) {
        int cnt = (end - start - 1) / step + 1;
        std::vector<int> result(cnt);
        std::generate(result.begin(), result.end(), [step, x = 0]() mutable {
            int y = x;
            x += step;
            return y;
        });
        return result;
    }
}
