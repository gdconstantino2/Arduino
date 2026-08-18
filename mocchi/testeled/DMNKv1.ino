#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h> 

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define WHITE SSD1306_WHITE
#define BLACK SSD1306_BLACK

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- Mapeamento de Hardware ---
const int PINO_TOUCH = 1;  
const int PINO_BUZZER = 2; 

// --- Definição da Máquina de Estados ---
enum EstadoRobo { BOOT, FELIZ, ANIMADO, BRAVO };
EstadoRobo estadoAtual = BOOT;

// --- Timers Assíncronos (millis) ---
unsigned long timerUltimoToque = 0;
unsigned long timerUltimaMensagem = 0;
unsigned long timerAnimacao = 0;
unsigned long timerSom = 0;
unsigned long timerEsperaTexto = 0; 

const unsigned long TEMPO_PARA_FICAR_BRAVO = 1800000; 
const unsigned long INTERVALO_MENSAGENS = 30000; 
const unsigned long DURACAO_MENSAGEM = 8000; 
const unsigned long DURACAO_ESTADO_ANIMADO = 5000; 
const unsigned long TEMPO_CARA_ANIMADA = 3000; 

// --- Controle do Display e Textos ---
bool mostrandoTexto = false;
bool aguardandoTextoAnimado = false; 
String textoAtual = "";

// Textos do Estado Feliz 
const char fFeliz1[] PROGMEM = "Amo vc!";
const char fFeliz2[] PROGMEM = "Que dia lindo!";
const char fFeliz3[] PROGMEM = "Nao se preocupe! Eu te projeto das formigas!";
const char fFeliz4[] PROGMEM = "Voce eh uma pessoa incrivel!!!";
const char fFeliz5[] PROGMEM = "O criador te ama muito!!!";
const char fFeliz6[] PROGMEM = "Ola, vidaaa!!!!";
const char fFeliz7[] PROGMEM = "Fiu, Fiu! Você tah gata, hein!";
const char fFeliz8[] PROGMEM = "Vinho eh a cor mais bonita!";
const char fFeliz9[] PROGMEM = "Eu gosto muito de lirios";
const char fFeliz10[] PROGMEM = "Mon amour, je t'aime";
const char fFeliz11[] PROGMEM = "As vezes me pergunto, como eh na Irlanda? E a França?";
const char fFeliz12[] PROGMEM = "Se vc estah lendo isso, saiba que o Gugu te ama!";
const char fFeliz13[] PROGMEM = "Soh passando pra lembrar que vc eh a mulher mais bonita do mundo, Elisa";
const char fFeliz14[] PROGMEM = "Canta Lisboa pra mim???";
const char fFeliz15[] PROGMEM = "Não sei pq, mas minha programaçao me faz ser time Jacob";
const char fFeliz16[] PROGMEM = "O Gugu te mandou um abraco bem apertado!";
const char fFeliz17[] PROGMEM = "O Gugu estah com saudade de vc!";
const char fFeliz18[] PROGMEM = "Vc eh a razão do meu viver!!";
const char fFeliz19[] PROGMEM = "Jah foi na academia hoje?";
const char fFeliz20[] PROGMEM = "Eu adorei a sua fantasia de McQueen e Sally! Muito criativo!";

const char* const listaFelizes[] PROGMEM = {fFeliz1, fFeliz2, fFeliz3, fFeliz4, fFeliz5, fFeliz6, fFeliz7, fFeliz8, fFeliz9,
fFeliz10, fFeliz11, fFeliz12, fFeliz13, fFeliz14, fFeliz15, fFeliz16, fFeliz17, fFeliz18, fFeliz19, fFeliz20};
const int qtdFelizes = sizeof(listaFelizes) / sizeof(listaFelizes[0]);

// Textos do Estado Animado 
const char fAnimado1[] PROGMEM = "Eba! Carinho!";
const char fAnimado2[] PROGMEM = "Mais, por favor!";
const char fAnimado3[] PROGMEM = "Adoro quando vc estah comigo!";

const char* const listaAnimadas[] PROGMEM = {fAnimado1, fAnimado2, fAnimado3};
const int qtdAnimadas = sizeof(listaAnimadas) / sizeof(listaAnimadas[0]);

// --- Estrutura da Máquina de Som ---
struct NotaMusical { int frequencia; int duracao; };
const NotaMusical melodiaFeliz[] = { {1047, 100}, {1319, 100}, {1568, 100}, {2093, 200}, {0, 0} };
int indiceNotaAtual = 0;
bool tocandoMelodia = false;

// =====================================================================
// MOTOR DE ANIMAÇÃO FACIAL (VETORIAL + TRANSLAÇÃO)
// =====================================================================
int cOlhoEsqX = 20, cOlhoDirX = 80, cOlhoY = 5, cOlhoW = 28, cOlhoH = 40;
int cPupEsqX = 30, cPupDirX = 90, cPupY = 16;
int cBocaH = 16; 

int tOlhoEsqX = 20, tOlhoDirX = 80, tOlhoY = 5, tOlhoW = 28, tOlhoH = 40;
int tPupEsqX = 30, tPupDirX = 90, tPupY = 16;
int tBocaH = 16;

// Eixos da Câmera (Offsets para mover o rosto inteiro)
int cOffsetX = 0, cOffsetY = 0;
int tOffsetX = 0, tOffsetY = 0;

unsigned long timerFrame = 0;
unsigned long timerPiscar = 0;
unsigned long timerMudarFace = 0;
int tempoAlvoPiscar = 3000;
int tempoAlvoFace = 4000;
bool estaPiscando = false;

// =====================================================================
// PROTÓTIPOS DE FUNÇÕES
// =====================================================================
void interpolar(int &atual, int alvo, int velocidade);
void definirExpressao(int tipo);
void atualizarRenderizacao(unsigned long tempoAtual);
void exibirTextoSobreposto();
void desenharRaivaEstatica();
void sortearTexto(const char* const* lista, int tamanhoMaximo);
void iniciarMelodia();
void processarAudio(unsigned long tempoAtual);
// =====================================================================

void interpolar(int &atual, int alvo, int velocidade) {
  if (atual < alvo) { atual += velocidade; if (atual > alvo) atual = alvo; }
  else if (atual > alvo) { atual -= velocidade; if (atual < alvo) atual = alvo; }
}

void definirExpressao(int tipo) {
  tOlhoEsqX = 20; tOlhoDirX = 80; tOlhoY = 5; tOlhoW = 28; tOlhoH = 40;
  tPupY = 16; tPupEsqX = 30; tPupDirX = 90;
  tBocaH = 16;

  switch(tipo) {
    case 0: break;
    case 1: tPupY = 25; tPupEsqX = 34; tPupDirX = 94; break;
    case 2: tBocaH = 6; break;
    case 3: tPupEsqX = 22; tPupDirX = 82; break;
    case 4: tOlhoY = 15; tOlhoH = 20; tPupY = 25; tPupEsqX = 34; tPupDirX = 94; tBocaH = 6; break;
    case 5: tOlhoY = 15; tOlhoH = 20; tPupY = 25; tPupEsqX = 34; tPupDirX = 94; break;
    case 6: tPupY = 25; tPupEsqX = 34; tPupDirX = 94; break;
    case 7: tOlhoY = 15; tOlhoH = 20; tPupY = 25; tPupEsqX = 34; tPupDirX = 94; break;
    case 8: tOlhoY = 15; tOlhoH = 15; break;
    case 9: 
      tOlhoY = 20; tOlhoH = 10; 
      tPupY = 25; tPupEsqX = 34; tPupDirX = 94; 
      tBocaH = 20; 
      break;
  }
}

void setup() {
  pinMode(PINO_TOUCH, INPUT);
  
  ledcAttach(PINO_BUZZER, 2000, 8); 
  ledcWriteTone(PINO_BUZZER, 0); 

  Wire.begin(8, 9);
  Wire.setClock(100000);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) for(;;);

  randomSeed(esp_random()); 
  display.clearDisplay();
  timerUltimoToque = millis();
}

void loop() {
  unsigned long tempoAtual = millis();
  
  bool toqueDetectado = digitalRead(PINO_TOUCH) == HIGH;

  if (toqueDetectado && estadoAtual != ANIMADO) {
    estadoAtual = ANIMADO;
    timerUltimoToque = tempoAtual;
    
    timerEsperaTexto = tempoAtual;
    aguardandoTextoAnimado = true;
    mostrandoTexto = false; 
    
    iniciarMelodia();
    definirExpressao(9); 
  }

  processarAudio(tempoAtual);

  switch (estadoAtual) {
    case BOOT:
      if (tempoAtual > 3000) { 
        estadoAtual = FELIZ; 
        timerUltimaMensagem = tempoAtual;
      }
      break;

    case FELIZ: 
      if (tempoAtual - timerUltimoToque >= TEMPO_PARA_FICAR_BRAVO) {
        estadoAtual = BRAVO;
        mostrandoTexto = false; 
        break;
      }

      if (!mostrandoTexto && (tempoAtual - timerUltimaMensagem >= INTERVALO_MENSAGENS)) {
        sortearTexto(listaFelizes, qtdFelizes); 
        timerUltimaMensagem = tempoAtual;
      }

      if (mostrandoTexto && (tempoAtual - timerAnimacao >= DURACAO_MENSAGEM)) {
        mostrandoTexto = false; 
      }
      break;

    case ANIMADO: 
      if (aguardandoTextoAnimado) {
        if (tempoAtual - timerEsperaTexto >= TEMPO_CARA_ANIMADA) {
          aguardandoTextoAnimado = false;
          sortearTexto(listaAnimadas, qtdAnimadas);
        }
      } else {
        if (tempoAtual - timerAnimacao >= DURACAO_ESTADO_ANIMADO) {
          estadoAtual = FELIZ; 
          mostrandoTexto = false;
          timerUltimaMensagem = tempoAtual;
        }
      }
      break;
      
    case BRAVO:
      break;
  }

  atualizarRenderizacao(tempoAtual);
}

void atualizarRenderizacao(unsigned long tempoAtual) {
  if (estadoAtual == BOOT) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(20, 20); display.println("Iniciando DMNK...");
    display.setCursor(30, 35); display.println("Ola, Elisa!");
    display.display();
    return;
  }

  // Define as coordenadas da câmera: Empurra o rosto para baixo e direita se for falar
  if (mostrandoTexto) {
    tOffsetX = 20;
    tOffsetY = 24; 
  } else {
    tOffsetX = 0;
    tOffsetY = 0;
  }

  if (estadoAtual == FELIZ && !mostrandoTexto && (tempoAtual - timerMudarFace > tempoAlvoFace)) {
    definirExpressao(random(0, 9)); 
    timerMudarFace = tempoAtual;
    tempoAlvoFace = random(2000, 6000); 
  }

  if (!estaPiscando && (tempoAtual - timerPiscar > tempoAlvoPiscar)) {
    estaPiscando = true;
    timerPiscar = tempoAtual;
  } else if (estaPiscando && (tempoAtual - timerPiscar > 150)) { 
    estaPiscando = false;
    timerPiscar = tempoAtual;
    tempoAlvoPiscar = random(2000, 5000);
  }

  if (tempoAtual - timerFrame > 15) {
    interpolar(cOlhoEsqX, tOlhoEsqX, 2); interpolar(cOlhoDirX, tOlhoDirX, 2);
    interpolar(cOlhoY, tOlhoY, 2); interpolar(cOlhoW, tOlhoW, 2); interpolar(cOlhoH, tOlhoH, 2);
    interpolar(cPupEsqX, tPupEsqX, 2); interpolar(cPupDirX, tPupDirX, 2); interpolar(cPupY, tPupY, 2);
    interpolar(cBocaH, tBocaH, 1);
    interpolar(cOffsetX, tOffsetX, 2); 
    interpolar(cOffsetY, tOffsetY, 2); 
    timerFrame = tempoAtual;
  }

  display.clearDisplay();

  if (estadoAtual == BRAVO) {
    desenharRaivaEstatica();
  } else {
    int renderOlhoH = cOlhoH;
    int renderOlhoY = cOlhoY;
    int renderBocaH = cBocaH;

    if (estaPiscando) {
      renderOlhoH = 4;
      renderOlhoY = cOlhoY + (cOlhoH / 2) - 2;
    }

    if (mostrandoTexto) {
      renderBocaH = 8 + (sin(tempoAtual / 60.0) * 8); 
    }

    // O rosto é desenhado somando a âncora de Offset a todos os pontos
    display.fillRoundRect(cOlhoEsqX + cOffsetX, renderOlhoY + cOffsetY, cOlhoW, renderOlhoH, 10, WHITE);
    display.fillRoundRect(cOlhoDirX + cOffsetX, renderOlhoY + cOffsetY, cOlhoW, renderOlhoH, 10, WHITE);
    
    if (renderOlhoH > 10) {
      display.fillCircle(cPupEsqX + cOffsetX, cPupY + cOffsetY, 5, BLACK);
      display.fillCircle(cPupDirX + cOffsetX, cPupY + cOffsetY, 5, BLACK);
    }

    display.fillCircle(64 + cOffsetX, 48 + cOffsetY, 14, WHITE);
    display.fillRect(48 + cOffsetX, 48 - renderBocaH + cOffsetY, 32, renderBocaH, BLACK); 
  }

  if (mostrandoTexto) {
    exibirTextoSobreposto();
  }

  display.display();
}

void exibirTextoSobreposto() {
  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK); 

  String linhas[6]; 
  int numLinhas = 0;
  String linhaAtual = "";
  String palavra = "";
  String textoTemp = textoAtual + " "; 

  for (int i = 0; i < textoTemp.length(); i++) {
    char c = textoTemp.charAt(i);
    if (c != ' ') {
      palavra += c;
    } else {
      if ((linhaAtual.length() + palavra.length() + 1) * 6 <= (SCREEN_WIDTH - 4)) {
        if (linhaAtual.length() > 0) linhaAtual += " ";
        linhaAtual += palavra;
      } else {
        if (numLinhas < 6) { linhas[numLinhas] = linhaAtual; numLinhas++; }
        linhaAtual = palavra; 
      }
      palavra = "";
    }
  }
  if (linhaAtual.length() > 0 && numLinhas < 6) { linhas[numLinhas] = linhaAtual; numLinhas++; }

  int startY = 0; 
  int startX = 2; 

  for (int i = 0; i < numLinhas; i++) {
    display.setCursor(startX, startY + (i * 9)); 
    display.print(linhas[i]);
  }
}

void desenharRaivaEstatica() {
  for(int i = -6; i <= 6; i++) {
    display.drawLine(15, 10 + i, 55, 32 + i, WHITE);
    display.drawLine(55, 32 + i, 15, 54 + i, WHITE);
    display.drawLine(113, 10 + i, 73, 32 + i, WHITE);
    display.drawLine(73, 32 + i, 113, 54 + i, WHITE);
  }
  display.fillRoundRect(42, 54, 44, 8, 3, WHITE);
}

void sortearTexto(const char* const* lista, int tamanhoMaximo) {
  int indiceSorteado = random(0, tamanhoMaximo);
  textoAtual = String(FPSTR(lista[indiceSorteado])); 
  mostrandoTexto = true;
  timerAnimacao = millis(); 
}

void iniciarMelodia() {
  indiceNotaAtual = 0;
  tocandoMelodia = true;
  timerSom = millis();
  ledcWriteTone(PINO_BUZZER, melodiaFeliz[0].frequencia);
}

void processarAudio(unsigned long tempoAtual) {
  if (tocandoMelodia) {
    int duracaoNota = melodiaFeliz[indiceNotaAtual].duracao;
    if (tempoAtual - timerSom >= duracaoNota) {
      indiceNotaAtual++;
      int proximaFrequencia = melodiaFeliz[indiceNotaAtual].frequencia;
      if (proximaFrequencia == 0 && melodiaFeliz[indiceNotaAtual].duracao == 0) {
        ledcWriteTone(PINO_BUZZER, 0); 
        tocandoMelodia = false;
      } else {
        ledcWriteTone(PINO_BUZZER, proximaFrequencia);
        timerSom = tempoAtual;
      }
    }
  }
}
