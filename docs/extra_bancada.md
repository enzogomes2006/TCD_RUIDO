# EXTRA - demonstracao fisica e coleta com tres ESP32

Esta extensao esta separada da entrega principal a pedido do estudante. Se o professor mantiver o enunciado PDF integral, parte dessas evidencias passa a ser necessaria para cumprir o trabalho; a montagem ainda nao foi realizada aqui.

## Materiais e circuito proposto para revisar

Atualizacao de 7/10/2026: o transistor disponivel e um BC547. O roteiro especifico das placas, pinagem e piloto esta em [montagem_bc547.md](montagem_bc547.md). O BC547 substitui o NPN proposto, respeitando sua propria pinagem; os pulsos precisam ser medidos com o componente real.

Tres ESP32 classicos com UART2 e GPIO16/17 livres; tres cabos USB; GND comum; transistor NPN BC547 com pinagem conferida; resistor serie da linha de 1 kohm; resistor de base proposto de 1 kohm; resistor base-emissor proposto de 10 kohm para mante-lo desligado; analisador logico/osciloscopio para validar pulsos.
Confirme a pinagem real do transistor pelo fabricante e a variante da placa. Nao conectar uma saida GPIO diretamente a outra saida para disputar niveis. O resistor serie limita a corrente quando NPN conduz; resistor de base tem funcao distinta.

```text
TX GPIO17 --- ponto A --- Rserie 1k --- ponto B --- RX GPIO16
                |                       |
                +-- NOISE GPIO16         +-- NOISE GPIO26
                +-- NOISE GPIO25         +-- coletor NPN
                                            emissor -- GND comum
NOISE GPIO27 -- Rbase 1k -- base NPN
                             |
                       Rpull 10k para GND
```

Ponto A e observado antes da perturbacao; ponto B depois. GPIO25 captura a primeira borda do corpo apos a pausa de armamento. GPIO16/26 permitem comparar os bytes observados. TX GPIO16 e RX GPIO17 podem ser cruzados para a ligacao de retorno pedida no desenho da disciplina, mas o protocolo nao transmite respostas nem retransmissoes.
Valide HIGH/LOW no ponto B e largura dos pulsos com a montagem real antes de coletar amostras. A documentacao nao comprova comportamento eletrico so por descrever o circuito.

## Preparacao e limites importantes

Compilar todos os sketches com a mesma biblioteca e registrar a versao Arduino-ESP32, placa/FQBN e commit. Os tres foram compilados em 7/10/2026 com Arduino-ESP32 3.3.12 e `esp32:esp32:esp32`; consulte [validacao_codigo.md](validacao_codigo.md). `tools/compilar.ps1` reproduz a compilacao, sem gravar placas.
O injetor usa GPTimer a 1 MHz, ISR de borda e eventos para ligar/desligar o NPN. Nao usa delayMicroseconds. Ter timer nao garante posicionamento: latencia de ISR, arredondamento e leitura UART devem ser medidos. TIMING_INVALIDO ou SEM_PULSO invalida a tentativa e exige corrigir o piloto.
O instante registrado e o inicio do corpo observado pelo injetor, nao o instante exato de cada borda eletrica. Use a posicao planejada junto com captura para comprovar cada pulso.
O sorteio e restrito aos 24 bits de dados dos bytes de payload/CHECK. Nao testa marcador/LEN/SEQ, start/stop ou perdas por enquadramento. Se a avaliacao exigir sorteio em todo o quadro, este prototipo precisa ser ampliado; nao declarar equivalencia.
Modo 3 une bits contiguos num pulso de aproximadamente 3..8 tempos de bit dentro de um byte. A extensao real de bits alterados pode ser menor porque havia zeros no intervalo.

## Piloto antes das 7000 amostras

1. Sem NPN ativo, confirmar TX/RX em modo 0 e pelo menos vinte quadros corretamente enquadrados.
2. Confirmar que o RX decide antes de acessar a referencia; verificar que a referencia segue intacta.
3. Medir intervalo entre quadros e entre bytes, incluindo a pausa de 2 ms. Ajustar timeout parcial e de ausencia, justificar a margem com dados.
4. Testar modo 1 sobre bit conhecido igual a 1 e sobre bit igual a 0. Conferir pulsos, antes/depois e RX; GPIO LOW deve desligar o NPN.
5. Testar modos 2 e 3 sem acertar start/stop UART ou referencia. Confirmar as posicoes do log com analisador.
6. Testar truncamento separado para justificar timeout e medir recuperacao do primeiro quadro valido. Esses testes nao substituem os modos 1..3.
7. Testar sequencia repetida, fora de ordem e wrap em uma rodada de validacao separada.

## Coleta de uma rodada

Configurar RX: `r`, `s` ou `c`, e o modo `0`..`3` como metadado. Configurar injetor: modo correspondente. Configurar TX: `s` ou `c`; primeiro enviar `t` para o piloto de 20 quadros, e depois `g` para 1000 quadros. Nao trocar tecnica/modo no meio da rodada.
Capturar os tres monitores simultaneamente por ao menos 110 segundos. O TX fica parado depois dos mil; reiniciar com `g` abre nova rodada a partir de seq=0. Separe arquivos por tecnica/modo/rodada.

```powershell
python -m pip install -r requirements.txt
# Exemplo de portas; substituir pelo que o computador identificar.
python scripts/capturar.py --port COM5 --out data/reais/sum8_modo1_rx.csv --seconds 120 --command r --command s --command 1
```

A captura abre a porta e pode reiniciar a placa; configure todas e aguarde estabilizar antes de iniciar TX. O script nao automatiza sincronizacao dos tres monitores nem junta suas linhas.
Linhas `#` sao eventos: removê-las da tabela classificada, preservando o log bruto. Reconciliar cada quadro do TX com RX e ruido. Quadros corrompidos nao entram novamente como perdidos por lacuna. Sem referencia ou captura incompleta: resolver a pendencia; nao inventar classe.
O analisador espera CSV final com cabecalho e exatamente 1000 sequencias distintas. Se tudo estiver resolvido, agrega as quatro classes e calcula taxa. A reconciliacao completa de perda de quadro+referencia e manual neste prototipo.

## Plano das duas rodadas

SUM8: modos 0,1,2,3; 1000 por modo. CRC8: modos 1,2,3; 1000 por modo. Total minimo: 7000 quadros, aproximadamente 11 min 40 s com periodo nominal de 100 ms, fora configuracao e pilotos. Baseline CRC extra pode ser util, mas nao e uma oitava rodada obrigatoria pelo PDF.
Mantenha payload, formato, ritmo, dominio de sorteio e circuito. A distribuicao dos bits originalmente 1 no campo CHECK pode diferir entre tecnicas; por isso reporte tentativas sem mudanca e alteracoes reais, sem presumir denominadores iguais.

## Caso real nao detectado

Buscar primeiro em SUM8 modo 2. Se nao ocorrer em 1000, continuar em rodadas extras identificadas, preservando a tabela original. Arquivar TX/RX hex, seq, posicoes, tempo, antes/depois e calculo manual do caso.
Um teste dirigido apagando dois MSBs pode ser apresentado como 'padrao construido' separado da amostra aleatoria; nao mudar os logs para fingir que foi aleatorio. O script de ruido atual nao tem comando de ataque dirigido.

## Demo extra de 5 a 8 minutos

Mostrar circuito e tres papeis; baseline; modo 1 e descarte; tentativas sobre zero; modo 2 e caso previamente registrado; comparacao com CRC; timeout/recuperacao testado separadamente; previsao dos dez antes de gerar. Essa extensao tem duracao propria e nao foi embutida na defesa teorica de 20 minutos.

## Encerrar com evidencias

Dois conjuntos de tabelas, logs brutos, CSVs reconciliados, capturas de bit/intervalo, tempos TX/RX, recuperacao em ms e versao de firmware compilada. A prova de 'mudou de fato' e a concordancia entre bytes observados, RX e captura, nao apenas um GPIO acionado.
