/*
 * buffered_message.h
 *
 *  Created on: Dec 19, 2015
 *      Author: dark_tim
 */

#ifndef BUFFERED_MESSAGE_H_
#define BUFFERED_MESSAGE_H_

#include <string>
#include <algorithm>

#include "wraps.h"

// Struct for messages with unlimited length
struct raw_message {
    raw_message();
    raw_message(const char* initial);
    raw_message(raw_message const& other);
    raw_message(raw_message&& other);

    raw_message &operator=(raw_message other);

    // Can we write or read in this message
    bool can_read() const;
    bool can_write() const;

    // Read or Write
    void read_from(file_descriptor const& fd);
    void write_to(file_descriptor const& fd);

    friend void swap(raw_message& first, raw_message& second);
private:
    static const size_t BUFFER_LENGTH = 8 * 1024;

    size_t read_length, write_length;
    char buffer[BUFFER_LENGTH];
};

#endif /* BUFFERED_MESSAGE_H_ */


