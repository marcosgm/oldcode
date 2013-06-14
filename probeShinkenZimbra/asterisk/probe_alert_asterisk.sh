#!/bin/bash

#set -x

if [ $# -lt 1 ]; then
	echo Fatal: Use text as argument.
	exit 1
fi

AstUser=asterisk
AstGroup=asterisk
AstOutgoingSpoolDir=/var/spool/asterisk/outgoing
TemplateFile=/etc/asterisk/zabbix/call_template_probe.txt
CallFile=`mktemp`
#CallFile=/tmp/nagiosremotetempfile

# Set text message, Warning! Escape commas for asterisk.
Message="$@"
#probetemp.wav is referenced in /etc/asterisk/exten/clients.conf.bak. Call properties are stored in call_template_probe.txt
echo Probe alert, probe alert: it seems that either Shinken monitoring or Zimbra mail system are down. ... $Message | text2wave -f 8000 >/var/lib/asterisk/sounds/probetemp.wav

# Prepare call file
cat $TemplateFile > $CallFile
chown $AstUser:$AstGroup $CallFile

# Trigger call
#cp -f $CallFile /tmp/toto
mv -f $CallFile $AstOutgoingSpoolDir

