#!/bin/sh

### If you have many URL's that you add to your vitunes database using the
### "addurl" e-command, a script such as this can be handy to automate the
### adding of them.  Especially when the database format changes.

### General format is the following (uncomment each line)
#echo "artist name\n\
#album name\n\
#title name\n\
#track number\n\
#year\n\
#genre\n\
#play-length\n\
#comment\n" | vitunes -e addurl "http://path.to/url"

# two samples follow...

# wvxu
echo "WVXU Online Radio\n\
Cincinnati Public Radio\n\
NPR\n\
\n\
\n\
Radio\n\
inf\n\
Local NPR Affiliate (All News)\n" | vitunes -e addurl "http://cpr.streamguys.net/wvxu"

# wnku
echo "WNKU Online Radio\n\
Northern Kentucky Public Radio\n\
NPR + Bluegrass\n\
\n\
\n\
Radio\n\
inf\n\
Local NPR Affliate (News+Music)\n" | vitunes -e addurl "http://www.publicbroadcast.net/wnku/ppr/wnku.pls"

