#!/usr/bin/env python3
"""
mkfs.py - Build a SOFS (SO File System) image from a directory.

Format:
  [fs_header: magic(4) + num_files(4)]
  [fs_file_entry[0]: name(64) + offset(4) + size(4)]
  ...
  [fs_file_entry[N-1]]
  [file 0 data (4-byte aligned)]
  ...
  [file N-1 data (4-byte aligned)]
"""

import struct
import os
import sys

FS_MAGIC = 0x534F4653
MAX_NAME = 64
HEADER_SIZE = 8
ENTRY_SIZE = MAX_NAME + 4 + 4  # 72 bytes


def align4(n):
    return (n + 3) & ~3


def build_fs(input_dir, output_path):
    filenames = sorted(
        f for f in os.listdir(input_dir)
        if os.path.isfile(os.path.join(input_dir, f))
    )

    if not filenames:
        print("error: no files found in", input_dir, file=sys.stderr)
        sys.exit(1)

    num_files = len(filenames)
    data_start = HEADER_SIZE + ENTRY_SIZE * num_files

    file_contents = []
    entries = []
    current_offset = data_start

    for fname in filenames:
        path = os.path.join(input_dir, fname)
        with open(path, "rb") as f:
            data = f.read()

        name_bytes = fname.encode("ascii")[:MAX_NAME - 1]
        name_bytes = name_bytes + b"\x00" * (MAX_NAME - len(name_bytes))

        entries.append((name_bytes, current_offset, len(data)))
        file_contents.append(data)
        current_offset = align4(current_offset + len(data))

    with open(output_path, "wb") as out:
        out.write(struct.pack("<II", FS_MAGIC, num_files))

        for name_bytes, offset, size in entries:
            out.write(name_bytes)
            out.write(struct.pack("<II", offset, size))

        for data in file_contents:
            out.write(data)
            padding = align4(len(data)) - len(data)
            if padding:
                out.write(b"\x00" * padding)

    total = os.path.getsize(output_path)
    print(f"mkfs: {num_files} file(s), {total} bytes -> {output_path}")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <input_dir> <output_file>", file=sys.stderr)
        sys.exit(1)
    build_fs(sys.argv[1], sys.argv[2])
