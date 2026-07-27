#!/bin/bash
for file in *320*; do
    mv "$file" "${file/320/_400}"
done
