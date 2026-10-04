"""Agrega logs classificados. Recusa referencia ausente, duplicatas e contagem errada."""
import argparse,csv
from collections import Counter
from pathlib import Path

STATUSES=('OK','CORROMPIDO','PERDIDO','NAO_DETECTADO')
def summarize(path,expected):
    with path.open(encoding='utf-8-sig',newline='') as stream: rows=list(csv.DictReader(stream))
    if len(rows)!=expected: raise ValueError(f'{path}: {len(rows)} linhas; esperado {expected}')
    seqs=[r['seq'] for r in rows]
    if len(set(seqs))!=len(seqs): raise ValueError(f'{path}: sequencias duplicadas')
    if any(r['status'] not in STATUSES for r in rows): raise ValueError(f'{path}: classe pendente/invalida')
    counts=Counter(r['status'] for r in rows)
    origin={r['origem'] for r in rows}; techniques={r['tecnica'] for r in rows}; modes={r['modo'] for r in rows}
    if any(len(v)!=1 for v in (origin,techniques,modes)): raise ValueError(f'{path}: rodadas misturadas')
    denom=counts['CORROMPIDO']+counts['NAO_DETECTADO']
    return [origin.pop(),techniques.pop(),modes.pop(),*[counts[s] for s in STATUSES],f'{counts["NAO_DETECTADO"]/denom:.6f}' if denom else 'N/A']

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__); p.add_argument('paths',nargs='+',type=Path)
    p.add_argument('--expected',type=int,default=1000); p.add_argument('--out',type=Path,default=Path('data/tabelas/resumo.csv'))
    a=p.parse_args(); results=[summarize(path,a.expected) for path in a.paths]
    a.out.parent.mkdir(parents=True,exist_ok=True)
    with a.out.open('w',newline='',encoding='utf-8') as stream:
        w=csv.writer(stream); w.writerow(['origem','tecnica','modo',*STATUSES,'taxa_nao_deteccao']); w.writerows(results)
    print(a.out)
