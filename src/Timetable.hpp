#ifndef TIMETABLE_HPP_
#define TIMETABLE_HPP_

#include "DataTypes.hpp"

#include <array>
#include <fstream>

using From = Stop;
using To = Stop;
using Id = size_t;

class Timetable {
public:

    // read all the csv files and create all Stops, Routes and Trips
    void readCSVData();

    // create transfers for real (not artificial) stops
    void createTransfers();

    // add transfers from artificial source and to artificial destination
    void addArtificialTransfers(Stop* src, Stop* dest);

    // create artificial source and destination stops
    std::pair<Stop&, Stop&> createArtificialStops(const std::string& startName,
                                                  const std::string& endName);

    // get all stops with the same name
    [[maybe_unused]]
    std::vector<Stop*> getStopsByName(const std::string& name);

    // get all marked stops for the raptor algorithm
    std::vector<Stop*> getMarkedStops();

    std::unordered_map<Id, Stop>& getStops() { return stops_; }

    [[maybe_unused]]
    std::unordered_map<Id, Route>& getRoutes() { return routes_; }

    [[maybe_unused]]
    std::unordered_map<Id, Trip>& getTrips() { return trips_; }

    std::unordered_map<From*, std::vector<To*>>& getTransfers() { return transfers_; }

private:
    // read stops.csv
    void readStops(std::ifstream& in);

    // read routes.csv
    void readRoutes(std::ifstream& in);

    // read trips.csv
    void readTrips(std::ifstream& in);

    // read stop_times.csv
    void readStopTimes(std::ifstream& in);

    // read line of csv file and get N columns
    template<size_t N>
    std::array<std::string, N> readLine(std::ifstream& in, char delim=',');

    static constexpr auto STOPS{"data/stops.csv"};
    static constexpr auto ROUTES{"data/routes.csv"};
    static constexpr auto TRIPS{"data/trips.csv"};
    static constexpr auto STOP_TIMES{"data/stop_times.csv"};

    static constexpr size_t STOPS_COLUMN_COUNT = 2;
    static constexpr size_t ROUTES_COLUMN_COUNT = 3;
    static constexpr size_t TRIPS_COLUMN_COUNT = 4;
    static constexpr size_t STOP_TIMES_COLUMN_COUNT = 4;

    // all stops
    std::unordered_map<Id, Stop> stops_;

    // all routes
    std::unordered_map<Id, Route> routes_;

    // all trips
    std::unordered_map<Id, Trip> trips_;

    // all possible transfers from every stop
    std::unordered_map<From*, std::vector<To*>> transfers_;
};

template<size_t N>
inline std::array<std::string, N> Timetable::readLine(std::ifstream& in, char delim) {
    std::array<std::string, N> tokens;
    std::string token;
    for (size_t i = 0; i < N - 1; ++i) {
        std::getline(in, token, delim);
        tokens[i] = std::move(token);
    }
    std::getline(in, token);
    tokens[N - 1] = std::move(token);
    return tokens;
}

#endif