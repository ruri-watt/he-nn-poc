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

void file_utils::read_matrix(std::ifstream &f, std::vector<std::vector<double>> &dest, char delimeter) {
	if(f.is_open()) {
		while (f.peek() != EOF && f.peek() != '\n'){
			read_line_of_doubles_(f, dest.emplace_back(), delimeter);
		}
	} else {
		std::cerr << "Could not open " << std::endl;
	}
}

void file_utils::read_doubles_mult(const std::string &file, std::vector<std::vector<double>> matrix_vector[3],
								char delimeter){

	std::ifstream f(file);
	std::string line;

	read_matrix(f, matrix_vector[0]);
	getline(f, line);
	read_matrix(f, matrix_vector[1]);
	getline(f, line);
	read_matrix(f, matrix_vector[2]);
/*	std::ifstream f(file);
	std::vector<double> v; 
    if (f.is_open()) {
		for(int i = 0; i < 3; i++){
			v = matrix_vector[i].emplace_back();
			while(f.peek() != EOF && f.peek() != '\n'){
				
				std::string line;
    			if (!getline(f, line)) {
        			return;
    			}
    			std::string token;
    			for (char c : line) {
        			if (c == delimeter) {
            			v.push_back(stod(token));
           				token = "";
        			} else {
            			token.push_back(c);
        			}
    			}
    			v.push_back(stod(token));
			}
			
		}
		f.close();
    } else {
        std::cerr << "Could not open " << file << std::endl;
    }*/
}