//
// Created by Xule Zhou on 6/4/18.
//

#include "tlv.h"
#include <cstring>

tlv8_item *tlv8_parse(uint8_t * data, unsigned int length) {
    const uint8_t * end = data + length;
    auto current_item = new tlv8_item();
    auto start_item = current_item;

    while (data < end) {
        current_item->type = static_cast<tlv8_type>(data[0]);
        current_item->length = data[1];
        current_item->value = &data[2];
        current_item->offset = 0;

        data += current_item->length + 2;

        if(end - data > 0){
            current_item->next = new tlv8_item();
            current_item->next->previous = current_item;
            current_item = current_item->next;
        }
    }

    return start_item;
}

void tlv8_free(tlv8_item *chain) {
    tlv8_item * current;

    //Gets to the first item
    while (chain->previous != nullptr){
        chain = chain->previous;
    }

    while (chain != nullptr){
        current = chain;
        chain = current->next;
        delete current;
    }
}

tlv8_item *tlv8_find(tlv8_item *chain, tlv8_type type) {
    //Start searching from the start of the chain
    while (chain->previous != nullptr){
        chain = chain->previous;
    }

    return tlv8_find_next(chain, type);
}

tlv8_item *tlv8_find_next(tlv8_item *chain, tlv8_type type) {
    while (chain != nullptr && chain->type != type){
        chain = chain->next;
    }
    return chain;
}

unsigned int tlv8_read(tlv8_item *item, uint8_t *buffer, unsigned int length) {
    unsigned int read = 0;

    //Get to the place where it has not been read
    while(item != nullptr && item->offset == 255){
        //If the next fragment does not have the same type as the current one, abort with 0 bytes read
        if(item->next->type != item->type) { return read; }
        item = item->next;
    }

    while(item != nullptr && read < length){
        unsigned int need_read = length - read;
        unsigned int can_read = item->length - item->offset;
        unsigned int to_read = can_read < need_read ? can_read : need_read;

        memcpy(buffer + read, item->value + item->offset, to_read);
        read += to_read;
        item->offset += to_read;

        //Jump to the next fragment
        if(item->offset >= 255){
            //Return if the next item is not of the same type
            if(item->next->type != item->type){ return read; }
            item = item->next;
        }
    }

    return read;
}

unsigned int tlv8_value_length(tlv8_item *item) {
    unsigned int length = item->length;
    while (item->next->type == item->type){
        item = item->next;
        length += item->length;
    }
    return length;
}

unsigned int tlv8_item_length(unsigned int value_length) {
    return value_length + (value_length / 255 + 1) * 2;
}

unsigned int tlv8_chain_length(tlv8_item *chain) {
    unsigned int length = 0;

    //Start from the start of the chain
    while (chain->previous != nullptr){
        chain = chain->previous;
    }

    //Iterates the chain and adds up the lengths of each item
    while (chain != nullptr){
        length += chain->length;
        chain = chain->next;
    }

    return length;
}

void tlv8_detach(tlv8_item *item) {
    item->previous->next = item->next;
    item->next->previous = item->previous;
    delete item;
}

tlv8_item *tlv8_insert(tlv8_item *chain, tlv8_type type, unsigned int length, uint8_t *data) {
    auto start_item = new tlv8_item();
    auto current_item = start_item;

    int _len = length;

    while (_len > 0){
        auto current_length = static_cast<uint8_t>(_len > 255 ? 255 : _len);
        current_item->type = type;
        current_item->length = current_length;
        current_item->value = data;
        current_item->offset = 0;

        _len -= current_length;
        data += current_length;

        if(_len > 0){
            current_item->next = new tlv8_item();
            current_item->next->previous = current_item;
            current_item = current_item->next;
        }
    }

    //The last item is 255 bytes, insert a spacer after this item to prevent confusion
    if(current_item->length == 255){
        current_item->next = new tlv8_item();
        current_item->next->previous = current_item;
        current_item = current_item->next;

        current_item->type = kTLVType_Separator;
        current_item->length = 0;
        current_item->value = nullptr;
        current_item->offset = 0;
    }

    if(chain){
        if(chain->previous){
            chain->previous->next = start_item;
            start_item->previous = chain->previous;
        }
        chain->previous = current_item;
        current_item->next = chain;
    }

    return start_item;
}

void tlv8_reset_chain(tlv8_item *chain) {
    //Start from the start of the chain
    while (chain->previous != nullptr){
        chain = chain->previous;
    }

    while (chain != nullptr){
        chain->offset = 0;
        chain = chain->next;
    }
}

void tlv8_encode(tlv8_item *chain, uint8_t *destination) {
    //Start from the start of the chain
    while (chain->previous != nullptr){
        chain = chain->previous;
    }

    while (chain != nullptr){
        destination[0] = chain->type;
        destination[1] = chain->length;
        memcpy(&destination[2], chain->value, chain->length);
        destination += chain->length + 2;
        chain = chain->next;
    }
}
