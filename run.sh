#!/bin/bash
#* ******************************************************************************
#*  If not stated otherwise in this file or this component's LICENSE
#*  file the following copyright and licenses apply:
#*
#*  Copyright 2025 RDK Management
#*
#*  Licensed under the Apache License, Version 2.0 (the "License");
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*  http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an "AS IS" BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#* ******************************************************************************

DEV_BINDER='/dev/binder'
RUN_DIR=$(realpath "$(dirname "$0")")

# Check binder device permission
if [ ! -w "$DEV_BINDER" ]; then
    echo "No permission to access $DEV_BINDER, consider executing this as root"
    exit 1
fi

# Set library path
export LD_LIBRARY_PATH="$RUN_DIR:$LD_LIBRARY_PATH"

# Check if servicemanager is already running
if pidof servicemanager > /dev/null; then
    echo "Service 'servicemanager' is already running"
    servicemanager_pid=$(pidof servicemanager)
else
    echo "Executing [servicemanager]"
    "$RUN_DIR/servicemanager" &
    servicemanager_pid=$!
    sleep 2
    if ! ps -p "$servicemanager_pid" > /dev/null; then
        echo "Failed to start servicemanager"
        exit 1
    fi
fi

# Prompt user to start EmulatePlaneControlService
echo "Do you want to start the mock service [EmulatePlaneControlService]? (y/n)"
read -r user_choice
case "$user_choice" in
    [Yy]*)
        echo "Executing [EmulatePlaneControlService]"
        "$RUN_DIR/EmulatePlaneControlService" &
        PlaneControlService_pid=$!
        sleep 2
        if ! ps -p "$PlaneControlService_pid" > /dev/null; then
            echo "Failed to start EmulatePlaneControlService"
            if [ -n "$servicemanager_pid" ] && ps -p "$servicemanager_pid" > /dev/null; then
                kill -9 "$servicemanager_pid"
            fi
            exit 1
        fi
        ;;
    [Nn]*)
        echo "Skipping [EmulatePlaneControlService]"
        PlaneControlService_pid=""
        ;;
    *)
        echo "Invalid input. Please enter 'y' or 'n'. Skipping [EmulatePlaneControlService]"
        PlaneControlService_pid=""
        ;;
esac

# Execute vts_planeControl
echo "Executing [vts_planeControl]"
"$RUN_DIR/vts_planeControl" "$@"
