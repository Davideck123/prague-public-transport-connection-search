#ifndef RAPTOR_HPP_
#define RAPTOR_HPP_

#include "Timetable.hpp"

class Raptor {
public:
    Raptor(Timetable& t, Stop& start, Stop& end, size_t startTime)
        : timetable_(t), start_(start), end_(end), startTime_(startTime) {}

    // run the raptor algorithm (the search)
    void raptor();

    // create human-readable time string from timeInSeconds
    static std::string toTimeString(size_t timeInSeconds, bool leadingZero=false,
                                    bool roundSeconds=false, bool roundNextDay=false);

    // get seconds from human-readable time string
    [[maybe_unused]]
    static size_t toSeconds(const std::string& timeString);

    // print the resulting connection (set pretty=true for the user)
    void printConnection(bool pretty=false) const;

    [[maybe_unused]]
    Timetable& getTimetable() { return timetable_; }

private:
    // initialize values for the raptor algorithm
    void initialization();

    // prepare routes that will be scanned in the current iteration
    void updateRoutesToScan(std::unordered_map<Route*, Stop*>& routesToScan);

    // traverse all prepared routes in the current iteration
    // the main part of the search
    void scanRoutes(std::unordered_map<Route*, Stop*>& routesToScan, size_t k);

    // transfers (footpaths) part of the raptor algorithm
    void scanTransfers(size_t k);

    // get sequence of trips with corresponding exit stops
    [[nodiscard]]
    std::vector<std::pair<Trip*, Stop*>> getConnection() const;

    // set upper bound for earliest arrival times in the k-th iteration
    [[maybe_unused]]
    void setEarliestTimes(size_t k);

    static constexpr size_t HOUR_SECONDS = 3600;
    static constexpr size_t MINUTE_SECONDS = 60;

    // max number of trips used in the search
    const size_t numberOfTrips_ = 5;

    const size_t changeTime_ = 30; // change trip at the exact same stop

    // time in seconds - added when transferring to another stop (same for all stops)
    const size_t transferTime_ = 120; // transfer/walk to another stop

    const size_t startTime_;
    Timetable& timetable_;

    // source/start stop
    Stop& start_;

    // end/destination stop
    Stop& end_;
};

#endif
