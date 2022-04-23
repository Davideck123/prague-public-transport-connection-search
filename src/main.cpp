#include "Raptor.hpp"
#include "InputReader.hpp"

#include <iostream>

// helper functions for debugging

[[maybe_unused]]
void printTransfers(Timetable& timetable) {
    for (auto&& [from, toStops]: timetable.getTransfers()) {
        for (auto&& to: toStops) {
            std::cout << from->getId() << ' ' << from->getName() << " >> "
                      << to->getId() << ' ' << to->getName() << std::endl;
        }
    }
}

[[maybe_unused]]
void printEarliestTimes(Timetable& timetable) {
    for (auto&& [_, stop]: timetable.getStops()) {
        if (stop.getEarliestTime() < SIZE_MAX) {
            std::cout << stop.getId() << ' ' << stop.getName() << ' ' << Raptor::toTimeString(stop.getEarliestTime()) << std::endl;
        }
    }
}

[[maybe_unused]]
void printKTimes(Timetable& timetable) {
    for (auto&& [_, stop]: timetable.getStops()) {
        std::cout << stop.getId() << ' ' << stop.getName() << ' ';
        for (auto&& ktime: stop.getArrTimesKTrips()) {
            std::cout << ((ktime == SIZE_MAX) ? "inf" : Raptor::toTimeString(ktime)) << ' ';
        }
        std::cout << std::endl;
    }
}

int main() {

    // try to make c++ streams faster
    //std::ios_base::sync_with_stdio(false);

    std::cout << "Loading data...\n";
    Timetable timetable;
    timetable.readCSVData();
    timetable.createTransfers();

    // read input from user
    InputReader reader{timetable.getStops()};
    reader.read();
    auto&& startName = reader.getStartName();
    auto&& endName = reader.getEndName();
    size_t startTime = Raptor::toSeconds(reader.getStartTime());

    //auto&& startName = "Bazar";
    //auto&& endName = "Malostranske namesti";
    //size_t startTime = Raptor::toSeconds("8");

    auto&& [startStop, endStop] = timetable.createArtificialStops(startName, endName);
    Raptor r{timetable, startStop, endStop, startTime};

    // search
    r.raptor();
    r.printConnection(true);
}

