#!/bin/bash

swig -python otp_tool_err.i
python3 setup.py build_ext --inplace
chmod a+x otp_tool_err.py
