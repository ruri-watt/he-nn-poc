#include <iostream>

#include "src/file_utils.h"
#include "src/Client.h"
#include "src/Server.h"

#include <string>

const std::string IMAGE_FILE = "../image.txt";
const std::string WEIGHTS_FILE = "../weights.txt";

std::ostream &operator<<(std::ostream &os, const std::vector<double> &v) {
    for (const auto d : v) {
        os << d << " ";
    }
    return os;
}

int main() {

    std::vector<double> image;
    file_utils::read_doubles(IMAGE_FILE, image);

    matrix<double> weights;
    file_utils::read_doubles(WEIGHTS_FILE, weights);

    std::vector<double> expected_result = weights * image;

    Server server(params_1);
    Client client(server.params(), server.scale());

    seal::Ciphertext image_ciphertext = client.encrypt(image);

    seal::Ciphertext encrypted_result = server.mul(weights, image_ciphertext, client.gal_keys(),
                                                   client.relin_keys());

    std::vector<double> result = client.decrypt(encrypted_result);
    // Remove trailing zeros from result.
    result.resize(weights.size());

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
    return 0;
}

