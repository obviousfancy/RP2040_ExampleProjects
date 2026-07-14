/**************************************************************************
   *  
 *  TicTacToe - Oled 
 *  
 *  It requires an Arduino Nano, Uno, Mini   Pro 
 * 
 * Hardware connections:
 *  DISPLAY          - Arduino Nano/Uno
   *         GND       -    GND
 *         VDD       -    3.3V depending on your   model
 *         SCL       -    A5
 *         SDA       -    A4
 *
   *   Arduino pin 2 <-- button MOVE <--+----- +3.3V or +5V
 *   Arduino pin 3 <--   button OK   <--+  
 *   
 *   Note: connect two 10...100 kohm pulldown resistors:   pin 2 -> GND; pin 3 -> GND
 *
 * Install the Adafruit SSD1306 libraries 
   * by  Arduino IDE, menu Tools -> Manage Libraries
 * 
 * apr/2022, Giovanni   Verrua
**************************************************************************/

//including   the needed libraries for the OLED display
#include <SPI.h>
#include <Wire.h>
#include   <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BUTTON_MOVE 8
#define   BUTTON_OK 9
#define SDA_PIN 0
#define SCL_PIN 1
// Declaration for an SSD1306 display connected to I2C (SDA,   SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset   pin)

Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);
 

int gameStatus   = 0;
int whosplaying = 0; //0 = Arduino, 1 = Human 

int winner = -1;  //-1   = Playing, 0 = Draw, 1 = Human, 2 = CPU


int board[]={0,0,0,  
             0,0,0,
              0,0,0}; //0 = blank, 1 = human (circle), 2 = computer (cross)

              


 

//--------------------------------------------------------------------------------------------------------
void   playhuman() {
     
    int humanMove = 0;  
    
    bool stayInLoop   = true;
    bool showDot = false;
    long timerPos = millis()-1000;    
     
    
    while (stayInLoop) {  //stay in loop until the player makes   his/her choice hitting the OK button.

         //If the current "?" position   isn't avaliable (the cell value is 1 or 2), this loop will
         //move the   "?" to the next free cell.
         //NOTE: there must be at least one empty   cell (or it will never exit from this loop [deadlock]).  
         //This is   granted, because if all the cells are used, there's a winner or it's a draft.
          //(the calling function [loop function] check it before to continue). 
          
         while (board[humanMove] != 0) {  //looking for an empty cell.
              humanMove ++;
             if (humanMove >8) humanMove = 0;
          }

         //--------------------------------------------------\\-
          //this makes the flashing "?" possible. Every 200 milliseconds the IF   condition becomes true and it will toogle the
         //showDot variable between   True and False (and reset the timerPos value at the current millis() value.
          if (timerPos + 200 < millis()) {  
             timerPos = millis();
              showDot = !showDot;    
             playhuman_showpos( humanMove,   showDot);  //calling the function that will draw (or delete) the "?"  
         }
          //--------------------------------------------------/-
                  
          if (digitalRead(BUTTON_MOVE)==LOW) {   //the player hit the MOVE button.
             playhuman_showpos( humanMove, false);  //delete the marker
              humanMove ++; //move the "?" to the next cell.

              while (digitalRead(BUTTON_MOVE)==LOW); //debounce

              bool showDot = false;  //this two lines make sure the "?" is displayed
              long timerPos =-1000;  //at the first round.
         }

          if (digitalRead(BUTTON_OK  )==LOW) stayInLoop = false;  //the player hit   the OK button and made his/her choice.

         delay(100); //required   for a correct display.          
    }
        
    board[humanMove] =   1;   //let's assign the chosen cell to the player.

    
}

//------------------------------------------------------------------

void   playhuman_showpos(int humanMove, bool showDot) {   //this function draw a flashing   "?"  (white = draw, black = delete)
  
   display.setTextSize(2);
        if   (humanMove == 0) display.setCursor( 5, 5); 
   else if (humanMove == 1) display.setCursor(25,   5); 
   else if (humanMove == 2) display.setCursor(45, 5); 
   else if (humanMove   == 3) display.setCursor( 5,25); 
   else if (humanMove == 4) display.setCursor(25,25);   
   else if (humanMove == 5) display.setCursor(45,25); 
   else if (humanMove   == 6) display.setCursor( 5,45); 
   else if (humanMove == 7) display.setCursor(25,45);   
   else if (humanMove == 8) display.setCursor(45,45); 
   
   //if (showDot)   {display.setTextColor(WHITE);display.print("?");}  else {display.setTextColor(BLACK);display.print("?");}   
   if (showDot) display.setTextColor(WHITE); else display.setTextColor(BLACK);
    
   display.print("?");
   display.display(); 
}
 

//--------------------------------------------------------------------------------------------------------
void   playcpu() {
     
     //NOTE: The player has almost no chance to win, since   the cpu will check every possible move.      
     //      Actually the only   way to beat the cpu is to have two winning move at the same time.
     
     //The   CPU has no real strategy, actually; it just prevents the player to win and put an   "X" if it has
     //a possible winning move. If no winning move are possible,   it just put an "X" into a random place.
     //It could seems a stupid AI,   however you will see the CPU will play rather well and it will be
     //hard   to beat it.

     int cpumove = checkboard(2);  //2 = cpu  let's check if   there's a cpu's winner move

     if (cpumove >=0) {
        board[cpumove]   = 2;    //cpu's winner move
     }
     else {    
         cpumove = checkboard(1);   //1=player check if the player has a chance to win (2 circles and an empty cell   in a row)   
         if (cpumove >=0) {  
            board[cpumove] = 2;   //this move will break the player's winner move
         }     
    
         //there's no possible winner move neither for the cpu, nor for the human;:   the CPU will put an "X" in a random cell
        while (cpumove < 0) {   //looking   for a random, empty cell.               
           int randomMove = random(10);
            if (randomMove >=0 && randomMove <=8 && board[randomMove] == 0) {
                cpumove = randomMove;
           }        
        }        
         board[cpumove] = 2;  //let's assign the empty cell to the CPU
     }   
}


//--------------------------------------------------------------------------------------------------------
int   checkboard(int x){   //x = 1 -> player, x = 2 -> cpu

   //this function checks   if the next move can be the winning move and return the cell that will
   //win   the game. It's used by the CPU to decide if it can win or if the player is going   to win
   //(and placing an "X" to prevent this chance). 
   //if no move   wins the game, it returns -1
   //the board[] index is 0 1 2
   //                     3   4 5
   //                     6 7 8
  
  
       if (board[0]==0 &&   board[1]==x && board[2]==x)  return  0;  //  0 1 1 
                                                                   //   . . .
                                                                   //   . . .
                                                                   
   else if (board[0]==x && board[1]==0 && board[2]==x)  return  1;  //  1 0 1 
                                                                    //  . . .
                                                                    //  . . .
                                                                    
  else if   (board[0]==x && board[1]==x && board[2]==0)  return  2;  //  1 1 0
                                                                   //   . . .
                                                                   //   . . .                                                                   
  //-------------------------------------------------
   else if (board[3]==0 && board[4]==x && board[5]==x)  return  3;  //  . . .
                                                                    //  0 1 1
                                                                    //  . . .
                                                                      
  else   if (board[3]==x && board[4]==0 && board[5]==x)  return  4;  //  . . .  
                                                                   //   1 0 1
                                                                   //   . . .                                                                 

   else if (board[3]==x && board[4]==x && board[5]==0)  return  5;  //  . . .
                                                                    //  1 1 0
                                                                    //  . . .
     //-------------------------------------------------
  else if (board[6]==0   && board[7]==x && board[8]==x)  return  6;  //  . . .
                                                                   //   . . .
                                                                   //   0 1 1
                                                                     
   else if (board[6]==x && board[7]==0 && board[8]==x)  return  7;  //  . . .  
                                                                    //  . . .
                                                                    //  1 0 1

   else if (board[6]==x && board[7]==x && board[8]==0)  return  8;  //  . . .
                                                                    //  . . .
                                                                    //  1 1 0

   //-------------------------------------------------
  else if (board[0]==0   && board[3]==x && board[6]==x)  return  0;  //  0 . .
                                                                   //   1 . .
                                                                   //   1 . .
  
  else if (board[0]==x && board[3]==0 && board[6]==x)  return  3;   //  1 . .
                                                                   //   0 . .
                                                                   //   1 . .
  
  else if (board[0]==x && board[3]==x && board[6]==0)  return  6;   //  1 . .
                                                                   //   1 . .
                                                                   //   0 . .  
                                                                   
   //-------------------------------------------------
  else if (board[1]==0   && board[4]==x && board[7]==x)  return  1;  //  . 0 .
                                                                   //   . 1 .
                                                                   //   . 1 .
  
  else if (board[1]==x && board[4]==0 && board[7]==x)  return  4;   //  . 1 .
                                                                   //   . 0 .
                                                                   //   . 1 .  
  
  else if (board[1]==x && board[4]==x && board[7]==0)  return   7;  //  . 1 .
                                                                   //   . 1 .
                                                                   //   . 0 .  
                                                                    
   //-------------------------------------------------
  else if (board[2]==0   && board[5]==x && board[8]==x)  return  2;  //  . . 0 
                                                                   //   . . 1 
                                                                   //   . . 1 
  
  else if (board[2]==x && board[5]==0 && board[8]==x)  return   5;  //  . . 1 
                                                                   //   . . 0 
                                                                   //   . . 1   
  
  else if (board[2]==x && board[5]==x && board[8]==0)  return   8;  //  . . 1 
                                                                   //   . . 1
                                                                   //   . . 0
                                                                    
   //-------------------------------------------------
  else if (board[0]==0   && board[4]==x && board[8]==x)  return  0;  //  0 . . 
                                                                   //   . 1 . 
                                                                   //   . . 1 
  
  else if (board[0]==x && board[4]==0 && board[8]==x)  return   4;  //  1 . . 
                                                                   //   . 0 .
                                                                   //   . . 1   
  
  else if (board[0]==x && board[4]==x && board[8]==0)  return   8;  //  1 . . 
                                                                   //   . 1 .
                                                                   //   . . 0

  //-------------------------------------------------
  else if   (board[2]==0 && board[4]==x && board[6]==x)  return  2;  //  . . 0 
                                                                   //   . 1 . 
                                                                   //   1 . . 

  else if (board[2]==x && board[4]==0 && board[6]==x)  return  4;   //  . . 1 
                                                                   //   . 0 . 
                                                                   //   1 . . 
    
  else if (board[2]==x && board[4]==x && board[6]==0)  return   6;  //  . . 1 
                                                                   //   . 1 . 
                                                                   //   0 . . 
  
  else                                                 return   -1;
}


//--------------------------------------------------------------------------------------------
void   checkWinner() {    //check the board to see if there is a winner

  winner   = 3;  //3=draft, 1= winner->player, 2=winner->cpu
    
  // circles win?
        if (board[0]==1 && board[1]==1 && board[2]==1)     winner=1;   
  else   if (board[3]==1 && board[4]==1 && board[5]==1)     winner=1;   
  else if (board[6]==1   && board[7]==1 && board[8]==1)     winner=1;     
  else if (board[0]==1 && board[3]==1   && board[6]==1)     winner=1;   
  else if (board[1]==1 && board[4]==1 && board[7]==1)      winner=1;   
  else if (board[2]==1 && board[5]==1 && board[8]==1)     winner=1;      
  else if (board[0]==1 && board[4]==1 && board[8]==1)     winner=1;   
   else if (board[2]==1 && board[4]==1 && board[6]==1)     winner=1; 
    
   // crosses win?
  else if (board[0]==2 && board[1]==2 && board[2]==2)     winner=2;    
  else if (board[3]==2 && board[4]==2 && board[5]==2)     winner=2;   
   else if (board[6]==2 && board[7]==2 && board[8]==2)     winner=2;     
  else   if (board[0]==2 && board[3]==2 && board[6]==2)     winner=2;   
  else if (board[1]==2   && board[4]==2 && board[7]==2)     winner=2;   
  else if (board[2]==2 && board[5]==2   && board[8]==2)     winner=2;     
  else if (board[0]==2 && board[4]==2 && board[8]==2)      winner=2;   
  else if (board[2]==2 && board[4]==2 && board[6]==2)     winner=2;    

  if (winner == 3) {      
     for(int i=0;i<9;i++) if (board[i]==0)   winner=0;  //there are some empty cells yet. 
  }   
     
 
}

//--------------------------------------------------------------------------------------------------------------

void   resetGame() {
  
  for(int i=0;i<9;i++) board[i]=0;   //Resetting the board.   0 = empty cell, 1 = player circle, 2 = CPU cross
  
  winner = 0;
  gameStatus   = 0; 
  
}

//--------------------------------------------------------------------------------------------------------------

void   boardDrawing() {

  display.clearDisplay();
  display.setTextColor(WHITE);
   
  display.drawFastHLine(0, 21, 64, WHITE); //horizontal lines
  display.drawFastHLine(0,   42, 64, WHITE);

  display.drawFastVLine(21, 0, 64, WHITE); //vertical lines
   display.drawFastVLine(42, 0, 64, WHITE);

  //drawing the content of the   nine cells: " ", "o", "x"
  display.setTextSize(2);
  display.setCursor(   5, 5); display.print(charBoard(0));  display.setCursor(25, 5); display.print(charBoard(1));   display.setCursor(45, 5); display.print(charBoard(2));
  display.setCursor( 5,25);   display.print(charBoard(3));  display.setCursor(25,25); display.print(charBoard(4));   display.setCursor(45,25); display.print(charBoard(5));
  display.setCursor( 5,45);   display.print(charBoard(6));  display.setCursor(25,45); display.print(charBoard(7));   display.setCursor(45,45); display.print(charBoard(8));  
  display.display();
   
  delay(200); //DON'T REMOVE!!!! needed for correct refresh and further flashing   "?" when it's the player turn!!!
}

//--------------------------------------------------------------------------------------------------------------
String   charBoard(int x) {  
       if (board[x] == 0) return " ";       
       if   (board[x] == 1) return "o";
       if (board[x] == 2) return "x";  

        return "?";  //error trap; but it's impossible it can return an "?" because   the board[] array is all initialized = 0
}

//--------------------------------------------------------------------------------------------------------------
void   setup() {  //function executed once, at every boot or reset.

  randomSeed(analogRead(0));   //resetting the random function behavior.
  Wire.setSCL(SCL_PIN);
  Wire.setSDA(SDA_PIN);
  Wire.setClock(400000);
  pinMode(BUTTON_MOVE,INPUT);     //Declaring the pin #2 as input (connected to the button move)
  pinMode(BUTTON_OK   ,INPUT);    //Declaring the pin #3 as input (connected to the button ok) 

//display.begin();                               //if the display doesn't work with the below instruction,
//display.begin(SSD1306_SWITCHCAPVCC,   0x3D);    //try one of these two ones.
  display.begin(SSD1306_SWITCHCAPVCC,   0x3C); 
  
  delay(500);                   //needed for display correct initializing
   display.clearDisplay();       //clearing the display
  display.setTextColor(WHITE);   //setting the display color
  display.display();            //executing the   above instructions. The SSD1306 dislay will not execute any command until to use   the ::display() command.

  whosplaying = 2;  //deciding who's the first player.   Set = 2 to force it entering in the following while loop.
  while ( whosplaying   <0 || whosplaying > 1) whosplaying = random(2);  //it will stay in the loop until   whosplaying isn't = 0 or = 1. Probably there's no
                                                                       //need   for a loop, since random(2) should return 0 or 1, but I'm an old programmer,
                                                                        //I've seen   many strange things in my programmer life and I like to be sure ;-)  
}


//--------------------------------------------------------------------------------------------------------------
void   loop() {   //main loop. Endlessly executed by Arduino.
                
  if   (gameStatus == 0){     //this is where I always put a menu in the Arduino games   I wrote. We don't need a menu here, so it's just a reset step.
     resetGame();   
     boardDrawing();     //drawing an empty board    
     gameStatus =   1;     //starting the game (see below)
     winner = 0;         //no winner for   now (winner = 1: player, winner = 2: cpu).      
  }

  //---------------------------------------------
   
  if (gameStatus == 1){   //starting the game
      
      while (winner   == 0) {  //game main loop: loop until no one wins the match.
        
        display.setTextSize(2);   
        
        if (whosplaying == 0) {  //whosplaying = 0: cpu turn
           
          display.setCursor( 72,25); display.print("CPU");
          display.display();
           delay(1000);
          
          playcpu();    //in this function   the CPU play its move.
          
          whosplaying =1;    //changing   the turn.
        }
        else { 
             
          display.setCursor(   72,25); display.print("You");
          display.display();          
          
           playhuman();  //in this function the player makes his/her move.
          
           whosplaying =0;  //changing the turn.       
        }

        boardDrawing();   //refreshing the board with all the moves already done.
        delay(500);
         
        checkWinner();  //this will check if there's a winner and assign   the winner variable

         if (winner > 0) {
            
            
             if (winner == 3) {              
                display.setTextSize(2);   display.setCursor( 68, 25); 
                display.print("Draft"); 
            }
             else {                
                //showing who's the winner
                 display.setTextSize(2); display.setCursor( 72, 25); 
                if   (winner == 1) { Serial.println(F("You")); display.print("You"); display.setCursor(   72, 45); display.print("win"); }
                else             { Serial.println(F("CPU"));   display.print("CPU"); display.setCursor( 72, 45); display.print("wins");}
                 
            }   
            display.display();                
             delay(1000);

            //debounce loop. It will not proceed   until both buttons are unpressed.
            while (digitalRead(BUTTON_MOVE)==HIGH   && digitalRead(BUTTON_OK  )==HIGH);
                      
         }
         
         //display.display();
        
      }

      //swap the first   move for the next match between CPU and human (one per match)
      if (whosplaying   == 0) whosplaying =1; else whosplaying =0; 

      gameStatus = 0;  //entering   the reset step
      delay(1000);     //just wait a second
         
  }
   
}


 
