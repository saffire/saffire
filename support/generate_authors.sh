#!/bin/sh

cat << EOH
Saffire is originally created by Joshua Thijssen, with the help of (many) contributors:

EOH

git log --format='%aN <%aE>' | sort |  uniq -c | sort -nr
