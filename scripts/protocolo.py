"""Modelo de referencia. Decisao nunca recebe payload verdadeiro como argumento."""
START=b'\xaa\x55'
LENGTH=5

def crc8(data):
    value=0
    for byte in data:
        value ^= byte
        for _ in range(8):
            value=((value << 1) ^ (7 if value & 128 else 0)) & 255
    return value

def code(data, algorithm):
    if algorithm=='sum8': return -sum(data) & 255
    if algorithm=='crc8': return crc8(data)
    raise ValueError('Tecnica invalida')

def frame(sequence, payload, algorithm):
    if len(payload)!=2: raise ValueError('Payload precisa de dois bytes')
    body=bytes([LENGTH])+sequence.to_bytes(2,'big')+bytes(payload)
    return START+body+bytes([code(body,algorithm)])

def decide(received, algorithm):
    if len(received)!=8 or received[:2]!=START or received[2]!=LENGTH:
        return 'SEM_CHECAGEM', None
    residue=(sum(received[2:]) & 255) if algorithm=='sum8' else crc8(received[2:])
    return ('ACEITO' if residue==0 else 'DESCARTADO'), residue

def classify(decision, received_payload, reference_payload):
    if decision=='SEM_CHECAGEM': return 'PERDIDO'
    if decision=='DESCARTADO': return 'CORROMPIDO'
    return 'OK' if received_payload==reference_payload else 'NAO_DETECTADO'

def sequence_event(expected, received):
    delta=(received-expected)&65535
    if delta>=32768: return 'REPETIDO_OU_ANTIGO',0,expected
    return 'NORMAL' if delta==0 else 'LACUNA',delta,(received+1)&65535
