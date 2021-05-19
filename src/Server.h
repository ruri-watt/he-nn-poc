#ifndef HE_NN_POC_SERVER_H
#define HE_NN_POC_SERVER_H

#include "seal/seal.h"
#include "Matrix.h"
#include "Client.h"

struct Params {
    const size_t polyModulusDegree;
    const double scale;
    const std::vector<int> coeffModulusBitSizes;
};

const Params params_1 = {
        8192,
        pow(2, 28),
        {39, 28, 28, 28, 28, 28, 39},
};

const Params params_2 = {
        16384,
        pow(2, 28),
        {39, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 39},
};

const Params params_3 = {
	 	16384,
        pow(2, 20),
        {30, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 30},
};

const Params params_4 = {
        32768,
        pow(2, 40),
        {55, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 55},
};

const Params params_5 = {
        32768,
        pow(2, 43),
        {60, 43, 43, 43, 43, 43, 60},
};


class Server {
    const double scale_;
    seal::EncryptionParameters params_;

public:
    explicit Server(const Params &params);

    const seal::EncryptionParameters &params();

    double scale() const;
	seal::Ciphertext evaluate(const matrix<double> &matrix0, const matrix<double> &matrix1,
									const matrix<double> &matrix2, const seal::Ciphertext &encrypted_vector,
                             const seal::GaloisKeys &gal_keys, const seal::RelinKeys &relin_keys);

    seal::Ciphertext mul(const matrix<double> &matrix, const seal::Ciphertext &encrypted_vector,
                         const seal::GaloisKeys &gal_keys, const seal::RelinKeys &relin_keys);

    seal::Ciphertext sigmoid_3(seal::Ciphertext &encrypted_vector, const seal::RelinKeys &relin_keys);



};


#endif //HE_NN_POC_SERVER_H
