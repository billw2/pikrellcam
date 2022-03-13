# PiKrellCam Fork

This repo contains a for of pikrellcam that is compatible with Debian, Arch and soon Alpine. See below for installation instructions and the original repo links.

## PiKrellCam

PiKrellCam is an audio/video recording motion detect program with an OSD web
interface that detects motion using the Raspberry Pi camera MMAL motion vectors.

Read about it at the original developers page:
[PiKrellCam webpage](http://billw2.github.io/pikrellcam/pikrellcam.html)

Git download with:
    $ git clone https://github.com/olipinski/pikrellcam
    
---

## Arch specific instructions

To install on Arch you might want the following package from AUR:

https://aur.archlinux.org/packages/mpack/

---

## Alpine specific instructions

Alpine installation requires it to be in ``sys`` mode.

``doas`` is used by default. If your Alpine installation uses ``sudo`` instead , you might want to install ``doas`` instead, or modify the bash files in this repo.

---

## Camera configuration in /boot/config.txt

Also you will need to setup the camera to work on Arch/Alpine. This can usually be done with edits to /boot/config.txt, but might depend on the specific Pi. For example on the PiZero this could be the config:

```
# See /boot/overlays/README for all available options
gpu_mem=128
start_file=start_x.elf
fixup_file=fixup_x.dat
initramfs initramfs-linux.img followkernel
cma_lwm= cma_hwm= cma_offline_start=
disable_camera_led=1
```
