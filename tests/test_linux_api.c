/*
 *                ______            ____       _
 *               / ____/___  ____  / __ \_____(_)   _____
 *              / / __/ __ \/ __ \/ / / / ___/ / | / / _ \
 * Project     / /_/ / /_/ / /_/ / /_/ / /  / /| |/ /  __/
 *             \____/\____/\____/_____/_/  /_/ |___/\___/
 *
 * Copyright (C) 2017 Pradeep Kumar <pradeep.tux@gmail.com>
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

#include <assert.h>
#include <linux-api.h>
#include <string.h>

void test_md5sum_str(void);

int main() {
	test_md5sum_str();
	return 0;
}

void test_md5sum_str(void) {
	/* MD5Sum for 'blah' */
	const char *expected = "6f1ed002ab5595859014ebf0951522d9";
	const char *retval = md5sum_str("blah");
	assert(strcmp(expected, retval) == 0);
}
