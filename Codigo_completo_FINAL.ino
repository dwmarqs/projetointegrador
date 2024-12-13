#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inicializa o LCD com o endereço I2C (0x27) e dimensões 16x2
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Definição dos pinos para os botões
const int botaoSelecionar = 2;
const int botaoVoltar = 4;
const int botaoCima = 3;
const int botaoBaixo = 5;

// Pinos das boias e do relé
const int boia20Pin = 6;
const int boia50Pin = 7;
const int boia100Pin = 8;
const int releAguaPin = 9;

// Pinos do sensor de umidade e do relé
const int pinDigital = 10;
const int pinAnalog = A0;
const int releUmidadePin = 11;

// Calibração do sensor de umidade
const int valorSeco = 1023;
const int valorUmido = 500;

// Variáveis de controle
float AnalogOutput = 0;
int umidadePercent = 0;
int opcaoMenu = 0;
int estadoMenu = 0;
bool reservatorioCheioInicial = false; // Indica se reservatório já foi verificado

// Variáveis de contagem
unsigned int contagemReleAgua = 0;
unsigned int contagemReleUmidade = 0;

// Definições para o debounce
unsigned long debounceDelay = 50;
unsigned long lastDebounceTime[4] = {0};
int lastButtonState[4] = {LOW, LOW, LOW, LOW};
bool buttonPressedState[4] = {false, false, false, false};

// Variáveis de tempo para atualização automática
unsigned long lastUpdateTime = 0;
unsigned long updateInterval = 1000;

void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(boia20Pin, INPUT);
  pinMode(boia50Pin, INPUT);
  pinMode(boia100Pin, INPUT);
  pinMode(releAguaPin, OUTPUT);
  pinMode(releUmidadePin, OUTPUT);
  pinMode(pinDigital, INPUT);

  pinMode(botaoSelecionar, INPUT);
  pinMode(botaoVoltar, INPUT);
  pinMode(botaoCima, INPUT);
  pinMode(botaoBaixo, INPUT);

  digitalWrite(releAguaPin, LOW);
  digitalWrite(releUmidadePin, LOW);

  lcd.setCursor(0, 0);
  lcd.print("Verificando...");
  delay(2000);

  verificarReservatorioInicial();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Bem-vindo");
  lcd.setCursor(0, 1);
  lcd.print("Auto Irrigacao");
  delay(2000);
  lcd.clear();
  mostrarMenu();
}

void loop() {
  if (millis() - lastUpdateTime >= updateInterval) {
    lastUpdateTime = millis();
    atualizarNivelAgua();
    atualizarUmidadeSolo();
    if (estadoMenu == 0) mostrarMenu();
    else if (estadoMenu == 1) {
      if (opcaoMenu == 0) displayNivelAgua();
      else if (opcaoMenu == 1) displayUmidadeSolo();
      else if (opcaoMenu == 2) displayContagemRele();
    }
  }
  if (buttonPressed(botaoCima, 0)) { moverMenu(-1); }
  if (buttonPressed(botaoBaixo, 1)) { moverMenu(1); }
  if (buttonPressed(botaoSelecionar, 2)) { selecionarOpcao(); }
  if (buttonPressed(botaoVoltar, 3)) { if (estadoMenu == 1) voltarMenu(); }
}

void verificarReservatorioInicial() {
  bool boia20State = digitalRead(boia20Pin);
  bool boia100State = digitalRead(boia100Pin);
  if (boia20State == LOW && boia100State == LOW) {
    lcd.setCursor(0, 0);
    lcd.print("Reserv.. vazio");
    lcd.setCursor(0, 1);
    lcd.print("Enchendo...");
    while (digitalRead(boia100Pin) == LOW) {
      digitalWrite(releAguaPin, HIGH);
    }
    digitalWrite(releAguaPin, LOW);
    lcd.clear();
    lcd.print("Reserv.. cheio");
    delay(2000);
  }

  if (digitalRead(boia50Pin) == HIGH) {
    int LeituraSensor = digitalRead(pinDigital);
    digitalWrite(releUmidadePin, LeituraSensor == HIGH ? HIGH : LOW); 
  } else {
    digitalWrite(releUmidadePin, LOW); 
  }

  reservatorioCheioInicial = true;
}

void atualizarNivelAgua() {
  bool boia20State = digitalRead(boia20Pin);
  bool boia100State = digitalRead(boia100Pin);

  if (boia20State == LOW && boia100State == LOW) {
    if (digitalRead(releAguaPin) == LOW) {
      contagemReleAgua++;
    }
    digitalWrite(releAguaPin, HIGH);
  } else if (boia100State == HIGH) {
    digitalWrite(releAguaPin, LOW);
  }
}

void atualizarUmidadeSolo() {
  AnalogOutput = analogRead(pinAnalog);
  umidadePercent = map(AnalogOutput, valorSeco, valorUmido, 0, 100);
  umidadePercent = constrain(umidadePercent, 0, 100);

  int LeituraSensor = digitalRead(pinDigital);
  if (reservatorioCheioInicial) {
    if (LeituraSensor == HIGH && digitalRead(releUmidadePin) == LOW) {
      contagemReleUmidade++;
    }
    digitalWrite(releUmidadePin, LeituraSensor == HIGH ? HIGH : LOW);
  }
}

void mostrarMenu() {
  lcd.clear();
  if (opcaoMenu == 0) {
    lcd.setCursor(0, 0); lcd.print("> Nivel d'Agua");
    lcd.setCursor(0, 1); lcd.print("  Umidade Terra");
  } else if (opcaoMenu == 1) {
    lcd.setCursor(0, 0); lcd.print("  Nivel d'Agua");
    lcd.setCursor(0, 1); lcd.print("> Umidade Terra");
  } else {
    lcd.setCursor(0, 0); lcd.print("  Umidade Terra");
    lcd.setCursor(0, 1); lcd.print("> Contagem Rele");
  }
}

void selecionarOpcao() {
  lcd.clear();
  estadoMenu = 1;
  if (opcaoMenu == 0) displayNivelAgua();
  else if (opcaoMenu == 1) displayUmidadeSolo();
  else if (opcaoMenu == 2) displayContagemRele();
}

void voltarMenu() {
  lcd.clear(); estadoMenu = 0; mostrarMenu();
}

void displayNivelAgua() {
  lcd.setCursor(0, 0); lcd.print("Nivel:");
  bool boia20State = digitalRead(boia20Pin);
  bool boia50State = digitalRead(boia50Pin);
  bool boia100State = digitalRead(boia100Pin);
  if (boia100State == HIGH) lcd.print("Cheio 100%");
  else if (boia50State == HIGH) lcd.print("50-100%    ");
  else if (boia20State == HIGH) lcd.print("20-50%     ");
  else lcd.print("Abaixo 20%");
  
  lcd.setCursor(0, 1);
  lcd.print("Bomba: ");
  if (digitalRead(releAguaPin) == HIGH) {
    lcd.print("Ligada   ");
  } else {
    lcd.print("Desligada");
  }
}

void displayUmidadeSolo() {
  lcd.setCursor(0, 0);
  lcd.print("               ");

  lcd.setCursor(0, 0); 
  lcd.print("Umidade: ");
  lcd.print(umidadePercent);
  lcd.print("%");

  lcd.setCursor(0, 1); 
  lcd.print("Rele:");
  if (digitalRead(pinDigital) == HIGH) lcd.print("Ligado    ");
  else lcd.print("Desligado");
}

void displayContagemRele() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Agua: ");
  lcd.print(contagemReleAgua);

  lcd.setCursor(0, 1);
  lcd.print("Umidade: ");
  lcd.print(contagemReleUmidade);
}

bool buttonPressed(int buttonPin, int buttonIndex) {
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState[buttonIndex]) {
    lastDebounceTime[buttonIndex] = millis();
  }
  if ((millis() - lastDebounceTime[buttonIndex]) > debounceDelay) {
    if (reading == HIGH && !buttonPressedState[buttonIndex]) {
      buttonPressedState[buttonIndex] = true;
      lastButtonState[buttonIndex] = reading;
      return true;
    } else if (reading == LOW) {
      buttonPressedState[buttonIndex] = false;
    }
  }
  lastButtonState[buttonIndex] = reading;
  return false;
}

void moverMenu(int direcao) {
  opcaoMenu += direcao;
  if (opcaoMenu < 0) opcaoMenu = 2;
  else if (opcaoMenu > 2) opcaoMenu = 0;
  mostrarMenu();
}
