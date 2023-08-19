import sys
import os
import glob
from os.path import basename
Import("env")

files = glob.glob('/dev/tty.usbmodem*')

platform = env.PioPlatform()
upload_port = env.GetProjectOption("upload_port") 
for f in files:
    upload_port = upload_port if upload_port is not None else f

def dfu_upload(source, target, env):
    firmware_path = str(source[0])
    firmware_name = basename(firmware_path)

    genpkg = "".join(["nrfutil pkg generate --hw-version 52 --sd-req=0x00 --application ", firmware_path, " --application-version 1 firmware.zip"])
    dfupkg = "".join(["nrfutil dfu usb-serial -pkg firmware.zip -p ", upload_port])
    print( genpkg )
    print( dfupkg )
    os.system( genpkg )
    os.system( dfupkg )

    print("Uploading done.")


# Custom upload command and program name
env.Replace(PROGNAME="firmware", UPLOADCMD=dfu_upload)