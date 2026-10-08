# Verificacoes no computador

Na raiz do projeto:

```powershell
python -m unittest discover -s tests -v
New-Item -ItemType Directory -Force tmp | Out-Null
g++ -std=c++11 -Wall -Wextra -pedantic tests/test_core.cpp -o tmp/test_core.exe
./tmp/test_core.exe
g++ -std=c++11 -Wall -Wextra -pedantic -Itests/stubs -Ilibraries/Telemetry/src tests/test_noise.cpp -o tmp/test_noise.exe
./tmp/test_noise.exe
```

O teste Python tem nove casos do protocolo e da analise. O C++ do nucleo testa o vetor CRC e a colisao de SUM8. O teste do injetor inclui o sketch real com dubles de UART, relogio e GPTimer: baseline iniciado apos 500 ms, timeout de captura parcial, rejeicao de quadros desemparelhados, aborto de alarme atrasado e encerramento de pulso pontual.

Os dubles em `stubs/` sao usados somente no teste do computador. Nao substituem as bibliotecas durante a compilacao Arduino. Nenhum destes testes mede latencia de ISR, tensao, pinagem, transistor ou largura de pulsos reais. A compilacao ESP32 e a verificacao na bancada sao etapas distintas.
