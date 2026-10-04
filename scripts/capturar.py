"""Captura linhas CSV sem alterar seus campos; comandos configuram metadados no firmware."""
import argparse,time
from pathlib import Path

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--port',required=True); p.add_argument('--out',required=True,type=Path)
    p.add_argument('--seconds',type=float,default=120); p.add_argument('--command',action='append',default=[])
    a=p.parse_args()
    import serial
    a.out.parent.mkdir(parents=True,exist_ok=True)
    # Modo x evita sobrescrever evidencias de uma rodada anterior.
    with a.out.open('x',encoding='utf-8',newline='') as target,serial.Serial(a.port,115200,timeout=.2) as port:
        time.sleep(2) # Abertura USB pode reiniciar a placa.
        for command in a.command: port.write((command+'\n').encode('ascii'))
        end=time.monotonic()+a.seconds
        while time.monotonic()<end:
            line=port.readline().decode('utf-8',errors='replace').strip()
            if line: target.write(line+'\n'); target.flush()
