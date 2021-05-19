#ifndef HE_NN_POC_FILE_UTILS_H
#define HE_NN_POC_FILE_UTILS_H


#include <vector>
#include <string>

namespace file_utils {
    void read_doubles(const std::string &file, std::vector<double> &dest, char delimeter = '\t');
    void read_doubles(const std::string &file, std::vector<std::vector<double>> &dest, char delimeter = '\t');
	void read_matrix(std::ifstream &f, std::vector<std::vector<double>> &dest, char delimeter = '\t');
	void read_doubles_mult(const std::string &file, std::vector<std::vector<double>> matrix_vector[3],
								char delimeter = '\t');
};


#endif //HE_NN_POC_FILE_UTILS_H
