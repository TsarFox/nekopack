#!/usr/bin/env python

"""Script that calculates the master XOR key for a Nekopara volume
by exploiting the cryptographic weakness in single-key XOR.
"""

import multiprocessing as mp
import copy
import os
import sys

KEY_SEARCH_SPACE = 4294967296
THREAD_COUNT = 8

KNOWN_MAGICS = [("png", b"\x89\x50\x4e\x47\x0d\x0a\x1a\x0a"),
                # ("mstand", b"\xff\xfe\x23\x00\x20\x00"),
                # ("sli", b"\x23\x32\x2e\x30\x0a"),
                ("ogg", b"\x4f\x67\x67\x53"),
                ("psb", b"\x50\x53\x42\x00"),
                ("mstand", b"\xff\xfe"),
                # ("tjs", b"\xff\xfe"),
                ("csv", b"\xff\xfe")]


def find_files(extension, current_path):
    """Returns files in a path that with a given extension."""
    found = []
    for entry in os.listdir(current_path):
        if os.path.isdir(os.path.join(current_path, entry)):
            found += find_files(extension, os.path.join(current_path, entry))
        elif entry[-len(extension):] == extension:
            found.append(os.path.join(current_path, entry))
    return found


def get_file_keys(file_keys_path, base_path):
    """Parses a comma-separated file containing keys into a hashmap
    associating each file path with its encryption key.
    """
    file_keys = {}
    with open(file_keys_path) as output:
        for line in output:
            path, key = line.split(',')
            file_keys[os.path.join(base_path, path)] = int(key, 16)
    return file_keys


def sane_header(encrypted_chunk, magic_number):
    """Performs a sanity check, returning true if a temporary
    key can be derived and used to decrypt the chunk.
    """
    decryption_buffer = bytearray()
    temporary_key = encrypted_chunk[1] ^ magic_number[1]
    for byte in encrypted_chunk[1:]:
        decryption_buffer.append(byte ^ temporary_key)
    return decryption_buffer == magic_number[1:]


def setup_structures(magic, paths, file_keys):
    """Returns a list of structures, each containing the first two bytes
    from the known plaintext, the first two encrypted bytes, and the
    value used to derive an encryption key.
    """
    structures = []
    for path in paths:
        file_key = file_keys[path]
        with open(path, "rb") as encrypted:
            encrypted_chunk = encrypted.read()[:len(magic)]
            if sane_header(encrypted_chunk, magic):
                encrypted.seek(0)
                encrypted_bytes = encrypted.read()[:2]
                structures.append((magic[:2], encrypted_bytes, file_key))
    return structures


def derive_primary_key(base_key):
    """Derives a single-XOR key from given base keys."""
    return (base_key >> 24 ^ base_key >> 16 ^ base_key >> 8 ^ base_key) & 0xff


def try_master_key(structures, key):
    """Attempts decryption on a list of structures, returning True if
    and only if it was successful in decrypting them.
    """
    for desired_bytes, encrypted_bytes, file_key in structures:
        base_key = file_key ^ key
        initial_key = base_key & 0xff
        primary_key = derive_primary_key(base_key)
        if primary_key == 0 or initial_key == 0:
            continue
        if encrypted_bytes[1] ^ primary_key != desired_bytes[1]:
            return False
        if encrypted_bytes[0] ^ initial_key ^ primary_key != desired_bytes[0]:
            return False
    return True


def attack_range(structures, key_start, key_end, pipe):
    """Multiprocessing target. Tries every key within a given
    range and sends successful keys back through a pipe.
    """
    potential_keys = []
    try:
        for key in range(key_start, key_end):
            if try_master_key(structures, key):
                potential_keys.append(key)
    except KeyboardInterrupt:
        pass
    finally:
        pipe.send(potential_keys)


if __name__ == "__main__":
    if len(sys.argv) < 3:
        sys.stderr.write("USAGE: %s [keys file] [path]\n" % sys.argv[0])
        sys.exit(1)

    base_path = sys.argv[2]
    file_keys_path = sys.argv[1]

    structures = []
    file_keys = get_file_keys(file_keys_path, base_path)
    for extension, magic in KNOWN_MAGICS:
        paths = find_files(extension, base_path)
        structures += setup_structures(magic, paths, file_keys)

    print("Initialized with attack surface of %d targets." % len(structures))

    processes = []
    process_pipes = []
    for i in range(THREAD_COUNT):
        parent, child = mp.Pipe()
        process_pipes.append(parent)

        key_start = i * KEY_SEARCH_SPACE // 8
        key_end = (i + 1) * KEY_SEARCH_SPACE // 8
        process = mp.Process(target=attack_range,
                             args=(structures, key_start,
                                   key_end, child))
        process.start()
        processes.append(process)

    try:
        for process in processes:
            process.join()
    except KeyboardInterrupt:
        pass
    finally:
        print("Potential keys:")
        for pipe in process_pipes:
            for key in pipe.recv():
                print(" * %s" % hex(key))
