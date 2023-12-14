#ifndef STR_UTILS_HPP
#define STR_UTILS_HPP

#include <vector>
#include <string>
#include <sstream>
#include <glm/glm.hpp>

using namespace std;
using namespace glm;

vector<string> SplitByWhitespace(istream& stream) {
    vector<string> result;
    string token;
    while (stream >> token) {
        result.push_back(token);
    }
    return result;
}
vector<string> SplitByWhitespace(const string& text) {
    stringstream iss(text);
    return SplitByWhitespace(iss);
}

vector<string> Split(string text, string delimiter, bool dropEmpty = false) {
    vector<string> result;
    size_t pos = 0;
    std::string token;
    while ((pos = text.find(delimiter)) != std::string::npos) {
        token = text.substr(0, pos);
        if (token.length() > 0 || !dropEmpty)
            result.push_back(token);
        text.erase(0, pos + delimiter.length());
        //cout << "Split token \"" << token << "\" ending at position " << pos << ", remaining text: " << text << endl;
    }
    // Append the final element
    if (text.length() > 0 || !dropEmpty)
        result.push_back(text);
    return result;
}
vector<string> Split(const string& text, char delimiter, bool dropEmpty = false) {
    return Split(text, string(1, delimiter), dropEmpty);
}

string MatrixToStr(const mat4& matrix) {
    string s = "[ ";
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            s += to_string(matrix[j][i]);
            s += " ";
        }
        s += "\n  ";
    }
    for (int j = 0; j < 4; j++) {
        s += to_string(matrix[j][3]);
        s += " ";
    }
    s += " ]";
    return s;
}

string VectorToStr(const vec4& vector) {
    string s = "(";
    for (int i = 0; i < 3; i++) {
        s += to_string(vector[i]);
        s += " ";
    }
    s += to_string(vector[3]);
    s += ")";
    return s;
}
string VectorToStr(const vec3& vector) {
    string s = "(";
    for (int i = 0; i < 2; i++) {
        s += to_string(vector[i]);
        s += " ";
    }
    s += to_string(vector[2]);
    s += ")";
    return s;    
}
string VectorToStr(const vec2& vector) {
    string s = "(";
    for (int i = 0; i < 1; i++) {
        s += to_string(vector[i]);
        s += " ";
    }
    s += to_string(vector[1]);
    s += ")";
    return s;    
}

template <typename T>
string VectorToStr(const vector<T>& values, const string& delimiter = " ", string(*stringify)(T) = to_string) {
    if (values.size() == 0) return "()";
    string s = "(";
    for (T v : values) {
        s += stringify(v);
        s += delimiter;
    }
    s.erase(s.size() - delimiter.size(), delimiter.size());
    s += ")";
    return s;
}

string VectorToStr(const vector<string>& values, const string& delimiter = " ") {
    // Note the '+' before the lambda expression, this is some sorcery that turns it into a plain function pointer instead of a closure
    // https://stackoverflow.com/questions/18889028/a-positive-lambda-what-sorcery-is-this
    return VectorToStr(values, delimiter, +[](string s){return '\"' + s + '\"';});
}

istream& NextNonComment(istream& stream, string& nextLine, char commentChar = '#') {
    string line;
    while (getline(stream, line)) {
        if (line.length() == 0) continue;
        if (line[0] == commentChar) continue;
        nextLine = line;
        break;
    }
    return stream;
}

template <typename T>
vector<T> ParseValues(stringstream& lineStream, T(*parse)(string)) {
    vector<string> values = SplitByWhitespace(lineStream);
    vector<T> result;

    for (string s : values) {
        result.push_back(parse(s));
    }

    return result;
}

vector<int> ParseIntegers(stringstream& lineStream) {
    return ParseValues<int>(lineStream, +[](string s){return static_cast<int>(stoi(s));});
}
vector<float> ParseFloats(stringstream& lineStream) {
    return ParseValues<float>(lineStream, +[](string s){return static_cast<float>(stof(s));});
}

inline bool EndsWith(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

// Gets the directory where the specified file is contained, postpended with a slash (/)
string GetParentDirectory(std::string filePath) {
    size_t lastSlashIndex;
    // Try to get the directory with /
    lastSlashIndex = filePath.find_last_of("/");
    // Otherwise, try to interpret \\ as the directory separator
    if (lastSlashIndex == std::string::npos) {lastSlashIndex = filePath.find_last_of("\\");}
    // If neither are present, assume we are in the Working Directory
    if (lastSlashIndex == std::string::npos) {return "./";}
    return filePath.substr(0,lastSlashIndex + 1);
}

string GetPathRelativeToFile(string sourceFile, string relativePath) {
    return (GetParentDirectory(sourceFile) + relativePath);
}

string ToLower(const string& original) {
    string result = "";
    for (int i = 0; i < original.size(); ++i) {
        result += tolower(original.at(i));
    }
    return result;
}
string ToUpper(const string& original) {
    string result = "";
    for (int i = 0; i < original.size(); ++i) {
        result += toupper(original.at(i));
    }
    return result;
}

#endif