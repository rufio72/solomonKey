# Generatore di ingressi casuali per gioca --riproduci (brief 52).
# Uso: python3 genera_ingressi.py seme file fine [passo]
# Moneta ai frame 1511-1530 (e602=04) e start 1561-1580 (e602=01) come in
# out/ingressi_replay.txt; dal frame 1900 un valore a caso fra 01 02 04 08 10 20
# in e600, cambiato ogni `passo` frame (predefinito 8).
import random,sys
seed=int(sys.argv[1]); out=sys.argv[2]; fine=int(sys.argv[3]); passo=int(sys.argv[4]) if len(sys.argv)>4 else 8
r=random.Random(seed); v=[0x01,0x02,0x04,0x08,0x10,0x20]
cur=0
with open(out,'w') as f:
    for fr in range(66,fine):
        e602=0
        if 1511<=fr<1531: e602=4
        if 1561<=fr<1581: e602=1
        if fr>=1900 and (fr-1900)%passo==0: cur=r.choice(v)
        e600=cur if fr>=1900 else 0
        f.write("%d %02X 00 %02X\n"%(fr,e600,e602))
