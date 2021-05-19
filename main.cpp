#include <iostream>

#include "src/file_utils.h"
#include "src/Client.h"
#include "src/Server.h"

#include <string>

const std::string IMAGE_FILE = "../image.txt";
const std::string WEIGHTS_FILE = "../weights2.txt";

std::ostream &operator<<(std::ostream &os, const std::vector<double> &v) {
    for (const auto d : v) {
        os << d << " ";
    }
    return os;
}
void unenc_sigmoid_3(std::vector<double> &v) {
	//0.5 + 1.20096(x/8) - 0.81562(x/8)^3
	for(int i = 0; i < v.size(); i++){
		v[i] = 0.5 + 1.20096*(v[i]/8) - 0.81562* pow((v[i]/8), 3);
	}
	
}
int main() {

    std::vector<double> image;
    file_utils::read_doubles(IMAGE_FILE, image);

   // matrix<double> weights0, weights1, weights2;
	matrix<double> matrix_arr[3];


    //file_utils::read_doubles(WEIGHTS_FILE, weights);
	file_utils::read_doubles_mult(WEIGHTS_FILE, matrix_arr);


	std::cout << "matrix_arr[0].size(): " << matrix_arr[0].size() << '\n';
	std::cout << "matrix_arr[0][0].size(): " <<matrix_arr[0][0].size() << '\n';

	std::cout << "matrix_arr[1].size(): " <<matrix_arr[1].size() << '\n';
	std::cout << "matrix_arr[1][0].size(): " <<matrix_arr[1][0].size() << '\n';

	std::cout << "matrix_arr[2].size(): " <<matrix_arr[2].size() << '\n';
	std::cout << "matrix_arr[2][0].size(): " <<matrix_arr[2][0].size() << '\n';

	std::cout << "image.size(): " << image.size() << '\n';

    std::vector<double> expected_result = matrix_arr[0] * image;
	unenc_sigmoid_3(expected_result);
	//expected_result = matrix_arr[1] * expected_result;
	//unenc_sigmoid_3(expected_result);
	//expected_result = matrix_arr[2] * expected_result;

	std::cout << "\nExpected_result size: " << expected_result.size() << "\n";
	/*std::cout << "\nExpected_result size: " << expected_result.size();
	expected_result = matrix_arr[1] * expected_result;
	std::cout << "\nExpected_result size: " << expected_result.size();
	unenc_sigmoid_3(expected_result);
	expected_result = matrix_arr[2] * expected_result;
*/
	//for(int i = 0; i < expected_result.size(); i++){
	//	std::cout << expected_result[i] << '\n';
	//}

	
    Server server(params_6);
    Client client(server.params(), server.scale());

    seal::Ciphertext image_ciphertext = client.encrypt(image);

    seal::Ciphertext encrypted_result = server.evaluate(matrix_arr[0], matrix_arr[1], matrix_arr[2], image_ciphertext, client.gal_keys(),
                                                   client.relin_keys());

    std::vector<double> result = client.decrypt(encrypted_result);
	result.resize(expected_result.size());

	double sum;
	for(int i = 0; i < expected_result.size(); i++){
		std::cout << result[i] << " - " << expected_result[i] << " = " << result[i] - expected_result[i] << "\n";
		sum += abs(result[i] - expected_result[i]);
	}

	std::cout << "\nsum: " << sum;
	std::cout << "\nresult.size(): " << result.size();
	std::cout << "\nAverage error\n" << abs(sum/result.size());

    // Remove trailing zeros from result.
    
/*
    std::cout << "Expected result:" << std::endl;
    std::cout << expected_result;
    std::cout << std::endl;

    std::cout << "Decrypted result:" << std::endl;
    std::cout << result;
    std::cout << std::endl;

    std::vector<double> error;
    for (int i = 0; i < expected_result.size(); i++) {
        error.push_back(result[i] - expected_result[i]);
    }

    double max_error = *std::max_element(error.cbegin(), error.cend());
    std::cout << "Max. error:" << std::endl;
    std::cout << max_error;
    std::cout << std::endl;

    encrypted_result = server.sigmoid_3(encrypted_result, client.relin_keys());
    result = client.decrypt(encrypted_result);
    result.resize(weights.size());
    std::cout << "Decrypted result after activation function:" << std::endl;
    std::cout << result;
    std::cout << std::endl;
	*/
    return 0;
}

