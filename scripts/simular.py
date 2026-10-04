"""Experimento didatico sintetico: nao comprova ruido real, timing ou UART."""
import argparse, csv, random
from pathlib import Path
from protocolo import frame, decide, classify

def positions(rng,mode):
    # Mesmo dominio do prototipo fisico: dados dos bytes 5,6,7.
    domain=[(byte,bit) for byte in (5,6,7) for bit in range(8)]
    if mode==0: return []
    if mode in (1,2): return rng.sample(domain,mode)
    byte=rng.choice((5,6,7)); size=rng.randint(3,8); start=rng.randint(0,8-size)
    return [(byte,bit) for bit in range(start,start+size)]

def run(algorithm,mode,n,seed,out):
    # Recria o RNG por modo/tecnica: mesmos payloads e posicoes nas duas familias.
    rng=random.Random(seed+mode)
    path=out/f'{algorithm}_modo{mode}.csv'
    with path.open('w',newline='',encoding='utf-8') as stream:
        fields=['origem','tecnica','modo','seq','residuo','decisao','payload_confere','status','tx_hex','rx_hex','bits_tentados','bits_alterados']
        writer=csv.DictWriter(stream,fieldnames=fields); writer.writeheader()
        for seq in range(n):
            payload=bytes([rng.randrange(256),rng.randrange(256)])
            tx=frame(seq&65535,payload,algorithm); rx=bytearray(tx)
            selected=positions(rng,mode); changed=[]
            for byte,bit in selected:
                if rx[byte] & (1<<bit): changed.append((byte,bit))
                rx[byte] &= ~(1<<bit) # LOW: nao e XOR simetrico.
            decision,residue=decide(rx,algorithm)
            same=rx[5:7]==payload
            writer.writerow(dict(origem='SIMULACAO',tecnica=algorithm,modo=mode,seq=seq,residuo=residue,decisao=decision,
                payload_confere='sim' if same else 'nao',status=classify(decision,rx[5:7],payload),
                tx_hex=tx.hex(' ').upper(),rx_hex=rx.hex(' ').upper(),
                bits_tentados=';'.join(f'{b}:{k}' for b,k in selected),bits_alterados=';'.join(f'{b}:{k}' for b,k in changed)))
    return path

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--n',type=int,default=1000); parser.add_argument('--seed',type=int,default=21)
    parser.add_argument('--out',type=Path,default=Path('data/simulacao'))
    args=parser.parse_args()
    if not 1<=args.n<=65536: parser.error('--n deve estar entre 1 e 65536')
    args.out.mkdir(parents=True,exist_ok=True)
    for algorithm,modes in [('sum8',range(4)),('crc8',range(1,4))]:
        for mode in modes: print(run(algorithm,mode,args.n,args.seed,args.out))
