#ifndef INPUTREADER_HPP_
#define INPUTREADER_HPP_

#include "Timetable.hpp"

#include <unordered_map>
#include <string>
#include <string_view>

class [[maybe_unused]] InputReader {
public:
    [[maybe_unused]]
    explicit InputReader(const std::unordered_map<size_t, Stop>& stops);

    // read and store all user input
    [[maybe_unused]]
    void read();

    [[maybe_unused]] [[nodiscard]]
    const std::string& getStartName() const { return startName_; }

    [[maybe_unused]] [[nodiscard]]
    const std::string& getEndName() const { return endName_; }

    [[maybe_unused]] [[nodiscard]]
    const std::string& getStartTime() const { return startTime_; }

private:
    // get names containing nameLower as a substring
    [[nodiscard]]
    std::vector<std::string_view> getSimilarNames(const std::string& nameLower) const;

    // get the name of a stop from user
    // start indicates the source/start stop
    [[nodiscard]]
    std::string readName(bool start) const;

    // get the departure/start time from user
    [[nodiscard]]
    std::string readTime() ;

    // max number of returned similar names
    static constexpr size_t MAX_COUNT = 5;

    // min length of a substring
    static constexpr size_t MIN_LENGTH = 2;

    // maps name in lowercase to original name
    std::unordered_map<std::string, std::string_view> stopNames_;

    std::string startName_;
    std::string endName_;
    std::string startTime_;
};

#endif
