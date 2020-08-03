#!/bin/bash

clang++ -Iinclude src/ssr_mixer.cpp --compile --target=wasm32-unknown-unknown-wasm --optimize=3 --output test.wasm