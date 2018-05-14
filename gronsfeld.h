#ifndef GRONSFELD_H
#define GRONSFELD_H

#include <map>
#include <fstream>

namespace gronsfeld {

namespace Mode {
enum Mode {
    ERROR,
    MOVE,
    DELETE
};
} // Mode

using Alphabet=std::pair<std::map<char, unsigned>,std::map<unsigned, char>>;

void encrypt (const Alphabet& alphabet, const unsigned char* key, std::size_t keySize,
              char* input, std::size_t inputSize, Mode::Mode mode);

void decrypt (const Alphabet& alphabet, const unsigned char* key, std::size_t keySize,
              char* input, std::size_t inputSize, Mode::Mode mode);


void encrypt (const Alphabet& alphabet, const unsigned char* key, std::size_t keySize,
              std::fstream& input, std::fstream& output, Mode::Mode mode);

void decrypt (const Alphabet& alphabet, const unsigned char* key, std::size_t keySize,
              std::fstream& input, std::fstream& output, Mode::Mode mode);


Alphabet latinAlphabet ();
Alphabet latinExtendedAlphabet ();
Alphabet allAlphabet ();

} // gronsfeld

#endif // GRONSFELD_H
