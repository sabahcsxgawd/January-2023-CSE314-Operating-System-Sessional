#! /usr/bin/bash
cd "/media/sabah/269CFD019CFCCC75/Level 3 Term 2/Sessional/January 2023 CSE314 Operating System Sessional"
git add --all
git rm -rf --cached xv6/xv6-riscv
git commit -m "$1"
git push -u origin master
