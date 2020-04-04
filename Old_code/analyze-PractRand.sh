#!/bin/bash

#paragrep -e "very suspicious|SUSPICIOUS|FAIL" $@ | grep "^.*log\|length" 

for FILE in "$@"
do
  (echo -ne "${FILE}\t" ; paragrep -e "very suspicious|SUSPICIOUS|FAIL" ${FILE} ) | grep --max-count=2 "^.*log\|length" 
done

