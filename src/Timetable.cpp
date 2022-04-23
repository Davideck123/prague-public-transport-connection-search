#include "Timetable.hpp"

#include <iostream>
#include <algorithm>

void Timetable::readStops(std::ifstream& in) {
    while (in.peek() != EOF) {
        auto&& [_id, name] = readLine<STOPS_COLUMN_COUNT>(in);
        auto id = static_cast<size_t>(std::stoi(_id));

        // create stop
        stops_.try_emplace(id, id, std::move(name));
    }
}

void Timetable::readRoutes(std::ifstream& in) {
    while (in.peek() != EOF) {
        auto&& [_id, name, _type] = readLine<ROUTES_COLUMN_COUNT>(in);
        auto id = static_cast<size_t>(std::stoi(_id));
        auto type = static_cast<size_t>(std::stoi(_type));

        // create route
        routes_.try_emplace(id, id, std::move(name), type);
    }
}

void Timetable::readTrips(std::ifstream& in) {
    while (in.peek() != EOF) {
        auto&& [_id, _routeId, headsign, _direction] = readLine<TRIPS_COLUMN_COUNT>(in);

        auto id = static_cast<size_t>(std::stoi(_id));
        auto routeId = static_cast<size_t>(std::stoi(_routeId));
        auto direction = static_cast<size_t>(std::stoi(_direction));

        auto&& route = routes_.at(routeId);

        // create trip
        auto&& [it, _] = trips_.try_emplace(id, id, &route, std::move(headsign), direction);

        auto&& trip = it->second;
        route.getTrips().emplace_back(&trip);
    }
}

void Timetable::readStopTimes(std::ifstream& in) {
    Trip* scannedTrip = nullptr;
    bool addStops;

    while (in.peek() != EOF) {
        auto&& [tripId_, arrTime_, depTime_, stopId_] = readLine<STOP_TIMES_COLUMN_COUNT>(in);
        auto tripId = static_cast<size_t>(std::stoi(tripId_));
        auto arrTime = static_cast<size_t>(std::stoi(arrTime_));
        auto depTime = static_cast<size_t>(std::stoi(depTime_));
        auto stopId = static_cast<size_t>(std::stoi(stopId_));

        auto&& trip = trips_.at(tripId);
        auto&& stop = stops_.at(stopId);

        // add arrival and departure time for stop in trip
        trip.addTimes(arrTime, depTime);

        auto&& stopRoutes = stop.getRoutes();
        auto&& route = trip.getRoute();

        // if stopRoutes don't contain this route already
        if (std::ranges::find(stopRoutes, route) == stopRoutes.end()) {
            stopRoutes.emplace_back(route);
        }

        // if there's a new trip
        if (scannedTrip != &trip) {
            scannedTrip = &trip;
            addStops = false;

            // if this route is scanned for the first time
            if (route->getStops().empty()) addStops = true;
        }

        if (addStops) route->getStops().emplace_back(&stop);
    }
}

void Timetable::readCSVData() {
    std::array filenames{STOPS, ROUTES, TRIPS, STOP_TIMES};

    for (auto&& filename: filenames) {
        std::ifstream file{filename};
        if (!file.good() || !file.is_open()) {
            std::cout << "Can't read " << filename << '\n';
            break;
        }
        std::string firstLine;
        std::getline(file, firstLine); // skip the first line - column names

        if (filename == STOPS) readStops(file);
        else if (filename == ROUTES) readRoutes(file);
        else if (filename == TRIPS) readTrips(file);
        else if (filename == STOP_TIMES) readStopTimes(file);
    }
}

std::vector<Stop*> Timetable::getMarkedStops() {
    std::vector<Stop*> marked;
    for (auto&& [_, stop]: stops_) {
        if (stop.isMarked()) marked.emplace_back(&stop);
    }
    return marked;
}

[[maybe_unused]]
std::vector<Stop*> Timetable::getStopsByName(const std::string& name) {
    std::vector<Stop*> stopsFound;
    for (auto&& [_, stop]: stops_) {
        if (stop.getName() == name) {
            stopsFound.emplace_back(&stop);
        }
    }
    return stopsFound;
}

void Timetable::createTransfers() {
    for (auto&& [_, stop]: stops_) {
        for (auto&& [_id, s]: stops_) {

            // compare names (or possibly node ids of the stops)
            if (stop != s && stop.getName() == s.getName()) {

                // add transfer from stop to s

                if (transfers_.contains(&stop)) {
                    transfers_[&stop].emplace_back(&s);
                }
                else {
                   transfers_.try_emplace(&stop, std::vector{&s});
                }
            }
        }
    }
}

void Timetable::addArtificialTransfers(Stop* src, Stop* dest) {
    transfers_.try_emplace(src);
    for (auto&& [_, stop]: stops_) {

        // compare names (or possibly node ids of the stops)
        if (src != &stop && src->getName() == stop.getName()) {

            // add transfer from src to stop
            transfers_[src].emplace_back(&stop);
        }

        // compare names (or possibly node ids of the stops)
        else if (&stop != dest && stop.getName() == dest->getName()) {

            // add transfer from stop to dest
            transfers_[&stop].emplace_back(dest);
        }
    }
}

std::pair<Stop&, Stop&> Timetable::createArtificialStops(const std::string& startName, const std::string& endName) {
    size_t startId = SIZE_MAX;
    size_t endId = SIZE_MAX - 1;

    // create startStop
    auto&& [it, _] = stops_.try_emplace(startId, startId, startName, true);
    auto&& startStop = it->second;

    // create endStop
    auto&& [_it, _bool] = stops_.try_emplace(endId, endId, endName, true);
    auto&& endStop = _it->second;

    addArtificialTransfers(&startStop, &endStop);
    return {startStop, endStop};
}
