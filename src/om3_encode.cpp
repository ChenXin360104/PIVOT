#include "TimerStats.h"
#include "Om3Encoder.h"

int main(int argc, char* argv[]) {
    TimerStats stats;
    
    try {
        if (argc > 1) {
            std::cout << "First Argument: " << argv[1] << std::endl;
        }
        stats.timestart("minmax_encode");
        DatasetEncoder::minmax_encode("../data/"+std::string(argv[1])+".csv");
        stats.timeend("minmax_encode");
    } 
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    stats.timetotal("minmax_encode");
    return 0;
}