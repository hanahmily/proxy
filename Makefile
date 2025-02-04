## Copyright 2017 Istio Authors
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.

TOP := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

SHELL := /bin/bash
BAZEL_STARTUP_ARGS ?=
BAZEL_BUILD_ARGS ?=
BAZEL_TARGETS ?= //...
# Some tests run so slowly under the santizers that they always timeout.
SANITIZER_EXCLUSIONS ?= -test/integration:mixer_fault_test
HUB ?=
TAG ?=

ifeq "$(origin CC)" "default"
CC := clang
endif
ifeq "$(origin CXX)" "default"
CXX := clang++
endif
PATH := /usr/lib/llvm-8/bin:$(PATH)

VERBOSE ?=
ifeq "$(VERBOSE)" "1"
BAZEL_STARTUP_ARGS := --client_debug $(BAZEL_STARTUP_ARGS)
BAZEL_BUILD_ARGS := -s $(BAZEL_BUILD_ARGS)
endif

UNAME := $(shell uname)
ifeq ($(UNAME),Linux)
BAZEL_CONFIG_DEV  = --config=libc++
BAZEL_CONFIG_REL  = --config=libc++ --config=release
BAZEL_CONFIG_ASAN = --config=clang-asan --config=libc++
BAZEL_CONFIG_TSAN = --config=clang-tsan --config=libc++
endif
ifeq ($(UNAME),Darwin)
BAZEL_CONFIG_DEV  = # macOS always links against libc++
BAZEL_CONFIG_REL  = --config=release
BAZEL_CONFIG_ASAN = --config=macos-asan
BAZEL_CONFIG_TSAN = # no working config
endif

build:
	export PATH=$(PATH) CC=$(CC) CXX=$(CXX) && bazel $(BAZEL_STARTUP_ARGS) build $(BAZEL_BUILD_ARGS) $(BAZEL_CONFIG_DEV) $(BAZEL_TARGETS)

build_envoy:
	export PATH=$(PATH) CC=$(CC) CXX=$(CXX) && bazel $(BAZEL_STARTUP_ARGS) build $(BAZEL_BUILD_ARGS) $(BAZEL_CONFIG_REL) //src/envoy:envoy

clean:
	@bazel clean

test:
	export PATH=$(PATH) CC=$(CC) CXX=$(CXX) && bazel $(BAZEL_STARTUP_ARGS) test $(BAZEL_BUILD_ARGS) $(BAZEL_CONFIG_DEV) $(BAZEL_TARGETS)
	GO111MODULE=on go test ./...

test_asan:
	export PATH=$(PATH) CC=$(CC) CXX=$(CXX) && bazel $(BAZEL_STARTUP_ARGS) test $(BAZEL_BUILD_ARGS) $(BAZEL_CONFIG_ASAN) -- $(BAZEL_TARGETS) $(SANITIZER_EXCLUSIONS)

test_tsan:
	export PATH=$(PATH) CC=$(CC) CXX=$(CXX) && bazel $(BAZEL_STARTUP_ARGS) test $(BAZEL_BUILD_ARGS) $(BAZEL_CONFIG_TSAN) --test_env=TSAN_OPTIONS=suppressions=$(TOP)/tsan.suppressions -- $(BAZEL_TARGETS) $(SANITIZER_EXCLUSIONS)

check:
	@echo >&2 "Please use \"make lint\" instead."
	@false

lint:
	@scripts/check_license.sh
	@scripts/check-repository.sh
	@scripts/check-style.sh

deb:
	export PATH=$(PATH) CC=$(CC) CXX=$(CXX) && bazel $(BAZEL_STARTUP_ARGS) build $(BAZEL_BUILD_ARGS) $(BAZEL_CONFIG_REL) //tools/deb:istio-proxy

artifacts:
	export PATH=$(PATH) CC=$(CC) CXX=$(CXX) BAZEL_BUILD_ARGS="$(BAZEL_BUILD_ARGS)" && ./scripts/push-debian.sh -p "$(ARTIFACTS_GCS_PATH)" -o "$(ARTIFACTS_DIR)"

test_release:
	export PATH=$(PATH) CC=$(CC) CXX=$(CXX) BAZEL_BUILD_ARGS="$(BAZEL_BUILD_ARGS)" && ./scripts/release-binary.sh -i

push_release:
	export PATH=$(PATH) CC=$(CC) CXX=$(CXX) BAZEL_BUILD_ARGS="$(BAZEL_BUILD_ARGS)" && ./scripts/release-binary.sh -d "$(RELEASE_GCS_PATH)"


.PHONY: build clean test check artifacts

include Makefile.common.mk
