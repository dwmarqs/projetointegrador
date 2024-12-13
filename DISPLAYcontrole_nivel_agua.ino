#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Configuração do LCD (endereço padrão 0x27, para 16 colunas e 2 linhas)
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int boia20Pin = 6;   // Pin da boia de 20%
const int boia50Pin = 7;   // Pin da boia de 50%
const int boia100Pin = 8;  // Pin da boia de 100%
const int relePin = 9;     // Pin do relé

bool releState = false;    // Estado inicial do relé (desligado)

void setup() {
  pinMode(boia20Pin, INPUT);
  pinMode(boia50Pin, INPUT);
  pinMode(boia100Pin, INPUT);
  pinMode(relePin, OUTPUT);

  Serial.begin(9600);

  // Inicializa o LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Controle de Agua");
  delay(2000);  // Exibe a mensagem de inicialização
}

void loop() {
  // Leitura das boias
  bool boia20State = digitalRead(boia20Pin);
  bool boia50State = digitalRead(boia50Pin);
  bool boia100State = digitalRead(boia100Pin);

  // Determinar o nível de água
  String nivelAgua;
  if (boia100State == HIGH) {
    nivelAgua = "Cheio (100%)";
  } else if (boia50State == HIGH) {
    nivelAgua = "50-100%";
  } else if (boia20State == HIGH) {
    nivelAgua = "20-50%";
  } else {
    nivelAgua = "Abaixo de 20%";
  }

  // Exibindo o estado das boias no monitor serial
  Serial.print("Boia 20%: ");
  Serial.println(boia20State == LOW ? "Aberta" : "Fechada");

  Serial.print("Boia 50%: ");
  Serial.println(boia50State == LOW ? "Abaixo de 50%" : "Acima de 50%");

  Serial.print("Boia 100%: ");
  Serial.println(boia100State == LOW ? "Aberta" : "Fechada");

  // Atualiza o LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Nivel: ");
  lcd.print(nivelAgua);

  // Lógica de controle do relé
  if (boia20State == LOW && boia100State == LOW) {  // Boia de 20% aberta e boia de 100% fechada
    if (!releState) {  // Se o relé não estiver ligado
      digitalWrite(relePin, HIGH);  // Liga o relé
      releState = true;
      Serial.println("Relé ligado");
    }
  } else if (boia100State == HIGH) {  // Boia de 100% fechada
    if (releState) {  // Se o relé estiver ligado
      digitalWrite(relePin, LOW);  // Desliga o relé
      releState = false;
      Serial.println("Relé desligado");
    }
  }

  delay(500);  // Atraso para leitura mais estável
}
