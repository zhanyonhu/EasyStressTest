#!/bin/bash
# Copyright (c) 2014, zhanyonhu <zhanyonhu@163.com>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

echo 'auto build the stress test project'

#Build libuv
if [ ! -f "third/libuv/gyp_uv.py" ]; then
	  echo 'git clone https://github.com/joyent/libuv.git third/libuv'
	  git clone https://github.com/joyent/libuv.git third/libuv
	  cd third/libuv

	#Download gyp
	if [ ! -d "build/gyp" ]; then
	  echo 'git clone https://git.chromium.org/external/gyp.git build/gyp'
	  git clone https://git.chromium.org/external/gyp.git build/gyp
	fi

	./gyp_uv.py -f make
	make -C out
	cd ../../
fi

./gyp_stresstest.py -f make
make -C output





