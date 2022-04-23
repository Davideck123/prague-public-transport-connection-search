#include "Raptor.hpp"

#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <sstream>

//#define DEBUG_RAPTOR_
//#define DEBUG_UPDATE_ROUTES_TO_SCAN_
//#define DEBUG_SCAN_ROUTES_
//#define DEBUG_SCAN_TRANSFERS_
//#define DEBUG_PRINT_CONNECTION_

std::string Raptor::toTimeString(size_t timeInSeconds, bool leadingZero, bool roundSeconds, bool roundNextDay) {
    if (timeInSeconds == SIZE_MAX) return "inf";

    if (roundNextDay) {
        timeInSeconds %= HOUR_SECONDS * 24;
    }

    if (roundSeconds) {
        auto seconds = timeInSeconds % MINUTE_SECONDS;
        if (seconds >= 30) { // round up seconds
            timeInSeconds += MINUTE_SECONDS - seconds;
        }
    }

    std::string time;
    auto&& hours = std::to_string(timeInSeconds / HOUR_SECONDS);
    time += hours.size() == 2 ? hours: (leadingZero ? "0" : "") + hours;
    timeInSeconds %= HOUR_SECONDS;
    auto&& minutes = std::to_string(timeInSeconds / MINUTE_SECONDS);
    time += ":" + ((minutes.size() == 2) ? minutes: "0" + minutes);

    if (roundSeconds) return time;

    timeInSeconds %= MINUTE_SECONDS;
    auto&& seconds = std::to_string(timeInSeconds);
    time += ":" + (seconds.size() == 2 ? seconds : "0" + seconds);
    return time;
}

[[maybe_unused]]
size_t Raptor::toSeconds(const std::string& timeString) {
    size_t seconds = 0;
    std::istringstream ss{timeString};
    std::string s;
    for (size_t i = 0; std::getline(ss, s, ':') && i < 3; ++i) {
        if (i == 0) seconds += std::stoi(s) * HOUR_SECONDS;
        else if (i == 1) seconds += std::stoi(s) * MINUTE_SECONDS;
        else if (i == 2) seconds += std::stoi(s);
    }
    return seconds;
}

void Raptor::initialization() {
    // initialize with inf
    for (auto&& [_, stop]: timetable_.getStops()) {
        auto&& arr = stop.getArrTimesKTrips();
        arr.resize(numberOfTrips_ + 1);
        std::fill_n(arr.begin(), numberOfTrips_ + 1, SIZE_MAX);
    }

    start_.getArrTimesKTrips()[0] = startTime_;
    start_.getEarliestTime() = startTime_;

    // first stop is the artificial source, mark real stops
    for (auto&& to: timetable_.getTransfers()[&start_]) {
        to->getArrTimesKTrips()[0] = startTime_;
        to->getEarliestTime() = startTime_;
        //transfer
        to->getTransferFrom() = &start_;
        to->mark();
    }
}

void Raptor::updateRoutesToScan(std::unordered_map<Route*, Stop*>& routesToScan) {
    routesToScan.clear();
    for (auto&& stop: timetable_.getMarkedStops()) {
#ifdef DEBUG_UPDATE_ROUTES_TO_SCAN_
        std::cout << "Marked: " << stop->getId() << ' ' << stop->getName() << std::endl;
#endif
        for (auto&& route: stop->getRoutes()) {
#ifdef DEBUG_UPDATE_ROUTES_TO_SCAN_
            std::cout << "  Route: " << route->getId() << ' ' << route->getName() << std::endl;
#endif
            if (auto&& it = routesToScan.find(route); it != routesToScan.end()) {
                if (auto&& [_, firstStop] = *it; route->isEarlier(stop, firstStop)) {
                    firstStop = stop;
                }
            }
            else {
                routesToScan.emplace(route, stop);
            }
        }
        stop->unmark();
    }
}

void Raptor::scanRoutes(std::unordered_map<Route*, Stop*>& routesToScan, size_t k) {
    for (auto&& [route, firstStop]: routesToScan) {
#ifdef DEBUG_SCAN_ROUTES_
        std::cout << "\nScanning route: " << route->getId() << ' ' << route->getName() << " from: "
            << firstStop->getId() << ' ' << firstStop->getName() << '\n';
#endif
        Trip* currentTrip = nullptr;
        auto&& routeStops = route->getStops();
        for (size_t i = route->getStopIndex(firstStop); i < routeStops.size(); ++i) {
            auto&& stop = routeStops[i];
#ifdef DEBUG_SCAN_ROUTES_
            std::cout << "  Stop: " << stop->getId() << ' ' << stop->getName() << '\n';
#endif
            if (currentTrip != nullptr) {

                // target pruning
                auto earliestArrTime = std::min(stop->getEarliestTime(), end_.getEarliestTime());
#ifdef DEBUG_SCAN_ROUTES_
                std::cout << " BestTillNow: " << Raptor::toTimeString(stop->getEarliestTime()) <<
                          " currArr: " << Raptor::toTimeString(currentTrip->getArrivalTimes()[i]) << ' '
                          << currentTrip->getRoute()->getName() << '\n';
#endif
                if (size_t currArrTime = currentTrip->getArrivalTimes()[i]; currArrTime < earliestArrTime) {
                    stop->getArrTimesKTrips()[k] = currArrTime;
                    stop->getEarliestTime() = currArrTime;
                    stop->mark();

                    // earliest trip
                    stop->getEarliestTrip() = currentTrip;

                    // earliest trip in the k-th iteration
                    stop->getEarliestTrips().insert_or_assign(k, currentTrip);
                }
            }

            size_t currentTime = stop->getArrTimesKTrips()[k - 1];

            // avoid overflow
            if (currentTime + changeTime_ >= currentTime) {
                // don't add changeTime_ in the first iteration
                currentTime += k > 1 ? changeTime_ : 0;
            }

            auto&& trips = route->getTrips();
            auto isBefore = [i](auto&& trip, size_t time){
                return time > trip->getDepartureTimes()[i];
            };

            // find the first trip that we can take at the currentTime
            auto it = std::lower_bound(trips.begin(), trips.end(), currentTime, isBefore);

            if (it != trips.end() &&
                (currentTrip == nullptr ||
                (*it)->getDepartureTimes()[i] < currentTrip->getDepartureTimes()[i]))
            {
                currentTrip = *it;

                // set the boarding stop of the current trip
                // only used for the connection reconstruction
                auto&& boardingStop = currentTrip->getBoardingStop();
                if (boardingStop == nullptr ||
                    (stop->getTransferFrom() == &start_ && route->isEarlier(stop, boardingStop))) {

                    currentTrip->getBoardingStop() = stop;
#ifdef DEBUG_SCAN_ROUTES_
                    std::cout << " BOARDING" << std::endl;
                    std::cout << " BestTillNow: " << Raptor::toTimeString(stop->getEarliestTime()) <<
                              " currArr: " << Raptor::toTimeString(currentTrip->getArrivalTimes()[i]) << ' '
                              << currentTrip->getRoute()->getName() << " currDep: " <<
                              Raptor::toTimeString(currentTrip->getDepartureTimes()[i]) << '\n';
#endif
                }
            }
        }
    }
}

void Raptor::scanTransfers(size_t k) {
    for (auto&& from: timetable_.getMarkedStops()) {
#ifdef DEBUG_SCAN_TRANSFERS_
        std::cout << "Transfers from: " << from->getId() << ' ' << from->getName() << '\n';
#endif
        for (auto&& to: timetable_.getTransfers()[from]) {
            // transfer: from -> to
            // in the last iteration, change transfers only to the artificial end/destination stop
            if (k < numberOfTrips_ || to->isArtificial()) {

                size_t currentTime = from->getArrTimesKTrips()[k];

                // avoid overflow
                if (currentTime + transferTime_ >= currentTime) {
                    currentTime += transferTime_;
                }

                to->getArrTimesKTrips()[k] = std::min(to->getArrTimesKTrips()[k], currentTime);

                if (to->getArrTimesKTrips()[k] < to->getEarliestTime()) {
                    to->getEarliestTime() = to->getArrTimesKTrips()[k];
                    to->mark();
                    to->getTransferFrom() = from;
                }
#ifdef DEBUG_SCAN_TRANSFERS_
                std::cout << "  to: " << to->getId() << ' ' << to->getName() << '\n';
#endif
            }
        }
    }
}

[[maybe_unused]]
void Raptor::setEarliestTimes(size_t k) {
    for (auto&& [_, stop]: timetable_.getStops()) {
        auto&& arr = stop.getArrTimesKTrips();
        arr[k] = arr[k - 1];
    }
}

void Raptor::raptor() {
    initialization();
    std::unordered_map<Route*, Stop*> routesToScan;
	
	for (size_t k = 1; k < numberOfTrips_ + 1; ++k) {
#ifdef DEBUG_RAPTOR_
        std::cout << "Iteration " << k << std::endl;
#endif
        //setEarliestTimes(k); // not needed in this version of the algorithm
        updateRoutesToScan(routesToScan);
        scanRoutes(routesToScan, k);
        scanTransfers(k);
		if (timetable_.getMarkedStops().empty()) break;
	}
}

[[nodiscard]]
std::vector<std::pair<Trip*, Stop*>> Raptor::getConnection() const {
    std::vector<std::pair<Trip*, Stop*>> legs;
    auto stop = end_.getTransferFrom();

    for (size_t k = numberOfTrips_; k > 0; --k) {
        Trip* trip = nullptr;

        // get the earliest arriving trip at stop in iteration <= k
        for (auto&& [iteration, t]: stop->getEarliestTrips()) {
            if (iteration <= k) {
                if (trip == nullptr ||
                    trip->getArrivalTimes()[trip->getStopIndex(stop)] >
                    t->getArrivalTimes()[t->getStopIndex(stop)])
                {
                    trip = t;
                }
            }
        }
        // trip and it's departure stop
        legs.emplace_back(trip, stop);

        auto&& boardingStop = trip->getBoardingStop();

        if (boardingStop->getTransferFrom() != nullptr) {
            stop = boardingStop->getTransferFrom();
        }
        else stop = boardingStop;

        // if start stop was reached
        if (stop->isArtificial()) break;
    }

    // legs are filled from end to start, reverse the order
    std::ranges::reverse(legs);

    return legs;
}

void Raptor::printConnection(bool pretty) const {
    if (end_.getEarliestTime() == SIZE_MAX) {
        std::cout << "No connection found!\n";
        //std::cout << "No connection found! Try choosing more transfers.\n";
        return;
    }
    for (auto&& [trip, end]: getConnection()) {
        auto&& route = trip->getRoute();
        size_t endIndex = route->getStopIndex(end);

        auto&& start = trip->getBoardingStop();
        size_t startIndex = route->getStopIndex(start);

        if (pretty) {
            // departure
            std::cout << Raptor::toTimeString(trip->getDepartureTimes()[startIndex], false, true, true)
                      << ' ' << start->getName() << " >> ";
            // arrival
            std::cout << toTimeString(trip->getArrivalTimes()[endIndex], false, true, true)
                      << ' ' << end->getName() << ' ' << route->getName() << '\n';
        }
        else {
            // used for debugging
            std::cout << "Departure: " << start->getId() << ' ' << start->getName() << ' '
                      << Raptor::toTimeString(trip->getDepartureTimes()[startIndex])
                      << ' ' << route->getName() << '\n';

            std::cout << "Arrival: " << end->getId() << ' ' << end->getName() << ' '
                      << Raptor::toTimeString(trip->getArrivalTimes()[endIndex])
                      << ' ' << route->getName() << '\n';
        }
    }
}
