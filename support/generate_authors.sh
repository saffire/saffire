#!/bin/sh

cat << EOH
Saffire is created by:
 Joshua Thijssen

Contributors:
EOH

git log --format='%aN <%aE>' | sort |  uniq -c | sort -nr
