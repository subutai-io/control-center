#!/bin/bash
export PROJECT_NUMBER_EXTERNAL=$(cat version)
doxygen tray_doxyfile
