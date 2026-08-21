# AMD BC250 Monitor Display (ESP32-C3 1.28 inch)

Monitor de temperatura da AMD BC250 utilizando uma ESP32-C3 com display GC9A01 de 1,28".

O sistema funciona em duas partes:

1. A ESP32-C3 controla o display e recebe os dados pela USB.
2. O CachyOS executa um serviço `systemd` que lê as temperaturas da CPU e GPU e envia os valores para a ESP32 a cada segundo.

A comunicação é feita exclusivamente por USB/Serial.

**Fluxo:**

`CachyOS` → `sensores CPU/GPU` → `Python` → `USB Serial` → `ESP32-C3` → `GC9A01`

Enquanto o serviço do CachyOS não estiver enviando dados, a ESP32 permanece na tela de inicialização. A tela de temperatura só é exibida depois que dados válidos são recebidos.

---

## 1. Hardware

### Componentes

- AMD BC250
- ESP32-C3
- Display GC9A01 1,28" 240x240
- Cabo USB para conexão entre o computador e a ESP32-C3

### Display

Pinagem utilizada pela ESP32-C3:

| Função | GPIO |
|---|---:|
| MOSI | 7 |
| SCLK | 6 |
| CS | 10 |
| DC | 2 |
| RST | -1 |
| Backlight | 3 |

---

## 2. Código da ESP32-C3

O código da ESP32 é o arquivo `.ino` presente neste projeto.

Exemplo de estrutura:

```text
AMD-BC250-Monitor-Display/
├── README.md
└── AMD_BC250_Monitor_Display.ino
