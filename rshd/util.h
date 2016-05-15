/*
 * util.h
 *
 *  Created on: Nov 28, 2015
 *      Author: dark_tim
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <string>
#include <string.h>
#include <algorithm>
#include <exception>
#include <map>

// Class of exception that saves errno variable
class annotated_exception : public std::exception {
public:
    annotated_exception() noexcept;
    annotated_exception(annotated_exception const &other) noexcept;
    annotated_exception(std::string tag, std::string message);
    annotated_exception(std::string tag, int errnum);
    virtual ~annotated_exception() noexcept = default;

    virtual const char *what() const noexcept override;
    int get_errno() const;
private:
    static const size_t BUFFER_LENGTH = 64;
    std::string message;
    int errnum;
};

inline std::string to_string(std::string const &message) {
    return message;
}

inline std::string to_string(char *message) {
    return std::string(message);
}

// Function for logging
template<typename S, typename T>
void log(S const &tag, T const &message) {
    using ::to_string;
    using std::to_string;
    printf("%s: %s\n", to_string(tag).c_str(), to_string(message).c_str());
}

void log(annotated_exception const &e);
#endif /* UTIL_H_ */
