#!/usr/bin/env python

import json as j
from os import path


def split_colors(color):
    numcolor = int(color, 16)
    return [
        hex(numcolor >> 16),
        hex((numcolor & 0x00FF00) >> 8),
        hex(numcolor & 0x0000FF),
    ]


def main():
    config = {}
    with open(file="config.json", mode="r") as jfile:
        config = j.load(jfile)

    config["bg_color"] = split_colors(config["bg_color"])
    print(config)

    pixmap = open(file=config["trident"]["pixmap"], mode="rb")
    data_header = open(file="img_data.h", mode="w")

    data_header.write("#ifndef IMG_DATA_H\n#define IMG_DATA_H\n")
    data_header.write("#define BG_R  " + config["bg_color"][0] + "\n")
    data_header.write("#define BG_G  " + config["bg_color"][1] + "\n")
    data_header.write("#define BG_B  " + config["bg_color"][2] + "\n")

    data_header.write(
        "#define PIXMAP_WIDTH  " + str(config["trident"]["dimens"][0]) + "\n"
    )
    data_header.write(
        "#define PIXMAP_HEIGHT  " + str(config["trident"]["dimens"][1]) + "\n"
    )
    data_header.write(
        "#define PIXMAP_PIXELS  "
        + str(int(path.getsize(config["trident"]["pixmap"]) / 4))
        + "\n"
    )
    data_header.write("uint32_t pixmap[] = {\n")
    for block in iter(
        lambda: [
            pixmap.read(4),
            pixmap.read(4),
            pixmap.read(4),
            pixmap.read(4),
        ],
        [b"", b"", b"", b""],
    ):
        for px in range(len(block)):
            block[px] = int.from_bytes(block[px], byteorder="big")
        data_header.write(
            "  " + ",".join(map("0x{:08x}".format, block)) + ",\n"
        )
    data_header.write("};\n")

    data_header.write("#endif /* IMG_DATA_H */\n")

    pixmap.close()
    data_header.close()
    return 0


if __name__ == "__main__":
    exit(main())
