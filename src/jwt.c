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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <json_object.h>
#include <json_util.h>

#include "base64url.h"
#include "jwt.h"
#include "linux-api.h"

static void build_jwt_header(char **dest);
static void build_jwt_claim_set(char *email_addr, char **dest);

void build_jwt(char *email_addr, char **jwt) {
    char *jwt_header = NULL;
    char *jwt_claim_set = NULL;
    build_jwt_header(&jwt_header);
    build_jwt_claim_set(email_addr, &jwt_claim_set);
}

static void build_jwt_header(char **dest) {
    /*
     * The JWT Header is '{"alg":"RS256","typ":"JWT"}'
     * In base64url encoding it is 'eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9'
     */

    *dest = malloc(37);
    strcpy(*dest, "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9");
}

static void build_jwt_claim_set(char *email_addr, char **dest) {
    char *user_md5sum = md5sum_str(email_addr);
    char *account_details_file_path = get_abs_path(get_config_dir_curruser(), user_md5sum);
    if (access(account_details_file_path, R_OK) != -1) {
        /*
         * TODO - When the file does not exist, or does not have permission, then there should be some method
         * to notify the user of this error.
         */
        json_object *token_file_obj = json_object_from_file(account_details_file_path); // The entire token file
 		json_object *refresh_token_obj;
 		if (token_file_obj == NULL) {
 			return NULL;
 		}
        char *service_acct = json_object_get_string(json_object_object_get(token_file_obj, "client_email"));

        time_t epoch = time(NULL);

        const char *claim_set_template = "{\"iss\":\"%s\",\"scope\":\"%s\",\"aud\":\"%s\",\"iat\":%lu,\"exp\":%lu}";
        const char *scope_drive = "https://www.googleapis.com/auth/drive";
        const char *aud_token = "https://www.googleapis.com/oauth2/v4/token";

        // 20 is for two epoch times, and 12 is for the format specifiers in the template
        size_t claim_set_len = strlen(claim_set_template) + strlen(service_acct) + strlen(scope_drive) + strlen(aud_token) + 20 - 12;
        char *claim_set = (char *)malloc(claim_set_len+1);
        sprintf(claim_set, claim_set_template, service_acct, scope_drive, aud_token, epoch, epoch + 3600);
        claim_set[claim_set_len] = NULL;
        size_t encoded_claim_set_len;
        printf("Claim set %s \n", claim_set);
        *dest = base64url_encode(claim_set, claim_set_len, &encoded_claim_set_len);
    }
}
