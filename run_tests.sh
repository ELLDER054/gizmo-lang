#!/usr/bin/env bash

if [ -f build/gizmo ]; then
    echo "Running gizmo tests"
    python3 -m unittest
else
    echo "ERROR: Failed to find build/gizmo needed for running tests"
fi
