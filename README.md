# PineTime OS

This is an experimental OS for the [Pine64 PineTime](https://wiki.pine64.org/index.php/PineTime)
based on the Zephyr RTOS.

## Setup
1. First, you'll need to make sure you have the Zephyr SDK and its command-line
   tool `west` installed, and use these to create a new project.
   Follow [this guide!](https://docs.zephyrproject.org/latest/getting_started/index.html)

2. Zephyr does not come with support for the PineTime, so we need to add a board
   definition for it. The board definition required by this project is
   in [this GitHub repository](https://github.com/najnesnaj/pinetime-zephyr), so
   follow the installation instructions to add it to your project.

3. Clone this repository into your Zephyr project.

Once you're all done, your directory structure contain at least these
directories (although it will have many more, this is a rough outline):

```
.
├── .west
├── zephyr
|   └── boards
|       └── arm
|           └── pinetime
|               ├── pinetime.dts
|               └── pinetime.yaml
└── pinetime <or whatever you cloned this repo as>
    ├── CMakeLists.txt
    ├── prj.conf
    ├── pinetime.overlay
    └── src
```

## Building
Once you've followed the setup instructions, open a terminal into **this repo**
(not the root of your Zephyr project), and invoke `west` to build the project:

```
west build -p -b pinetime
```

Building will produce a binary file you can flash to your PineTime, in
`build/zephyr/zephyr.bin`. If you're not sure how to flash, there'll probably
be a relevant article on the [Pine64 wiki](https://wiki.pine64.org/index.php/PineTime)
for your hardware under _Manual/Articles_.

Remember that you need to run some commands once per environment before you
invoke `west`, to set your environment up:

```bash
# This might be different if you're building on other architectures
# If you followed the Getting Started guide, this should work
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr

# If you installed the SDK through the AUR, it'll be at /opt/zephyr-sdk
# If you installed it manually, you should know where it is!
export ZEPHYR_SDK_INSTALL_DIR=/opt/zephyr-sdk

# Run this from the root of your Zephyr project
source zephyr/zephyr-env.sh
```