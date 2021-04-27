//
// Created by rw on 4/27/21.
//

#include <fstream>
#include <iostream>
#include "file_utils.h"

bool read_line_of_doubles_(std::ifstream &f, std::vector<double> &dest, char delimeter) {
    std::string line;
    if (!getline(f, line)) {
        return false;
    }
    std::string token;
    for (char c : line) {
        if (c == delimeter) {
            dest.push_back(stod(token));
            token = "";
        } else {
            token.push_back(c);
        }
    }
    dest.push_back(stod(token));
    return true;
}

void file_utils::read_doubles(const std::string &file, std::vector<double> &dest, char delimeter) {
    std::ifstream f(file);
    if (f.is_open()) {
        while (f.peek() != EOF){
            read_line_of_doubles_(f, dest, delimeter);
        }
        f.close();
    } else {
        std::cerr << "Could not open " << file << std::endl;
    }
}

void file_utils::read_doubles(const std::string &file, std::vector<std::vector<double>> &dest, char delimeter) {
    std::ifstream f(file);
    if (f.is_open()) {
        while (f.peek() != EOF){
            read_line_of_doubles_(f, dest.emplace_back(), delimeter);
        }
        f.close();
    } else {
        std::cerr << "Could not open " << file << std::endl;
    }
}
