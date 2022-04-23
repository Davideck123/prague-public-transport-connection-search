#include "InputReader.hpp"

#include <algorithm>
#include <vector>
#include <iostream>
#include <cctype>

[[maybe_unused]]
InputReader::InputReader(const std::unordered_map<size_t, Stop>& stops) {
    // fill in stopNames_
    for (auto&& [_, stop]: stops) {
        if (!stopNames_.contains(stop.getName())) {
            std::string_view name{stop.getName()}; // original name
            std::string nameLower{stop.getName()}; // name in lowercase
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(),
                           [](unsigned char c){ return std::tolower(c); });
            stopNames_.emplace(std::move(nameLower), name);
        }
    }
}

std::vector<std::string_view> InputReader::getSimilarNames(const std::string& nameLower) const {
    std::vector<std::string_view> similarNames;
    for (auto&& [nLower, n]: stopNames_) {

        // if nLower contains nameLower as a substring
        if (nLower.find(nameLower) != std::string::npos) {
            similarNames.emplace_back(n);
        }
        if (similarNames.size() == MAX_COUNT) break;
    }
    return similarNames;
}

std::string InputReader::readName(bool start) const {
    std::string name;
    while (true) {
        std::cout << "Enter the " << (start ? "starting point": "destination") << ": ";
        std::getline(std::cin, name);
        auto nameLower = name;
        std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(),
                       [](unsigned char c){ return std::tolower(c); });

        // if user entered valid name and thus nameLower exists
        if (auto it = stopNames_.find(nameLower); it != stopNames_.end()) {

            if (!start && startName_ == it->second) {
                // start and end stops must have different names
                std::cout << "Can't enter the same stop.\n";
                continue;
            }
            return std::string{it->second};
        }
        std::cout << "Invalid name!";
        if (nameLower.size() >= MIN_LENGTH) {

            // find similar names and print them
            auto&& similar = getSimilarNames(nameLower);
            if (!similar.empty()) {
                std::cout << " Did you mean:\n";
                for (auto&& n: similar) {
                    std::cout << "  " << n << '\n';
                }
            }
            else std::cout << '\n';
        }
        else std::cout << '\n';
    }
}

std::string InputReader::readTime() {
    std::string time;
    while (true) {
        std::cout << "Enter the departure time: ";
        std::getline(std::cin, time);

        if (!time.empty() && time.size() <= 5) {
            auto isDigit = [](unsigned char c){ return std::isdigit(c); };

            if (time.size() <= 2) { // hh or h format
                // hour must be in a range 0-23
                if (std::all_of(time.cbegin(), time.cend(), isDigit) && std::stoi(time) < 24) {
                    break;
                }
            }
            else if (time.size() > 3) { // h:mm or hh:mm format, ':' represents any non-digit separator
                auto&& minutes = time.substr(time.size() - 2, time.size());
                auto&& hours = time.substr(0, time.size() - 3);

                // hours must be in a range 0-23, minutes in a range 0-59 and separator must be valid
                if (std::all_of(minutes.cbegin(), minutes.cend(), isDigit) && std::stoi(minutes) < 60
                    && std::all_of(hours.cbegin(), hours.cend(), isDigit) && std::stoi(hours) < 24
                    && !std::isdigit(time[time.size() - 3])) {
                    time[time.size() - 3] = ':';
                    break;
                }
            }
        }
        std::cout << "Invalid time format! Use hh:mm or hh.\n";
    }
    return time;
}

[[maybe_unused]] void InputReader::read() {
    startName_ = readName(true);
    endName_ = readName(false);
    startTime_ = readTime();
}

