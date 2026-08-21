# AMD-BC250-Display-Status

# ⭐ STELLAR — Thermal Monitor

Monitoramento térmico em tempo real para o projeto **STELLAR**, utilizando um **ESP32-C3 + display GC9A01 240×240**, conectado via USB a um computador Linux.

O sistema coleta a temperatura da **CPU e GPU** no CachyOS e envia os dados continuamente para o ESP32 através da USB Serial.

Enquanto o sistema operacional ainda não iniciou ou o serviço ainda não está enviando dados, o ESP32 mantém a tela de inicialização do STELLAR em loop.

Assim que dados válidos são recebidos, o display muda automaticamente para a interface térmica.

---

## 📸 Visão geral

```text
┌──────────────────────────────┐
│          CACHYOS             │
│                              │
│        lm_sensors            │
│             │                │
│             ▼                │
│    stellar_monitor.py        │
│             │                │
└─────────────┼────────────────┘
              │
              │ USB Serial
              │ 115200 baud
              ▼
┌──────────────────────────────┐
│          ESP32-C3             │
│                              │
│     Recepção Serial USB      │
│             │                │
│             ▼                │
│        GC9A01 240×240        │
│                              │
│       STELLAR DISPLAY        │
└──────────────────────────────┘
```

---

# 📁 Estrutura do projeto

```text
STELLAR/
│
├── README.md
│
├── CachyOS/
│   ├── stellar_monitor.py
│   └── stellar-monitor.service
│
└── ESP32/
    └── STELLAR.ino
```

---

# 🔧 Hardware

## ESP32

- ESP32-C3
- ESP32-2424S012
- Display GC9A01
- Resolução 240×240
- Comunicação USB Serial

## Computador

O sistema foi desenvolvido e testado no:

- CachyOS
- CPU AMD
- GPU AMD
- Linux com systemd

---

# 📺 Display

O display utilizado possui resolução:

```text
240 × 240 pixels
```

## Mapeamento dos pinos

| Função | GPIO |
|---|---:|
| MOSI | 7 |
| SCLK | 6 |
| CS | 10 |
| DC | 2 |
| RST | -1 |
| Backlight | 3 |

O projeto utiliza:

```cpp
#include <Arduino.h>
#include <U8g2lib.h>
#include <Arduino_GFX_Library.h>
```

---

# 💻 Parte 1 — Configuração do CachyOS

## 1. Atualizar o sistema

```bash
sudo pacman -Syu
```

---

## 2. Verificar Python

```bash
python --version
```

Exemplo:

```text
Python 3.14.6
```

---

## 3. Instalar lm_sensors

```bash
sudo pacman -S lm_sensors
```

Teste:

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

O monitor utiliza:

### CPU

```text
k10temp
Tctl
```

### GPU AMD

```text
amdgpu
edge
```

---

# 🔌 4. Conectar o ESP32

Conecte o ESP32 ao computador através do USB.

Verifique a porta:

```bash
ls /dev/ttyACM*
```

Normalmente será:

```text
/dev/ttyACM0
```

ou:

```text
/dev/ttyACM1
```

Também é possível verificar o dispositivo USB:

```bash
lsusb
```

---

# 🔐 5. Permissões da porta USB

Verifique os grupos do usuário:

```bash
groups
```

Caso `uucp` não esteja presente:

```bash
sudo usermod -aG uucp $USER
```

Também pode ser necessário:

```bash
sudo usermod -aG dialout $USER
```

Depois reinicie o computador:

```bash
reboot
```

Após reiniciar:

```bash
groups
```

O grupo deverá aparecer na lista.

---

# 🐍 6. Instalar PySerial

Instale:

```bash
python -m pip install pyserial
```

Teste:

```bash
python -c "import serial; print('pyserial OK')"
```

Resultado esperado:

```text
pyserial OK
```

---

# 📄 7. Criar o monitor Python

Crie:

```bash
nano ~/stellar_monitor.py
```

O arquivo deverá ficar em:

```text
/home/SEU_USUARIO/stellar_monitor.py
```

Exemplo:

```text
/home/isaacalvex/stellar_monitor.py
```

O programa possui as seguintes funções:

- Detectar o ESP32 automaticamente
- Detectar `/dev/ttyACM*`
- Detectar `/dev/ttyUSB*`
- Ler temperatura da CPU
- Ler temperatura da GPU AMD
- Enviar os dados pela USB
- Reconectar automaticamente
- Continuar funcionando caso a conexão USB seja interrompida
- Enviar os dados aproximadamente a cada 1 segundo

---

# 📡 Protocolo de comunicação

A comunicação utiliza:

```text
USB Serial
```

Baud rate:

```text
115200
```

O CachyOS envia os dados neste formato:

```text
CPU:46.6;GPU:44.0
```

Cada pacote termina com:

```text
\n
```

Portanto:

```text
CPU:46.6;GPU:44.0\n
```

---

# 🧪 8. Testar o monitor manualmente

Execute:

```bash
python ~/stellar_monitor.py
```

Resultado esperado:

```text
==================================================
        STELLAR THERMAL MONITOR
        CachyOS -> ESP32 USB
==================================================
Conectando ao ESP32 em /dev/ttyACM0...
ESP32 conectado!
CPU:  46.6 °C | GPU:  44.0 °C
```

A tela do ESP32 deverá receber os dados e mudar para a interface térmica.

Para interromper o teste:

```text
CTRL + C
```

---

# ⚙️ 9. Criar serviço systemd

Para que o monitor funcione continuamente, não é necessário deixar um terminal aberto.

O projeto utiliza um serviço **systemd do usuário**.

Crie a pasta:

```bash
mkdir -p ~/.config/systemd/user
```

Crie o arquivo:

```bash
nano ~/.config/systemd/user/stellar-monitor.service
```

Utilize:

```ini
[Unit]
Description=STELLAR Thermal Monitor - CachyOS to ESP32
After=graphical-session.target
Wants=graphical-session.target

[Service]
Type=simple
ExecStart=/usr/bin/python /home/SEU_USUARIO/stellar_monitor.py
Restart=always
RestartSec=2

Environment=PYTHONUNBUFFERED=1

[Install]
WantedBy=default.target
```

Substitua:

```text
SEU_USUARIO
```

pelo usuário do computador.

Exemplo:

```ini
ExecStart=/usr/bin/python /home/isaacalvex/stellar_monitor.py
```

---

# ▶️ 10. Ativar o serviço

Depois de salvar o arquivo:

```bash
systemctl --user daemon-reload
```

Ative a inicialização automática:

```bash
systemctl --user enable stellar-monitor.service
```

Inicie:

```bash
systemctl --user start stellar-monitor.service
```

Verifique:

```bash
systemctl --user status stellar-monitor.service
```

O resultado esperado é:

```text
Active: active (running)
```

---

# 🔄 11. Execução contínua

O serviço possui:

```ini
Restart=always
```

Caso o Python seja encerrado inesperadamente, o systemd irá iniciá-lo novamente.

Também existe:

```ini
RestartSec=2
```

Assim, o sistema aguarda aproximadamente 2 segundos antes de tentar iniciar novamente.

O monitor também tenta reconectar automaticamente ao ESP32 caso a porta USB desapareça.

---

# 📜 12. Ver logs

Para acompanhar o monitor em tempo real:

```bash
journalctl --user -u stellar-monitor.service -f
```

Para visualizar os últimos 50 registros:

```bash
journalctl --user -u stellar-monitor.service -n 50
```

---

# 🛑 13. Parar o serviço

```bash
systemctl --user stop stellar-monitor.service
```

---

# ▶️ 14. Iniciar novamente

```bash
systemctl --user start stellar-monitor.service
```

---

# 🔄 15. Reiniciar

```bash
systemctl --user restart stellar-monitor.service
```

---

# ❌ 16. Desativar inicialização automática

```bash
systemctl --user disable stellar-monitor.service
```

---

# 🖥️ Parte 2 — ESP32

O ESP32 possui duas funções principais:

1. Controlar a interface visual.
2. Receber as temperaturas enviadas pelo CachyOS.

O firmware é responsável por:

- Inicializar o GC9A01
- Exibir a animação STELLAR
- Manter a animação enquanto não houver dados
- Receber dados pela Serial USB
- Validar os dados recebidos
- Exibir CPU
- Exibir GPU
- Atualizar a interface térmica

---

# 🚀 Estado inicial

Ao ligar o ESP32, a tela **não assume temperaturas padrão como dados reais**.

Enquanto nenhum pacote válido for recebido, a tela permanece na inicialização:

```text
┌────────────────────────┐
│                        │
│        ☄️              │
│                        │
│       STELLAR          │
│                        │
└────────────────────────┘
```

A animação fica em loop.

---

# 🟢 Ativação da tela térmica

Quando o CachyOS inicia o serviço e começa a enviar:

```text
CPU:46.6;GPU:44.0
```

o ESP32 reconhece os dados e ativa a tela térmica.

Isso evita que o display mostre temperaturas antigas ou simuladas antes do computador começar a enviar informações.

---

# 🌡️ Interface térmica

A interface exibe:

```text
CPU
47°

──────────────

GPU
44°
```

Os valores são atualizados continuamente.

---

# 🎨 Sistema de cores

As temperaturas utilizam transição gradual:

| Temperatura | Cor |
|---:|---|
| ≤ 30 °C | 🔵 Azul |
| 30–45 °C | Azul → Verde |
| 45–60 °C | Verde → Laranja |
| 60–75 °C | Laranja → Vermelho |
| > 75 °C | 🔴 Vermelho |

---

# 📚 Bibliotecas Arduino

O firmware utiliza:

```cpp
#include <Arduino.h>
#include <U8g2lib.h>
#include <Arduino_GFX_Library.h>
```

Instale na Arduino IDE:

```text
U8g2
Arduino_GFX
```

---

# 🔧 Configuração da Arduino IDE

Selecione a placa correspondente ao ESP32-C3.

Normalmente:

```text
ESP32C3 Dev Module
```

Selecione a porta USB correspondente:

```text
/dev/ttyACM0
```

ou:

```text
/dev/ttyACM1
```

Compile e faça o upload do firmware.

---

# ⚠️ Porta USB ocupada

Durante o upload, o serviço Python pode estar utilizando a mesma porta USB.

Se aparecer:

```text
Could not open /dev/ttyACM0
```

pare temporariamente o serviço:

```bash
systemctl --user stop stellar-monitor.service
```

Faça o upload pela Arduino IDE.

Depois reative:

```bash
systemctl --user start stellar-monitor.service
```

---

# ⚠️ Permission denied

Se aparecer:

```text
Permission denied: '/dev/ttyACM0'
```

verifique:

```bash
groups
```

Adicione:

```bash
sudo usermod -aG uucp $USER
```

e:

```bash
sudo usermod -aG dialout $USER
```

Reinicie:

```bash
reboot
```

---

# 🧪 Teste completo

Depois de configurar tudo:

### 1. Verificar ESP32

```bash
ls /dev/ttyACM*
```

### 2. Verificar sensores

```bash
sensors
```

### 3. Testar Python

```bash
python ~/stellar_monitor.py
```

### 4. Verificar serviço

```bash
systemctl --user status stellar-monitor.service
```

### 5. Ver logs

```bash
journalctl --user -u stellar-monitor.service -f
```

### 6. Reiniciar computador

```bash
reboot
```

---

# 🔄 Fluxo completo

O funcionamento após a instalação é:

```text
PC LIGADO
    │
    ▼
ESP32 inicia
    │
    ▼
Tela STELLAR em loop
    │
    ▼
CachyOS inicia
    │
    ▼
systemd inicia
    │
    ▼
stellar_monitor.py
    │
    ▼
Detecta CPU/GPU
    │
    ▼
Abre USB Serial
    │
    ▼
Envia dados
    │
    ▼
ESP32 recebe
    │
    ▼
Valida dados
    │
    ▼
Ativa tela térmica
    │
    ▼
Atualiza aproximadamente
a cada 1 segundo
```

---

# 🔌 Reconexão

O sistema foi desenvolvido para tolerar interrupções temporárias.

Se o ESP32 for desconectado:

```text
ESP32 desconectado
        │
        ▼
Python detecta erro
        │
        ▼
Fecha porta
        │
        ▼
Procura ESP32 novamente
        │
        ▼
Reconecta
        │
        ▼
Continua enviando
```

Se o processo Python for encerrado:

```text
Python encerrado
       │
       ▼
systemd detecta
       │
       ▼
aguarda 2 segundos
       │
       ▼
Python reinicia
```

---

# 🛠️ Diagnóstico rápido

## ESP32 não aparece

```bash
ls /dev/ttyACM*
```

Depois:

```bash
lsusb
```

Verifique também o cabo USB.

---

## Python não conecta

```bash
python ~/stellar_monitor.py
```

Verifique:

```bash
ls -l /dev/ttyACM*
```

E:

```bash
groups
```

---

## Serviço não iniciou

```bash
systemctl --user status stellar-monitor.service
```

Depois:

```bash
journalctl --user -u stellar-monitor.service -n 100
```

---

## CPU não aparece

Execute:

```bash
sensors
```

Procure:

```text
k10temp
```

e:

```text
Tctl
```

---

## GPU não aparece

Execute:

```bash
sensors
```

Procure:

```text
amdgpu
```

e:

```text
edge
```

---

# 📦 Instalação rápida em uma nova máquina

Após instalar o CachyOS, execute:

```bash
sudo pacman -Syu
```

```bash
sudo pacman -S lm_sensors
```

```bash
python --version
```

```bash
python -m pip install pyserial
```

```bash
sudo usermod -aG uucp $USER
```

```bash
sudo usermod -aG dialout $USER
```

Reinicie:

```bash
reboot
```

Depois:

```bash
mkdir -p ~/.config/systemd/user
```

Coloque:

```text
stellar_monitor.py
```

em:

```text
/home/SEU_USUARIO/
```

E:

```text
stellar-monitor.service
```

em:

```text
/home/SEU_USUARIO/.config/systemd/user/
```

Depois:

```bash
systemctl --user daemon-reload
```

```bash
systemctl --user enable stellar-monitor.service
```

```bash
systemctl --user start stellar-monitor.service
```

Verifique:

```bash
systemctl --user status stellar-monitor.service
```

---

# 📊 Especificações

| Item | Informação |
|---|---|
| MCU | ESP32-C3 |
| Display | GC9A01 |
| Resolução | 240×240 |
| Comunicação | USB Serial |
| Baud Rate | 115200 |
| Atualização | ~1 segundo |
| Sistema operacional | CachyOS |
| CPU monitorada | AMD |
| GPU monitorada | AMD |
| Backend | Python |
| Comunicação Python | PySerial |
| Serviço | systemd |
| Biblioteca gráfica | Arduino_GFX |
| Biblioteca de fontes | U8g2 |

---

# 🧩 Tecnologias

```text
C++
Arduino
ESP32-C3
Arduino_GFX
U8g2
GC9A01
Python
PySerial
lm_sensors
systemd
Linux
CachyOS
USB Serial
```

---

# ⭐ STELLAR

Projeto desenvolvido para criar um monitor térmico dedicado e independente da interface principal do computador.

O sistema combina:

```text
CachyOS
   │
   ├── lm_sensors
   │
   ├── Python
   │
   ├── PySerial
   │
   └── systemd
          │
          │ USB
          ▼
       ESP32-C3
          │
          ▼
       GC9A01
          │
          ▼
    STELLAR Thermal
```

O objetivo é fornecer **monitoramento térmico contínuo**, com uma interface visual dedicada e inicialização independente do sistema operacional.

---

## 📜 Licença

Defina aqui a licença do projeto, por exemplo:

```text
MIT License
```

ou outra licença de sua preferência.
