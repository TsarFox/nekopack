# ![Nekopack](https://raw.github.com/TsarFox/nekopack/master/Nekopack_Logo.png "Nekopack")
## Software programmed and maintained by [Jakob.](http://jakob.space/)
Nekopack is a work-in-progress attempt at reverse engineering the archive format used by Nekopara.

Please use this software responsibly. I chose to develop and release this because I see it as a very valuable teaching tool in accompaniment to my writeup, and I think that people who legally purchased Nekopara should have the right to view the game's content. I do not condone piracy or the redistribution of copyrighted materials in any way, shape, or form.

Nekopack is free software, licensed under the [GNU General Public License.](http://gnu.org/licenses/gpl.html)


## Compilation
Nekopack can be compiled on Linux by running `make` from the repository's root directory.

*BSD users will have to install and run `gmake` to build Nekopack.

The test suite is run with `make test` or `gmake test`, depending on your platform.

You can add specific flags to the Makefile with `USER_CFLAGS` and/or `USER_LDFLAGS`.


## TODO
* Full unit testing, test static functions.
* Clean up `main.c`, move functions like `load_table` and `make_dirs` into more appropriate files.
* Clean up `create_archive`.
* Strict warnings.
* Game-compatible file encryption on created archives.
