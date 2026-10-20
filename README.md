# AMD BC250 Display Status

Display de status e temperatura para a **AMD BC250**, usando Linux/CachyOS para leitura dos sensores e um **ESP32-C3 + GC9A01 240x240** como interface visual.

Este é o repositório principal do projeto.

## Como funciona

O Linux lê as temperaturas reais da CPU e GPU da AMD BC250. O script `stellar_monitor.py` envia os dados ao ESP32 pela USB Serial a **115200 baud**:

```text
CPU:46.6;GPU:44.0
```

O firmware do ESP32 exibe essas informações no display circular. Enquanto ainda não houver uma leitura válida de CPU e GPU, o display permanece na tela de inicialização. Nenhuma temperatura fictícia é enviada.

## Arquivos

```text
AMD-BC250-Display-Status/
├── README.md
├── setup.sh
├── stellar_monitor.py
└── stellar_monitor.ino
```

O arquivo `stellar_monitor.ino` deste repositório é o firmware principal do display ESP32.

## Hardware

- AMD BC250
- ESP32-C3
- Display GC9A01 1.28"
- Resolução 240x240
- Comunicação USB Serial

## Arduino IDE

Instale:

- `esp32 by Espressif Systems`
- `Arduino_GFX_Library`
- `U8g2`

Para o ESP32-C3, use uma configuração compatível com:

```text
Board: ESP32C3 Dev Module
USB CDC On Boot: Enabled
```

### USB CDC On Boot

**USB CDC On Boot precisa estar em Enabled** quando a comunicação com o Linux é feita pela USB nativa do ESP32-C3.

Isso faz com que o ESP32 disponibilize a interface serial USB usada pelo `stellar_monitor.py`. No Linux, ela normalmente aparece em:

```text
/dev/ttyACM*
```

ou de forma persistente em:

```text
/dev/serial/by-id/
```

Se o firmware grava normalmente, mas o Linux não encontra o ESP32 depois da inicialização, verifique primeiro:

```text
USB CDC On Boot: Enabled
```

e reinicie a placa.

## Instalação no CachyOS / Arch

Execute:

```bash
curl -sSL https://raw.githubusercontent.com/isaacalvex/AMD-BC250-Display-Status/main/setup.sh | bash
```

O instalador configura:

- Python
- PySerial
- `lm_sensors`
- permissão serial
- grupo `uucp` no Arch/CachyOS
- serviço systemd do usuário
- inicialização automática do monitor

Se o instalador adicionar seu usuário ao grupo `uucp`, encerre a sessão e entre novamente ou reinicie o computador.

## Verificar sensores

```bash
sensors
```

Na AMD BC250, o monitor procura principalmente:

```text
Tctl:
```

para CPU e sensores `amdgpu` / `edge` / hwmon para GPU.

## Serviço

Status:

```bash
systemctl --user status stellar-monitor.service
```

Logs:

```bash
journalctl --user -u stellar-monitor.service -f
```

Reiniciar:

```bash
systemctl --user restart stellar-monitor.service
```

## Comunicação

```text
AMD BC250
   |
   v
sensors / hwmon
   |
   v
stellar_monitor.py
   |
   | CPU:xx.x;GPU:xx.x
   | USB Serial - 115200 baud
   v
ESP32-C3
   |
   v
GC9A01 240x240
```

O script procura primeiro `/dev/serial/by-id/*` e depois `/dev/ttyACM*` e `/dev/ttyUSB*`.

## Repositório

O desenvolvimento passa a ser mantido somente aqui:

https://github.com/isaacalvex/AMD-BC250-Display-Status

O repositório antigo `stellar-monitor` não deve mais ser usado como fonte principal para instalação ou atualizações.


## Exibição de progresso Steam com 0,01% (PCTF)

O firmware acompanha o ciclo completo de cada tarefa Steam (download e instalação) com o pacote enviado pelo programa host:

```text
DL:1;PCTF:8.37;PCT:8;SPEED:28.4;NAME:Street Fighter 6
DL:0
```

O campo novo `PCTF` aceita porcentagem com duas casas e controla o anel com 360 pontos e número central. O campo antigo `PCT` continua sendo aceito se `PCTF` não existir. O protocolo não é exclusivo de nenhum jogo. Se `SPEED:-1`, aparece `-- MB/s`. O desenho continua a ser atualizado aproximadamente uma vez por segundo quando o host fornece leituras novas.

Para obter essa precisão, **compile e grave este novo firmware na placa ESP32-C3**. Apenas atualizar o monitor Bazzite não muda o firmware já gravado. Pela interface gráfica do repositório companion, use a aba Firmware: o instalador recompila esta origem ao detectar hash alterado. Para conferir o recebimento do pacote, use **Diagnóstico → Testar tela de download no ESP32**, que envia 65,41% a 65,46% em seis segundos. A leitura real de MB/s e dos bytes vem do serviço CEF do host e não do ESP32.


## Protocolo Steam: exibição de Mbps

O host pode transmitir `DL:1;UNIT:Mbps;PCTF:37.81;PCT:38;SPEED:35.3;NAME:Jogo`. O campo `SPEED` mantém a velocidade em **MB/s** para compatibilidade com firmware antigo. Ao receber **`UNIT:Mbps`**, este firmware converte na tela por **8** e mostra **282,4 Mbps**, mantendo o anel de progresso e a porcentagem anterior sem alteração.

Em mensagens legadas sem `UNIT:Mbps`, o display continua mostrando MB/s. Em caso de velocidade indisponível (`SPEED:-1`), mostra `-- Mbps` ou `-- MB/s` conforme o marcador. O marcador é opcional, posicionado antes de `PCTF`, e não muda os delimitadores existentes.

Regrave o ESP32 com o código atualizado para alterar a unidade visível. O aplicativo Bazzite/CachyOS pode ser atualizado sem regravar o dispositivo, mas firmware antigo continuará mostrando MB/s.
