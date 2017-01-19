#!/usr/bin/env python

"""Script that calculates the master XOR key for a Nekopara volume
by exploiting the cryptographic weakness in single-key XOR.
"""

import copy
import os
import sys

KEY_SEARCH_SPACE = 4294967296

# Because single-key XOR is used, only one byte is really needed. It's
# not the first byte because that's often encrypted with another key.
KNOWN_MAGICS = [("png", b"\x50")]


def find_files(extension, current_path):
    """Returns a list of all paths with a given
    extension from the given base path.
    """
    found = []
    for entry in os.listdir(current_path):
        if os.path.isdir(entry):
            found += find_files(extension, entry)
        elif entry[-len(extension):] == extension:
            found.append(current_path + "/" + entry)
    return found


def get_file_keys(path):
    """Parses a comma-separated file containing
    file keys and their respective paths.
    """
    file_keys = {}
    with open(path) as output:
        for line in output:
            encrypted, path, key = line.split(',')
            if encrypted:
                file_keys[path] = int(key, 16)
    return file_keys


def setup_structures(magic_byte, paths, file_keys):
    """Parses every file in a given list of paths such that a structure
    exists containing a byte from the known plaintext, the encrypted
    byte, and the integer used to derive an encryption key. This
    massively decreases IO overhead.
    """
    structures = []
    for path in paths:
        file_key = file_keys[path]
        with open(path, "rb") as encrypted:
            encrypted_byte = encrypted.read()[1]
            structures.append((magic_byte, encrypted_byte, file_key))
    return structures


def derive_primary_key(master_key, file_key):
    """Derives a single-XOR key from given master and file keys."""
    base_key = file_key ^ master_key
    return (base_key >> 24 ^ base_key >> 16 ^ base_key >> 8 ^ base_key) & 0xff


def try_master_key(structures, key):
    """Attempts decryption on a list of structures, returning True if
    and only if it was successful in decrypting them.
    """
    for desired_byte, encrypted_byte, file_key in structures:
        primary_key = derive_primary_key(key, file_key)
        if primary_key == 0:
            continue
        elif key ^ file_key == 0 or key ^ file_key & 0xff == 1:
            continue
        if encrypted_byte ^ primary_key != desired_byte:
            return False
    return True


def get_progress_message(key):
    """Returns a message preceeded with a carraige-return
    displaying how far the script is in bruteforcing.
    """
    percent_complete = key / KEY_SEARCH_SPACE * 100
    return "\r%.1f%% (Trying %s)" % (percent_complete, hex(key))


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.stderr.write("USAGE: %s [file keys] [path]\n" % sys.argv[0])
        sys.exit(1)

    structures = []
    file_keys = get_file_keys(sys.argv[1])
    for extension, magic in KNOWN_MAGICS:
        paths = find_files(extension, ".")
        structures += setup_structures(magic, paths, file_keys)

    potential_keys = []
    for key in range(KEY_SEARCH_SPACE):
        print(get_progress_message(key), end="")
        if try_master_key(structures, key):
            potential_keys.append(hex(key))
    print("Potential keys:")
    for key in potential_keys:
        print("* %s" % key)
