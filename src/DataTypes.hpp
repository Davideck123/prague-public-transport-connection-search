#ifndef DATATYPES_HPP_
#define DATATYPES_HPP_

#include <string>
#include <vector>
#include <unordered_map>

class Route;
class Stop;
class Trip;

class Stop {
public:
    Stop(size_t id, std::string name, bool artificial=false) :
        id_(id), name_(std::move(name)), artificial_(artificial) {}

	bool operator==(const Stop& other) const { return id_ == other.id_; }

	void mark() { marked_ = true; }

	void unmark() { marked_ = false; }

	[[nodiscard]]
    bool isMarked() const { return marked_; }

    [[nodiscard]]
    bool isArtificial() const { return artificial_; }

    std::vector<size_t>& getArrTimesKTrips() { return arrTimesKTrips_; }

	size_t& getEarliestTime() { return earliestArrTime_; }

	std::vector<Route*>& getRoutes() { return routes_; }

    [[nodiscard]]
    size_t getId() const { return id_; }

    [[nodiscard]]
    const std::string& getName() const { return name_; }

    [[maybe_unused]]
    Trip*& getEarliestTrip() { return earliestTrip_; }

    Stop*& getTransferFrom() { return transferFrom_; }

    std::unordered_map<size_t, Trip*>& getEarliestTrips() { return earliestTrips_; }

private:

    const size_t id_;
    const std::string name_;

    // mark for raptor algorithm
	bool marked_ = false;

    const bool artificial_;

    // all routes that use this stop
    std::vector<Route*> routes_;

    // k-th value represents the earliest arrival time at this stop
    // in the k-th iteration (using at most k trips)
    std::vector<size_t> arrTimesKTrips_;

    // the earliest arrival time at this stop (overall)
	size_t earliestArrTime_ = SIZE_MAX;

    // the earliest trip that arrives at this stop
    Trip* earliestTrip_ = nullptr;

    // stop with the earliest arrival time,
    // from which we transferred to this stop
    Stop* transferFrom_ = nullptr;

    // map of iteration -> the earliest trip
    // used for the connection reconstruction
    std::unordered_map<size_t , Trip*> earliestTrips_;
};

class Route {
public:
    Route(size_t id, std::string name, size_t type) :
        id_(id), name_(std::move(name)), type_(type) {}

	// true if stop s1 is before stop s2 on this route
	[[nodiscard]]
    bool isEarlier(const Stop* s1, const Stop* s2) const;

    // get the index of s for this route
	[[nodiscard]]
    size_t getStopIndex(const Stop* s) const;

	std::vector<Stop*>& getStops() { return stops_; }

	std::vector<Trip*>& getTrips() { return trips_; }

    [[maybe_unused]] [[nodiscard]]
    size_t getType() const { return type_; }

	[[nodiscard]]
    size_t getId() const { return id_; }

    [[nodiscard]]
    const std::string& getName() const { return name_; }

private:

	const size_t id_;
	const std::string name_;
	const size_t type_;

    // sequence of stops on this route sorted from start to finish
	std::vector<Stop*> stops_;

    // ascending sequence of trips operating on this route,
    // sorted by departure time
	std::vector<Trip*> trips_;
};

class Trip {
public:
    Trip(size_t id, Route* route, std::string headsign, size_t direction) :
        id_(id), headsign_(std::move(headsign)),
        direction_(direction), route_(route) {}

    // get the index of s for route, on which operates this trip
    [[maybe_unused]] [[nodiscard]]
    size_t getStopIndex(const Stop* s) const { return route_->getStopIndex(s); }

    // add arrival and departure time for the actual stop of this trip
    void addTimes(size_t arr, size_t dep) {
        arrivalTimes_.push_back(arr); departureTimes_.push_back(dep);
    }

    [[nodiscard]]
    size_t getId() const { return id_; }

    Route* getRoute() { return route_; }

    [[maybe_unused]] [[nodiscard]]
    const std::string& getHeadsign() const { return headsign_; }

    [[maybe_unused]] [[nodiscard]]
    size_t getDirection() const { return direction_; }

    [[nodiscard]]
    const std::vector<size_t>& getArrivalTimes() const { return arrivalTimes_; }

    [[nodiscard]]
    const std::vector<size_t>& getDepartureTimes() const { return departureTimes_; }

    Stop*& getBoardingStop() { return boardingStop_; }

private:
	const size_t id_;
    const std::string headsign_;
    const size_t direction_;

    // a route on which operates this trip
    Route* const route_;

    // stop at which this trip was boarded
    Stop* boardingStop_ = nullptr;

    // sequence of arrival times at the stops of this trip
    std::vector<size_t> arrivalTimes_;

    // sequence of departure times at the stops of this trip
    std::vector<size_t> departureTimes_;
};

#endif