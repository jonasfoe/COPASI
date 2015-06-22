#!/usr/bin/bash
#
# create video clip with Ken Burns effect
rm -rf credits/*
#stills2dv -fastrender -tmpdir panpapers panpapers.s2d
stills2dv -tmpdir credits credits.s2d
mencoder mf://credits/*.jpg -mf w=800:h=600:fps=25:type=jpg -ovc lavc -lavcopts vcodec=mpeg4:mbd=2:trell:vbitrate=16000 -oac copy -o credits.avi
