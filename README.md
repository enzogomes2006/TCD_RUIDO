# TCD_RUIDO - Telemetria UART com ESP32

Material de apoio para estudo, implementacao e defesa do trabalho de deteccao de erros.
Repositorio: https://github.com/enzogomes2006/TCD_RUIDO

**Entrega principal neste material:** roteiro teorico, especificacao, modelo editavel do relatorio e demonstracao didatica no computador.
**Extra separado:** montagem e demonstracao com tres ESP32.

O PDF de atividade fornecido menciona experimentos reais e demonstracao ao vivo. A orientacao do estudante foi tratar a parte fisica como extra. A matriz registra essa diferenca: somente a orientacao atual do professor pode dispensar os requisitos experimentais. A simulacao nao comprova cumprimento de ruido real.

## Onde esta cada coisa

Para validar a estrutura fisica iniciada com **BC547**, comece por [montagem_bc547.md](docs/montagem_bc547.md): posicoes das tres placas, ligacoes GPIO, dois resistores de 1 kohm e piloto de 20 quadros (`t` no TX).

| Pasta | Conteudo |
|---|---|
| `output/pdf/` | Guia em PDF para preparar a entrega e a defesa |
| `docs/roteiro.md` | Roteiro de apresentacao, falas de estudo e perguntas |
| `docs/relatorio_modelo.md` | Estrutura do relatorio a ser escrita pela equipe |
| `docs/especificacao.md` | Formato, campos protegidos, algoritmos e politicas RX |
| `docs/requisitos.md` | Requisito, evidencia necessaria e pendencias |
| `docs/extra_bancada.md` | Montagem, piloto, coleta real e limites do prototipo |
| `firmware/tx/tx.ino` | Transmissor: quadro, codigo, sequencia e referencia |
| `firmware/rx/rx.ino` | Receptor: checagem independente e classificacao posterior |
| `firmware/noise/noise.ino` | Injetor experimental com GPTimer e dois pontos de observacao |
| `libraries/Telemetry/` | Nucleo comum do protocolo em C++ |
| `scripts/` | Modelo em Python, simulacao, analise e captura serial |
| `tests/` | Vetor CRC, falhas, colisao, classificacao e sequencia |
| `data/reais/` | Lugar reservado para evidencias reais da equipe |
| `data/tabelas/` | Duas tabelas vazias para preencher com evidencias |

## Comecar pela entrega principal

1. Leia o roteiro e escreva o protocolo no papel antes de estudar a implementacao.
2. Resolva o exemplo manual `AA 55 05 00 01 80 80 FA`.
3. Execute os testes e a simulacao. Todos os resultados dela sao SINTETICOS.
4. Escreva a analise com suas palavras e prepare a defesa individual.
5. Preencha o modelo de relatorio com o escopo combinado com o professor e as evidencias produzidas. Exporte o texto final em PDF e inclua o link deste repositorio.

Com Python 3.10 ou mais recente, na raiz do repositorio:

```powershell
python -m unittest discover -s tests -v
python scripts/simular.py --n 1000 --seed 21
python scripts/analisar.py data/simulacao/sum8_modo0.csv data/simulacao/sum8_modo1.csv data/simulacao/sum8_modo2.csv data/simulacao/sum8_modo3.csv data/simulacao/crc8_modo1.csv data/simulacao/crc8_modo2.csv data/simulacao/crc8_modo3.csv --out data/simulacao/resumo.csv
```

Nao ha bibliotecas externas para esses comandos. `PERDIDO=0` nesta simulacao e uma limitacao do modelo: ele nao modela perda UART, marcadores atingidos, jitter ou framing. Ela nao mede tempo de ESP32.

## Extra: compilar para ESP32

Placa-alvo proposta: ESP32 classico, sem PSRAM ocupando GPIO16/17. O codigo usa APIs de Arduino-ESP32 3.x e GPTimer do ESP-IDF 5.x. Confirme a versao instalada e a placa exata.

```powershell
# Instalar Arduino CLI do site oficial e o pacote Espressif, caso ainda nao existam.
arduino-cli core update-index --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core install esp32:esp32@3.3.12 --additional-urls https://espressif.github.io/arduino-esp32/package_esp32_index.json
powershell -ExecutionPolicy Bypass -File tools/compilar.ps1
```

Alternativa Arduino IDE: adicionar a pasta `libraries/Telemetry` como biblioteca local e abrir cada sketch no seu diretorio de mesmo nome.

**Verificacao feita em 7/10/2026:** nove testes Python, nucleo C++, cinco cenarios do sketch do injetor com dubles e compilacao dos tres sketches com Arduino CLI 1.5.1 / Arduino-ESP32 3.3.12 / `esp32:esp32:esp32`. Detalhes e memoria: [registro de validacao](docs/validacao_codigo.md). **Pendente:** gravacao nas placas, continuidade eletrica, pulsos e medicoes reais. Nenhum resultado de bancada foi produzido. O injetor continua experimental.

## Autoria e dados

Este material recebeu apoio de IA. Nao e relatorio final pronto para ser atribuido ao grupo. A atividade avisa que trabalhos notadamente baseados em IA nao serao avaliados: confirmem os limites desse apoio, estudem, implementem/revisem e escrevam as conclusoes com suas evidencias. Nao removam a identificacao de simulacao nem inventem medidas.

Cada integrante registra suas contribuicoes em `docs/contribuicoes.md`. Os tres precisam explicar o sistema inteiro.

## Referencias tecnicas

- Curso proprio fornecido pelo estudante: *Curso Completo de 21 Dias - Telemetria UART com ESP32*, 47 paginas.
- Enunciado fornecido: *Deteccao de Erro em Telemetria sob Ruido Real (ESP32)*, 2 paginas.
- [UART Espressif](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/serial.html).
- [GPTimer Espressif](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/gptimer.html).
- [esp_timer Espressif](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/esp_timer.html).

Os PDFs originais nao foram publicados neste repositorio. O guia e novo material derivado para esta preparacao.
