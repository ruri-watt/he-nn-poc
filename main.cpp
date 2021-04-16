#include <iostream>

#include "seal/seal.h"
#include "src/Matrix.h"

#include <iostream>
#include <fstream>
#include <string>

using namespace std;
using namespace seal;

const string IMAGE_FILE = "../image.txt";
const string WEIGHTS_FILE = "../weights.txt";

const size_t POLY_MODULUS_DEGREE = 8192;
const vector<int> COEFF_MODULUS_BIT_SIZES = {60, 40, 40, 60};
const double SCALE = pow(2, 40);


std::ostream &operator<<(std::ostream &os, const std::vector<double> &v) {
    for (const auto d : v) {
        os << d << " ";
    }
    return os;
}

void read_image(string file, vector<double> &dest) {
    ifstream image_file(file);
    if (image_file.is_open()) {
        string line;
        while (getline(image_file, line)) {
            string token;
            for (char c : line) {
                if (c == '\t') {
                    dest.push_back(stod(token));
                    token = "";
                } else {
                    token.push_back(c);
                }
            }
            dest.push_back(stod(token));
        }
        image_file.close();
    } else {
        cout << "could not open " << IMAGE_FILE << endl;
    }
}

void read_weights(string file, vector<vector<double>> &dest) {
    ifstream image_file(file);
    if (image_file.is_open()) {
        string line;
        while (getline(image_file, line)) {
            auto &row = dest.emplace_back();
            string token;
            for (char c : line) {
                if (c == '\t') {
                    row.push_back(stod(token));
                    token = "";
                } else {
                    token.push_back(c);
                }
            }
            row.push_back(stod(token));
        }
        image_file.close();
    } else {
        cout << "could not open " << IMAGE_FILE << endl;
    }
}

int main() {

    vector<double> image;

    read_image(IMAGE_FILE, image);

    matrix<double> weights;
    read_weights(WEIGHTS_FILE, weights);

    cout << "image: " << image.size() << endl;
    cout << "weights: " << weights.size() << "x" << weights[0].size() << endl;

    cout << "Expected result: \n" << weights * image << endl;

    // Encryption parameters
    EncryptionParameters parms(scheme_type::ckks);
    parms.set_poly_modulus_degree(POLY_MODULUS_DEGREE);
    parms.set_coeff_modulus(CoeffModulus::Create(POLY_MODULUS_DEGREE, COEFF_MODULUS_BIT_SIZES));

    SEALContext context(parms);

    KeyGenerator keygen(context);

    // Keys
    SecretKey secret_key;
    PublicKey public_key;
    RelinKeys relin_keys;
    GaloisKeys gal_keys;

    secret_key = keygen.secret_key();
    keygen.create_public_key(public_key);
    keygen.create_relin_keys(relin_keys);
    keygen.create_galois_keys(gal_keys);

    CKKSEncoder encoder(context);
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);

    // Encrypt image
    Plaintext image_plaintext;
    Ciphertext image_ciphertext;
    encoder.encode(image, SCALE, image_plaintext);
    encryptor.encrypt(image_plaintext, image_ciphertext);

    // Convert weights to Plaintext
    vector<Plaintext> weights_plaintext(weights.size());
    for (int i = 0; i < weights.size(); i++) {
        encoder.encode(weights[i], SCALE, weights_plaintext[i]);
    }

    // Multiply the node weights with the image pixels element-wise to get a vector of ciphertexts
    // (a ciphertext equates to an encrypted vector of doubles).
    vector<Ciphertext> result_ciphertexts(weights.size());
    cout << "nr nodes " << weights.size() << endl;
    for (int i = 0; i < weights.size(); i++) {
        evaluator.multiply_plain(image_ciphertext, weights_plaintext[i], result_ciphertexts[i]);
    }

    // Sum the elements in each ciphertext by rotation and elementwise summation and reduce to a single ciphertext
    // (has to be done this way due to the limited number of operations that can be performed on a ciphertext).
    int nr_slots = static_cast<int>(encoder.slot_count());
    cout << "nr slots: " << nr_slots << endl;
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
        encoder.encode(mask, SCALE, mask_plaintext);
        mask[mask.size() - 1] = 0;
        mask.push_back(1);
        evaluator.multiply_plain_inplace(ciphertext, mask_plaintext);
        if (&ciphertext != &result_ciphertexts[0]) {
            evaluator.add_inplace(result_ciphertexts[0], ciphertext);
        }
    }
    cout << "Memory footprint: " << ((double) MemoryManager::GetPool().alloc_byte_count()) / (1024 * 1024) << "MB"
         << endl;

    // Now we are back at the client and want to check the result
    // Decrypt ciphertext result to Plaintext
    Plaintext result_plaintext;
    decryptor.decrypt(result_ciphertexts[0], result_plaintext);

    vector<double> result;
    encoder.decode(result_plaintext, result);
    // Remove trailing zeros from result.
    result.resize(weights.size());

    cout << "Decrypted result: \n";
    cout << result;
    cout << endl;

    return 0;
}

