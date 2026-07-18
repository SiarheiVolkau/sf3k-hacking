#!/bin/sh
# zhijack.sh for sf3000 — GENERATED from hijack/zhijack.tpl.sh by
# build_release.sh. Do not edit on the SD; edit the template and regenerate.
#
# Reached via stock boot: rkgame (verified, untouched) -> setting.xml autorun ->
# libemu_tfhijack.so forks this script (rkgame stays alive, keeping the
# cubevol gpio -> /tmp/joy_key input pipeline up). Everything device-specific
# is HARDCODED below — no runtime device detection. /tmp/tfdevice.env is still
# written because picoarch and the standalone frontends (pcsx4all, lgpt,
# pico286) read it.
mkdir /tmp/zhijack.lock 2>/dev/null || exit 0

# Diagnostics are OPT-IN so we don't chew the SD card in normal use: logging is
# ON only if the user pre-created /mnt/sdcard/log.txt. Otherwise LOG=/dev/null and
# every write below is a no-op (and picoarch's own dbg_log is gated the same way,
# on log.txt existing). To debug: drop an empty log.txt on the card and reboot.
if [ -f /mnt/sdcard/log.txt ]; then
    LOG=/mnt/sdcard/log.txt
    mv "$LOG" "$LOG.prev" 2>/dev/null
    : > "$LOG"
    echo "=== zhijack boot [sf3000] $(date '+%H:%M:%S' 2>/dev/null) ===" >> "$LOG"
    sync
else
    LOG=/dev/null
fi

# patch_mtd patches SPL binary to enable "USB upgrade" mode and also:
# - fixes bug in USB device enumeration
# - increases USB upgrade mode timeout from 5 sec to 30 sec
# WARNING: this was tested only on a single SF3000 device and might brick your
# device easily, dump SPINOR first. DO NOT uncomment if you're unsure what are you doing.
#/usr/sbin/patch_mtd 2>&1 >> $LOG || echo "Launch patch_mtd failed with code $?" >> $LOG

# reboots device into "USB upgrade" routine
#reset -u

# dump entire SPINOR flash
#cat /proc/mtd >> "$LOG"
#dd if=/dev/mtd0 of=/mnt/sdcard/mtd0.dump || echo "mtd0 dd failed" >> "$LOG"

# dumping BootROM, takes time
# devmem initially absent in rootfs, luckily it is busybox and it was built
# with devmem support - just copy sdcard/rootfs/bin/cp to sdcard/rootfs/bin/devmem
# and you'll get working devmem tool
#i=0
#while [ $i -lt 32768 ]; do
#    devmem $((0x1fc00000 + i)) >> "$LOG"
#    i=$((i + 4))
#done


# Freeze icube (the respawner), THEN kill rkgame. Devices that boot through
# icube (R36SX, SF3000, GB350) respawn any killed rkgame, and the fresh
# instance redraws the stock menu over our frames (flicker/ghosting); a merely
# frozen rkgame (SIGSTOP) glitched the display too. Frozen icube = no respawn;
# dead rkgame = nothing reacting to SELECT+START (its stock exit-game hotkey).
# On rkgame-direct devices (SF3500/HD/SF3100) there is no icube: STOP no-ops.
kill -STOP $(pidof icube) 2>/dev/null
killall rkgame 2>/dev/null
echo "icube frozen, rkgame killed" >> "$LOG"

cat > /tmp/tfdevice.env <<EOF
TF_DEVICE=SF3000
TF_PANEL_W=854
TF_PANEL_H=480
TF_UI_SCALE=150
TF_ASPECT_NUM=16
TF_ASPECT_DEN=9
TF_ROTATE=90
TF_PRESENT=dispframe
TF_DRIVER=/mnt/sdcard/cubegm/driver_sf3000.so
EOF
export TF_DEVICE=SF3000 TF_PANEL_W=854 TF_PANEL_H=480 TF_UI_SCALE=150

# Some "SF3000"-branded units are really SF3500-class hardware (the "v3" / HDMI
# variant): same 854x480 panel geometry, but the SF3500 audio+display driver. The
# reliable tell is the stock driver.so format: classic SF3000 ships a PLAIN ELF
# driver, SF3500-class ships an ENCRYPTED one. If we're booting as SF3000 but the
# stock driver is encrypted, switch to driver_sf3500.so. Otherwise the classic
# SF3000 driver's audio (AUDDEC/I2SO) init fails on this hardware and picoarch
# SIGSEGVs on the NULL sound handle (+0x270) → black-screen boot loop.
if [ "$TF_DEVICE" = SF3000 ] && [ -f /mnt/sdcard/cubegm/driver_sf3500.so ] && \
   [ "$(head -c4 /mnt/sdcard/cubegm/driver.so 2>/dev/null)" != "$(printf '\177ELF')" ]; then
    sed -i -e 's/^TF_DEVICE=.*/TF_DEVICE=SF3500/' \
           -e 's|^TF_DRIVER=.*|TF_DRIVER=/mnt/sdcard/cubegm/driver_sf3500.so|' /tmp/tfdevice.env
    export TF_DEVICE=SF3500
    echo "SF3000 with encrypted (SF3500-class) driver detected → using driver_sf3500.so" >> "$LOG"
fi


echo "processes at boot:" >> "$LOG"; ps >> "$LOG" 2>&1; [ "$LOG" = /dev/null ] || sync

export LD_LIBRARY_PATH=/mnt/sdcard/cubegm/lib:/mnt/sdcard/cubegm/usr/lib:$LD_LIBRARY_PATH

# CPU: force max-performance governor (helps every emulator).
for g in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
    [ -w "$g" ] && echo performance > "$g" 2>/dev/null
done
for c in /sys/devices/system/cpu/cpu*/cpufreq; do
    mx=$(cat "$c/cpuinfo_max_freq" 2>/dev/null)
    [ -n "$mx" ] && [ -w "$c/scaling_min_freq" ] && echo "$mx" > "$c/scaling_min_freq" 2>/dev/null
done

# Input: cubevol (reads gpio -> /tmp/joy_key shm) must be up; picoarch reads the shm.
pidof cubevol >/dev/null 2>&1 || { [ -x /usr/bin/cubevol ] && /usr/bin/cubevol & }
sleep 0.5

# Optional: disable the power-button sleep (FrogUI Settings -> "Disable Sleep",
# default off, applies after restart). Live-patch every cubevol instance in RAM
# so the on-disk binary stays byte-identical -> passes SF3500 boot verification.
# The watcher re-patches each cubevol respawn (FrogUI restarts it for the OSD).
#  is the per-device set of sleep-arm text addresses (empty = device
# not supported). Long-press power-off is a separate path, untouched.
TF_NOSLEEP_ADDRS=""
if [ -n "$TF_NOSLEEP_ADDRS" ] && grep -q '^disable_sleep=on' /mnt/sdcard/frogui/settings.txt 2>/dev/null; then
    echo 0 > /proc/sys/kernel/yama/ptrace_scope 2>/dev/null
    [ -f /mnt/sdcard/cubegm/nosleep ] && /mnt/sdcard/cubegm/nosleep -w $TF_NOSLEEP_ADDRS >/dev/null 2>&1 &
fi

PICOARCH=/mnt/sdcard/cubegm/picoarch
PICOARCH_HI=/mnt/sdcard/cubegm/picoarch_hi
FROGUI_CORE=/mnt/sdcard/cubegm/cores/frogui_libretro.so
LAUNCH=/tmp/frogui_launch.txt

# Self-healing HW-render fallback (disp_frame devices, non-R36SX). A few units 
# can't drive the HW path and picoarch ABORTS on it (SIGABRT/SIGBUS) before it 
# ever renders a frame. Crash-only: count such crashes, and after 2 strikes in 
# one boot switch permanently to software (marker on SD). Crash-only cannot    
# false-trigger on a healthy unit (which never crashes). Once HW has proven    
# itself (/tmp/hw_rendered), later crashes are game bugs, not HW, so ignored.   
FORCE_SW_FLAG=/mnt/sdcard/cubegm/force_sw.flag
HW_CRASH_N=0
if [ -f "$FORCE_SW_FLAG" ]; then export TF_FORCE_SW=1; echo "display: SW forced (marker present)" >> "$LOG"; fi
hw_crash_check() {
    [ -n "$FORCE_SW_FLAG" ] || return 0
    [ -f "$FORCE_SW_FLAG" ] && return 0
    [ -f /tmp/hw_rendered ] && return 0
    [ "$1" -ge 129 ] 2>/dev/null || return 0
    HW_CRASH_N=$((HW_CRASH_N+1))
    echo "HW crash rc=$1 (strike $HW_CRASH_N, before any HW frame)" >> "$LOG"
    if [ "$HW_CRASH_N" -ge 2 ]; then
        touch "$FORCE_SW_FLAG"; export TF_FORCE_SW=1; sync
        echo "2x HW crash → forcing software rendering permanently" >> "$LOG"
    fi
}
# R36SX (no FORCE_SW_FLAG): make hw_crash_check a harmless no-op.
[ -n "$FORCE_SW_FLAG" ] || hw_crash_check() { :; }

ITER=0
while true; do
    ITER=$((ITER+1))
    rm -f "$LAUNCH"
    killall rkgame 2>/dev/null
    echo "--- iter $ITER: frogui ---" >> "$LOG"
    "$PICOARCH" "$FROGUI_CORE" "$FROGUI_CORE" >> "$LOG" 2>&1
    RC=$?
    echo "frogui exited rc=$RC" >> "$LOG"
    hw_crash_check "$RC"

    if [ -f "$LAUNCH" ]; then
        CORE_PATH=$(sed -n '1p' "$LAUNCH")
        ROM_PATH=$(sed -n '2p' "$LAUNCH")
        rm -f "$LAUNCH"
        if [ -n "$CORE_PATH" ] && [ -n "$ROM_PATH" ]; then
            killall rkgame 2>/dev/null
            sleep 0.3
            BIN="$PICOARCH"
            case "$CORE_PATH" in
                *gpsp*|*pcsx*|*ps1*) [ -f "$PICOARCH_HI" ] && BIN="$PICOARCH_HI" ;;
            esac
            echo "--- iter $ITER: game [$CORE_PATH] via $BIN ---" >> "$LOG"
            "$BIN" "$CORE_PATH" "$ROM_PATH" >> "$LOG" 2>&1
            GRC=$?
            echo "game exited rc=$GRC" >> "$LOG"
            hw_crash_check "$GRC"
        fi
    fi
    sleep 0.2
done
