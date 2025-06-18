
# LTEJammer - An Open-source LTE Downlink Smart Jammer 

**LTEJammer** is An Open-source LTE Downlink Smart Jammer 

Draft:
"This is general introduction to what this project is about"


## LTEJammer in layman's terms



## Ethical Consideration

The main purpose of LTEJammer is to support security and analysis research on the cellular network. As this project builds on LTESniffer and due to the collection of uplink-downlink user data, any use of LTESniffer must follow the local regulations on sniffing. We are not responsible for any illegal purposes for causing disruption of service to other users.

## Features
"Add what new implmentations were added to this project"

## Hardware and Software Requirement
### OS Requirement
We utilized Ubuntu 20.04.

### Hardware Requirement
Please refer to LTESniffer for Hardware Requirement "link"

**The following hardware is recommended**
- Intel i7 CPU with at least 8 physical cores
- At least 16Gb RAM
- 256 Gb SSD storage

### SDR

"Mention about 3 SDR used for each purposes 

## Installation
**Important note: To avoid unexpected errors, please follow the following steps on Ubuntu 20.04

**Dependencies**
- **Important dependency**: [UHD][uhd] library version >= 4.0 must be installed in advance (recommend building from source).Refer to UHD Manual for full installation guidance. 

UHD dependencies:
```bash
sudo apt update
sudo apt-get install autoconf automake build-essential ccache cmake cpufrequtils doxygen ethtool \
g++ git inetutils-tools libboost-all-dev libncurses5 libncurses5-dev libusb-1.0-0 libusb-1.0-0-dev \
libusb-dev python3-dev python3-mako python3-numpy python3-requests python3-scipy python3-setuptools \
python3-ruamel.yaml
```
Clone and build UHD from source (make sure that the current branch is higher than 4.0)
```bash
git clone https://github.com/EttusResearch/uhd.git
cd <uhd-repo-path>/host
mkdir build
cd build
cmake ../
make -j 4
make test
sudo make install
sudo ldconfig
```
Download firmwares for USRPs:
```bash
sudo uhd_images_downloader
```
We use a [10Gb card](https://www.ettus.com/all-products/10gige-kit/) to connect USRP X310 to PC, refer to UHD Manual [[1]](https://files.ettus.com/manual/page_usrp_x3x0.html), [[2]](https://files.ettus.com/manual/page_usrp_x3x0_config.html) to configure USRP X310 and 10Gb card interface. For USRP B210, it should be connected to PC via a USB 3.0 port.

Test the connection and firmware (for USRP X310 only):
```bash
sudo sysctl -w net.core.rmem_max=33554432
sudo sysctl -w net.core.wmem_max=33554432
sudo ifconfig <10Gb card interface> mtu 9000
sudo uhd_usrp_probe
```

- srsRAN dependencies:
```bash
sudo apt-get install build-essential git cmake libfftw3-dev libmbedtls-dev libboost-program-options-dev libconfig++-dev libsctp-dev
```

- LTESniffer dependencies:
```bash
sudo apt-get install libglib2.0-dev libudev-dev libcurl4-gnutls-dev libboost-all-dev qtdeclarative5-dev libqt5charts5-dev
```

**Build LTESniffer from source:**
```bash
git clone https://github.com/SysSec-KAIST/LTESniffer.git
cd LTESniffer
mkdir build
cd build
cmake ../
make -j 4 (use 4 threads)
```
## Usage
Mention about utilizing certain bands and faraday cage.

### General Usage of Jammer
mention about the additonal parameters -J argument that was added




### Output of LTEJammer


Information for debugging

## Credits
We appreciate LTESniffer/[FALCON][falcon] and [SRS team][srsran] for making this project possible.

## Contributor
Special thanks to all the contributors 

1. early Contributor [fatemonkey]


[falcon]: https://github.com/falkenber9/falcon
[srsran]: https://github.com/srsran/srsRAN_4G
[uhd]:    https://github.com/EttusResearch/uhd
[paper]:  https://syssec.kaist.ac.kr/pub/2023/wisec2023_tuan.pdf
[pcap]:   pcap_file_example/README.md
[app]:    https://play.google.com/store/apps/details?id=make.more.r2d2.cellular_z&hl=en&gl=US&pli=1
[watching]: https://syssec.kaist.ac.kr/pub/2022/sec22summer_bae.pdf
[multi-readme]: https://github.com/SysSec-KAIST/LTESniffer/tree/LTESniffer-multi-usrp
[capture-readme]: https://github.com/SysSec-KAIST/LTESniffer/tree/LTESniffer-record-subframe
[cellular77]: https://github.com/cellular777
[Cemaxecuter]: https://www.youtube.com/@cemaxecuter7783
