# Prague journey planner 🔍

## *Search engine for Prague public transport*

Contains all metro🚇, tram🚊 and non-regional bus lines🚍.
It doesn't take into account specific day, only time of the day.

To use this search, clone this repo and build and run the only cmake target 
available `JourneyPlanner`.
I recommend using the release mode since loading all the data takes a little while.

The usage is pretty straightforward - enter the name of the start stop🚏, end stop🚏 and the departure time🕑 and 
you'll get the connection if it exists. If you enter just a part of the stop name, you might get
a hint of the name you want.

## Documentation

The search uses `Raptor algorithm` presented at [https://www.microsoft.com/en-us/research/wp-content/uploads/2012/01/raptor_alenex.pdf](https://www.microsoft.com/en-us/research/wp-content/uploads/2012/01/raptor_alenex.pdf)
or in longer version at [https://renatowerneck.files.wordpress.com/2016/06/dpw14-raptor.pdf](https://renatowerneck.files.wordpress.com/2016/06/dpw14-raptor.pdf).
The most important thing about the algorithm is that it is not based on Dijkstra algorithm or a graph, 
but it takes advantage of the `GTFS` format of the data ([https://developers.google.com/transit/gtfs/reference/](https://developers.google.com/transit/gtfs/reference/)).

The specific data included in this project come from Prague public transport (available with
some documentation at [https://pid.cz/o-systemu/opendata/](https://pid.cz/o-systemu/opendata/)), 
but the data were preprocessed and transformed for my purposes.

In order to understand how this program really works, it is kind of necessary to read about the Raptor algorithm
and about the GTFS format, but I tried to provide a lot of comments in the code, so you can at least 
have a general idea of how it works.

Also, following is some information about all source files:

### `DataTypes.hpp`, `Route.cpp`
- These files contain `Stop`, `Route` and `Trip` classes, which are essential terms in GTFS

### `Timetable.hpp`, `Timetable.cpp`
- Timetable class - provides an interface for getting all the data (the stops, routes, trips and other stuff)

### `Raptor.hpp`, `Raptor.cpp`
- Raptor class - this class does the whole search, recreates the connection (gets all the lines 
used) and shows the result

### `InputReader.hpp`, `InputReader.cpp`
- InputReader class - reads and takes care of all user input

### `main.cpp`
- the entry point, just merges everything together