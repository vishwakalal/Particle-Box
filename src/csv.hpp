#pragma once

#include <string>
#include <fstream>
#include <vector>

class CSVWriter {
public:
    CSVWriter(const std::string& filename, bool append = false);
    ~CSVWriter();
    
    void writeRow(const std::vector<std::string>& values);
    void flush();
    bool isOpen() const { return file_.is_open(); }
    
private:
    std::ofstream file_;
};
