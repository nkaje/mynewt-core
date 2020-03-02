#!/usr/bin/env python3

"""
setup.py file for SWIG
"""

from distutils.core import setup, Extension


#No sources, just export the definitions in otp_tool_err.h
otp_tool_err_module = Extension('_otp_tool_err',
                           sources=['otp_tool_err_wrap.c'],
                           )

setup (name         = 'otp_tool_err',
       version      = '1.0',
       author       = "Juul Labs Inc",
       description  = """OTP Programmer Definition Module""",
       ext_modules  = [otp_tool_err_module],
       py_modules   = ["otp_tool_err"],
       )
