#include <Arduino.h>
#include <U8g2lib.h>
#include <Arduino_GFX_Library.h>

// ============================================================
// STELLAR — BC250 THERMAL MONITOR
// ESP32-C3 + ESP32-2424S012 + GC9A01
// ============================================================

// ============================================================
// DISPLAY
// ============================================================

#define TFT_MOSI 7
#define TFT_SCLK 6
#define TFT_CS   10
#define TFT_DC   2
#define TFT_RST  -1
#define TFT_BL   3

#define SCREEN_W 240
#define SCREEN_H 240

// ============================================================
// CORES
// ============================================================

#define BLACK  0x0000
#define WHITE  0xFFFF

#define BLUE   0x001F
#define GREEN  0x07E0
#define ORANGE 0xFD20
#define RED    0xF800

// ============================================================
// BUS SPI
// ============================================================

Arduino_DataBus *bus = new Arduino_ESP32SPI(
  TFT_DC,
  TFT_CS,
  TFT_SCLK,
  TFT_MOSI,
  NOT_A_PIN,
  FSPI
);

// ============================================================
// GC9A01
// ============================================================

Arduino_GFX *tft = new Arduino_GC9A01(
  bus,
  TFT_RST,
  0,
  true
);

// ============================================================
// CANVAS
// ============================================================

Arduino_Canvas *canvas = new Arduino_Canvas(
  SCREEN_W,
  SCREEN_H,
  tft,
  0,
  0
);

// ============================================================
// FONTES
// ============================================================

// CPU / GPU
#define FONT_MEDIUM u8g2_font_helvB12_tf

// Temperatura
#define FONT_LARGE u8g2_font_logisoso42_tf

// STELLAR
// Mesma fonte usada na temperatura
#define FONT_STELLAR u8g2_font_logisoso42_tf

// ============================================================
// TEMPERATURAS
// ============================================================

float cpuTemp = 0.0;
float gpuTemp = 0.0;

// ============================================================
// ESTADO DOS DADOS
// ============================================================

// Só fica true depois que um pacote válido vindo do CachyOS
// for recebido.

bool dadosRecebidos = false;

// ============================================================
// CONTROLE DE COMUNICAÇÃO
// ============================================================

// Tempo do último pacote válido recebido.

unsigned long ultimoDadoRecebido = 0;

// Se passar desse tempo sem receber dados, volta para o splash.

const unsigned long TIMEOUT_DADOS = 5000;

// ============================================================
// TIMERS
// ============================================================

unsigned long ultimaAtualizacao = 0;

const unsigned long INTERVALO = 1000;

// ============================================================
// SPLASH
// ============================================================

// O splash agora não possui duração fixa.
// Ele continua rodando até receber os dados.

unsigned long inicioSplash = 0;

// ============================================================
// ESTRELAS
// ============================================================

struct Estrela {

  int x;
  int y;

  uint8_t brilho;
  uint8_t tamanho;
};

// ============================================================
// CAMPO DE ESTRELAS
// ============================================================

const Estrela estrelas[] = {

  {8, 14, 180, 1},
  {21, 31, 120, 1},
  {37, 12, 230, 1},
  {52, 24, 100, 1},
  {67, 8, 160, 1},
  {82, 30, 220, 1},
  {98, 15, 130, 1},
  {113, 7, 190, 1},
  {128, 23, 110, 1},
  {143, 11, 240, 1},
  {159, 29, 150, 1},
  {176, 14, 210, 1},
  {193, 27, 130, 1},
  {211, 10, 190, 1},
  {228, 25, 150, 1},

  {12, 49, 140, 1},
  {29, 65, 220, 1},
  {46, 52, 110, 1},
  {61, 79, 180, 1},
  {78, 61, 240, 1},
  {94, 47, 120, 1},
  {109, 73, 190, 1},
  {124, 56, 150, 1},
  {139, 81, 230, 1},
  {156, 51, 130, 1},
  {173, 75, 200, 1},
  {190, 58, 110, 1},
  {207, 82, 240, 1},
  {224, 55, 170, 1},

  {7, 93, 200, 1},
  {25, 108, 120, 1},
  {42, 91, 230, 1},
  {59, 119, 150, 1},
  {75, 101, 210, 1},
  {92, 124, 100, 1},
  {108, 96, 180, 1},
  {126, 118, 230, 1},
  {143, 99, 140, 1},
  {161, 121, 200, 1},
  {180, 95, 120, 1},
  {198, 116, 220, 1},
  {216, 91, 150, 1},
  {232, 120, 210, 1},

  {12, 143, 180, 1},
  {28, 161, 100, 1},
  {45, 138, 230, 1},
  {62, 174, 150, 1},
  {79, 151, 210, 1},
  {96, 179, 130, 1},
  {113, 147, 200, 1},
  {131, 171, 110, 1},
  {149, 145, 240, 1},
  {167, 178, 160, 1},
  {186, 151, 220, 1},
  {204, 170, 130, 1},
  {222, 144, 200, 1},

  {9, 195, 220, 1},
  {26, 214, 130, 1},
  {43, 188, 180, 1},
  {61, 207, 240, 1},
  {78, 193, 120, 1},
  {96, 218, 210, 1},
  {114, 196, 150, 1},
  {132, 215, 230, 1},
  {150, 191, 110, 1},
  {169, 219, 190, 1},
  {188, 198, 240, 1},
  {207, 214, 140, 1},
  {225, 191, 210, 1},

  {17, 228, 160, 1},
  {35, 235, 220, 1},
  {53, 226, 120, 1},
  {72, 237, 190, 1},
  {91, 230, 140, 1},
  {110, 235, 230, 1},
  {129, 226, 150, 1},
  {148, 237, 210, 1},
  {168, 228, 120, 1},
  {187, 235, 200, 1},
  {207, 227, 150, 1},
  {226, 236, 220, 1}
};

const int TOTAL_ESTRELAS =
  sizeof(estrelas) /
  sizeof(estrelas[0]);

// ============================================================
// PROTÓTIPOS
// ============================================================

void telaInicializacao();

void desenharEstrelas();

void desenharStellar(
  uint8_t intensidade
);

void desenharCometa(
  int x,
  int y,
  int comprimento
);

void desenharInterface();

void desenharCPU(
  float temperatura
);

void desenharGPU(
  float temperatura
);

void desenharTemperatura(
  float temperatura,
  int centroX,
  int centroY
);

void desenharSeparador();

void desenharBordaFade(
  float cpu,
  float gpu
);

uint16_t corTemperatura(
  float temperatura
);

uint16_t interpolarCor(
  uint16_t corA,
  uint16_t corB,
  float quantidade
);

uint16_t misturarCor(
  uint16_t cor,
  uint16_t fundo,
  uint8_t intensidade
);

void receberDados();

// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(500);

  Serial.println();
  Serial.println("================================");
  Serial.println("          STELLAR");
  Serial.println("      BC250 THERMAL MONITOR");
  Serial.println("================================");
  Serial.println();

  // ==========================================================
  // BACKLIGHT
  // ==========================================================

  pinMode(
    TFT_BL,
    OUTPUT
  );

  digitalWrite(
    TFT_BL,
    HIGH
  );

  delay(100);

  // ==========================================================
  // DISPLAY
  // ==========================================================

  Serial.println(
    "Inicializando GC9A01..."
  );

  if (!tft->begin()) {

    Serial.println(
      "ERRO: GC9A01 nao iniciou!"
    );

    while (true) {
      delay(1000);
    }
  }

  // ==========================================================
  // ROTACAO
  // ==========================================================

  tft->setRotation(3);

  // ==========================================================
  // CANVAS
  // ==========================================================

  canvas->begin();

  canvas->fillScreen(
    BLACK
  );

  canvas->flush();

  Serial.println(
    "GC9A01 OK."
  );

  Serial.println(
    "Canvas OK."
  );

  // ==========================================================
  // SPLASH
  // ==========================================================

  telaInicializacao();

  // ==========================================================
  // SE RECEBEU DADOS
  // ==========================================================

  if (dadosRecebidos) {

    desenharInterface();

    Serial.println(
      "STELLAR THERMAL MONITOR iniciado."
    );
  }
}

// ============================================================
// LOOP
// ============================================================

void loop() {

  // ==========================================================
  // RECEBE DADOS DO CACHYOS
  // ==========================================================

  receberDados();

  // ==========================================================
  // SE AINDA NÃO RECEBEU NENHUM DADO
  // ==========================================================

  if (!dadosRecebidos) {

    // O splash já está sendo executado dentro
    // de telaInicializacao().
    //
    // Caso tenha retornado para cá por algum motivo,
    // executa novamente.

    telaInicializacao();

    return;
  }

  // ==========================================================
  // TIMEOUT
  // ==========================================================

  if (
    millis() -
    ultimoDadoRecebido >
    TIMEOUT_DADOS
  ) {

    Serial.println();
    Serial.println(
      "Sem dados do CachyOS."
    );

    Serial.println(
      "Voltando para tela inicial..."
    );

    dadosRecebidos = false;

    canvas->fillScreen(
      BLACK
    );

    canvas->flush();

    delay(100);

    telaInicializacao();

    return;
  }

  // ==========================================================
  // ATUALIZA DISPLAY
  // ==========================================================

  if (
    millis() -
    ultimaAtualizacao >=
    INTERVALO
  ) {

    ultimaAtualizacao =
      millis();

    desenharInterface();
  }

  // ==========================================================
  // PEQUENO DELAY
  // ==========================================================

  delay(5);
}

// ============================================================
// RECEBER DADOS
// ============================================================
//
// Formato esperado:
//
// CPU:46.6;GPU:44.0
//
// ou:
//
// CPU:46.6;GPU:44.0\n
//
// ============================================================

void receberDados() {

  static String buffer = "";

  while (
    Serial.available() > 0
  ) {

    char caractere =
      Serial.read();

    // ========================================================
    // FIM DO PACOTE
    // ========================================================

    if (
      caractere == '\n' ||
      caractere == '\r'
    ) {

      if (
        buffer.length() > 0
      ) {

        // ======================================================
        // PROCURA CPU
        // ======================================================

        int posCPU =
          buffer.indexOf(
            "CPU:"
          );

        // ======================================================
        // PROCURA GPU
        // ======================================================

        int posGPU =
          buffer.indexOf(
            ";GPU:"
          );

        // ======================================================
        // VALIDA PACOTE
        // ======================================================

        if (
          posCPU >= 0 &&
          posGPU > posCPU
        ) {

          String cpuString =
            buffer.substring(
              posCPU + 4,
              posGPU
            );

          String gpuString =
            buffer.substring(
              posGPU + 5
            );

          cpuString.trim();
          gpuString.trim();

          float novaCPU =
            cpuString.toFloat();

          float novaGPU =
            gpuString.toFloat();

          // ====================================================
          // VALIDA VALORES
          // ====================================================

          if (
            novaCPU >= -20.0 &&
            novaCPU <= 120.0 &&
            novaGPU >= -20.0 &&
            novaGPU <= 120.0
          ) {

            cpuTemp =
              novaCPU;

            gpuTemp =
              novaGPU;

            // ==================================================
            // MARCA COMO CONECTADO
            // ==================================================

            bool primeiroPacote =
              !dadosRecebidos;

            dadosRecebidos =
              true;

            ultimoDadoRecebido =
              millis();

            // ==================================================
            // PRIMEIRO PACOTE
            // ==================================================

            if (
              primeiroPacote
            ) {

              Serial.println();
              Serial.println(
                "================================"
              );

              Serial.println(
                "DADOS DO CACHYOS RECEBIDOS!"
              );

              Serial.print(
                "CPU: "
              );

              Serial.print(
                cpuTemp,
                1
              );

              Serial.println(
                " C"
              );

              Serial.print(
                "GPU: "
              );

              Serial.print(
                gpuTemp,
                1
              );

              Serial.println(
                " C"
              );

              Serial.println(
                "Saindo do splash..."
              );

              Serial.println(
                "================================"
              );

              // =================================================
              // LIMPA A TELA
              // =================================================

              canvas->fillScreen(
                BLACK
              );

              canvas->flush();

              delay(100);

              // =================================================
              // MOSTRA A PRIMEIRA TELA REAL
              // =================================================

              desenharInterface();

              ultimaAtualizacao =
                millis();
            }
          }
        }

        // ======================================================
        // LIMPA BUFFER
        // ======================================================

        buffer = "";
      }

    } else {

      // ========================================================
      // ADICIONA CARACTERE
      // ========================================================

      buffer += caractere;

      // ========================================================
      // PROTEÇÃO CONTRA BUFFER GIGANTE
      // ========================================================

      if (
        buffer.length() > 100
      ) {

        buffer = "";
      }
    }
  }
}

// ============================================================
// TELA DE INICIALIZAÇÃO
// ============================================================
//
// Fica rodando INDEFINIDAMENTE.
//
// Só termina quando receber dados válidos do CachyOS.
//
// ============================================================

void telaInicializacao() {

  unsigned long inicio =
    millis();

  // ==========================================================
  // LOOP DO SPLASH
  // ==========================================================

  while (
    !dadosRecebidos
  ) {

    // ========================================================
    // IMPORTANTE:
    //
    // Verifica a USB antes de desenhar cada frame.
    //
    // Assim o ESP32 pode sair imediatamente do splash
    // quando o serviço começar a enviar.
    // ========================================================

    receberDados();

    if (
      dadosRecebidos
    ) {

      return;
    }

    // ========================================================
    // TEMPO DA ANIMAÇÃO
    // ========================================================

    unsigned long tempo =
      millis() -
      inicio;

    // ========================================================
    // ANIMAÇÃO INFINITA
    // ========================================================

    // A animação completa dura aproximadamente 4200 ms.
    // Depois começa novamente.

    const float DURACAO_ANIMACAO =
      4200.0f;

    float progresso =
      fmod(
        (float)tempo,
        DURACAO_ANIMACAO
      ) /
      DURACAO_ANIMACAO;

    // ========================================================
    // FUNDO
    // ========================================================

    canvas->fillScreen(
      BLACK
    );

    // ========================================================
    // ESTRELAS
    // ========================================================

    desenharEstrelas();

    // ========================================================
    // ESTRELAS GRANDES
    // ========================================================

    canvas->fillCircle(
      23,
      43,
      2,
      0x7BEF
    );

    canvas->fillCircle(
      210,
      48,
      2,
      0x7BEF
    );

    canvas->fillCircle(
      30,
      186,
      2,
      0x5AEB
    );

    canvas->fillCircle(
      216,
      183,
      2,
      0x7BEF
    );

    // ========================================================
    // COMETA
    // ========================================================

    int cometaX =
      -100 +
      (int)(
        progresso *
        440.0f
      );

    int cometaY =
      125 -
      (int)(
        progresso *
        70.0f
      );

    desenharCometa(
      cometaX,
      cometaY,
      125
    );

    // ========================================================
    // FADE DO STELLAR
    // ========================================================

    uint8_t intensidadeTexto =
      0;

    if (
      progresso < 0.18f
    ) {

      intensidadeTexto =
        0;

    } else {

      float fade =
        (
          progresso -
          0.18f
        ) /
        0.55f;

      if (
        fade > 1.0f
      ) {

        fade = 1.0f;
      }

      if (
        fade < 0.0f
      ) {

        fade = 0.0f;
      }

      intensidadeTexto =
        (uint8_t)(
          fade *
          255.0f
        );
    }

    // ========================================================
    // STELLAR
    // ========================================================

    desenharStellar(
      intensidadeTexto
    );

    // ========================================================
    // LINHA DECORATIVA
    // ========================================================

    if (
      progresso > 0.35f
    ) {

      float p =
        (
          progresso -
          0.35f
        ) /
        0.25f;

      if (
        p > 1.0f
      ) {

        p = 1.0f;
      }

      int comprimento =
        (int)(
          110.0f *
          p
        );

      int linhaX =
        (
          SCREEN_W -
          comprimento
        ) / 2;

      uint16_t glow =
        misturarCor(
          0x39E7,
          BLACK,
          70
        );

      canvas->drawFastHLine(
        linhaX,
        190,
        comprimento,
        glow
      );

      canvas->drawFastHLine(
        linhaX,
        189,
        comprimento,
        0x39E7
      );
    }

    // ========================================================
    // FRAME
    // ========================================================

    canvas->flush();

    // ========================================================
    // VELOCIDADE DA ANIMAÇÃO
    // ========================================================

    delay(25);
  }
}

// ============================================================
// ESTRELAS
// ============================================================

void desenharEstrelas() {

  for (
    int i = 0;
    i < TOTAL_ESTRELAS;
    i++
  ) {

    const Estrela &e =
      estrelas[i];

    uint8_t intensidade =
      e.brilho;

    uint8_t r =
      (31 *
       intensidade) /
      255;

    uint8_t g =
      (63 *
       intensidade) /
      255;

    uint8_t b =
      (31 *
       intensidade) /
      255;

    uint16_t cor =
      (r << 11) |
      (g << 5) |
      b;

    if (
      e.tamanho <= 1
    ) {

      canvas->drawPixel(
        e.x,
        e.y,
        cor
      );

    } else {

      canvas->fillCircle(
        e.x,
        e.y,
        e.tamanho,
        cor
      );
    }
  }
}

// ============================================================
// STELLAR
// ============================================================

void desenharStellar(
  uint8_t intensidade
) {

  const char *texto =
    "STELLAR";

  // ==========================================================
  // FONTE
  // ==========================================================

  canvas->setFont(
    FONT_STELLAR
  );

  // ==========================================================
  // DIMENSÕES
  // ==========================================================

  int16_t x1;
  int16_t y1;

  uint16_t largura;
  uint16_t altura;

  canvas->getTextBounds(
    texto,
    0,
    0,
    &x1,
    &y1,
    &largura,
    &altura
  );

  // ==========================================================
  // CENTRALIZAÇÃO
  // ==========================================================

  int x =
    (
      SCREEN_W -
      largura
    ) / 2;

  // ==========================================================
  // POSIÇÃO VERTICAL
  // ==========================================================

  int y =
    158;

  // ==========================================================
  // COR
  // ==========================================================

  uint16_t corTexto =
    misturarCor(
      WHITE,
      BLACK,
      intensidade
    );

  // ==========================================================
  // BORDA PRETA
  // ==========================================================

  canvas->setTextColor(
    BLACK
  );

  canvas->setCursor(
    x - 2,
    y
  );

  canvas->print(
    texto
  );

  canvas->setCursor(
    x + 2,
    y
  );

  canvas->print(
    texto
  );

  canvas->setCursor(
    x,
    y - 2
  );

  canvas->print(
    texto
  );

  canvas->setCursor(
    x,
    y + 2
  );

  canvas->print(
    texto
  );

  canvas->setCursor(
    x - 1,
    y - 1
  );

  canvas->print(
    texto
  );

  canvas->setCursor(
    x + 1,
    y - 1
  );

  canvas->print(
    texto
  );

  canvas->setCursor(
    x - 1,
    y + 1
  );

  canvas->print(
    texto
  );

  canvas->setCursor(
    x + 1,
    y + 1
  );

  canvas->print(
    texto
  );

  // ==========================================================
  // TEXTO
  // ==========================================================

  canvas->setTextColor(
    corTexto
  );

  canvas->setCursor(
    x,
    y
  );

  canvas->print(
    texto
  );
}

// ============================================================
// COMETA
// ============================================================

void desenharCometa(
  int x,
  int y,
  int comprimento
) {

  // ==========================================================
  // CAUDA
  // ==========================================================

  for (
    int i = comprimento;
    i > 0;
    i--
  ) {

    int px =
      x - i;

    if (
      px < 0 ||
      px >= SCREEN_W
    ) {

      continue;
    }

    float fator =
      1.0f -
      (
        (float)i /
        (float)comprimento
      );

    int largura =
      1 +
      (int)(
        fator *
        14.0f
      );

    uint8_t intensidade =
      (uint8_t)(
        fator *
        210.0f
      );

    uint16_t cor =
      misturarCor(
        WHITE,
        BLACK,
        intensidade
      );

    int py =
      y +
      (i / 4);

    // ========================================================
    // CAUDA
    // ========================================================

    for (
      int yy = -largura;
      yy <= largura;
      yy++
    ) {

      int finalY =
        py +
        yy;

      if (
        finalY >= 0 &&
        finalY < SCREEN_H
      ) {

        canvas->drawPixel(
          px,
          finalY,
          cor
        );
      }
    }

    // ========================================================
    // GLOW AZUL
    // ========================================================

    if (
      i % 3 == 0
    ) {

      uint16_t glow =
        misturarCor(
          0x39E7,
          BLACK,
          intensidade / 3
        );

      if (
        py -
        largura -
        3 >= 0
      ) {

        canvas->drawPixel(
          px,
          py -
          largura -
          3,
          glow
        );
      }

      if (
        py +
        largura +
        3 <
        SCREEN_H
      ) {

        canvas->drawPixel(
          px,
          py +
          largura +
          3,
          glow
        );
      }
    }
  }

  // ==========================================================
  // AURA
  // ==========================================================

  if (
    x >= -30 &&
    x < SCREEN_W + 30 &&
    y >= -30 &&
    y < SCREEN_H + 30
  ) {

    canvas->fillCircle(
      x,
      y,
      25,
      misturarCor(
        0x39E7,
        BLACK,
        20
      )
    );

    canvas->fillCircle(
      x,
      y,
      20,
      misturarCor(
        0x39E7,
        BLACK,
        35
      )
    );

    canvas->fillCircle(
      x,
      y,
      16,
      misturarCor(
        0x5AEF,
        BLACK,
        60
      )
    );

    canvas->fillCircle(
      x,
      y,
      12,
      misturarCor(
        0x7BEF,
        BLACK,
        110
      )
    );

    // ========================================================
    // NÚCLEO
    // ========================================================

    canvas->fillCircle(
      x,
      y,
      9,
      0x7BEF
    );

    canvas->fillCircle(
      x,
      y,
      6,
      WHITE
    );

    canvas->fillCircle(
      x,
      y,
      3,
      WHITE
    );
  }
}

// ============================================================
// INTERFACE PRINCIPAL
// ============================================================

void desenharInterface() {

  canvas->fillScreen(
    BLACK
  );

  desenharBordaFade(
    cpuTemp,
    gpuTemp
  );

  desenharCPU(
    cpuTemp
  );

  desenharSeparador();

  desenharGPU(
    gpuTemp
  );

  canvas->flush();
}

// ============================================================
// CPU
// ============================================================

void desenharCPU(
  float temperatura
) {

  canvas->setFont(
    FONT_MEDIUM
  );

  canvas->setTextColor(
    WHITE
  );

  canvas->setCursor(
    98,
    54
  );

  canvas->print(
    "CPU"
  );

  desenharTemperatura(
    temperatura,
    120,
    101
  );
}

// ============================================================
// GPU
// ============================================================

void desenharGPU(
  float temperatura
) {

  canvas->setFont(
    FONT_MEDIUM
  );

  canvas->setTextColor(
    WHITE
  );

  canvas->setCursor(
    98,
    145
  );

  canvas->print(
    "GPU"
  );

  desenharTemperatura(
    temperatura,
    120,
    191
  );
}

// ============================================================
// TEMPERATURA
// ============================================================

void desenharTemperatura(
  float temperatura,
  int centroX,
  int centroY
) {

  char valor[8];

  sprintf(
    valor,
    "%.0f",
    temperatura
  );

  canvas->setFont(
    FONT_LARGE
  );

  canvas->setTextColor(
    WHITE
  );

  int16_t x1;
  int16_t y1;

  uint16_t largura;
  uint16_t altura;

  canvas->getTextBounds(
    valor,
    0,
    0,
    &x1,
    &y1,
    &largura,
    &altura
  );

  int x =
    centroX -
    (largura / 2) -
    7;

  canvas->setCursor(
    x,
    centroY
  );

  canvas->print(
    valor
  );

  // ==========================================================
  // GRAU
  // ==========================================================

  int grauX =
    x +
    largura +
    8;

  int grauY =
    centroY -
    39;

  canvas->fillCircle(
    grauX,
    grauY,
    3,
    WHITE
  );
}

// ============================================================
// SEPARADOR
// ============================================================

void desenharSeparador() {

  float temperatura =
    (cpuTemp > gpuTemp)
    ? cpuTemp
    : gpuTemp;

  uint16_t corBase =
    corTemperatura(
      temperatura
    );

  // ==========================================================
  // GLOW EXTERNO
  // ==========================================================

  uint16_t glow1 =
    misturarCor(
      corBase,
      BLACK,
      45
    );

  canvas->drawFastHLine(
    62,
    118,
    116,
    glow1
  );

  canvas->drawFastHLine(
    62,
    123,
    116,
    glow1
  );

  // ==========================================================
  // GLOW INTERNO
  // ==========================================================

  uint16_t glow2 =
    misturarCor(
      corBase,
      BLACK,
      100
    );

  canvas->drawFastHLine(
    62,
    119,
    116,
    glow2
  );

  canvas->drawFastHLine(
    62,
    122,
    116,
    glow2
  );

  // ==========================================================
  // NÚCLEO
  // ==========================================================

  canvas->drawFastHLine(
    62,
    120,
    116,
    corBase
  );

  canvas->drawFastHLine(
    62,
    121,
    116,
    corBase
  );
}

// ============================================================
// BORDA
// ============================================================

void desenharBordaFade(
  float cpu,
  float gpu
) {

  float temperatura =
    (cpu > gpu)
    ? cpu
    : gpu;

  uint16_t corBase =
    corTemperatura(
      temperatura
    );

  // ==========================================================
  // BORDA EXTERNA
  // ==========================================================

  for (
    int raio = 116;
    raio >= 108;
    raio--
  ) {

    int distancia =
      116 - raio;

    float intensidade =
      255.0f -
      (
        distancia *
        18.0f
      );

    if (
      intensidade < 100
    ) {

      intensidade = 100;
    }

    uint16_t cor =
      misturarCor(
        corBase,
        BLACK,
        (uint8_t)
        intensidade
      );

    canvas->drawCircle(
      120,
      120,
      raio,
      cor
    );
  }

  // ==========================================================
  // FADE INTERNO
  // ==========================================================

  for (
    int raio = 107;
    raio >= 78;
    raio--
  ) {

    float distancia =
      107.0f -
      raio;

    float progresso =
      distancia /
      29.0f;

    float suavizacao =
      1.0f -
      (
        progresso *
        progresso
      );

    float intensidade =
      75.0f *
      suavizacao;

    if (
      intensidade < 1
    ) {

      intensidade = 1;
    }

    uint16_t cor =
      misturarCor(
        corBase,
        BLACK,
        (uint8_t)
        intensidade
      );

    canvas->drawCircle(
      120,
      120,
      raio,
      cor
    );
  }
}

// ============================================================
// COR DA TEMPERATURA
// ============================================================

uint16_t corTemperatura(
  float temperatura
) {

  // ==========================================================
  // <= 30 AZUL
  // ==========================================================

  if (
    temperatura <= 30.0f
  ) {

    return BLUE;
  }

  // ==========================================================
  // AZUL -> VERDE
  // ==========================================================

  if (
    temperatura <= 45.0f
  ) {

    float progresso =
      (
        temperatura -
        30.0f
      ) /
      15.0f;

    return interpolarCor(
      BLUE,
      GREEN,
      progresso
    );
  }

  // ==========================================================
  // VERDE -> LARANJA
  // ==========================================================

  if (
    temperatura <= 60.0f
  ) {

    float progresso =
      (
        temperatura -
        45.0f
      ) /
      15.0f;

    return interpolarCor(
      GREEN,
      ORANGE,
      progresso
    );
  }

  // ==========================================================
  // LARANJA -> VERMELHO
  // ==========================================================

  if (
    temperatura <= 75.0f
  ) {

    float progresso =
      (
        temperatura -
        60.0f
      ) /
      15.0f;

    return interpolarCor(
      ORANGE,
      RED,
      progresso
    );
  }

  // ==========================================================
  // >75 VERMELHO
  // ==========================================================

  return RED;
}

// ============================================================
// INTERPOLAÇÃO
// ============================================================

uint16_t interpolarCor(
  uint16_t corA,
  uint16_t corB,
  float quantidade
) {

  if (
    quantidade < 0.0f
  ) {

    quantidade = 0.0f;
  }

  if (
    quantidade > 1.0f
  ) {

    quantidade = 1.0f;
  }

  float rA =
    (corA >> 11) &
    0x1F;

  float gA =
    (corA >> 5) &
    0x3F;

  float bA =
    corA &
    0x1F;

  float rB =
    (corB >> 11) &
    0x1F;

  float gB =
    (corB >> 5) &
    0x3F;

  float bB =
    corB &
    0x1F;

  uint8_t r =
    rA +
    (
      (rB - rA) *
      quantidade
    );

  uint8_t g =
    gA +
    (
      (gB - gA) *
      quantidade
    );

  uint8_t b =
    bA +
    (
      (bB - bA) *
      quantidade
    );

  return (
    (r << 11) |
    (g << 5) |
    b
  );
}

// ============================================================
// MISTURA
// ============================================================

uint16_t misturarCor(
  uint16_t cor,
  uint16_t fundo,
  uint8_t intensidade
) {

  uint8_t r1 =
    (cor >> 11) &
    0x1F;

  uint8_t g1 =
    (cor >> 5) &
    0x3F;

  uint8_t b1 =
    cor &
    0x1F;

  uint8_t r2 =
    (fundo >> 11) &
    0x1F;

  uint8_t g2 =
    (fundo >> 5) &
    0x3F;

  uint8_t b2 =
    fundo &
    0x1F;

  uint8_t r =
    r2 +
    (
      (r1 - r2) *
      intensidade
    ) / 255;

  uint8_t g =
    g2 +
    (
      (g1 - g2) *
      intensidade
    ) / 255;

  uint8_t b =
    b2 +
    (
      (b1 - b2) *
      intensidade
    ) / 255;

  return (
    (r << 11) |
    (g << 5) |
    b
  );
}