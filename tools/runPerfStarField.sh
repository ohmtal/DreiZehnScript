#!/bin/sh
# perf stat -e cycles,instructions,branches,branch-misses,L1-icache-load-misses ./DreiZehn scripts/bcLoader.13 StarField2 bench

# when paranoid is set to 2 , make it less permanent less paranoid ;) - NOTE handle with care
# echo "kernel.perf_event_paranoid = 1" | sudo tee /etc/sysctl.d/99-perf.conf
# sudo sysctl --system



perf record -g -- ./DreiZehn scripts/bcLoader.13 StarField2 bench
perf report -g
