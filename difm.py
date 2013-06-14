import urllib
import re

#print "INICIO programa"
#Programa para descargarse la lista de radios de la web DI.FM. Las radios son URLs de ficheros acabados en .pls, pero por cada estilo de musica hay varios enlaces .pls, segun la calidad de la musica, asi que solo se descargara un enlace PLS de MP3 a 96kbps. Ademas, abrira cada enlace y se lo descargara, ofreciendo como resultado en pantalla las URL con los streams de musica Shoutcast. El resultado que aparece en pantalla estará en formato M3U:

#Ejemplo de URL de fichero PLS:
#'<td><a href="http://www.sky.fm/mp3/world.pls">\n<d><a href="/mp3/trance.pls"><img sr'

#Ejemplo de M3U resultante:
#EXTM3U
#EXTINF:0,Electro House - Digitally Imported Radio 
#http://scfire-ntc-aa03.stream.aol.com:80/stream/1025


MP3_STR="/mp3/"
AAC_STR="/aacplus/"
DIFM_URL="http://www.di.fm"
SKY_URL="http://www.sky.fm"
        
class DIFM:
    def __init__(self):
        variable=2
        parser = DIFM_HTML_Parser()
        plsList = parser.getPLSurls(MP3_STR)
        handler = PLS_Parser()
        #radioList = handler.getAllUrl(plsList)
        m3u = handler.getM3U(plsList)
        for line in m3u:
            print line
    def printme(self, text):
        print text

class DIFM_HTML_Parser:
    def __init__(self):
        self.listPLS =[]
        self.getHTML()
    def getHTML(self):
        #print "Voy a obtener "+DIFM_URL
        self.html=urllib.urlopen(DIFM_URL)
        self.htmtBuff=self.html.read()
    def getPLSurls(self, typeOfStream):
        self.skyFilter=SKY_URL+typeOfStream+'\w+\.pls'
        self.difmFilter='(?<!'+SKY_URL+')'+typeOfStream+'\w+\.pls' #solo si NO esta precedido por la SKY_URL
        
        #test1='<td><a href="http://www.sky.fm/mp3/world.pls">\n<d><a href="/mp3/trance.pls"><img sr'
        #p = re.compile(SKY_URL+'/mp3/\w+\.pls|/mp3/\w+\.pls')
        #m= p.search('<td> "/mp3/prueba.pls">')
        p = re.compile(self.difmFilter)
        l=p.findall(self.htmtBuff)
        if l:
            for match in l:
                self.listPLS.append (DIFM_URL+match)
        else:
            print "EH TU: Not matches found for DI.FM"
        numDIFM = len(self.listPLS)
        #print "Se han encontrado ",numDIFM," radios de DI.FM"
            
        p = re.compile(self.skyFilter)
        l=p.findall(self.htmtBuff)
        if l:
            for match in l:
                self.listPLS.append (match)
        else:
            print "EH TU: Not matches found for SKY"
        #print "Se han encontrado ",len(self.listPLS)-numDIFM," radios de SKY.FM"
        return self.listPLS
        
class PLS_Parser:
    def getM3U (self,PLSlist):
        buf = ["#EXTM3U"]
        for pls in PLSlist:
            dl = self.downloadPLS(pls)
            title = self.getFirstTitle(dl)
            url = self.getFirstUrl(dl)
            buf.append("#EXTINF:0,"+title)
            buf.append(url)
        return buf
    def getAllUrl(self,PLSlist):
        urllist = []
        for pls in PLSlist:
            dl = self.downloadPLS(pls)
            urllist.append (self.getFirstUrl(dl))
        return urllist
    def getFirstUrl(self,text):
        text2match="File1="
        pos1= text.find(text2match)
        pos2= text.find("\n",pos1)
        pos1+=len(text2match)
        url = text[pos1:pos2]
        #print "Found url: "+url
        return url
    def getFirstTitle(self,text):
        text2match="Title1="
        pos1= text.find(text2match)
        pos2= text.find("\n",pos1)
        pos1+=len(text2match)
        title = text[pos1:pos2]
        #print "Found : "+title
        return title
    def downloadPLS(self, url):
        return urllib.urlopen(url).read()
        
        
d = DIFM()
