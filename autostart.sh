#!/bin/bash
USER_HOME=$(eval echo ~${SUDO_USER})
if [ ! -d ${USER_HOME}/.config/autostart ] 
then
    mkdir -p ${USER_HOME}/.config/autostart
fi
echo -e "[Desktop Entry]\nType=Application\nExec=ashmon" > ${USER_HOME}/.config/autostart/ashmon.desktop
echo "ashmon configured as an autostart application successfuly."