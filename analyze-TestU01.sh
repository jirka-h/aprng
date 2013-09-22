#!/bin/sh

#grep -A20 "Summary results of [^ ]*Crush" $@
#grep -A9 "Results of speed test" $@
grep -A30 "Summary results of [^ ]*BigCrush" $@ | 
 paragrep -e "Generator:|      Test                          p-value|passed" | grep -v "Version:\|statistics:\|following\|means"
#grep -A30 "Summary results of [^ ]*Crush" $@
