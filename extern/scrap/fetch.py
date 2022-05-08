import bs4
import re

fin = open('pirate.html', 'r')
fout = open('pirate.txt', 'w')
html = fin.read()
fin.close()

root=bs4.BeautifulSoup(html, 'lxml')

for idx, item in enumerate(root.findAll(id='st')):
    buf = ''
    txt = item.decode_contents()
    size = re.search('value=\"(\d+)', txt).group(1)
    detail = item.findAll('a')
    extra = item.findAll('span')
    
    buf += '\nidx:' + str(idx) + '\nsize:' + size
    buf += '\nmajor:' + detail[0].string
    buf += '\nminor:' + detail[1].string
    buf += '\nname:' + detail[2].string
    buf += '\ndate:' + extra[2].string
    buf += '\nseeds:' + extra[5].string
    buf += '\nleech:' + extra[6].string
    buf += '\n' + detail[3]['href'] + '\nxdi\n'
    fout.write(buf)
fout.close()

