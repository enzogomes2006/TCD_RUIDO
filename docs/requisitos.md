# Matriz de atendimento e evidencias

O usuario definiu a demonstracao fisica como EXTRA. Esta matriz preserva a distincao entre essa orientacao e o PDF, que solicita ruido real. 'Previsto' nao significa 'comprovado'.

| Requisito | Onde preparar | Evidencia para fechar | Situacao |
|---|---|---|---|
| Relatorio PDF bem explicado e link GitHub | modelo + guia + README | PDF autoral com link funcionando | modelo pronto; texto final pendente |
| Defesa individual em grupo de 3 | roteiro + contribuicoes | cada aluno explica todo o sistema | ensaio pendente |
| Especificacao antes do codigo | especificacao.md | equipe revisa e registra versao | proposta v0.1 |
| START, SEQ, payload >=2, uma tecnica, LEN/fim | protocolo de 8 bytes | desenho e exemplo | proposto e testado no modelo |
| RX calcula residuo incluindo CHECK | biblioteca + scripts + RX | exemplos de aceito e descartado | modelo testado; placa pendente |
| Descarte sem entregar dados/retransmitir | especificacao + RX | codigo e log | descrito; validacao pendente |
| Lacunas, wrap, repetido, fora de ordem | especificacao + testes | teste de 65535->0, lacuna e repeticao | modelo testado; reconciliacao real pendente |
| Timeout medido e recuperacao | extra + TX/RX | captura ou log real, justificativa, tempo em ms | valores iniciais; nao medidos |
| UART2, 115200, GND, tres ESP32 | extra + firmware | montagem e captura | extra por orientacao do usuario |
| NPN + resistor e LOW assimetrico | extra | foto/circuito e log alteracao real | extra; nao montado |
| Modos 0..3 via Monitor Serial | noise.ino | troca sem regravar | prototipo pendente |
| Pulso menor que byte; rajada no mesmo byte | noise.ino + especificacao | analisador logico confirma posicao/duracao | temporizacao pendente |
| Timer adequado; sem delayMicroseconds | noise.ino | GPTimer e captura temporal | implementacao proposta |
| Log ruido: seq/modo/bits/tempo/mudou | noise.ino | log cruzado com RX e TX | nao coletado |
| Referencia depois da decisao e sem ruido | TX/RX + protocolo | captura e revisao fluxo | modelo correto; bancada pendente |
| 1000/modo: rodada1 0..3; rodada2 1..3 | simulacao + extra | 7000 quadros reais se exigidos | simulacao disponivel; reais ausentes |
| Duas tabelas, quatro classes somam 1000 | tabelas + analisador | CSV reconciliado sem pendencias | modelos vazios |
| Taxa ND/(CORROMPIDO+ND) | roteiro + analisador | denominador e N/A se zero | implementada |
| Tempo TX/RX, overhead, recuperacao | TX/RX + protocolo | medidas reais e metodo | overhead teorico 6 bytes; tempos pendentes |
| Um erro nao detectado REAL byte a byte | roteiro + extra | TX hex + RX hex + log ruido + calculo | exemplo teorico pronto; real ausente |
| Predicao individual dos proximos 10 | roteiro | previsoes anteriores e resultados | ensaio pendente |
| Quebrar o proprio protocolo | roteiro + testes | padrao + algebra + evidencia exigida | colisao teorica SUM8 testada |
| tx.ino, rx.ino, noise.ino compilaveis | firmware + compilar.ps1 | compilacao com versao/placa registradas | compilacao ESP32 nao executada |
| Autoria e dominio individual | contribuicoes + README | registro honesto e defesa pessoal | a construir pela equipe |

Nota editorial: a linha 'OK' no PDF contem 'checagem indicou erro', embora diga 'aceitou'. Interpretacao consistente com o restante: OK significa checagem passou e payload correto. Explicitem isso no relatorio.
