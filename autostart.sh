#!/bin/bash
USER_HOME=$(eval echo ~${SUDO_USER})
if [ ! -d ${USER_HOME}/.config/autostart ] 
then
    mkdir -p ${USER_HOME}/.config/autostart
fi

DESKTOP_FILE=${USER_HOME}/.config/autostart/ashmon.desktop

echo "[Desktop Entry]" > ${DESKTOP_FILE}
echo "Type=Application" >> ${DESKTOP_FILE}
echo "Exec=ashmon" >> ${DESKTOP_FILE}