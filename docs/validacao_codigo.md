# Registro de validacao do codigo - 7 de outubro de 2026

## Resultado no computador

Os tres sketches compilaram sem erros para ESP32 classico (`esp32:esp32:esp32`), com a biblioteca Telemetry deste repositorio. Compilar nao confirma conexoes, funcionamento em placa ou ruido eletrico real.

| Sketch | Flash usada (bytes) | Globais em RAM (bytes) |
|---|---|---|
| TX | 269916 | 22140 |
| RX | 271376 | 22172 |
| RUIDO | 281664 | 22732 |

Ferramentas: Arduino CLI 1.5.1 (commit 01f3d4f2b), pacote oficial Espressif Arduino-ESP32 3.3.12. Limites reportados pela configuracao: flash de programa 1310720 bytes e RAM 327680 bytes. Esses valores pertencem a esta compilacao/configuracao, nao medem consumo dinamico durante a execucao.

Nove testes Python passaram, incluindo CRC-8/SMBUS com vetor `123456789` -> `F4`, erros de um bit, rajadas ate oito bits, colisao SUM8, classes exclusivas e sequencia. O teste C++ do nucleo tambem passou. Cinco cenarios do injetor passaram usando o sketch real com dubles de UART, relogio e GPTimer; instrucoes em `tests/README.md`.

## Correcoes desta revisao

- TX recebeu `t` para piloto de vinte quadros; `g` continua com mil.
- TX/RX usam a instancia Serial2 do pacote Arduino; as UARTs do injetor usam Serial1/Serial2, com saidas TX explicitamente reservadas em GPIO18/19, sem conexao.
- Captura parcial do injetor expira pelo ultimo byte observado. Antes, o baseline podia descartar bytes usando o instante antigo de armamento, inclusive antes de receber o cabecalho completo.
- Um alarme atrasado mais de 3 us desliga o BC547 e aborta a tentativa. Falha de rearmamento tambem desliga a saida. Isso reduz uma fonte de pulsos indevidos; nao prova a latencia real da ISR de inicio.
- Cabecalho, sequencia e referencia devem corresponder nos pontos A/B antes de comparar bits. Capturas desemparelhadas geram CAPTURA_INVALIDA, sem fingir evidencia de alteracao.
- Documentacao identifica BC547, resistores com funcoes separadas, mapa GPIO e piloto em tres etapas. A distribuicao de papeis das placas e proposta, pois o estudante ainda nao as havia programado.

## Reproduzir

Instalar Arduino CLI e o pacote Espressif 3.3.12, conforme README, e executar na raiz:

```powershell
powershell -ExecutionPolicy Bypass -File tools/compilar.ps1
```

O script tambem aceita `-CliPath` e `-ConfigFile` para instalacoes isoladas. Nesta pasta local, detecta automaticamente a instalacao em `tools/vendor/`; ela nao e publicada no GitHub. Logs e binarios ficam em `build/`, tambem fora do repositorio. A compilacao nao grava nem abre a porta serial das placas.

## Evidencias ainda pendentes

Nao foram gravadas placas nem coletados dados de bancada nesta verificacao. As fotos nao confirmam continuidade, pinagem exata C/B/E, valores dos dois resistores, HIGH/LOW, posicionamento dos pulsos, referencia intacta, tempos ou recuperacao. Nao ha dados reais em `data/reais/`.

O proximo passo e o piloto de `docs/montagem_bc547.md`: primeiro vinte quadros TX/RX sem coletor conectado; depois vinte com o injetor em modo 0; so entao testar modos 1..3 e medir A/B/GPIO27. A demonstracao fisica permanece EXTRA.
