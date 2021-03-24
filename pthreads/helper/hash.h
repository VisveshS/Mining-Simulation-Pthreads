#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <openssl/sha.h>

using namespace std;

string sha256(string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    // cout << "Inner:: ";
    // for (int i = 0; i< SHA256_DIGEST_LENGTH; i++)
    //     printf("%02x",hash[i]);
    // printf("\n");
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

string gethash(int nonce, string prevhash, int mr, int pubkey) {
    string concat_str = std::to_string(nonce) + prevhash + std::to_string(mr) \
                            + std::to_string(pubkey);
    string hash = sha256(concat_str);
    return hash;
}
