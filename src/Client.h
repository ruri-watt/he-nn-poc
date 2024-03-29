#ifndef HE_NN_POC_CLIENT_H
#define HE_NN_POC_CLIENT_H


#include <cstddef>
#include "seal/seal.h"

class Client {
    const double scale_;
    const seal::SEALContext seal_context_;
    seal::KeyGenerator keygen_;
    seal::PublicKey public_key_;
    seal::RelinKeys relin_keys_;
    seal::GaloisKeys gal_keys_;
    seal::CKKSEncoder encoder_;

public:
    explicit Client(const seal::EncryptionParameters &params, double scale);

    const seal::PublicKey &public_key() const;

    const seal::GaloisKeys &gal_keys() const;

    const seal::RelinKeys &relin_keys();

    seal::Ciphertext encrypt(const std::vector<double> &data);

    std::vector<double> decrypt(const seal::Ciphertext &data);
};


#endif //HE_NN_POC_CLIENT_H
