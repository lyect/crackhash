#include "worker/Worker.hpp"

#include <iostream>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        std::cerr << "Please, pass path to config file as a parameter." << std::endl;
        return 1;
    }

    boost::asio::io_context ioc;
    while (true) {
        try {
            std::stringstream ss;
            ss << "\n\n\n";
            ss << "\t##########################################################\n";
            ss << "\t#                                                        #\n";
            ss << "\t#                  STARTING WORKER TASK                  #\n";
            ss << "\t#                                                        #\n";
            ss << "\t##########################################################\n";
            ss << "\n\n\n";
            std::cerr << ss.str();
            ch::worker::Worker worker(ioc);
            return worker.run(argv[1]);
        }
        catch (const std::runtime_error &re) {
            std::stringstream ss;
            ss << "\n\n\n";
            ss << "\t##########################################################\n";
            ss << "\t#                                                        #\n";
            ss << "\t#           RESTARTING WORKER DUE TO EXCEPTION           #\n";
            ss << "\t#                                                        #\n";
            ss << "\t##########################################################\n";
            ss << "\tException:\n";
            ss << "\t\t" << re.what();
            ss << "\n\n\n";
            std::cerr << ss.str();
        }
    }
}