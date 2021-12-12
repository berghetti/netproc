#!/bin/sh
git describe 2>/dev/null | cut -d '-' -f 1
