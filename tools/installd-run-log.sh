#!/bin/sh

ant installd || exit $?
adb shell am start -a android.intent.action.MAIN -n chaos.app/chaos.app.ChaosActivity
adb logcat | gawk '{printf("%s - %s\n", strftime("<%FT%T>"), $0);}'
