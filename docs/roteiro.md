# Roteiro da entrega principal e defesa - 20 minutos

Use como guia de estudo, sem decorar frases para atribuir autoria a um texto que a equipe nao escreveu. O curso e a base; a equipe deve explicar o raciocinio com suas palavras. A montagem fisica fica no arquivo `extra_bancada.md`.

## Preparacao em ordem

1. Dias 1..7 do curso: revisar binario, hex, mascaras e UART. Produto: desenhar 80 e 00 em bits e calcular 8,68 us/bit.
2. Dias 8..14: revisar quadro, SEQ, RX, soma, CRC e timeout. Produto: especificacao revisada e exemplo manual resolvido.
3. Dias 15..20: estudar o modelo de ruido e as metricas. Para a entrega principal, executar o modelo no PC e identificar claramente que os dados sao simulados. A bancada e uma extensao separada.
4. Dia 21: ensaiar a defesa com tempo, perguntas e troca de papeis. Produto: cada integrante explica uma parte que nao implementou.
5. Escrever o relatorio a partir das evidencias existentes; conferir a matriz de requisitos; exportar PDF; testar o link GitHub sem depender da conta do autor.

## Defesa cronometrada

| Tempo | Responsavel inicial | Mostrar | Explicar |
|---|---|---|---|
| 0..2 min | A | objetivo e escopo | medir dados errados aceitos; teoria/simulacao e extra separados |
| 2..5 min | A | desenho dos oito bytes e UART | START, LEN, SEQ, payload e CHECK; uma tecnica por vez |
| 5..8 min | B | fluxo RX e dois exemplos | decide sem conhecer verdade; residuo, descarte, referencia posterior |
| 8..11 min | C | colisao manual | dois bits 1->0, soma diminui 256, payload errado aceito |
| 11..14 min | C | CRC e desenho experimental | polinomio, parametros, 7000 quadros no plano, variaveis constantes |
| 14..16 min | B | CSV/tabelas rotulados | classes exclusivas, taxa e limites do modelo |
| 16..18 min | os tres | previsoes de 10 quadros | expectativa estatistica antes da execucao; comparar depois |
| 18..20 min | os tres | conclusao e perguntas | responder com evidencias e reconhecer o que falta medir |

Essa e uma distribuicao sugerida. Cada aluno estuda todas as partes porque a nota e individual. Se o professor exigir demo real dentro dos 20 minutos, o grupo precisa ajustar o roteiro; nao acrescente bancada obrigatoria silenciosamente ao escopo teorico.

## Demonstração teorica 1: quadro correto

Escrever no papel:

```text
START LEN SEQ_H SEQ_L P0 P1 CHECK
AA 55  05   00    01   80 80  FA
```

Soma protegida: 5+0+1+128+128=262. 262 mod 256=6. CHECK=-6 mod 256=250=FA.
RX processa 5+0+1+128+128+250=512; 512 mod 256=0. Aceita. Apenas depois compara payload com a referencia: 80 80 == 80 80, portanto OK.

## Demonstração teorica 2: um erro detectado

Apagar apenas o MSB de P0: `AA 55 05 00 01 00 80 FA`.
RX soma 5+0+1+0+128+250=384; residuo 128=80, diferente de zero. Descarta. Classe CORROMPIDO mesmo que apenas o CHECK tivesse mudado; a classe e dada pela falha de checagem.

## Demonstração teorica 3: quebrar SUM8

Apagar os MSBs dos dois bytes: `AA 55 05 00 01 00 00 FA`.
P0: 10000000 -> 00000000. P1: 10000000 -> 00000000.
Soma recebida: 5+0+1+0+0+250=256. Residuo zero. RX aceita. A referencia posterior revela que 00 00 != 80 80: NAO_DETECTADO.
Delta total=-128-128=-256, que equivale a zero modulo 256. Isso ilustra um padrao compativel com LOW. Nao usar como unica explicacao o exemplo 10+20 -> 11+1F, porque ele exige uma mudanca de 0 para 1.
Este caso e matematico e reproduzido pelos testes, sem provar uma ocorrencia em fio. O enunciado PDF pede caso real: essa evidencia fica pendente se nao houver dispensa ou bancada.

## CRC: explicar e calcular

Usar CRC-8/SMBUS: largura 8, poly 07, init 00, sem reflexao, xor final 00. Para cada byte: XOR com o registrador; repetir oito shifts, com XOR 07 quando o bit superior era 1. Mostrar no quadro um passo e deixar o calculo completo disponivel.
No mesmo corpo `05 00 01 80 80`, obter o CHECK pelo codigo e verificar os seis bytes incluindo CHECK. Repetir apos apagar os dois MSBs. O teste confirma que esse padrao e rejeitado pelo CRC escolhido.
CRC de grau 8 detecta todo erro nao nulo cuja extensao matematica e no maximo oito bits, desde que o codigo seja avaliado sobre um bloco devidamente montado. Isso nao garante contra perda de bytes, falso alinhamento, todo padrao mais longo ou qualquer protocolo. Zero erros vistos nao significa impossibilidade de erros.

## Demonstracao no PC

Abrir tres janelas/arquivos: especificacao, terminal e CSV. Rodar os comandos do README. Explicar que o modelo forca LOW em memoria, mantem o framing e usa apenas payload/check. Por isso nao valida PERDIDO, timeout eletrico ou recuperacao real.
Abrir `sum8_modo2.csv`, filtrar NAO_DETECTADO e conferir TX hex, RX hex, bits tentados e bits alterados. Calcular manualmente o caso encontrado. Se o grupo desejar mostrar exatamente o exemplo anterior, usar o teste `test_low_only_collision` como exemplo construido e rotula-lo assim.
Gerar duas tabelas apenas para demonstracao. Nao transferir esses CSVs para `data/reais/`.

## Predicao dos proximos 10

Para cada classe, p=count/1000; expectativa de dez quadros E=10*p. Se a tabela de estudo tiver 200 OK, 750 CORROMPIDOS, 0 PERDIDOS e 50 ND, expectativas sao 2;7,5;0;0,5. E um exemplo hipotetico, nao resultado do grupo.
Cada aluno registra uma previsao inteira somando 10 e justifica a variabilidade. A expectativa decimal nao e uma promessa de contagem exata. A referencia do modo escolhido precisa estar disponivel antes de prever; se o professor nao revela nenhum indicio do modo, so e possivel uma previsao condicional ou agregada, e isso deve ser esclarecido.
Na demonstracao teorica, compare com outro bloco simulado de dez quadros. Na bancada extra, compare com o que RX registrou. Grave previsoes antes dos resultados, evitando retrospectiva.

## Perguntas para os tres

- Por que UART 8N1 tem dez bits por byte? Qual bit de dados sai primeiro?
- Por que SEQ nao substitui CHECK? O que acontece no wrap?
- Por que referencia nao pode ajudar a aceitar? Como evitar vazamento da verdade?
- Qual diferenca entre CORROMPIDO e PERDIDO? Como evitar dupla contagem?
- Por que o exemplo de dois MSBs passa na soma? Qual o residuo exato?
- Quais parametros identificam o CRC? Qual e a ordem matematica versus UART?
- Por que um pulso sobre bit zero nao muda o dado?
- O que a simulacao deixou de modelar? Que conclusoes nao podem ser tiradas dela?
- O timeout foi medido ou proposto? Como o grupo o justificaria?
- Por que a taxa nao inclui OK nem PERDIDO? O que fazer com denominador zero?
- Qual contribuicao individual voce consegue demonstrar no historico?

## Checklist de envio

Relatorio autoral PDF; link GitHub e versao; especificacao coerente com codigo; duas tabelas com origem; fontes; contribuicoes; exemplos manuais; limites; tudo que foi medido acompanhado de registro. Nao declarar cumprimento experimental quando so ha simulacao.
