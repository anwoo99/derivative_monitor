#!/bin/bash

export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:$HOME/rabbit/lib:$HOME/fep/lib:$HOME/grms/lib:$HOME/Monitor/lib:

echo "========= MONSERVER  ========="

# Check if monserver is already running
if pgrep -x "monserver" >/dev/null; then
	echo "monserver is already running."
else
	${HOME}/Monitor/bin/monserver &
	echo "monserver started."
fi

echo "    "
