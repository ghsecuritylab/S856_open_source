#!/bin/bash

if [ x$1 != x ]; then
    WORK_PATH=$1;
else
    WORK_PATH=".";
fi

# optional debug
find $WORK_PATH -iname "*.mk"|xargs sed -i 's/LOCAL_MODULE_TAGS *:= *optional debug/LOCAL_MODULE_TAGS := optional/g'

# optional eng
find $WORK_PATH -iname "*.mk"|xargs sed -i 's/LOCAL_MODULE_TAGS *:= *optional eng/LOCAL_MODULE_TAGS := optional/g'

# debug eng
find $WORK_PATH -iname "*.mk"|xargs sed -i 's/LOCAL_MODULE_TAGS *:= *debug eng/LOCAL_MODULE_TAGS := optional/g'

# eng debug
find $WORK_PATH -iname "*.mk"|xargs sed -i 's/LOCAL_MODULE_TAGS *:= *eng debug/LOCAL_MODULE_TAGS := optional/g'

# debug
find $WORK_PATH -iname "*.mk"|xargs sed -i 's/LOCAL_MODULE_TAGS *:= *debug/LOCAL_MODULE_TAGS := optional/g'

# eng
find $WORK_PATH -iname "*.mk"|xargs sed -i 's/LOCAL_MODULE_TAGS *:= *eng/LOCAL_MODULE_TAGS := optional/g'

# check the result
find $WORK_PATH -iname "*.mk"|xargs grep LOCAL_MODULE_TAGS|grep debug
find $WORK_PATH -iname "*.mk"|xargs grep LOCAL_MODULE_TAGS|grep eng
