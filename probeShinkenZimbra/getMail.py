#!/usr/bin/python
import getpass, imaplib
import yaml

cfgfile="/var/local/probeShinkenZimbra/probecfg.yaml"

f=open(cfgfile)
cfg=yaml.load(f)



M = imaplib.IMAP4(cfg['mailcfg']['server'])
M.login(cfg['mailcfg']['username'], cfg['mailcfg']['password'])
M.select(cfg['mailcfg']['mailfolder'])
typ, data = M.search(None, 'ALL')
for num in data[0].split():
    typ, data = M.fetch(num, '(RFC822)')
    print 'Message %s\n%s\n' % (num, data[0][1])
M.close()
M.logout()
