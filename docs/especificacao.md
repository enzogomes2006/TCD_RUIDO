# Especificacao proposta v0.1 - revisar e congelar antes da bancada

## Enlace e formato

UART2, 115200 baud, 8N1, GND comum. Sem retransmissao. Uma tecnica por rodada.
Checksum SUM8 e CRC-8 usam exatamente o mesmo campo de 1 byte; nenhuma rodada usa ambos.

| Offset | Campo | Bytes | Regra |
|---|---|---|---|
| 0..1 | START | 2 | AA 55 |
| 2 | LEN | 1 | 05: tamanho de SEQ+PAYLOAD+CHECK |
| 3..4 | SEQ | 2 | uint16 big endian; comeca em 0 e volta de 65535 para 0 |
| 5..6 | PAYLOAD | 2 | P0 e P1; RX nao conhece o gerador |
| 7 | CHECK | 1 | SUM8 ou CRC-8/SMBUS |

Quadro: 8 bytes. Payload: 2 bytes. Overhead: 6 bytes = 300% do payload, ou 75% do quadro. Especifique o denominador ao usar percentual. Referencia e USB nao entram no overhead.
Escolhemos comprimento para delimitar por contagem, evitando tratar um marcador de fim dentro do payload como terminador. LEN e fixo e deve ser 05. START pode aparecer nos dados: durante quadro reconhecido ele nao reinicia o parser; fora do quadro ele e procurado novamente.

Protegidos: LEN, SEQ alto, SEQ baixo, P0 e P1. O RX tambem processa CHECK na verificacao. START nao e protegido: alteracao nele pode causar perda de sincronismo.

## SUM8

CHECK = -(LEN + SEQ_H + SEQ_L + P0 + P1) mod 256.
RX: soma dos seis bytes, incluindo CHECK, modulo 256. Residuo esperado: 0.
Essa convencao difere do exemplo do curso que envia a soma positiva e depois compara. Aqui a convencao de residuo satisfaz explicitamente o enunciado.

## CRC-8/SMBUS

width=8; poly=0x07 (x^8+x^2+x+1); init=0x00; refin=false; refout=false; xorout=0x00.
Processamento matematico MSB primeiro por byte. UART envia cada byte LSB primeiro: sao ordens distintas e isso deve ser explicado.
CHECK = CRC dos cinco bytes protegidos. RX processa os seis bytes. Residuo esperado: 00. Vetor de teste ASCII `123456789`: F4.

## Temporizacao proposta

Tbit = 1/115200 = 8,6806 us. Byte 8N1 = 10 bits = 86,8056 us.
8 bytes exigem 694,44 us de transmissao efetiva. Depois dos primeiros cinco bytes o TX reserva uma pausa nominal de 2 ms para armar o prototipo do injetor. A duracao real pode ser maior pelo escalonamento.
Periodo nominal entre quadros: 100 ms. A referencia vem apos o quadro e uma pausa nominal de 1 ms.
Essas pausas sao constantes nas duas tecnicas e precisam aparecer na medicao; nao sao bytes de overhead.

Timeout parcial inicial: 10 ms desde o ultimo byte consumido, em FRAME ou DEBUG. Timeout de ausencia inicial: 250 ms desde a ultima checagem concluida, ou desde inicializacao/reset.
Sao valores propostos, nao medidos. Confirme no piloto os intervalos entre bytes, a pausa de armamento e o periodo entre quadros. Justifique a margem e ajuste as constantes com os dados. Logs de consumo USB/UART nao substituem captura dos pulsos do injetor.

## Decisao, referencia e ressincronizacao

1. Procure AA 55. Leia LEN e, se igual a 05, conte os cinco bytes seguintes.
2. Ao terminar, calcule o residuo antes de procurar/ler a referencia. Aceite apenas se zero; se falhar, descarte o payload da aplicacao.
3. Salve bytes e decisao para auditoria; volte a procurar marcadores a partir do byte imediatamente posterior ao ultimo byte consumido. A especificacao nao faz busca retroativa no quadro rejeitado.
4. LEN invalido ou timeout abandona a montagem e volta a SEEK. Pode haver falso alinhamento; os testes de recuperacao precisam mostrar sua duracao.
5. Referencia separada: D3 3D SEQ_H SEQ_L P0 P1. Tem seis bytes, nao recebe codigo de deteccao e nao altera a decisao. Pode associar a sequencia verdadeira ao registro somente depois da decisao.
6. Sem checagem concluida e com referencia identificada: PERDIDO. Com checagem falha: CORROMPIDO. Com checagem zero e payload igual: OK. Com checagem zero e payload diferente: NAO_DETECTADO.
7. Referencia ausente: SEM_REFERENCIA e bloqueio de fechamento da tabela. Nao chamar automaticamente de OK ou PERDIDO.

## Sequencia sem dupla contagem

Para quadros aceitos, delta = (SEQ_recebida - SEQ_esperada) mod 65536. Delta=0 e normal. 1..32767 indica lacuna observada; depois espera SEQ_recebida+1 mod 65536. 32768..65535 indica repetido/antigo/ambiguidade; nao avanca expected nem infere milhares de perdas.
A primeira sequencia aceita ancora expected. O RX nao confia em SEQ de um quadro rejeitado. Um SEQ corrompido que passou pela checagem pode enganar o rastreamento; isso e uma limitacao e deve ser auditado.
Lacuna observada nao e contagem final de PERDIDO: pode incluir quadros anteriormente CORROMPIDOS. O timeout registra evento provisório, nunca acrescenta a mesma perda a cada passagem do loop. Para fechar uma rodada, cruze TX e RX por sequencia verdadeira e atribua uma unica classe a cada quadro transmitido. Se quadro e referencia desaparecerem, o log TX e necessario para a reconciliacao; os sketches nao automatizam toda essa reconciliacao.

## Dominio de ruido do prototipo extra

Uniforme sobre os 24 bits de dados dos offsets 5,6,7. Modo 1: um bit; modo 2: duas posicoes distintas; modo 3: byte escolhido e rajada de 3..8 bits dentro dele. Tenta forcar LOW, nunca elevar 0 para 1. Portanto comprimento tentado nao e numero de bits efetivamente alterados.
Escolha aleatoria restrita a payload/check e uma simplificacao declarada. Ela nao experimenta corrupcao do marcador, LEN ou SEQ. Para avaliar perda/ressincronizacao, faca testes separados de truncamento/perda de bytes; ampliar o injetor para todo o quadro exige outro mecanismo de sincronizacao e revisao do codigo.
