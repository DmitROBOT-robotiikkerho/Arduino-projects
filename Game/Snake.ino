#include <LiquidCrystal.h>
LiquidCrystal lcd(9,6,5,4,3,2);
uint32_t myTimer1;

#include <LedControl.h>
LedControl lc = LedControl(12, 11, 10, 1);  //MAX7219: DIN, CLK, CS
#define BUZZER_PIN 8
#define X A1
#define Y A0
#define Z 7
#define ANALOG_PIN_FOR_RND A3
enum {NOTOUCH,RIGHT,LEFT,DOWN,UP};

struct Store {int value;bool flag;};

struct Snakes {
  int x;
  int y;
  bool flagX;
  bool flagY;
};

struct Mouses {
  int x;
  int y;
};

Store store;
Snakes snake;
Mouses mouse;

unsigned long timeOfMovement = 0;
int movement = 400; // speed snake
int player, playerOld;
int tailX[64], tailY[64];

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(Z, INPUT);

  lc.shutdown(0, false);
  lc.setIntensity(0, 4);
  lc.clearDisplay(0);

  randomSeed(analogRead(ANALOG_PIN_FOR_RND));
  Serial.begin(9600);
  draw_clear();

  lcd.begin(16,2);
  lcd.clear();

  for(int i=0;i<=10;i++)
  {
  lcd.setCursor(0,0);
  lcd.print("Game 'SNAKE'");
  lcd.setCursor(0,1);
  lcd.print("Start:");
  tone(BUZZER_PIN, 440, 100);
  lcd.print(i);
  delay(1000);
  }
  lcd.clear();
}

void loop() {
  at_the_start();
  while (store.flag) {
    player_move('x');
    player_move('y');

    // Проверка съедания мыши / Checking if the snake ate the mouse
    if (snake.x == mouse.x && snake.y == mouse.y) {
      bool valid = false;
      while (!valid) {
        valid = true;
        mouse.x = random(0, 8);
        mouse.y = random(0, 8);
        for (int i = 0; i < store.value; i++) {
          if (mouse.x == tailX[i] && mouse.y == tailY[i]) {
            valid = false;
            break;
          }
        }
      }
      store.value++;
      lcd.setCursor(0,1);
      lcd.print("Your score: ");
      lcd.print(store.value);
      tone(BUZZER_PIN, 440, 100);
      
    }

    // Движене змейки по таймеру / Snake movement by timer
    if (millis() - timeOfMovement > movement) {
      draw_clear();
      draw_point(mouse.x, mouse.y, true);
      snake_drow();

      // Проверка выхода за границы / Boundary check
      if (snake.x < 0 || snake.x >= 8 || snake.y < 0 || snake.y >= 8) {
        game_over();
      }

      // Проверка самопересечения / Self-collision check
      for (int i = 1; i <= store.value; i++) {
        if (snake.x == tailX[i] && snake.y == tailY[i]) {
          game_over();
        }
      }

      timeOfMovement = millis();
    }
  }
}

//Функция начала игры / Game start function
void at_the_start() {
  draw_clear();
  store = {0, false};
  player = NOTOUCH;
  playerOld = NOTOUCH;

  player_move('z'); // ждем кнопку старта / Waiting for start button

  snake = {random(0, 8), random(0, 8), true, true};
  mouse = {random(0, 8), random(0, 8)};

  draw_point(mouse.x, mouse.y, true);
  delay(movement);
}

//Конец игры / Game over
void game_over() 
{
  
  midi_gameover();
  lcd.clear();
  draw_pouring(true);
  delay(300);
  draw_pouring(false);

  lcd.setCursor(0,0);
  lcd.print("GAME OVER");

  lcd.setCursor(0,1);
  lcd.print("Your score: ");
  lcd.print(store.value);
  if(millis()-myTimer1>=100000)
  {myTimer1=millis();}

  store.flag = false;
  randomSeed(analogRead(ANALOG_PIN_FOR_RND));
  delay(500);
}

//Управление игроком / Player control
void player_move(char coordinate) {
  switch (coordinate) {
    case 'x': {
      int x = analogRead(X);
      if (x < 350 && snake.flagX) {
        player = LEFT;
        snake.flagX = false;
      }
      if (x > 850 && snake.flagX) {
        player = RIGHT;
        snake.flagX = false;
      }
      if (x >= 350 && x <= 850) {
        snake.flagX = true;
      }
      break;
    }
    case 'y': {
      int y = analogRead(Y);
      if (y < 350 && snake.flagY) {
        player = UP;
        snake.flagY = false;
      }
      if (y > 850 && snake.flagY) {
        player = DOWN;
        snake.flagY = false;
      }
      if (y >= 350 && y <= 850) {
        snake.flagY = true;
      }
      break;
    }
    case 'z': {
      while (digitalRead(Z) == LOW) {
        // ждем нажатия
        delay(50);
      }
      store.flag = true;
      break;
    }
  }
}

//Отрисовка змеи / Snake rendering
void snake_drow() {
  switch (player) {
    case RIGHT:
      if (playerOld != LEFT) snake.x++;
      break;
    case LEFT:
      if (playerOld != RIGHT) snake.x--;
      break;
    case UP:
      if (playerOld != DOWN) snake.y--;
      break;
    case DOWN:
      if (playerOld != UP) snake.y++;
      break;
  }
  playerOld = player;

  tail_create();
  for (int i = 0; i <= store.value; i++) {
    draw_point(tailX[i], tailY[i], true);
  }
}

//Формирование хвоста / Tail creation
void tail_create() {
  int bufferX = tailX[0];
  int bufferY = tailY[0];
  int bufferX2, bufferY2;
  tailX[0] = snake.x;
  tailY[0] = snake.y;
  for (int i = 1; i <= store.value; i++) {
    bufferX2 = tailX[i];
    bufferY2 = tailY[i];
    tailX[i] = bufferX;
    tailY[i] = bufferY;
    bufferX = bufferX2;
    bufferY = bufferY2;
  }
}

//Отрисовка точки / Drawing a point
void draw_point(int x, int y, bool on) {
  if (x >= 0 && x < 8 && y >= 0 && y < 8) {
    lc.setLed(0, y, x, on);
  }
}

//Очистка / Clearing
void draw_clear() {
  lc.clearDisplay(0);
}

//Заливка / Filling
void draw_pouring(bool on) {
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      lc.setLed(0, y, x, on);
    }
  }
}


void midi_gameover() 
{
tone(BUZZER_PIN, 523, 150.0);
delay(166.666666667);
delay(166.666666667);
tone(BUZZER_PIN, 391, 150.0);
delay(166.666666667);
delay(166.666666667);
tone(BUZZER_PIN, 329, 225.0);
delay(250.0);
tone(BUZZER_PIN, 440, 172.5);
delay(191.666666667);
delay(2.77777777778);
tone(BUZZER_PIN, 493, 172.5);
delay(191.666666667);
delay(2.77777777778);
tone(BUZZER_PIN, 440, 175.0);
delay(194.444444444);
tone(BUZZER_PIN, 415, 225.0);
delay(250.0);
tone(BUZZER_PIN, 466, 225.0);
delay(250.0);
tone(BUZZER_PIN, 415, 225.0);
delay(250.0);
tone(BUZZER_PIN, 391, 112.5);
delay(125.0);
tone(BUZZER_PIN, 349, 112.5);
delay(125.0);
tone(BUZZER_PIN, 391, 30.0);
delay(33.3333333333);
tone(BUZZER_PIN, 349, 600.0);
delay(666.666666667);
tone(BUZZER_PIN, 349, 2.5);
delay(2.77777777778);
}
