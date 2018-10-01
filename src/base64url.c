/*
 *                ______            ____       _
 *               / ____/___  ____  / __ \_____(_)   _____
 *              / / __/ __ \/ __ \/ / / / ___/ / | / / _ \
 * Project     / /_/ / /_/ / /_/ / /_/ / /  / /| |/ /  __/
 *             \____/\____/\____/_____/_/  /_/ |___/\___/
 *
 * Copyright (C) 2017-2018 Pradeep Kumar <pradeep.tux@gmail.com>
 *
 * This file is part of project GooDrive.
 *
 * GooDrive is free software: You can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GooDrive is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GooDrive.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stddef.h>
#include <stdlib.h>

#include "base64url.h"

static char base64_alphabets[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                                    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                                    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                                    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'};

char *base64url_encode(const unsigned char *input_str, ssize_t input_len, size_t *output_len) {
    if (input_len == 0) {
        *output_len = 0;
        return NULL;
    }

    /* The number of characters in the base64url encoded string is calculated as follows:
     * If length of input string is a multiple of 3, then the output length is (4 * number of triplets in input string)
     * Otherwise, it will be (4 * number of complete triplets in input string) + (number of characters in last triplet) + 1
     */
    if (input_len % 3 == 0) {
        *output_len = 4*(input_len/3);
    } else {
        int num_complete_triplets = (input_len/3);
        *output_len = 4*num_complete_triplets + (input_len - (num_complete_triplets*3)) + 1;
    }

    ssize_t in = 0, out = 0;

    /* Allocate 1 additional space for NULL character. */
    char *encoded_str = malloc((*output_len) + 1);

    unsigned char first_six_bit_mask = 252;
    unsigned char last_two_bit_mask = 3;
    unsigned char first_four_bit_mask = 240;
    unsigned char last_four_bit_mask = 15;
    unsigned char first_two_bit_mask = 192;
    unsigned char last_six_bit_mask = 63;

    // Take three characters from the input, and convert them to four in Base64URL encoding
    unsigned char in_1, in_2, in_3;
    unsigned char out_1=0, out_2=0, out_3=0, out_4=0;

    // For the complete triplets
    for (in=0; in<input_len-3;) {
        // printf("%d < %d ? --> %d \n", in, (((ssize_t)input_len)-3), (in<(((ssize_t)input_len)-3)));
        in_1 = input_str[in++];
        in_2 = input_str[in++];
        in_3 = input_str[in++];

        out_1 = (in_1 & first_six_bit_mask) >> 2;
        out_2 = (((in_1 & last_two_bit_mask) << 4) | ((in_2 & first_four_bit_mask) >> 4));
        out_3 = (((in_2 & last_four_bit_mask) << 2) | ((in_3 & first_two_bit_mask) >> 6));
        out_4 = (in_3 & last_six_bit_mask);

        encoded_str[out++] = base64_alphabets[out_1];
        encoded_str[out++] = base64_alphabets[out_2];
        encoded_str[out++] = base64_alphabets[out_3];
        encoded_str[out++] = base64_alphabets[out_4];
    }

    // For the remaining characters
    in_1 = input_str[in++];
    out_1 = (in_1 & first_six_bit_mask) >> 2;
    out_2 = ((in_1 & last_two_bit_mask) << 4);

    encoded_str[out] = base64_alphabets[out_1];
    encoded_str[out+1] = base64_alphabets[out_2];

    if (in < input_len) {
        in_2 = input_str[in++];
        out_2 = out_2 | ((in_2 & first_four_bit_mask) >> 4);
        encoded_str[out+1] = base64_alphabets[out_2];
        out_3 = ((in_2 & last_four_bit_mask) << 2);
        encoded_str[out+2] = base64_alphabets[out_3];
        if (in < input_len) {
            in_3 = input_str[in++];
            out_3 = out_3 | ((in_3 & first_two_bit_mask) >> 6);
            encoded_str[out+2] = base64_alphabets[out_3];
            out_4 = in_3 & last_six_bit_mask;
            encoded_str[out+3] = base64_alphabets[out_4];
        }
    }

    encoded_str[*output_len] = 0;
    return encoded_str;
}
