# 🌌 STELLAR — BC250 Thermal Monitor

Monitor térmico para **CPU e GPU** utilizando um **ESP32-C3 + ESP32-2424S012 (GC9A01)** conectado via USB a um computador com **CachyOS/Linux**.

O ESP32 possui uma interface gráfica própria com tela de inicialização, estrelas e cometa. Enquanto o computador ainda não estiver enviando dados, a tela permanece na inicialização. Assim que o serviço do Linux começa a enviar temperaturas válidas, o display muda automaticamente para o monitor térmico.

---

## ✨ Funcionalidades

- 🌌 Tela de inicialização STELLAR
- ☄️ Cometa animado
- ⭐ Campo de estrelas
- 🖥️ Monitoramento da CPU
- 🎮 Monitoramento da GPU AMD
- 🌡️ Atualização a cada 1 segundo
- 🔌 Comunicação via USB Serial
- 🚀 Inicialização automática com `systemd`
- 🔄 Reinício automático do monitor em caso de falha
- 🔍 Detecção automática do ESP32
- 📡 Reconexão automática caso o USB seja desconectado
- 💾 Funcionamento local, sem necessidade de Internet
- 🎨 Interface com cores baseadas na temperatura

---

# 📁 Estrutura do projeto

```text
STELLAR/
│
├── arduino/
│   └── stellar_display.ino
│
├── cachyos/
│   ├── stellar_monitor.py
│   └── instalar_stellar.sh
│
└── README.md
```

---

# 🔧 Hardware

## ESP32

O projeto utiliza:

- ESP32-C3
- ESP32-2424S012
- Display GC9A01
- Resolução 240 × 240
- Comunicação USB Serial

### Pinagem

```cpp
#define TFT_MOSI 7
#define TFT_SCLK 6
#define TFT_CS   10
#define TFT_DC   2
#define TFT_RST  -1
#define TFT_BL   3
```

---

# 💻 Software

O computador precisa possuir:

- CachyOS ou outra distribuição Linux compatível
- Python 3
- PySerial
- `lm_sensors`
- systemd
- USB

---

# 🔄 Como funciona

O projeto possui duas partes principais.

## ESP32

O ESP32 é responsável por toda a interface gráfica.

Ao ligar, ele inicia a animação:

```text
STELLAR
```

com:

- estrelas;
- cometa;
- efeito de fade;
- animação de inicialização.

Enquanto nenhum dado válido for recebido pela USB, a tela permanece nessa animação.

---

## CachyOS

O computador executa:

```text
stellar_monitor.py
```

O programa coleta:

```text
CPU
GPU
```

e envia os dados ao ESP32 através da USB.

O pacote enviado possui o formato:

```text
CPU:46.6;GPU:44.0
```

Cada pacote termina com uma quebra de linha.

Os dados são enviados aproximadamente a cada:

```text
1 segundo
```

---

# 🔌 Fluxo do sistema

```text
┌──────────────────────┐
│       ESP32-C3       │
│                      │
│  Tela STELLAR        │
│  Estrelas            │
│  Cometa              │
└──────────┬───────────┘
           │
           │ USB
           │ Serial
           ▼
┌──────────────────────┐
│       CachyOS        │
│                      │
│ stellar_monitor.py   │
└──────────┬───────────┘
           │
       ┌───┴───┐
       ▼       ▼
      CPU     GPU
       │       │
       └───┬───┘
           │
           ▼
      USB Serial
           │
           ▼
        ESP32
           │
           ▼
   Tela de temperatura
```

---

# 🚀 Instalação no CachyOS

## 1. Conectar o ESP32

Conecte o ESP32 ao computador através do USB.

Verifique se foi detectado:

```bash
ls /dev/ttyACM*
```

ou:

```bash
ls /dev/ttyUSB*
```

Exemplo:

```text
/dev/ttyACM0
```

ou:

```text
/dev/ttyACM1
```

---

# 2. Verificar Python

Execute:

```bash
python --version
```

Exemplo:

```text
Python 3.14.6
```

---

# 3. Verificar PySerial

Execute:

```bash
python -c "import serial; print('pyserial OK')"
```

Resultado esperado:

```text
pyserial OK
```

Caso não esteja instalado:

```bash
sudo pacman -S python-pyserial
```

---

# 4. Instalar sensores

Instale o pacote:

```bash
sudo pacman -S lm_sensors
```

Depois teste:

```bash
sensors
```

Exemplo:

```text
k10temp-pci-00c3
Adapter: PCI adapter
Tctl: +45.8°C

amdgpu-pci-0100
Adapter: PCI adapter
edge: +42.0°C
```

O script utiliza esses dados para identificar as temperaturas.

---

# 5. Configurar acesso USB

O usuário precisa ter permissão para acessar a porta serial.

Adicione o usuário ao grupo `uucp`:

```bash
sudo usermod -aG uucp $USER
```

Em sistemas que utilizam `dialout`, utilize:

```bash
sudo usermod -aG dialout $USER
```

Depois faça **logout/login** ou reinicie o computador.

Verifique:

```bash
groups
```

Deve aparecer `uucp` ou `dialout`.

---

# 🐍 Monitor Python

O arquivo:

```text
cachyos/stellar_monitor.py
```

é responsável por:

1. Detectar o ESP32.
2. Conectar através da USB.
3. Ler a temperatura da CPU.
4. Ler a temperatura da GPU AMD.
5. Enviar os valores ao ESP32.
6. Detectar desconexões.
7. Reconectar automaticamente.
8. Continuar executando continuamente.

---

# ▶️ Teste manual

Antes de criar o serviço, execute:

```bash
python ~/stellar_monitor.py
```

O resultado esperado:

```text
==================================================
        STELLAR THERMAL MONITOR
        CachyOS -> ESP32 USB
==================================================

Conectando ao ESP32 em /dev/ttyACM0...
ESP32 conectado!

CPU: 46.6 °C | GPU: 44.0 °C
```

Os valores devem continuar sendo atualizados.

Para interromper:

```text
Ctrl+C
```

---

# ⚙️ Serviço systemd

Depois de confirmar que o script funciona manualmente, ele pode ser executado permanentemente através do `systemd`.

O arquivo do serviço fica em:

```text
~/.config/systemd/user/stellar-monitor.service
```

Criar a pasta:

```bash
mkdir -p ~/.config/systemd/user
```

Criar o serviço:

```bash
nano ~/.config/systemd/user/stellar-monitor.service
```

Exemplo:

```ini
[Unit]
Description=STELLAR Thermal Monitor - CachyOS to ESP32
After=default.target

[Service]
Type=simple
ExecStart=/usr/bin/python /home/SEU_USUARIO/stellar_monitor.py
Restart=always
RestartSec=2

[Install]
WantedBy=default.target
```

> Substitua `SEU_USUARIO` pelo seu usuário do Linux.

---

# ▶️ Ativar o serviço

Recarregar o systemd:

```bash
systemctl --user daemon-reload
```

Ativar no login:

```bash
systemctl --user enable stellar-monitor.service
```

Iniciar:

```bash
systemctl --user start stellar-monitor.service
```

---

# 🔎 Verificar o serviço

```bash
systemctl --user status stellar-monitor.service
```

Resultado esperado:

```text
● stellar-monitor.service
     Loaded: loaded
     Active: active (running)
```

---

# 📜 Ver logs

Para acompanhar os logs em tempo real:

```bash
journalctl --user -u stellar-monitor.service -f
```

---

# 🔄 Reiniciar

```bash
systemctl --user restart stellar-monitor.service
```

---

# 🛑 Parar

```bash
systemctl --user stop stellar-monitor.service
```

---

# 🚫 Desativar inicialização automática

```bash
systemctl --user disable stellar-monitor.service
```

---

# 🛡️ Funcionamento contínuo

O serviço utiliza:

```ini
Restart=always
RestartSec=2
```

Isso significa que, caso o Python seja encerrado inesperadamente, o `systemd` tentará iniciá-lo novamente.

Fluxo:

```text
Python encerra
      │
      ▼
systemd detecta
      │
      ▼
aguarda 2 segundos
      │
      ▼
Python reinicia
      │
      ▼
ESP32 reconecta
```

---

# 🔌 Reconexão USB

O monitor procura automaticamente por portas:

```text
/dev/ttyACM*
```

e:

```text
/dev/ttyUSB*
```

Portanto, não é obrigatório que o ESP32 esteja sempre em `/dev/ttyACM0`.

Exemplo:

```text
/dev/ttyACM0
```

ou:

```text
/dev/ttyACM1
```

O monitor tenta localizar automaticamente a porta disponível.

---

# 🌡️ Temperaturas

O sistema monitora:

## CPU

A temperatura da CPU é obtida através do `lm_sensors`.

Exemplo:

```text
Tctl: +46.6°C
```

## GPU

Para GPUs AMD, o script procura informações do:

```text
amdgpu
```

Exemplo:

```text
edge: +44.0°C
```

---

# 🎨 Cores da temperatura

A interface utiliza uma transição de cores:

| Temperatura | Cor |
|-------------|-----|
| ≤ 30 °C | 🔵 Azul |
| 30–45 °C | 🔵 Azul → 🟢 Verde |
| 45–60 °C | 🟢 Verde → 🟠 Laranja |
| 60–75 °C | 🟠 Laranja → 🔴 Vermelho |
| > 75 °C | 🔴 Vermelho |

---

# 🌌 Tela de inicialização

Ao ligar o ESP32:

```text
┌────────────────────────┐
│                        │
│          ☄️            │
│                        │
│       STELLAR          │
│                        │
│                        │
└────────────────────────┘
```

A animação continua em loop enquanto o ESP32 não recebe dados válidos.

---

# 📡 Recebimento dos dados

O ESP32 recebe pacotes no formato:

```text
CPU:46.6;GPU:44.0
```

Após receber e validar os dados, ele muda para a tela térmica.

Exemplo:

```text
┌────────────────────────┐
│          CPU           │
│                        │
│          47°           │
│                        │
│────────────────────────│
│          GPU           │
│                        │
│          44°           │
└────────────────────────┘
```

---

# ⏳ Estado inicial

Uma característica importante do projeto é que o ESP32 **não utiliza temperaturas simuladas na inicialização**.

O comportamento é:

```text
ESP32 liga
     │
     ▼
Tela STELLAR
     │
     ▼
Aguarda dados USB
     │
     ├── Não recebeu → continua STELLAR
     │
     └── Recebeu dados válidos
                  │
                  ▼
          Tela de temperatura
```

Isso evita que a tela mostre valores antigos ou fictícios antes do sistema operacional iniciar.

---

# 🖥️ Instalação do Arduino IDE

Para programar o ESP32, instale o Arduino IDE.

Depois instale o suporte para placas ESP32 através do gerenciador de placas.

Selecione uma placa compatível com o ESP32-C3.

Exemplo:

```text
ESP32C3 Dev Module
```

Selecione a porta USB:

```text
/dev/ttyACM0
```

ou:

```text
/dev/ttyACM1
```

Depois compile e envie:

```text
arduino/stellar_display.ino
```

---

# 📚 Bibliotecas Arduino

O projeto utiliza:

```cpp
#include <Arduino.h>
#include <U8g2lib.h>
#include <Arduino_GFX_Library.h>
```

Bibliotecas necessárias:

- U8g2
- Arduino_GFX_Library

---

# 📺 Display GC9A01

Configuração:

```text
Resolução: 240 × 240
Controlador: GC9A01
```

No código:

```cpp
#define SCREEN_W 240
#define SCREEN_H 240
```

Rotação:

```cpp
tft->setRotation(3);
```

---

# 📡 Comunicação serial

Baudrate utilizado:

```text
115200
```

Exemplo de pacote:

```text
CPU:46.6;GPU:44.0
```

Atualização:

```text
1 segundo
```

---

# 🧪 Teste completo

Depois de instalar tudo:

### 1. Ligue o ESP32

A tela deverá iniciar com:

```text
STELLAR
```

### 2. Ligue o computador

O CachyOS inicia.

### 3. O systemd inicia automaticamente

Verifique:

```bash
systemctl --user status stellar-monitor.service
```

### 4. O monitor encontra o ESP32

```text
ESP32 conectado!
```

### 5. As temperaturas começam a ser enviadas

```text
CPU: 46.6 °C | GPU: 44.0 °C
```

### 6. O ESP32 muda de tela

A tela STELLAR é substituída pelo monitor térmico.

---

# 🔧 Diagnóstico

## ESP32 não aparece

Execute:

```bash
ls /dev/ttyACM*
```

Depois:

```bash
lsusb
```

Desconecte e reconecte o ESP32.

---

## Permission denied

Se aparecer:

```text
Permission denied: '/dev/ttyACM0'
```

execute:

```bash
sudo usermod -aG uucp $USER
```

Depois faça logout/login ou reinicie.

Verifique:

```bash
groups
```

---

## Serviço não iniciou

Execute:

```bash
systemctl --user status stellar-monitor.service
```

Depois:

```bash
journalctl --user -u stellar-monitor.service -n 100
```

---

## Monitorar logs em tempo real

```bash
journalctl --user -u stellar-monitor.service -f
```

---

# 📦 Instalação automática

O projeto possui um instalador:

```text
cachyos/instalar_stellar.sh
```

O objetivo é automatizar toda a configuração.

O usuário poderá executar:

```bash
chmod +x instalar_stellar.sh
```

e depois:

```bash
./instalar_stellar.sh
```

O instalador poderá realizar:

```text
┌─────────────────────────────────┐
│       instalar_stellar.sh       │
├─────────────────────────────────┤
│                                 │
│ ✓ Verificar sistema             │
│ ✓ Verificar Python              │
│ ✓ Instalar dependências         │
│ ✓ Configurar sensores           │
│ ✓ Configurar permissões USB     │
│ ✓ Detectar ESP32                │
│ ✓ Instalar monitor Python       │
│ ✓ Criar serviço systemd         │
│ ✓ Ativar serviço                │
│ ✓ Iniciar monitor               │
│ ✓ Mostrar status                │
│                                 │
└─────────────────────────────────┘
```

---

# 🔐 Funcionamento local

O STELLAR não depende de serviços externos.

Não são necessários:

- APIs externas;
- servidores;
- banco de dados;
- serviços em nuvem;
- conexão com a Internet.

A comunicação acontece diretamente através do USB:

```text
CachyOS
   │
   │ USB Serial
   ▼
ESP32
   │
   ▼
GC9A01
```

---

# 🏗️ Arquitetura do projeto

```text
                         STELLAR
                            │
             ┌──────────────┴──────────────┐
             │                             │
             ▼                             ▼
          Arduino                        CachyOS
             │                             │
             ▼                             ▼
          ESP32-C3                  stellar_monitor.py
             │                             │
             ▼                       ┌─────┴─────┐
          GC9A01                     │           │
             │                      CPU         GPU
             │                       │           │
             │                       └─────┬─────┘
             │                             │
             └──────── USB Serial ◄────────┘
```

---

# 📂 Arquivos

## `arduino/stellar_display.ino`

Código completo do ESP32.

Responsável por:

- inicialização;
- interface;
- estrelas;
- cometa;
- STELLAR;
- temperaturas;
- cores;
- comunicação USB;
- atualização do display.

---

## `cachyos/stellar_monitor.py`

Código completo do monitor Linux.

Responsável por:

- detectar o ESP32;
- ler CPU;
- ler GPU;
- enviar dados;
- reconectar USB;
- funcionamento contínuo.

---

## `cachyos/instalar_stellar.sh`

Instalador automático.

Responsável por preparar uma nova máquina para executar o monitor.

---

# 🎯 Objetivo do projeto

O STELLAR foi desenvolvido para funcionar como um **painel térmico dedicado para computadores**, utilizando um ESP32-C3 e display GC9A01.

O projeto prioriza:

- simplicidade;
- funcionamento local;
- comunicação USB;
- inicialização automática;
- recuperação automática;
- baixa dependência externa;
- fácil instalação;
- fácil reprodução em outra máquina.

---

# 🌌 STELLAR

```text
███████╗████████╗███████╗██╗     ██╗      █████╗ ██████╗
██╔════╝╚══██╔══╝██╔════╝██║     ██║     ██╔══██╗██╔══██╗
███████╗   ██║   █████╗  ██║     ██║     ███████║██████╔╝
╚════██║   ██║   ██╔══╝  ██║     ██║     ██╔══██║██╔══██╗
███████║   ██║   ███████╗███████╗███████╗██║  ██║██║  ██║
╚══════╝   ╚═╝   ╚══════╝╚══════╝╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝

              BC250 THERMAL MONITOR
```

**ESP32-C3 + GC9A01 + CachyOS + USB Serial**
