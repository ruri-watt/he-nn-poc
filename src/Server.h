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
        pow(2, 40),
        {60, 40, 40, 40, 40, 40, 60},
};

class Server {
    const double scale_;
    seal::EncryptionParameters params_;

public:
    explicit Server(const Params &params);

    const seal::EncryptionParameters &params();

    double scale() const;

    seal::Ciphertext mul(const matrix<double> &matrix, const seal::Ciphertext &encrypted_vector,
                         const seal::GaloisKeys &gal_keys, const seal::RelinKeys &relin_keys);

    seal::Ciphertext sigmoid_3(seal::Ciphertext &encrypted_vector, const seal::RelinKeys &relin_keys);
};


#endif //HE_NN_POC_SERVER_H
