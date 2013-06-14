#!/usr/bin/python
import getpass, imaplib, email
import yaml
import re
cfgfile="/var/local/probeShinkenZimbra/probecfg.yaml"

f=open(cfgfile)
cfg=yaml.load(f)

def validateMail(mail="",notificationtype="",state=""):
        first_cond=re.compile("\nNotification Type: "+notificationtype)
        second_cond=re.compile("\nState: "+state)
        if (first_cond.search(mail) and second_cond.search(mail)):
                return True
        else:
                return False

def validateMailKO(mail=""):
        warn=validateMail(mail,"PROBLEM","WARNING")
        critical=validateMail(mail,"PROBLEM","CRITICAL")
        return warn or critical

def validateMailOK(mail=""):
        return validateMail(mail,"RECOVERY","OK")

M = imaplib.IMAP4(cfg['mailcfg']['server'])
M.login(cfg['mailcfg']['username'], cfg['mailcfg']['password'])
M.select(cfg['mailcfg']['mailfolder'])
typ, data = M.search(None, 'ALL')
for num in data[0].split():
        typ, data = M.fetch(num, '(RFC822)')
        mail=email.message_from_string(data[0][1])
        print 'Message %s : %s' % (num, mail['subject'])

        if (validateMailOK(data[0][1])):
                print "Message is an OK"
                M.copy(num,'Probes') #Store mails for future study
                M.store(num, '+FLAGS', '\\Deleted')
        elif (validateMailKO(data[0][1])):
                print "Message is a KO"
                M.copy(num,'Probes') #Store mails for future study
                M.store(num, '+FLAGS', '\\Deleted')
        else:
                print "Unknown Message ", data[0][1]
                M.copy(num,'Unknown') #Store mails for future study
                M.store(num, '+FLAGS', '\\Deleted')

M.close() #Deleted messages are removed from writable mailbox.
M.logout()
