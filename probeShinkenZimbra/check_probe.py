#!/usr/bin/python
import yaml
import optparse

statusfile="/var/local/probeShinkenZimbra/probestatus.yaml"

parser=optparse.OptionParser("usage: %prog [-s]")
parser.add_option("-s","--set", dest="setprobe", type="int", help="Set probe to 0 (OK), 1 (WARNING) or 2 (CRITICAL)")

(options,args)=parser.parse_args()
if options.setprobe!=None:
        set_probe=options.setprobe
        if set_probe not in (0, 1, 2):
                parser.error("Set probe must be 0, 1 or 2")
        f=open(statusfile,"rw")
        cfg=yaml.load(f)
        print "Changing status from %d to %d"%(cfg['status'],set_probe)
        cfg['status']=set_probe
        f.close()
        stream=file(statusfile,"w")
        yaml.dump(cfg,stream)

elif len(args) == 0:
        f=open(statusfile)
        cfg=yaml.load(f)
        #print yaml.dump (cfg)
	if cfg['status'] == 2: res="CRITICAL"
	elif cfg['status'] == 1: res="WARNING"
	elif cfg['status'] == 0: res="OK"
	print res,"Probe status is %d"%cfg['status']
        exit(cfg['status'])
