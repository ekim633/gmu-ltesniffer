
# LTEJammer - An Open-source LTE Downlink Smart Jammer 

## Read the Research Paper
[LTEJammer-Targeted-and-Persistent-Jamming-Attack-in-LTE.pdf](https://github.com/user-attachments/files/21448933/LTEJammer-Targeted-and-Persistent-Jamming-Attack-in-LTE.pdf)

**LTEJammer** is An Open-source LTE Downlink Smart Jammer 

We extended LTE-Sniffer with support for targeted jamming by introducing the -J parameter, enabling jamming of Downlink Control Information (DCI) within the Physical Downlink Control Channel (PDCCH).

Our implementation includes a user interface that provides real-time visualization of resource blocks and allows toggling the jamming function on or off.

To identify a specific user, we map the TMSI (Temporary Mobile Subscriber Identity) to its corresponding RNTI (Radio Network Temporary Identifier). Using this, the jammer can persistently launch Denial of Service (DoS) attacks by further resolving the RNTI to the IMSI (International Mobile Subscriber Identity), a permanent identifier.

By leveraging the LTE-Sniffer API, we enable selective and persistent jamming of targeted user equipment (UE) in real time.


## Ethical Consideration

The primary purpose of LTEJammer is to support security research and analysis of cellular networks. This project builds upon LTESniffer and involves the collection of both uplink and downlink user data. As such, any use of LTESniffer or LTEJammer must comply with applicable local laws and regulations regarding wireless communication and signal interception.

We do take responsibility for any misuse of this tool, including activities that cause disruption to legitimate network services or violate privacy laws.

## Features
New Implementations
- Visual Interface (Resource Blocks that is allocated to specific user) 
- Selective Jamming Capability
- Automatic mapping of RNTI to IMSI

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

We used the USRP X310 as the sniffer due to its high reliability in decoding both uplink and downlink traffic.
A second SDR, connected to the same PC, was designated as the jammer. Two separate threads were implemented to switch between sniffing and jamming operations.

A third SDR, running on a separate machine, was used to simulate a private LTE network for testing and evaluation.

## Installation
Before setting up this project, please install and run LTESniffer as a prerequisite. Refer to the official LTESniffer documentation for installation and configuration instructions.
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
We recommend using [inspectrum], a spectrum analysis tool, to ensure there are no nearby cellular devices operating on the selected frequency band. While Band 3 is generally less congested and suitable for experimentation, please verify this using the tool before proceeding.

For additional security and to prevent unintended interference, we strongly advise conducting all experiments inside a Faraday cage.

Please refer to the srsRAN 4G documentation for instructions on setting up a private LTE network. In our setup, we used a Pixel 3 XL as the user equipment (UE) along with a programmable SIM card.

### General Usage of Jammer
To enable jamming functionality, simply add the -J flag when running the tool.
Use the -f and -u options to specify the downlink and uplink frequencies.

## Credits
We appreciate LTESniffer/[FALCON][falcon] and [SRS team][srsran] for making this project possible.

## Contributor
Special thanks to all the contributors 

1. early Contributor [fatemonkey]

[inspectrum]: https://github.com/miek/inspectrum
[lteSniffer]: https://github.com/SysSec-KAIST/LTESniffer
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
