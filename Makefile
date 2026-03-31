CXX := clang++

CXXFLAGS := -std=c++23 -Wall -Wextra -Wshadow -Wpedantic \
	-g -O2 -D_GLIBCXX_DEBUG \
	-Isrc

LDFLAGS := -fsanitize=address,undefined

SRCS := $(shell find src -name '*.cpp')

.PHONY: all clear run-test

all: artist

artist:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $@ $(LDFLAGS)

clear:
	rm -f artist

run-test: artist
	mkdir -p out
	./artist --algo random --graph SmallGraph --space hyperbolic --initial-placement zero --projection euclidean --output out/SmallGraph_random.json

