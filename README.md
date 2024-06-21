# Simple Terminal Typer

## Usage

```console
 user@computer:~$ stt --help
 ______     ______   ______
/\  ___\   /\__  _\ /\__  _\
\ \___  \  \/_/\ \/ \/_/\ \/
 \/\_____\    \ \_\    \ \_\
  \/_____/     \/_/     \/_/
   Simple    Terminal   Typer

Usage: stt [OPTION] SOURCE

Options:
  -h,--help                     Display help message
  -f,--file                     Use local file as source
  -u,--url                      Use URL to a webpage as source
  -p,--preserve_formatting      Preserve original formatting of the source
```

## If you want to try it out

```console
user@computer:~$ stt -u https://whatthecommit.com/index.txt
```

OR for a harder challenge

```console
user@computer:~$ stt -u https://sherlock-holm.es/stories/plain-text/scan.txt
```

If you want to preserve formatting (for example, to use source code):

```console
user@computer:~$ stt -fp <project_dir>/src/main.c
```


## Building and compatibility

To build use included `Makefile`

NOTE: Using example values for lib and include directories

```console
user@computer:~$ make INCLUDE_DIR=/usr/include LIB_DIR=/usr/lib
```

### Improvements

Project is being built with debug enabled, without optimizations.

Future plans to add release build feature.

`make install` is not currently implemented.

***

Compatible with Linux and other UNIX derivatives.

Tested on Linux & macOS.
