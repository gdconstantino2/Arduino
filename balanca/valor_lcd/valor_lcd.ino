#include "HX711.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD 16x2 no endereço I2C 0x27 (o mais comum)
// Se não aparecer nada no LCD, tente trocar 0x27 por 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Pinos do módulo HX711
#define pinCELULA_DOUT 2
#define pinCELULA_SCK 4

HX711 balanca;

void setup() {
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");
  delay(1000);

  balanca.begin(pinCELULA_DOUT, pinCELULA_SCK);

  // Testes iniciais — exibidos no LCD (resumido, pois LCD é pequeno)
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Teste ADC:");
  lcd.setCursor(0, 1);
  lcd.print(balanca.read());
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Media 20:");
  lcd.setCursor(0, 1);
  lcd.print(balanca.read_average(20));
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Valor 5:");
  lcd.setCursor(0, 1);
  lcd.print(balanca.get_value(5));
  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Unidades:");
  lcd.setCursor(0, 1);
  lcd.print(balanca.get_units(5), 1);
  delay(2000);

  // ---- CALIBRAÇÃO ----
  balanca.set_scale(515.4750476190476190); 
  balanca.tare();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrado!");
  delay(1500);
}

void loop() {
  float leitura = balanca.get_units(1);
  float media = balanca.get_units(10);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Peso:");
  lcd.print(leitura, 1);
  lcd.print(" g");

  lcd.setCursor(0, 1);
  lcd.print("Media:");
  lcd.print(media, 1);
  lcd.print(" g");

  delay(1000);
}
