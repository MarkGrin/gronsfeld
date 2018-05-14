#include "gronsfeld.h"
#include <QDebug>
#include <set>
#include <cctype>
#include <type_traits>
#include <memory>
#include <cstring>

namespace {

using ShiftFuncPtr = std::add_pointer<unsigned (unsigned, unsigned, unsigned)>::type;
class Shifter {
    const gronsfeld::Alphabet& alphabet;
    const gronsfeld::Mode::Mode mode;
    const unsigned char* key;
    std::size_t keySize;
    std::size_t position = 0;
    ShiftFuncPtr shiftFunc;
    Shifter () = delete;

public:

    Shifter (const gronsfeld::Alphabet& alpha, const unsigned char* key_, std::size_t keySize_, gronsfeld::Mode::Mode mode_, ShiftFuncPtr shift)
        : alphabet(alpha), mode(mode_), key(key_), keySize(keySize_), shiftFunc(shift) {
        if (alphabet.first.size() != alphabet.second.size())
            throw std::logic_error ("invalid alphabet");
        if (!key || !keySize)
            throw std::logic_error ("invalid key");
    }


    bool shift (char* from, char* to) {
        auto inputIt = alphabet.first.find(*from);
        if (inputIt == alphabet.first.end()) {
            if (mode == gronsfeld::Mode::ERROR)
                throw std::logic_error ((std::isprint(*from) ? std::string{*from}
                                                          : "(" + std::to_string(*from + 0u) + ")")
                                    + std::string{" is not in input alphabet"});
            else if (mode == gronsfeld::Mode::MOVE) {
                *to = *from;
                return true;
            }
            else
                return false;
        }

        auto newKey = shiftFunc(inputIt->second, key[position], alphabet.first.size());

        auto outputIt = alphabet.second.find(newKey);
        if (outputIt == alphabet.second.end()) {
            if (mode == gronsfeld::Mode::ERROR)
                throw std::logic_error (std::to_string(newKey) + std::string{" is not in output alphabet"});
            else if (mode == gronsfeld::Mode::DELETE)
                return false;
            else {
                *to = *from;
                return true;
            }
        }

        *to = outputIt->second;
        position = (position + 1) % keySize;
        return true;
    }

    std::size_t shift (char* from, std::size_t size) {
        char* to = from;
        for (; size; --size) {
            if ( shift(from, to) )
                to++;
            from++;
        }
        *to = 0;
        return from - to;
    }
};

void byteShift(const gronsfeld::Alphabet &alphabet, const unsigned char *key, std::size_t keySize, char *input,
               std::size_t inputSize, gronsfeld::Mode::Mode mode, ShiftFuncPtr ptr) {
    Shifter (alphabet, key, keySize, mode, ptr).shift(input, inputSize);
}

void fileByteShift(const gronsfeld::Alphabet &alphabet, const unsigned char *key, std::size_t keySize,
                   std::fstream& in, std::fstream& out, gronsfeld::Mode::Mode mode, ShiftFuncPtr ptr) {
    Shifter shifter (alphabet, key, keySize, mode, ptr);
    while (in) {
        char byte = 0;
        if ( !(in.read(&byte, 1)) )
            return ;
        if (byte=='\n')
            return ;
        if ( shifter.shift(&byte, &byte) ) {
            if ( !(out.write(&byte,1)) )
                throw std::logic_error("error on write");
        }
    }
}

std::pair<std::unique_ptr<unsigned char>,std::size_t> getKey (std::fstream& file) {
    std::string key;
    char input = 0;
    file >> input;
    while (input != ',' && input != '\n' && file) {
        qDebug () << input << "\n";
        if (!std::isdigit(input))
            throw std::logic_error("Non digit key");

        key.push_back(input - '0');

        if (!(file>>input))
            throw std::logic_error("Have key but no text");
    }
    if (key.empty())
        return {nullptr, 0};
    std::unique_ptr<unsigned char> result(new unsigned char[key.size()]);
    std::memcpy(result.get(), key.data(), key.size());
    qDebug() << "End:" << key.size() << "\n";
    return std::make_pair(std::move(result), key.size());
}

void fileCrypt(const gronsfeld::Alphabet &alphabet,
                   std::fstream& in, std::fstream& out, gronsfeld::Mode::Mode mode, ShiftFuncPtr ptr) {
    while(true) {
        if (!in)
            return ;
        auto key = getKey(in);
        if (!key.first || !in)
            return ;
        for (std::size_t i = 0; i < key.second; i++)
            out << static_cast<unsigned int>(key.first.get()[i]);
        out << ",";
        fileByteShift(alphabet, key.first.get(), key.second, in, out, mode, ptr);
        out << "\n";
    }
}


} // namespace
namespace gronsfeld {

void encrypt(const Alphabet &alphabet, const unsigned char *key, std::size_t keySize,
             char *input, std::size_t inputSize, Mode::Mode mode)
{
    byteShift(alphabet, key, keySize, input, inputSize, mode,
              [](unsigned x, unsigned k, unsigned m) {return (k + x) % m;});
}

void decrypt(const Alphabet &alphabet, const unsigned char *key, std::size_t keySize,
             char *input, std::size_t inputSize, Mode::Mode mode)
{
    byteShift(alphabet, key, keySize, input, inputSize, mode,
              [](unsigned x, unsigned k, unsigned m) {return (m - k + x) % m;});
}

void encrypt (const Alphabet& alphabet,
              std::fstream& input, std::fstream& output, Mode::Mode mode) {
    fileCrypt(alphabet, input, output, mode,
              [](unsigned x, unsigned k, unsigned m) {return (k + x) % m;});
}

void decrypt (const Alphabet& alphabet,
              std::fstream& input, std::fstream& output, Mode::Mode mode) {
    fileCrypt(alphabet, input, output, mode,
              [](unsigned x, unsigned k, unsigned m) {return (m - k + x) % m;});
}

Alphabet latinAlphabet()
{
    Alphabet alphabet;
    alphabet.first['a'] = 0;
    alphabet.first['b'] = 1;
    alphabet.first['c'] = 2;
    alphabet.first['d'] = 3;
    alphabet.first['e'] = 4;
    alphabet.first['f'] = 5;
    alphabet.first['g'] = 6;
    alphabet.first['h'] = 7;
    alphabet.first['i'] = 8;
    alphabet.first['j'] = 9;
    alphabet.first['k'] = 10;
    alphabet.first['l'] = 11;
    alphabet.first['m'] = 12;
    alphabet.first['n'] = 13;
    alphabet.first['o'] = 14;
    alphabet.first['p'] = 15;
    alphabet.first['q'] = 16;
    alphabet.first['r'] = 17;
    alphabet.first['s'] = 18;
    alphabet.first['t'] = 19;
    alphabet.first['u'] = 20;
    alphabet.first['v'] = 21;
    alphabet.first['w'] = 22;
    alphabet.first['x'] = 23;
    alphabet.first['y'] = 24;
    alphabet.first['z'] = 25;
    for (auto pair : alphabet.first)
        alphabet.second[pair.second] = pair.first;
    return alphabet;
}

Alphabet loadAlphabet(std::fstream& file) {
    Alphabet result;
    char input = 0;
    unsigned int count = 0;
    while (file.read(&input, 1)) {
        if (input == '\n')
            continue;
        result.second[count] = input;
        count++;
    }
    for (auto& pair : result.second) {
        result.first[pair.second] = pair.first;
    }
    return result;
}


} // gronsfeld
