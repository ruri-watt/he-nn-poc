#include "Client.h"

Client::Client(const seal::EncryptionParameters &params) : seal_context_(params), keygen_(seal_context_),
                                                           encoder_(seal_context_) {
    keygen_.create_public_key(public_key_);
    keygen_.create_relin_keys(relin_keys_);
    keygen_.create_galois_keys(gal_keys_);
}

const seal::PublicKey &Client::public_key() const {
    return public_key_;
}

const seal::GaloisKeys &Client::gal_keys() const {
    return gal_keys_;
}

seal::Ciphertext Client::encrypt(const std::vector<double> &data, double scale) {
    seal::Ciphertext result;
    seal::Plaintext plaintext;
    encoder_.encode(data, scale, plaintext);
    seal::Encryptor(seal_context_, public_key_)
            .encrypt(plaintext, result);
    return result;
}

std::vector<double> Client::decrypt(const seal::Ciphertext &data) {
    std::vector<double> result;
    seal::Plaintext plaintext;
    seal::Decryptor(seal_context_, keygen_.secret_key())
            .decrypt(data, plaintext);
    encoder_.decode(plaintext, result);
    return result;
}
