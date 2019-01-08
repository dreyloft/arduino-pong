/*
Pong!
Original By Pete Lamonica
modified by duboisvb
updated by James Bruce (http://www.makeuseof.com/tag/author/jbruce)
modified by Steffen Roth (http://www.dreyloft.com)
*/

#include <TVout.h>
#include <fontALL.h>

// Pins
#define WHEEL_ONE_PIN 0   // analog player 1 left
#define WHEEL_TWO_PIN 1   // analog player 2 right
#define WHEEL_THREE_PIN 2 // analog player 3 top
#define WHEEL_FOUR_PIN 3  // analog player 4 four
#define BUTTON_ONE_PIN 2  // digital start button
#define BUZZER_PIN 10     // digital for sound (buzzer)

// fixed paddle definitions
#define PADDLE_LENGTH 14
#define RIGHT_PADDLE_X 124 // (hres - 4)
#define LEFT_PADDLE_X 2
#define TOP_PADDLE_Y 2
#define BOTTOM_PADDLE_Y 94 // (vres - 4)

// game states
#define MAIN_MENU 1        // in menu state
#define GAME_OVER_SCREEN 2 // game over state
#define INFO 3             // info state
#define RUNNING_GAME 0     // in game state - draw the dynamic part of the game
#define BACKGROUND 0       // in game state - draw constants of the game box
#define XHALF 64           // x screen half
#define YHALF 49           // y screen half

// fix positions etc.
#define SCORE_Y 4
#define MAX_Y_VELOCITY 8
#define WIN_SCORE 10
#define DEFAULT_GAME_SPEED 50
#define INCREASE_GAME_SPEED 3

// sounds
#define SOUND_WALL 1500 // frequency for ball hit wall sound
#define SOUND_SCORE 500 // frequency for player scores sound

TVout TV;

// paddels
int rightPaddleY = 0;
int leftPaddleY = 0;
int topPaddleX = 0;
int bottomPaddleX = 0;
int lastContact = 0;

// ball
unsigned char ballX = XHALF;
unsigned char ballY = YHALF;
char ballVolX = 1;
char ballVolY = 1;

// player
bool fourPlayerMode = false;
char leftPlayerScore = 0;
char rightPlayerScore = 0;
char topPlayerScore = 0;
char bottomPlayerScore = 0;

// states
bool buttonStatus = false;
bool firstStart = true;
char state = MAIN_MENU;
int gameSpeed = DEFAULT_GAME_SPEED;
 
void buttonPress() {
  buttonStatus = (digitalRead(BUTTON_ONE_PIN));
  
  if ((buttonStatus == 0) && (state == GAME_OVER_SCREEN))
  {
    drawMenu();
  }
}

void drawGameScreen() {
  // draw right paddle
  rightPaddleY = ((analogRead(WHEEL_ONE_PIN) / 8) * (98 - PADDLE_LENGTH)) / 128;
  TV.draw_line(RIGHT_PADDLE_X + 1, rightPaddleY, RIGHT_PADDLE_X + 1, rightPaddleY + PADDLE_LENGTH, 1);

  // draw left paddle
  leftPaddleY = ((analogRead(WHEEL_TWO_PIN) / 8) * (98 - PADDLE_LENGTH)) / 128;
  TV.draw_line(LEFT_PADDLE_X, leftPaddleY, LEFT_PADDLE_X, leftPaddleY + PADDLE_LENGTH, 1);

  if (fourPlayerMode) {
    // draw top paddle
    topPaddleX = ((analogRead(WHEEL_THREE_PIN) / 8) * (98 - PADDLE_LENGTH)) / 128 + 15;
    TV.draw_line(topPaddleX, TOP_PADDLE_Y, topPaddleX + PADDLE_LENGTH, TOP_PADDLE_Y, 1);
  
    // draw bottom paddle
    bottomPaddleX = ((analogRead(WHEEL_FOUR_PIN) / 8) * (98 - PADDLE_LENGTH)) / 128 + 15;
    TV.draw_line(bottomPaddleX, BOTTOM_PADDLE_Y, bottomPaddleX + PADDLE_LENGTH, BOTTOM_PADDLE_Y, 1);
  }
  
  // draw score
  if (!fourPlayerMode) {
    TV.print_char(58, SCORE_Y, '0' + leftPlayerScore);
    TV.print_char(68, SCORE_Y, '0' + rightPlayerScore);
  } else {
    TV.print_char(10, SCORE_Y, '0' + leftPlayerScore);
    TV.print_char(113, SCORE_Y, '0' + topPlayerScore); // X = 113 -> TV.hres() -4 -10 -1 (hres -char width -space -linewidth)
    TV.print_char(113, 88, '0' + rightPlayerScore);    // Y = 88  -> TV.vres() -4 -6     (vres -char height -SCORE_Y)
    TV.print_char(10, 88, '0' + bottomPlayerScore);
  }
  
  // draw ball
  TV.set_pixel(ballX, ballY, 1);
}

// calculates the current scores
void playerScored(int lastContact) {
  if (lastContact == 1) leftPlayerScore++;
  if (lastContact == 2) rightPlayerScore++;
  if (lastContact == 3) topPlayerScore++;
  if (lastContact == 4) bottomPlayerScore++;
  
  // check for game end
  if (leftPlayerScore == WIN_SCORE || rightPlayerScore == WIN_SCORE || topPlayerScore == WIN_SCORE || bottomPlayerScore == WIN_SCORE) {
    fourPlayerMode = false;
    state = GAME_OVER_SCREEN;
  }
  
  ballVolX = -ballVolX;
}

// draw static play field
void drawBox() {
  TV.clear_screen();

  if (!fourPlayerMode) {
    // net only needed in 2 player mode
    for(int i = 2; i < 98 - 4; i += 6) {
      TV.draw_line(XHALF, i, XHALF, i + 3, 1);
    }
    // lines
    TV.draw_line(0, 0, 127, 0, 1);   // top
    TV.draw_line(0, 97, 127, 97, 1); // bottom
  } else {
    // lines
    TV.draw_line(0,0,16,0,1);      // top left
    TV.draw_line(112,0,128,0,1);   // top right
    TV.draw_line(0,97,15,97,1);    // bottom left
    TV.draw_line(112,97,128,97,1); // bottom right
  }
  
  state = RUNNING_GAME;
}

// start screen
void drawMenu() {
  // wait for continue
  while(buttonStatus){
    buttonPress();
  }

  int pos = 0;
  int oldPos = -1; // for first start
  
  while(!buttonStatus){
    // menu refresh
    oldPos = pos;
    pos = analogRead(WHEEL_ONE_PIN); // pos = 0 ... 1023 

    // prevents unnecessary framedrops
    if ((oldPos != pos) && (firstStart || (oldPos <= 500 && pos > 500) || (oldPos >= 500 && pos < 500) || (oldPos <= 1000 && pos > 1000) || (oldPos >= 1000 && pos < 1000))) {
      fourPlayerMode = false;
      firstStart = false;
      
      drawBox();
    
      // static text
      printText(0, 0, 1, 45, 17, "PONG!");
      TV.print(21, 85, "PRESS BUTTON TO SELECT");
      
      // main menu game mode selection
      printText(pos, 0, 500, 28, 44, "CLASSIC 2P");    // 2 palyer mode
      printText(pos, 500, 1000, 28, 55, "CLASSIC 4P"); // 4 player mode
      printText(pos, 1000, 1024, 28, 66, "INFO");      // smaller range because of less importance      
    }
    
    buttonPress();
  }

  if (pos >= 500 && pos < 1000) {
      fourPlayerMode = true;
  }
    
  // start game
  if (pos < 1000) {
    state = BACKGROUND;
  } else {
    state = INFO;
  }
}

// center text for font4x6 -> (Screen Width / 2) - (4 * stringLength) / 2 + 1
// center text for font8x8 -> (Screen Width / 2) - (8 * stringLength) / 2 + 1
// print text function incl function to make active menu point bigger
void printText (int pos, int minEntry, int maxEntry, int xPos, int yPos, char* text){
  int corr = 0;
  
  if ( pos >= minEntry && pos < maxEntry ){
    TV.select_font(font8x8);
    corr = 2;
  }
  
  TV.print(xPos, yPos - corr, text);
  TV.select_font(font4x6);
}

void setup() {
  // setup sound and TV specs
  pinMode(BUZZER_PIN, OUTPUT);
  TV.begin(_NTSC, 128, 98);
}

void loop() {
  if (state == MAIN_MENU) {
    buttonPress();
    gameSpeed = DEFAULT_GAME_SPEED;
    lastContact = 0;
    drawBox();
    drawMenu();
  }

  if (state == INFO) {
    drawBox();

    printText(0, 0, 1, 49, 17, "INFO");
    TV.print(41, 44, "(C) 2017 BY\n\n          STEFFEN ROTH"); 
    //TV.print(41, 44, "(C) 2017 BY\n\n        SEBASTIAN SCHMIDT\n\n               AND\n\n           STEFFEN ROTH"); 
    
    // debouncing / prevent douple push
    while(buttonStatus) {
      buttonPress();
    }
    while(!buttonStatus) {
      buttonPress();
    }
    state = MAIN_MENU;
  }
  
  if (state == BACKGROUND) {
    drawBox();
  }
  
  // dynamic game content
  if (state == RUNNING_GAME) {
    ballX += ballVolX;
    ballY += ballVolY;

    // change if hit top or bottom wall
    if (((ballY == 1 || ballY == 97) && !fourPlayerMode) || ((ballY == 1 || ballY >= 97) && fourPlayerMode && (ballX <= 16 || ballX >= 112))){
      ballVolY = -ballVolY;
      tone(BUZZER_PIN, SOUND_WALL);
    }
    
    // test left side for paddle hit
    if((ballX <= LEFT_PADDLE_X + 1) && (ballY >= leftPaddleY) && (ballY <= leftPaddleY + PADDLE_LENGTH)){
      ballVolX = -ballVolX;
      ballVolY += 2 * ((ballY - leftPaddleY) - (PADDLE_LENGTH / 2)) / (PADDLE_LENGTH / 2);
      lastContact = 1;
      gameSpeed = gameSpeed - INCREASE_GAME_SPEED;
      tone(BUZZER_PIN, SOUND_WALL);
    }
    
    // test right side for paddle hit
    if((ballX - 1 >= RIGHT_PADDLE_X - 1) && (ballY >= rightPaddleY) && (ballY <= rightPaddleY + PADDLE_LENGTH)){
      ballVolX = -ballVolX;
      ballVolY += 2 * ((ballY - rightPaddleY) - (PADDLE_LENGTH / 2)) / (PADDLE_LENGTH / 2);
      lastContact = 2;
      gameSpeed = gameSpeed - INCREASE_GAME_SPEED;
      tone(BUZZER_PIN, SOUND_WALL);
    }

    if (fourPlayerMode) {
      // test top paddle for hit
      if ((ballY <= TOP_PADDLE_Y + 1) && (ballX >= topPaddleX) && (ballX <= topPaddleX + PADDLE_LENGTH)) {
        ballVolY = -ballVolY;
        ballVolX += 2 * ((ballX - topPaddleX) - (PADDLE_LENGTH / 2)) / (PADDLE_LENGTH / 2);
        lastContact = 3;
        gameSpeed = gameSpeed - INCREASE_GAME_SPEED;
        tone(BUZZER_PIN, SOUND_WALL);
      }

      // test bottom paddle for hit
      if ((ballY >= BOTTOM_PADDLE_Y - 1) && (ballX >= bottomPaddleX) && (ballX <= bottomPaddleX + PADDLE_LENGTH)) {
        ballVolY = -ballVolY;
        ballVolX += 2 * ((ballX - bottomPaddleX) - (PADDLE_LENGTH / 2)) / (PADDLE_LENGTH / 2);
        lastContact = 4;
        gameSpeed = gameSpeed - INCREASE_GAME_SPEED;
        tone(BUZZER_PIN, SOUND_WALL);
      }
    }

    // limit vertical speed
    if (ballVolY > MAX_Y_VELOCITY) {
      ballVolY = MAX_Y_VELOCITY;
    }
    if (ballVolY < -MAX_Y_VELOCITY) {
      ballVolY = -MAX_Y_VELOCITY;
    }
    // limit horizontal speed
    if (ballVolX > MAX_Y_VELOCITY / 2) {
      ballVolX = MAX_Y_VELOCITY / 2;
    }
    if (ballVolX < -MAX_Y_VELOCITY / 2) {
      ballVolX = -MAX_Y_VELOCITY / 2;
    }
    
    // a player scores
    if (fourPlayerMode && (ballX <= 0 || ballX >= 128 || ballY <= 0 || ballY >= 98 )) {
      playerScored(lastContact); // add point to player
      lastContact = 0;           // reset if four player
      tone(BUZZER_PIN, SOUND_SCORE); // sound
      gameSpeed = DEFAULT_GAME_SPEED;
      ballX = XHALF;                 // place ball in screen center
      ballY = YHALF;
    }

    if (!fourPlayerMode && (ballX <= 0 || ballX >= 128)){
      if (ballX <= XHALF) {      // set correct player for scoring in 2p game mode
        lastContact = 2;  
      } else {
        lastContact = 1;
      }
      playerScored(lastContact);
      tone(BUZZER_PIN, SOUND_SCORE); // sound
      gameSpeed = DEFAULT_GAME_SPEED;
      ballX = XHALF;                 // place ball in screen center
      ballY = YHALF;
    }
      
    
    drawGameScreen();
  }
  
  // game over screen
  if (state == GAME_OVER_SCREEN) {
    drawBox();
    buttonStatus = false;
    
    printText(0, 0, 1, 29, 35, "GAME OVER");
    
    if (rightPlayerScore == WIN_SCORE){
      TV.print(29, 45, "RIGHT PLAYER WINS!");
    } else if (leftPlayerScore == WIN_SCORE){
      TV.print(31, 45, "LEFT PLAYER WINS!");
    } else if (topPlayerScore == WIN_SCORE){
      TV.print(33, 45, "TOP PLAYER WINS!");
    } else {
      TV.print(27, 45, "BOTTOM PLAYER WINS!");
    }

    // debouncing / prevent douple push
    while(buttonStatus) {
      buttonPress();
    }
    while(!buttonStatus) {
      buttonPress();
    }
    
    // reset the scores
    leftPlayerScore = 0;
    rightPlayerScore = 0;
    
    state = MAIN_MENU;
  }
  // noise cancellation
  TV.delay_frame(1);

  if (gameSpeed >= 0) {
    delay(gameSpeed);
  }
}
