# Deteccao de erro em telemetria UART com ESP32

**Modelo de estrutura, nao relatorio final.** Escrever os paragrafos a partir do estudo e das evidencias da equipe. Exportar em PDF somente apos completar e revisar.

Instituicao: [preencher]. Disciplina: [preencher]. Professor: [preencher]. Data: [preencher]. Integrantes: [tres nomes].
Codigo e documentacao: https://github.com/enzogomes2006/TCD_RUIDO
Versao/commit analisado: [preencher].

## 1. Objetivo e escopo autorizado

Escrevam qual pergunta o trabalho responde: quando dados corrompidos passam pela checagem? Registrem se a avaliacao principal e teorica e a bancada e extra, conforme orientacao atual do professor. Identifiquem quais evidencias sao teoricas, simuladas ou medidas. Nao anunciem 'ruido real' como resultado se nao houve coleta real.

## 2. Fundamentacao

Expliquem bit, byte, hexadecimal, UART 8N1, 8,68 us/bit, checksum modular, residuo e CRC com parametros. Mostrem calculo manual. Usem o curso como apoio e citem as fontes realmente estudadas.

## 3. Especificacao

Incluam a tabela byte a byte, endianness, campos protegidos, criterio de aceito/descartado, referencia fora da decisao, uma tecnica por rodada e ausencia de retransmissao. Justifiquem LEN, politica de sequencia, wrap, repetidos e timeouts. Diagramem a maquina de estados.

## 4. Metodo e reproducibilidade

Expliquem versoes, placa-alvo, parametros de ruido e tamanho da amostra. Se usarem simulacao, escrevam 'forcar bits para zero em memoria, sem modelar camada eletrica'. Se usarem bancada extra, incluam circuito, componentes, posicoes, modo, timer, validacao e CSVs. Nao misturem os conjuntos de dados.

## 5. Resultados e limites

Insiram uma tabela por familia. Cada linha representa uma rodada; quatro classes somam 1000. Taxa = ND/(CORROMPIDO+ND); N/A para denominador zero. Se os dados forem simulados, identifiquem isso no titulo e na coluna origem.
Nao preencher tempo TX, tempo RX, timeout justificado ou recuperacao fisica com tempos medidos no PC. Calculem overhead de 6 bytes sem referencia e expliquem o denominador do percentual.
Discutam tentativas sem mudanca, erros no campo CHECK, perdas que nao entraram no denominador e limite de observar zero casos numa amostra.

## 6. Estudo de caso byte a byte

Para o exemplo teorico: enviado AA 55 05 00 01 80 80 FA; recebido AA 55 05 00 01 00 00 FA. Expliquem os dois MSBs e a soma 0x100 modulo 256 igual a zero.
Se o professor exigir caso real, substituam/adicionem o registro real com sequencia, TX, RX, log do injetor, timestamp, mudancas confirmadas e calculo feito pela equipe. Exemplo teorico nao prova medicao.

## 7. Predicao e defesa

Descrevam como obter p_classe = contagem/1000 e E_classe = 10*p_classe. Guardem previsao individual antes da rodada de 10 quadros, justifiquem incerteza e comparem apos a execucao.

## 8. Conclusao autoral

Respondam a pergunta inicial com as evidencias que existem. Nao escrevam que CRC elimina todo erro. Indiquem limites, mudancas que a equipe faria e quais afirmacoes nao puderam ser verificadas.

## 9. Contribuicoes e referencias

Documentem contribuicoes individuais e apoio de ferramentas conforme a regra da disciplina. Incluam curso do autor, enunciado, documentacao Espressif e outras fontes que de fato usaram.

## 10. Anexos

Link/commit de codigo, especificacao, instrucoes de execucao, tabelas, CSVs e, se houver, capturas reais. Nunca apresentar CSV sintetico como bancada.
