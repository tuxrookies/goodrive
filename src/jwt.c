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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <json_object.h>
#include <json_util.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#include "base64url.h"
#include "jwt.h"
#include "linux-api.h"

#define SUCCESSFUL_OPERATION 1
#define FAILED_OPERATION -1

static int build_jwt_header(char **dest, size_t *encoded_header_len);
static int build_jwt_claim_set(char **dest, json_object *token_file_obj, size_t *encoded_claim_set_len);
static int build_jwt_signature(char **dest, json_object *token_file_obj, const char *jwt_header, const char *jwt_claim_set, size_t *encoded_sig_len);

void build_jwt(char *email_addr, char **jwt) {
    char *jwt_header = NULL;
    char *jwt_claim_set = NULL;
    char *jwt_signature = NULL;
    char *user_md5sum = md5sum_str(email_addr);
    char *account_details_file_path = get_abs_path(get_config_dir_curruser(), user_md5sum);
    if (access(account_details_file_path, R_OK) != -1) {
        json_object *token_file_obj = json_object_from_file(account_details_file_path); // The entire token file
        if (token_file_obj == NULL) {
            return;
        }
        size_t encoded_header_len, encoded_claim_set_len, encoded_sig_len;
        build_jwt_header(&jwt_header, &encoded_header_len);
        build_jwt_claim_set(&jwt_claim_set, token_file_obj, &encoded_claim_set_len);
        build_jwt_signature(&jwt_signature, token_file_obj, jwt_header, jwt_claim_set, &encoded_sig_len);
        *jwt = malloc(encoded_header_len + encoded_claim_set_len + encoded_sig_len + 3);
        if (*jwt == NULL) {
            printf("Failed to allocate memory\n");
            return;
        }
        sprintf(*jwt, "%s.%s.%s", jwt_header, jwt_claim_set, jwt_signature);
        (*jwt)[encoded_header_len + encoded_claim_set_len + encoded_sig_len + 2] = 0;
        free(jwt_header);
        free(jwt_claim_set);
        free(jwt_signature);
    }
}

static int build_jwt_header(char **dest, size_t *encoded_header_len) {
    /*
     * The JWT Header is '{"typ":"JWT","alg":"RS256"}'
     * In base64url encoding it is 'eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiJ9'
     */

    *dest = malloc(37);
    if (dest == NULL) {
        return FAILED_OPERATION;
    }
    strcpy(*dest, "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiJ9");
    *encoded_header_len = 36;
    (*dest)[36] = 0;
    return SUCCESSFUL_OPERATION;
}

static int build_jwt_claim_set(char **dest, json_object *token_file_obj, size_t *encoded_claim_set_len) {
    json_object *service_acct_node;
    json_object_object_get_ex(token_file_obj, "client_email", &service_acct_node);
    const char *service_acct = json_object_get_string(service_acct_node);

    time_t epoch = time(NULL);

    const char *claim_set_template = "{\"iss\":\"%s\",\"scope\":\"%s\",\"aud\":\"%s\",\"iat\":%lu,\"exp\":%lu}";
    const char *scope_drive = "https://www.googleapis.com/auth/drive";
    const char *aud_token = "https://www.googleapis.com/oauth2/v4/token";

    // 20 is for two epoch times, and 12 is for the format specifiers in the template
    size_t claim_set_len = strlen(claim_set_template) + strlen(service_acct) + strlen(scope_drive) + strlen(aud_token) + 20 - 12;
    char *claim_set = (char *)malloc(claim_set_len+1);
    sprintf(claim_set, claim_set_template, service_acct, scope_drive, aud_token, epoch, epoch + 3600);
    claim_set[claim_set_len] = 0;
    *dest = base64url_encode(claim_set, claim_set_len, encoded_claim_set_len);
    return SUCCESSFUL_OPERATION;
}

static int build_jwt_signature(char **dest, json_object *token_file_obj, const char *jwt_header, const char *jwt_claim_set, size_t *encoded_sig_len) {
    json_object *private_key_node;
    json_object_object_get_ex(token_file_obj, "private_key", &private_key_node);
    const char *private_key = json_object_get_string(private_key_node);

    BIO *bio = BIO_new_mem_buf(private_key, (int) strlen(private_key));
    if (bio == NULL) {
        return FAILED_OPERATION;
    }

    EVP_PKEY *pkey = PEM_read_bio_PrivateKey(bio, NULL, 0, NULL);
    if (pkey == NULL) {
        return FAILED_OPERATION;
    }
    EVP_MD_CTX *ctx = EVP_MD_CTX_create();
    if (ctx == NULL) {
        return FAILED_OPERATION;
    }
    const EVP_MD *md = EVP_sha256();
    if (md == NULL) {
        return FAILED_OPERATION;
    }
    if (EVP_DigestInit_ex(ctx, md, NULL) != 1) {
        return FAILED_OPERATION;
    }
    if (EVP_DigestSignInit(ctx, NULL, md, NULL, pkey) != 1) {
        return FAILED_OPERATION;
    }
    if (EVP_DigestSignUpdate(ctx, jwt_header, strlen(jwt_header)) != 1) {
        return FAILED_OPERATION;
    }
    if (EVP_DigestSignUpdate(ctx, ".", 1) != 1) {
        return FAILED_OPERATION;
    }
    if (EVP_DigestSignUpdate(ctx, jwt_claim_set, strlen(jwt_claim_set)) != 1) {
        return FAILED_OPERATION;
    }
    size_t req = 0;
    if (EVP_DigestSignFinal(ctx, NULL, &req) != 1 || req <= 0) {
        return FAILED_OPERATION;
    }
    unsigned char *signature = OPENSSL_malloc(req);
    if (signature == NULL) {
        return FAILED_OPERATION;
    }
    size_t written_bytes = req;
    if ((EVP_DigestSignFinal(ctx, signature, &req) != 1) || (written_bytes != req)) {
        return FAILED_OPERATION;
    }
    *dest = base64url_encode(signature, written_bytes, encoded_sig_len);
    OPENSSL_free(signature);
    return SUCCESSFUL_OPERATION;
}
