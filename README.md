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
  -h,--help     Display help message
  -f,--file     TEXT Path to file, used for typing
  -u,--url      TEXT URL of a PLAINTEXT page, used for typing
```

## If you want to try it out

```console
user@computer:~$ stt -u https://whatthecommit.com/index.txt
```

OR for a harder challenge

```console
user@computer:~$ stt -u https://sherlock-holm.es/stories/plain-text/scan.txt
```

## Building and compatibility

To build use included `Makefile`

***

Compatible with Linux and UNIX derivatives.

Tested on Linux & MacOS.
