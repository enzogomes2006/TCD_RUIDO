import unittest,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from protocolo import crc8,frame,decide,classify,sequence_event
from analisar import summarize
import tempfile,csv

class ProtocolTests(unittest.TestCase):
    def test_crc_known_vector(self): self.assertEqual(crc8(b'123456789'),0xF4)
    def test_valid_frames(self):
        for alg in ('sum8','crc8'):
            for seq in (0,1,255,256,65535): self.assertEqual(decide(frame(seq,b'\x80\x80',alg),alg),('ACEITO',0))
    def test_every_single_protected_bit(self):
        for alg in ('sum8','crc8'):
            tx=frame(17,b'\xa5\x5a',alg)
            for byte in range(2,8):
                for bit in range(8):
                    rx=bytearray(tx);rx[byte]^=1<<bit
                    self.assertNotEqual(decide(rx,alg)[0],'ACEITO')
    def test_low_only_collision(self):
        tx=frame(1,b'\x80\x80','sum8');rx=bytearray(tx);rx[5]=rx[6]=0
        self.assertEqual(tx.hex(' '),'aa 55 05 00 01 80 80 fa')
        self.assertEqual(decide(rx,'sum8'),('ACEITO',0))
        self.assertEqual(classify('ACEITO',rx[5:7],tx[5:7]),'NAO_DETECTADO')
    def test_crc_catches_same_pattern(self):
        rx=bytearray(frame(1,b'\x80\x80','crc8'));rx[5]=rx[6]=0
        self.assertEqual(decide(rx,'crc8')[0],'DESCARTADO')
    def test_all_bursts_up_to_eight_bits(self):
        tx=frame(5,b'\xff\xff','crc8')
        for size in range(1,9):
            for start in range(48-size+1):
                rx=bytearray(tx)
                # Bits na ordem matematica MSB; exclui marcador e inclui CHECK.
                for pos in range(start,start+size): rx[2+pos//8]^=1<<(7-pos%8)
                self.assertNotEqual(decide(rx,'crc8')[0],'ACEITO')
    def test_sequence_wrap_and_old(self):
        self.assertEqual(sequence_event(65535,0),('LACUNA',1,1))
        self.assertEqual(sequence_event(0,0),('NORMAL',0,1))
        self.assertEqual(sequence_event(10,9),('REPETIDO_OU_ANTIGO',0,10))
    def test_classification_exclusive(self):
        self.assertEqual(classify('DESCARTADO',b'xx',b'xx'),'CORROMPIDO')
        self.assertEqual(classify('SEM_CHECAGEM',b'',b'xx'),'PERDIDO')
    def test_analysis_rejects_pending_or_duplicate(self):
        with tempfile.TemporaryDirectory() as folder:
            path=Path(folder)/'x.csv'
            path.write_text('seq,status,origem,tecnica,modo\n1,SEM_REFERENCIA,REAL,sum8,0\n',encoding='utf-8')
            with self.assertRaises(ValueError): summarize(path,1)
            path.write_text('seq,status,origem,tecnica,modo\n1,OK,REAL,sum8,0\n1,OK,REAL,sum8,0\n',encoding='utf-8')
            with self.assertRaises(ValueError): summarize(path,2)

if __name__=='__main__': unittest.main()
