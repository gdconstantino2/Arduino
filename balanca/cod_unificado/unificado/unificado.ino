#include "HX711.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Configurações do LCD e HX711
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define pinCELULA_DOUT 2
#define pinCELULA_SCK 4

HX711 balanca;

// --- PARÂMETROS DE CALIBRAÇÃO E FILTRAGEM ---
const float FATOR_CALIBRACAO = 425.81; // Seu valor calculado
float peso_filtrado = 0;
float alpha = 0.15; // Coeficiente do Filtro EMA (0.0 a 1.0)
// Quanto menor o alpha, mais ignora o ruído, mas a leitura fica mais lenta.

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("Olá!");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");

  balanca.begin(pinCELULA_DOUT, pinCELULA_SCK);
  balanca.set_scale(FATOR_CALIBRACAO);
  
  // Aguarda um pouco e faz a Tara
  delay(2000);
  balanca.tare();
  
  lcd.clear();
  lcd.print("Balança Pronta!");
  delay(1000);
  lcd.clear();
}

void loop() {
  if (balanca.is_ready()) {
    float leitura_bruta = balanca.get_units(3);

    // Filtro EMA
    peso_filtrado = (alpha * leitura_bruta) + ((1.0 - alpha) * peso_filtrado);

    // --- IMPLEMENTAÇÃO DE ZONA MORTA (Deadzone) ---
    // Se o valor for menor que 10g (positivo ou negativo), força o zero no LCD
    float peso_exibicao = peso_filtrado;
    if (abs(peso_filtrado) < 10.0) { 
      peso_exibicao = 0.0;
    }

    // Atualização do LCD usando o valor com zona morta
    lcd.setCursor(6, 0);
    lcd.print(peso_exibicao, 1);
    lcd.print(" g   "); // Espaços extras para apagar resíduos de números anteriores

    // --- COMANDO DE TARA VIA SERIAL ---
    if (Serial.available() > 0) {
      char comando = Serial.read();
      if (comando == 't' || comando == 'T') {
        balanca.tare();
        peso_filtrado = 0; // Reseta o filtro para não dar salto
        Serial.println("Balança Zerada com sucesso!");
      }
    }
  }
  delay(50);
}