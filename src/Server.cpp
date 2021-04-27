#include "Server.h"

Server::Server() : params_(seal::scheme_type::ckks) {
    size_t polyModulusDegree = 16384;
    std::cout << "Max bit count: " << seal::CoeffModulus::MaxBitCount(16384) << std::endl;
    std::vector<int> coeffModulusBitSizes = {60, 40, 40, 40, 40, 40, 60};
    params_.set_poly_modulus_degree(polyModulusDegree);
    params_.set_coeff_modulus(seal::CoeffModulus::Create(polyModulusDegree, coeffModulusBitSizes));
}

const seal::EncryptionParameters &Server::params() {
    return params_;
}

seal::Ciphertext Server::mul(const matrix<double> &matrix, const seal::Ciphertext &encrypted_vector, double scale,
                             const seal::GaloisKeys &gal_keys, const seal::RelinKeys &relin_keys) {
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
        evaluator.relinearize_inplace(result_ciphertexts[i], relin_keys);
        evaluator.rescale_to_next_inplace(result_ciphertexts[i]);
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
        evaluator.mod_switch_to_inplace(mask_plaintext, ciphertext.parms_id());
        evaluator.multiply_plain_inplace(ciphertext, mask_plaintext);
        evaluator.relinearize_inplace(ciphertext, relin_keys);
        evaluator.rescale_to_next_inplace(ciphertext);
        if (&ciphertext != &result_ciphertexts[0]) {
            evaluator.add_inplace(result_ciphertexts[0], ciphertext);
        }
    }
    return result_ciphertexts[0];
}

seal::Ciphertext Server::sigmoid_3(seal::Ciphertext &encrypted_vector, double scale, const seal::RelinKeys &relin_keys) {
    // 0.5 + 1.20096(x/8) - 0.81562(x/8)^3
    using namespace seal;
    Ciphertext result;
    SEALContext ctx(params_);
    CKKSEncoder encoder(ctx);
    Evaluator evaluator(ctx);

    Plaintext one_eighth, one_half, c1, c3;
    encoder.encode(0.125, scale,one_eighth);
    encoder.encode(0.5, scale,one_half);
    encoder.encode(1.20096, scale,c1);
    encoder.encode(-0.81562, scale,c3);

    Ciphertext x_div_8, x_div_8_sq, c1_x_div_8, c3_x_div_8, c3_x_div_8_cubed;

    evaluator.mod_switch_to_inplace(one_eighth, encrypted_vector.parms_id());
    evaluator.multiply_plain(encrypted_vector, one_eighth, x_div_8);
    evaluator.relinearize_inplace(x_div_8, relin_keys);
    evaluator.rescale_to_next_inplace(x_div_8);

    evaluator.square(x_div_8, x_div_8_sq);
    evaluator.relinearize_inplace(x_div_8_sq, relin_keys);
    evaluator.rescale_to_next_inplace(x_div_8_sq);

    evaluator.mod_switch_to_inplace(c1, x_div_8.parms_id());
    evaluator.multiply_plain(x_div_8, c1, c1_x_div_8);
    evaluator.relinearize_inplace(c1_x_div_8, relin_keys);
    evaluator.rescale_to_next_inplace(c1_x_div_8);

    evaluator.mod_switch_to_inplace(c3, x_div_8.parms_id());
    evaluator.multiply_plain(x_div_8, c3, c3_x_div_8);
    evaluator.relinearize_inplace(c3_x_div_8, relin_keys);
    evaluator.rescale_to_next_inplace(c3_x_div_8);

    evaluator.multiply(c3_x_div_8, x_div_8_sq, c3_x_div_8_cubed);
    evaluator.relinearize_inplace(c3_x_div_8_cubed, relin_keys);
    evaluator.rescale_to_next_inplace(c3_x_div_8_cubed);

    evaluator.mod_switch_to_inplace(one_half, c1_x_div_8.parms_id());
    c1_x_div_8.scale() = scale;
    evaluator.add_plain(c1_x_div_8, one_half, result);

    evaluator.mod_switch_to_inplace(result, c3_x_div_8_cubed.parms_id());
    c3_x_div_8_cubed.scale() = scale;
    evaluator.add_inplace(result, c3_x_div_8_cubed);

    return result;
}

