#!/bin/bash 
if [ "$#" -ne 1 ]; then
    echo "Usage: create_icons_from_svg.sh harbour-appname.svg"
else
    array=( 108x108  128x128  256x256  86x86 )
    for res in "${array[@]}"
    do
        mkdir -p $res
        # wahh imagemagick convert does not capable of converting svg to png with transparent background properly -> use inkscape then resize
        inkscape --export-png=$res/$(basename "$1" | cut -d. -f1).png --export-dpi=1200 --export-background-opacity=0 --without-gui $1
        convert  -resize $res -background transparent $res/$(basename "$1" | cut -d. -f1).png $res/$(basename "$1" | cut -d. -f1).png
    done
fi
