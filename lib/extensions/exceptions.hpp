#ifndef EXCEPTION_HPP
#define EXCEPTION_HPP

#include <string>
#include <stdexcept>

class NotImplementedException : public std::logic_error {
public:
    NotImplementedException(std::string feature) : std::logic_error("The called feature (" + feature + ") was not implemented.") { }
};

#endif