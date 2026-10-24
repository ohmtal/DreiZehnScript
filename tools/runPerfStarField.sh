#!/bin/sh
# perf stat -e cycles,instructions,branches,branch-misses,L1-icache-load-misses ./DreiZehn scripts/bcLoader.13 StarField2 bench


perf record -g -- ./DreiZehn scripts/bcLoader.13 StarField2 bench
perf report -g
