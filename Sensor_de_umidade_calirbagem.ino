#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define pinDigital 10
#define pinAnalog 0
#define pinRelay 11

// Valores de calibração - ajuste após medições de solo seco e úmido
const int valorSeco = 1023;  // Ajuste este valor após a calibração
const int valorUmido = 500;  // Ajuste este valor após a calibração

float AnalogOutput = 0;
float voltage = 0;
int LeituraSensor = 0;
int umidadePercent = 0;
int umidadePercentAnterior = -1;  // Para armazenar a última umidade exibida

unsigned long tempoAnterior = 0;
const unsigned long intervaloAtualizacao = 500;  // Atualiza a cada 500ms

// Configuração do display LCD com I2C (endereço I2C, colunas, linhas)
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Ajuste 0x27 para o endereço correto se necessário


void setup() {
  pinMode(pinDigital, INPUT);
  pinMode(pinRelay, OUTPUT);
  Serial.begin(9600);

  // Inicializa o display LCD I2C
  lcd.init();
  lcd.backlight();  // Ativa a luz de fundo do display
  lcd.print("Inicializando...");
  delay(2000);  // Espera para exibir a mensagem inicial

  lcd.clear();  // Limpa o display após a mensagem inicial

  // Cria o caractere "gota" no LCD
  lcd.createChar(0, gota);  // Gota de água como caractere 0
}

void loop() {
  AnalogOutput = analogRead(pinAnalog);
  LeituraSensor = digitalRead(pinDigital);
  voltage = AnalogOutput * (5.0 / 1023.0);

  // Calcula a porcentagem de umidade com base na calibração
  umidadePercent = map(AnalogOutput, valorSeco, valorUmido, 0, 100);
  umidadePercent = constrain(umidadePercent, 0, 100);  // Limita entre 0 e 100

  // Exibe o valor de umidade no Serial Monitor
  Serial.print("Umidade: ");
  Serial.println(AnalogOutput);
  Serial.println("%");

  // Controle do relé com as novas condições
  if (LeituraSensor == HIGH) {
    digitalWrite(pinRelay, HIGH);   // Liga o relé (solo seco)
    Serial.print("Relé LIGADO - Solo Seco com Umidade: ");
    Serial.print(umidadePercent);
    Serial.println("%");
  }
  else if (LeituraSensor == LOW) {
    digitalWrite(pinRelay, LOW);   
    Serial.print("Relé DESLIGADO - Solo Úmido com Umidade: ");
    Serial.print(umidadePercent);
    Serial.println("%");
  }

  // Verifica se é hora de atualizar o display
  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnterior >= intervaloAtualizacao) {
    tempoAnterior = tempoAtual;

    // Só atualiza o display se o valor de umidade mudou
    if (umidadePercent != umidadePercentAnterior) {
      umidadePercentAnterior = umidadePercent;

      lcd.setCursor(0, 0);
      lcd.print("Umidade: ");
      lcd.print(umidadePercent);
      lcd.print("% ");
    }


}
