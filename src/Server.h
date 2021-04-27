#ifndef HE_NN_POC_SERVER_H
#define HE_NN_POC_SERVER_H

#include "seal/seal.h"
#include "Matrix.h"
#include "Client.h"

class Server {
    seal::EncryptionParameters params_;

public:
    explicit Server();

    seal::Ciphertext mul(const matrix<double> &matrix, const seal::Ciphertext &encrypted_vector, double scale,
                         const seal::GaloisKeys &gal_keys);

    const seal::EncryptionParameters &params();
};


#endif //HE_NN_POC_SERVER_H
