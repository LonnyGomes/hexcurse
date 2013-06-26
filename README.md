hexcurse v1.55-2
=====================================
![hexcurse screenshot](http://lonnygomes.github.io/screenshots/hexcurse2-ss.jpg)

Bug reports: https://github.com/LonnyGomes/hexcurse/issues


Description
-----------
Hexcurse is a curses-base hex editing utility that can open, edit, and save files, editing both the hexadecimal and decimal values.

It was written by [Lonny Gomes](https://twitter.com/lonnygomes) and [James Stephenson](https://plus.google.com/u/0/103174459258175070784/about) but we haven't maintained it for some time. We recently saw an old tarball of the code floating around the net and thought it would be good to start maintaining the codebase again.

It currently supports searching, hex and decimal address output, jumping to specified locations in the file, "undo" capabilities, "bolded" modifications, EBCDIC mode, and quick keyboard shortcuts to commands.


Requirements
------------
You must have the ncurses development libraries (version 5+) to compile this program


Installation
------------

    ./configure
    make
    make install

Usage
-----

    usage: hexcurse [-?|help] [-a] [-r rnum] [-o outputfile] [[-i] infile]

        -a      Output addresses in decimal format initially
        -e      Output characters in EBCDIC format rather than ASCII
        -r rnum Resize the display to "rnum" bytes wide
        -o outfile  Write output to outfile by default
        -? | -help  Display usage and version of hexcurse program
        [-i] infile Read from data from infile (-i required if not last argument)
