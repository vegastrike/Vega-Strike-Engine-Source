#!/bin/bash

while getopts "a:b:h" opt; do
    case $opt in
        a)  echo " a -> ${OPTARG}" ;;
        b)  echo " b -> ${OPTARG}" ;;
        h) echo "HELP !" ;;
        *) echo "Bad or missingad option" ;;
    esac
done

