#! /bin/bash
ls assets/src/ | while read in; do inkscape --export-filename=assets/gen/$in.pdf assets/src/$in; done
