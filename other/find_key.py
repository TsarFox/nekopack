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


def derive_primary_key(master_key, file_key):
    """Derives a single-XOR key from given master and file keys."""
    base_key = file_key ^ master_key
    return (base_key >> 24 ^ base_key >> 16 ^ base_key >> 8 ^ base_key) & 0xff


def try_master_key(master_key, file_keys, files_path):
    """Attempts a master key, returning True if and only if it
    was able to decrypt every file in the given files_path.
    """
    for extension, magic_byte in KNOWN_MAGICS:
        files = find_files(extension, files_path)
        for path in files:
            file_key = file_keys.get(path)
            if file_key is None:
                sys.stderr.write("%s: No file key.\n" % path)
                continue
            primary_key = derive_primary_key(key, file_key)
            if primary_key == 0:
                continue
            elif master_key ^ file_key == 0 or master_key ^ file_key & 0xff == 1:
                continue
            with open(path, "rb") as encrypted:
                if encrypted.read()[1] ^ primary_key != magic_byte:
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

    potential_keys = []
    file_keys = get_file_keys(sys.argv[1])
    for key in range(KEY_SEARCH_SPACE):
        print(get_progress_message(key), end="")
        if try_master_key(key, file_keys, sys.argv[2]):
            potential_keys.append(hex(key))
    print("Potential keys:")
    for key in potential_keys:
        print("* %s" % key)
