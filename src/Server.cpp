#include "Server.h"

Server::Server(const Params &params) : scale_(params.scale), params_(seal::scheme_type::ckks) {
    params_.set_poly_modulus_degree(params.polyModulusDegree);
    params_.set_coeff_modulus(seal::CoeffModulus::Create(params.polyModulusDegree, params.coeffModulusBitSizes));
}

const seal::EncryptionParameters &Server::params() {
    return params_;
}
seal::Ciphertext Server::evaluate(const matrix<double> &matrix0, const matrix<double> &matrix1,
									const matrix<double> &matrix2, const seal::Ciphertext &encrypted_vector,
                             const seal::GaloisKeys &gal_keys, const seal::RelinKeys &relin_keys){
	using namespace seal;
	SEALContext ctx(params_);
	Evaluator evaluator(ctx);							 

	seal::Ciphertext encrypted_result = mul(matrix0, encrypted_vector, gal_keys, relin_keys);
	encrypted_result = sigmoid_3(encrypted_result, relin_keys);
	
	//encrypted_result = mul(matrix1, encrypted_result, gal_keys, relin_keys);
	//encrypted_result = sigmoid_3(encrypted_result, relin_keys);

	//encrypted_result = mul(matrix2, encrypted_result, gal_keys, relin_keys);

	return encrypted_result;
							
	}
seal::Ciphertext Server::mul(const matrix<double> &matrix, const seal::Ciphertext &encrypted_vector,
                             const seal::GaloisKeys &gal_keys, const seal::RelinKeys &relin_keys) {
    using namespace std;
    using namespace seal;

    SEALContext ctx(params_);
    CKKSEncoder encoder(ctx);
    Evaluator evaluator(ctx);

    // Convert matrix to Plaintext
    vector<Plaintext> matrix_plaintext(matrix.size());
    for (int i = 0; i < matrix.size(); i++) {
        encoder.encode(matrix[i], scale_, matrix_plaintext[i]);
		evaluator.mod_switch_to_inplace(matrix_plaintext[i], encrypted_vector.parms_id());
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
        encoder.encode(mask, scale_, mask_plaintext);
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

seal::Ciphertext
Server::sigmoid_3(seal::Ciphertext &encrypted_vector, const seal::RelinKeys &relin_keys) {
    // 0.5 + 1.20096(x/8) - 0.81562(x/8)^3
    using namespace seal;
    Ciphertext result;
    SEALContext ctx(params_);
    CKKSEncoder encoder(ctx);
    Evaluator evaluator(ctx);

    Plaintext one_eighth, one_half, c1, c3;
    encoder.encode(0.125, scale_, one_eighth);
    encoder.encode(0.5, scale_, one_half);
    encoder.encode(1.20096, scale_, c1);
    encoder.encode(-0.81562, scale_, c3);

    Ciphertext x_div_8, x_div_8_sq, c1_x_div_8, c3_x_div_8, c3_x_div_8_cubed;
	//x_div_8 = (x/8)
    evaluator.mod_switch_to_inplace(one_eighth, encrypted_vector.parms_id());
    evaluator.multiply_plain(encrypted_vector, one_eighth, x_div_8);
    evaluator.relinearize_inplace(x_div_8, relin_keys);
    evaluator.rescale_to_next_inplace(x_div_8);
	//x_div_8_sq = (x/8)^2
    evaluator.square(x_div_8, x_div_8_sq);
    evaluator.relinearize_inplace(x_div_8_sq, relin_keys);
	evaluator.rescale_to_next_inplace(x_div_8_sq);
	//c1_x_div_8 = 1,2 * (x/8)
    evaluator.mod_switch_to_inplace(c1, x_div_8.parms_id());
    evaluator.multiply_plain(x_div_8, c1, c1_x_div_8);
    evaluator.relinearize_inplace(c1_x_div_8, relin_keys);
    evaluator.rescale_to_next_inplace(c1_x_div_8);
	//c3_x_div_8 = -0.81562 *(x/8)
    evaluator.mod_switch_to_inplace(c3, x_div_8.parms_id());
    evaluator.multiply_plain(x_div_8, c3, c3_x_div_8);
    evaluator.relinearize_inplace(c3_x_div_8, relin_keys);
    evaluator.rescale_to_next_inplace(c3_x_div_8);
	//c3_x_div_cubed = -0.81562 * (x/8) * (x/8)^2
    evaluator.multiply(c3_x_div_8, x_div_8_sq, c3_x_div_8_cubed);
    evaluator.relinearize_inplace(c3_x_div_8_cubed, relin_keys);
    evaluator.rescale_to_next_inplace(c3_x_div_8_cubed);
	//result = 0.5 + 1.2 * (x/8)
    evaluator.mod_switch_to_inplace(one_half, c1_x_div_8.parms_id());
    c1_x_div_8.scale() = scale_;
  	evaluator.add_plain(c1_x_div_8, one_half, result);
	//result = 0.5 + 1.2 * (x/8) - 0.81562 * (x/8)^3
    evaluator.mod_switch_to_inplace(result, c3_x_div_8_cubed.parms_id());
    c3_x_div_8_cubed.scale() = scale_;
    evaluator.add_inplace(result, c3_x_div_8_cubed);

    return result;
}

double Server::scale() const {
    return scale_;
}

