#pragma once

#include "sim_config.hpp"
#include <vector>
#include <string>

class CLI {
public:
    static SimConfig parse(int argc, char* argv[]);
    static void print_usage(const char* progname);
    
private:
    static std::vector<std::string> split_string(const std::string& s, char delim);
    static float parse_float(const std::string& s);
    static int parse_int(const std::string& s);
    static uint64_t parse_uint64(const std::string& s);
};


