#!/usr/bin/env python

import json as j
from os import path
from sys import stderr, exit

imgtypes = ["RAW", "RLE", "RLE1"]


def split_colors(color):
    numcolor = int(color, 16)
    return [
        hex(numcolor >> 16),
        hex((numcolor & 0x00FF00) >> 8),
        hex(numcolor & 0x0000FF),
    ]


def write_rle1data(data_header, pixmap, img):
    last_pixel = None
    count = 0
    packed = [0, 0]

    def pack(byte, packed):
        packed[0] = packed[0] << 8
        packed[0] += byte
        packed[1] += 1
        if packed[1] == 4:
            data_header.write("  0b{:032b},\n".format(packed[0]))
            packed = [0, 0]
        return packed

    for pixel in iter(lambda: pixmap.read(4), b""):
        pixel = int.from_bytes(pixel, byteorder="big") & 1
        if pixel != last_pixel:
            if last_pixel is not None:
                packed = pack((last_pixel << 7) + count, packed)
            count = 1
            last_pixel = pixel
        elif pixel == last_pixel:
            count += 1
            if count == 127:
                packed = pack((last_pixel << 7) + count, packed)
                count = 0
                last_pixel = None
    packed = pack((last_pixel << 7) + count, packed)
    while packed[1]:
        packed = pack(0, packed)


def write_rledata(data_header, pixmap, img):
    data_header.write("uint32_t " + img["name"] + "_rledata[] = {\n")
    last_pixel = None
    count = 0
    for pixel in iter(lambda: pixmap.read(4), b""):
        pixel = int.from_bytes(pixel, byteorder="big")
        if pixel != last_pixel:
            if last_pixel is not None:
                data_header.write("0x{:08x},\n".format(count))
            count = 1
            data_header.write("  0x{:08x},".format(pixel))
            last_pixel = pixel
        elif pixel == last_pixel:
            count += 1
            if count == 0xFFFFFFFF:
                data_header.write("0x{:08x},\n".format(count))
                count = 0
                last_pixel = None
    data_header.write("0x{:08x},\n".format(count) + "};\n")


def write_rawdata(data_header, pixmap, img):
    data_header.write("uint32_t " + img["name"] + "_rawdata[] = {\n")
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


def main():
    config = {}
    with open(file="config.json", mode="r") as jfile:
        config = j.load(jfile)

    config["bg_color"] = split_colors(config["bg_color"])
    print(config)

    data_header = open(file="img_data.h", mode="w")
    data_header.write("#ifndef IMG_DATA_H\n#define IMG_DATA_H\n")
    data_header.write('#include "trident.h"\n')
    data_header.write("#define BG_R  " + config["bg_color"][0] + "\n")
    data_header.write("#define BG_G  " + config["bg_color"][1] + "\n")
    data_header.write("#define BG_B  " + config["bg_color"][2] + "\n")

    for img in config["gfx"]:
        pixmap = open(file=img["pixmap"], mode="rb")

        if img["type"] == 0:
            write_rawdata(data_header, pixmap, img)
            datavar = "  .data = " + img["name"] + "_rawdata\n"
        elif img["type"] == 1:
            write_rledata(data_header, pixmap, img)
            datavar = "  .data = " + img["name"] + "_rledata\n"
        elif img["type"] == 2:
            data_header.write(
                "uint32_t "
                + img["name"]
                + "_rle1data[] = {\n  0x"
                + img["fgcolor"]
                + "ff,\n"
            )
            write_rle1data(data_header, pixmap, img)
            data_header.write("};\n")
            datavar = "  .data = " + img["name"] + "_rle1data\n"
        else:
            print("ERROR: image data type not implemented!!", file=stderr)
            pixmap.close()
            data_header.close()

        data_header.write(
            "pixmap_t "
            + img["name"]
            + " = {\n"
            + "  .width = "
            + str(img["dimens"][0])
            + ",\n"
            + "  .height = "
            + str(img["dimens"][1])
            + ",\n"
            + "  .n_pixels = "
            + str(int(path.getsize(img["pixmap"]) / 4))
            + ",\n"
            + "  .datatype = "
            + imgtypes[img["type"]]
            + ",\n"
            + datavar
            + "};\n"
        )

    data_header.write("#endif /* IMG_DATA_H */\n")

    pixmap.close()
    data_header.close()
    return 0


if __name__ == "__main__":
    exit(main())
