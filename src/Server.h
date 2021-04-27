#ifndef HE_NN_POC_SERVER_H
#define HE_NN_POC_SERVER_H

#include "seal/seal.h"
#include "Matrix.h"
#include "Client.h"

class Server {
    seal::EncryptionParameters params_;

public:
    explicit Server();

    const seal::EncryptionParameters &params();

    seal::Ciphertext mul(const matrix<double> &matrix, const seal::Ciphertext &encrypted_vector, double scale,
                         const seal::GaloisKeys &gal_keys, const seal::RelinKeys &relin_keys);
    seal::Ciphertext sigmoid_3(seal::Ciphertext &encrypted_vector, double scale, const seal::RelinKeys &relin_keys);
};


#endif //HE_NN_POC_SERVER_H
