#include "Include.h"

#include <utility>

Include::Include(bool isAngled, fs::path path) : is_angled(isAngled), path(std::move(path)) {}
