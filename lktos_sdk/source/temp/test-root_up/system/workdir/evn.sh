#!/bin/sh

ROOT=$PWD

export LANG=UTF-8
export CHARSET=UTF-8
export LD_LIBRARY_PATH=$ROOT/lib:$ROOT/lib/gstreamer-0.10:$LD_LIBRARY_PATH

PATH=$PATH:$ROOT/bin
export PATH
export WORKDIR=$ROOT

#for system
#coredump unlimited
ulimit -c unlimited
#stack in KB
ulimit -s 512

#for QT
export QT_QWS_FONTDIR=$ROOT/lib/fonts
export QWS_MOUSE_PROTO="TSLIB:/dev/event0"
export POINTERCAL_FILE=$TS_CALIBFILE
export QT_PLUGIN_PATH=$ROOT/lib

#for ALSA
export ALSA_CONFIG_DIR=$ROOT/lib/alsa/
export ALSA_CONFIG_PATH=$ALSA_CONFIG_DIR/alsa.conf

#for MPlayer
export MPLAYER_HOME=$ROOT/misc

#for UI
export M1UI_CONFIG=$ROOT/misc/ui_config.xml
export M1UI_BACKEND_CONFIG=$ROOT/misc/m1ui_backend_config.json
export PLAYER_SAVE_DATA=$ROOT/misc/player_save_data.dat
export M1UI_FILE_PATH_PREFIX=$ROOT/misc/
export M1UI_WIFI_PATH_PREFIX=$ROOT/wifi/

ln -s /dev /dev/snd
echo "auto set envirnoment OK"
