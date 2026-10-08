# Montagem BC547 e primeira validacao - 7 de outubro de 2026

Este documento orienta a montagem que o estudante iniciou nas fotos. As posicoes abaixo sao a distribuicao proposta; as placas ainda nao foram gravadas nem a continuidade eletrica confirmada. A bancada permanece EXTRA.

## Identificar as placas

Na segunda foto, olhando como ela foi enviada: superior esquerda = TX; superior direita = RX; inferior = RUIDO. Coloque etiquetas nas placas e anote a porta COM de cada uma. Use os nomes impressos dos GPIOs; a placa inferior esta invertida na foto, por isso nao copie posicoes direita/esquerda entre placas.

## Componentes

- BC547 NPN: confirmar a marcacao completa/fabricante. No BC547 TO-92 usual, face plana com texto de frente e pernas para baixo: coletor, base, emissor da esquerda para a direita. O datasheet do componente real prevalece.
- R1 = 1 kohm em serie com o sinal TX->RX.
- R2 = 1 kohm entre GPIO27 da placa RUIDO e a base do BC547.
- R3 = 10 kohm entre base e emissor, recomendado para manter desligado enquanto o GPIO nao e controlado. Se nao houver R3, a condicao de reset deve ser verificada antes de habilitar o injetor.
- GND comum entre as tres placas; cada placa alimentada pelo seu USB. Nao interligar os pinos VIN dos tres USBs.

Dois resistores aparecem nas fotos, mas o estudante informou '1 kohm' sem confirmar separadamente os dois: medir/identificar R1 e R2 antes de considerar o inventario completo.

## Circuito e pontos de observacao

```text
TX GPIO17 --- A --- R1 1k --- B --- RX GPIO16
             |              |
             + RUIDO GPIO16  + RUIDO GPIO26
             + RUIDO GPIO25  + coletor BC547

RUIDO GPIO27 --- R2 1k --- base BC547
                            |
                           R3 10k (recomendado)
                            |
GND TX = GND RX = GND RUIDO = emissor BC547
```

A = sinal original, antes de R1. B = sinal perturbado, depois de R1. Nenhum jumper pode unir A e B desviando R1. R1 limita a corrente quando o transistor puxa B para GND. O transistor so tenta alterar 1 para 0.

| Placa | Pino impresso | Ligar em |
|---|---|---|
| TX | GPIO17 / TX2 | A |
| RX | GPIO16 / RX2 | B |
| RUIDO | GPIO16 / RX2 | A |
| RUIDO | GPIO25 / D25 | A |
| RUIDO | GPIO26 / D26 | B |
| RUIDO | GPIO27 / D27 | R2 -> base |
| Todas | GND | GND comum e emissor |

GPIO16 e GPIO25 da placa RUIDO observam o mesmo ponto A, com funcoes diferentes. GPIO26 observa B. Isso exige quatro GPIOs mais GND na placa RUIDO; as fotos anteriores nao mostram com clareza todas essas conexoes.
GPIO18 e GPIO19 da RUIDO ficam reservados como TX das UARTs de observacao e devem ficar sem fios; o injetor nao transmite por essas UARTs. Isso evita depender da selecao automatica de pinos TX pelo pacote Arduino.
O retorno RX GPIO17 -> TX GPIO16 nao e usado pelos programas. Se o professor exigir os dois fios cruzados no desenho, pode ser ligado, mas nao ha ACK nem retransmissao neste protocolo.

## Conferir sem alimentacao

1. Tirar os cabos USB. Conferir C/B/E pelo fabricante e manter as tres pernas em grupos eletricos distintos da protoboard. Em cada lado do canal central, os cinco furos da mesma fileira normalmente sao conectados entre si.
2. Confirmar continuidade entre GND das tres placas e emissor.
3. Confirmar continuidade entre TX17 e A, A e RUIDO16/25; B e RX16/RUIDO26/coletor.
4. Confirmar R1 de 1 kohm entre A e B sem fio que o desvie. Medir resistores preferencialmente com uma perna fora do circuito, pois caminhos paralelos podem alterar a leitura.
5. Confirmar R2 de 1 kohm entre RUIDO27 e base; nao ligar GPIO27 diretamente a A/B.
6. Confirmar ausencia de curto de montagem entre alimentacao e GND. Cabos USB separados compartilham somente GND/sinais previstos.

## Etapa 1 - TX/RX sem transistor conectado a B

Comece com o coletor desligado de B e sem necessidade de ligar a placa RUIDO. Mantenha R1, TX17->A->R1->B->RX16 e GND comum.
Gravar `firmware/tx/tx.ino` na TX e `firmware/rx/rx.ino` na RX com a biblioteca `libraries/Telemetry`. Monitor USB de ambas a 115200 baud.
No RX, enviar `r`, depois `s`, depois `0`. No TX, enviar `s`, depois `t` para vinte quadros. Os comandos sao letras/digitos ASCII minusculos; podem ser enviados um por vez.
Resultado esperado: vinte registros RX, seq 0..19, residuo 0, ACEITO, payload_confere sim, status OK. O TX envia dados artificiais; nao ha sensor nesta versao.
Para o primeiro quadro (seq=0), a referencia teorica e `AA 55 05 00 00 80 80 FB`. O piloto precisa confirmar os bytes reais no log; essa referencia nao e uma medicao.
Se nao houver resultado: conferir placa/porta/sketch, GND, TX17/RX16, tecnica igual em ambas e biblioteca. Evento AUSENCIA antes de iniciar o TX e esperado: nao prova falha na fiação.

## Etapa 2 - injetor conectado, modo 0

Gravar `firmware/noise/noise.ino` na RUIDO. Ligar os quatro GPIOs e GND conforme a tabela. Com a placa inicializada, enviar `0` no monitor da RUIDO antes de conectar o coletor a B. Confirmar GPIO27 em LOW/desligado.
Conectar o coletor a B, repetir RX `r,s,0` e TX `s,t`. Esperado: vinte OK no RX e vinte registros BASELINE na RUIDO com as mesmas sequencias. Se conectar o circuito introduzir erros no modo 0, interromper o piloto e verificar C/B/E, resistor serie, shorts e desligamento do transistor.

## Etapa 3 - modos de ruido

Somente depois de baseline estavel, configurar RX `r,s,1`, RUIDO `1`, TX `s,t`. Repetir para 2 e 3 em arquivos separados. O digito no RX e metadado, nao aciona ruido; a placa RUIDO precisa receber seu proprio comando.
Nao exigir que toda tentativa produza CORROMPIDO: forcar um zero para zero nao altera o dado. Cruzar o log do injetor com o RX. SEM_PULSO, TIMING_INVALIDO, CAPTURA_INCOMPLETA e CAPTURA_INVALIDA exigem corrigir a tentativa/piloto; nao considerar evidencia de injecao valida.
Capturar A, B e GPIO27 com analisador logico/osciloscopio para verificar pulsos, start/stop, referencia e latencia. Compilacao e logs por si so nao comprovam posicao eletrica.
Trocar para CRC em rodada separada: RX `r,c,modo`; TX `c,t`; manter o mesmo modo na RUIDO. Para as rodadas completas, substituir `t` por `g` (1000 quadros).

## Situacao da validacao

O mapa esta conferido contra o codigo. A continuidade, o funcionamento nas placas, os pulsos e os resultados reais precisam ser confirmados pela equipe. Registrar observacoes sem antecipar que a foto comprova cada ligacao.

Fonte da pinagem: [BC547 - onsemi](https://www.onsemi.com/pdf/datasheet/bc546-d.pdf). APIs: [UART Espressif](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/serial.html) e [GPTimer Espressif](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/gptimer.html).
