import sys
from PIL import Image

def rd(p):
    with open(p,'rb') as f: return f.read()

def bit(data, off):
    return (data[off>>3] >> (7-(off&7))) & 1

def decode(data, w, h, planes, xoff, yoff, charsize, count):
    tiles=[]
    for n in range(count):
        base=n*charsize
        t=[[0]*w for _ in range(h)]
        for y in range(h):
            for x in range(w):
                v=0
                for p,po in enumerate(planes):
                    v |= bit(data, base+po+xoff[x]+yoff[y]) << p
                t[y][x]=v
        tiles.append(t)
    return tiles

def sheet(tiles, w, h, cols, path, scale=2):
    rows=(len(tiles)+cols-1)//cols
    img=Image.new('P',(cols*w,rows*h))
    pal=[]
    # 16-level palette, index 0 dark blue to spot transparency
    for i in range(16):
        g=i*17
        pal += [g,g,g]
    pal[0:3]=[40,0,60]
    img.putpalette(pal+[0]*(768-len(pal)))
    px=img.load()
    for n,t in enumerate(tiles):
        ox=(n%cols)*w; oy=(n//cols)*h
        for y in range(h):
            for x in range(w):
                px[ox+x,oy+y]=t[y][x]
    img=img.resize((img.width*scale,img.height*scale),Image.NEAREST)
    img.save(path)
    print(path, len(tiles), "tiles", img.size)

d, out = sys.argv[1], sys.argv[2]
def tile8(a,b,name):
    data=rd(a)+rd(b)
    planes=[0,1,2,3]; xo=[i*4 for i in range(8)]; yo=[i*32 for i in range(8)]
    t=decode(data,8,8,planes,xo,yo,32*8,len(data)//32)
    sheet(t,8,8,64,out+'/'+name+'.png')
def spr16(files,name):
    data=b''.join(rd(f) for f in files); q=len(data)//4*8
    planes=[0,q,2*q,3*q]; xo=list(range(8))+[64+i for i in range(8)]
    yo=[i*8 for i in range(8)]+[128+i*8 for i in range(8)]
    t=decode(data,16,16,planes,xo,yo,32*8,len(data)//4//32)
    sheet(t,16,16,32,out+'/'+name+'.png')
set_=sys.argv[3]
if set_=='us':
    tile8(d+'/12.3t',d+'/11.3r','us_chars')
    tile8(d+'/10.3p',d+'/9.3m','us_tiles')
    spr16([d+'/2.5lm',d+'/3.6lm',d+'/4.7lm',d+'/5.8lm'],'us_sprites')
else:
    tile8(d+'/slmn_12.bin',d+'/slmn_11.bin','jp_chars')
