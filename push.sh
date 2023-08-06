#!/usr/bin/bash
git add --all
git rm -f --cached xv6/xv6-riscv
git commit -m "$1"
git push -u origin master
