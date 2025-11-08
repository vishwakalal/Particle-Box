#include "csv.hpp"
#include <iostream>

CSVWriter::CSVWriter(const std::string& filename, bool append) {
    if (append) {
        file_.open(filename, std::ios::app);
    } else {
        file_.open(filename);
    }
    if (!file_.is_open()) {
        std::cerr << "Warning: Could not open CSV file: " << filename << std::endl;
    }
}

CSVWriter::~CSVWriter() {
    if (file_.is_open()) {
        file_.close();
    }
}

void CSVWriter::writeRow(const std::vector<std::string>& values) {
    if (!file_.is_open()) return;
    
    for (size_t i = 0; i < values.size(); ++i) {
        file_ << values[i];
        if (i < values.size() - 1) {
            file_ << ",";
        }
    }
    file_ << "\n";
}

void CSVWriter::flush() {
    if (file_.is_open()) {
        file_.flush();
    }
}
