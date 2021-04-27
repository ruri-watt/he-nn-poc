#include "Server.h"

Server::Server() : params_(seal::scheme_type::ckks) {
    size_t polyModulusDegree = 8192;
    std::vector<int> coeffModulusBitSizes = {60, 40, 40, 60};
    params_.set_poly_modulus_degree(polyModulusDegree);
    params_.set_coeff_modulus(seal::CoeffModulus::Create(polyModulusDegree, coeffModulusBitSizes));
}

seal::Ciphertext Server::mul(const matrix<double> &matrix, const seal::Ciphertext &encrypted_vector, double scale,
                             const seal::GaloisKeys &gal_keys) {
    using namespace std;
    using namespace seal;

    SEALContext ctx(params_);
    CKKSEncoder encoder(ctx);
    Evaluator evaluator(ctx);

    // Convert matrix to Plaintext
    vector<Plaintext> matrix_plaintext(matrix.size());
    for (int i = 0; i < matrix.size(); i++) {
        encoder.encode(matrix[i], scale, matrix_plaintext[i]);
    }

    // Multiply each row of the matrix with the encrypted vector (element-wise) to get a vector of ciphertexts
    vector<Ciphertext> result_ciphertexts(matrix.size());
    for (int i = 0; i < matrix.size(); i++) {
        evaluator.multiply_plain(encrypted_vector, matrix_plaintext[i], result_ciphertexts[i]);
    }

    // Sum the elements in each ciphertext by rotation and elementwise summation and reduce to a single ciphertext
    // (has to be done this way due to the limited number of operations that can be performed on a ciphertext).
    int nr_slots = static_cast<int>(encoder.slot_count());
    Ciphertext rotated;
    vector<double> mask = {1};
    Plaintext mask_plaintext;

    for (auto &ciphertext : result_ciphertexts) {
        // Sum ciphertext elements
        // Resulting ciphertext will have the sum in all slots.
        for (int rot = nr_slots / 2; rot > 0; rot /= 2) {
            rotated = ciphertext;
            evaluator.rotate_vector_inplace(rotated, rot, gal_keys);
            evaluator.add_inplace(ciphertext, rotated);
        }
        // Mask ciphertext result so that the sum only appears in the diagonal positions, and sum to get a single ciphertext
        encoder.encode(mask, scale, mask_plaintext);
        mask[mask.size() - 1] = 0;
        mask.push_back(1);
        evaluator.multiply_plain_inplace(ciphertext, mask_plaintext);
        if (&ciphertext != &result_ciphertexts[0]) {
            evaluator.add_inplace(result_ciphertexts[0], ciphertext);
        }
    }
    return result_ciphertexts[0];
}

const seal::EncryptionParameters &Server::params() {
    return params_;
}
