sudo sysctl -w net.core.wmem_max=33554432
sudo sysctl -w net.core.rmem_max=33554432
sudo sysctl -w net.core.wmem_default=33554432
sudo sysctl -w net.core.rmem_default=33554432

sudo ifconfig enp2s0f0 mtu 9000
sudo ifconfig enp2s0f1 mtu 9000

sudo ethtool -G enp2s0f0 tx 4096 rx 4096
sudo ethtool -G enp2s0f1 tx 4096 rx 4096

for ((i=0;i<$(nproc --all);i++)); do sudo cpufreq-set -c $i -r -g performance; done

