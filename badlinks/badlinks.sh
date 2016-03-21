#!/bin/bash
weekago=`date -d 'week ago' +%s`

walk() {
    for name in "$1"/* 
    do
        if [ -d "${name}" ]
            then
                walk ${name}
            else
                if [ -h "${name}" -a `stat -c %Y "${name}"` -lt "$weekago" ] 
                    then
                        echo ${name}
                fi
        fi
    done
}

walk $1
