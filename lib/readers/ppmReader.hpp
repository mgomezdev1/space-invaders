#ifndef PPM_READER_HPP
#define PPM_READER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <stdint.h>
#include <string>

using namespace std;

#include "../extensions/strUtils.hpp"
#include "../texture.hpp"

vector<int> ReadInts(istream& stream) {
    string line;
    vector<int> result;
    int n;
    while (NextNonComment(stream, line)) {
        // This will allow us to process each 'word' (space-separated token) separately
        istringstream ss(line);
        while (!ss.eof()) {
            // Try to push the next 'word' into the integer variable
            if (ss >> n) {
                result.push_back(n);
            }
        }
    }
    return result;
}

class PpmReader {
public:
    static Image ReadPpm(const string& filename, bool verbose = false) {
        ifstream in;
        in.open(filename);
        Image result;

        string line;
        // Process the magic number
        while (NextNonComment(in, line)) {
            if (line == "P3") break;
            cerr << "Received incompatible magic number code: " << line << ". Aborting..." << endl;
            throw 1;
        }

        if(verbose) cout << "PPM > Detected P3 signature" << endl;

        auto values = ReadInts(in);
        in.close();

        if(verbose) cout << "PPM > Finished reading all numbers" << endl;
        //for (int n : values) {
        //    cout << n << " ";
        //}
        //cout << endl;

        if (values.size() < 3) {
            cerr << "Unable to process PPM files without at least width, height, and range provided. Aborting..." << endl;
            throw 1;
        }

        int i = 0;
        // Process dimensions
        result.width = values.at(i++);
        result.height = values.at(i++);
        // Process range
        int range = values.at(i++);
        if (range > 255) {
            cerr << "Unable to process PPM files with a range higher than 255, received range " << range << ". Aborting..." << endl;
            throw 1;
        }

        if(verbose) cout << "PPM > Parsed width, height, and range, (" << result.width << "x" << result.height << ") r=" << range << endl;

        // We will use this factor to normalize our pixel data to the 0-256 (incl. excl.) range.
        float normFactor = 256/(range+1);

        // Process RGB value sequence
        int value;
        vector<uint8_t> rawBytes;
        while (i < values.size()) {
            value = values.at(i++);
            if (value < 0 || value > range) {
                cerr << "WARN: Ignoring value outside of specified range while reading RGB values: " << value << endl;
                continue;
            }
            rawBytes.push_back((int)(value * normFactor));
        }

        if (rawBytes.size() != 3 * result.height * result.width) {
            cerr << "WARN: PPM dimensions (" << result.width << "x" << result.height 
            << ") and the number of pixel bytes (" << rawBytes.size() 
            << ") do not match, expected to receive " << 3 * result.height * result.width << " values" << endl;
        }

        for (i = 0; i < rawBytes.size() - 2; i += 3) {
            result.pixels.push_back(Pixel(rawBytes.at(i),rawBytes.at(i+1),rawBytes.at(i+2)));
        }
        return result;

        //cout << "Finished processing file " << fileName << ", parsed " << m_pixelData.size() << " rgb bytes worth of data." << endl;
    }
    
    static void SavePPM(const Image& img, const string& outputFileName) {
        ofstream file;
        file.open(outputFileName);

        file << "P3" << endl;
        file << img.width << " " << img.height << endl;
        file << 255 << endl;

        for (Pixel p : img.pixels) {
            file << p.r << " " << p.g << " " << p.b << endl;
        }
        file.close();
    }
};

#endif