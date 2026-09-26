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
