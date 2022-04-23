#include "DataTypes.hpp"

#include <algorithm>

bool Route::isEarlier(const Stop* s1, const Stop* s2) const {
    return getStopIndex(s1) < getStopIndex(s2);
}

size_t Route::getStopIndex(const Stop* s) const {
    auto&& it = std::ranges::find(stops_, s);
    return it - stops_.cbegin();
}