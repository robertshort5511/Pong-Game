#include <SDL.h>
#include <SDL_mixer.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>
using namespace std;

/**
Title: PONG Game
Author: Robert Short
Date: 10/19/20
Description: Allow user to play a game of pong in which
if the ball goes off the players side of the screen they lose a life.
The player will try to hit the ball and make it hit a target
on the left side. If it does hit they receive a point.
The game ends after the player has 0 lives.
The game will play until the user clicks X to close it.
Initial Algorithm: n/a
Data Requirements: graphics folder
Input Variables: n/a
Output Variables: n/a
Addition Variables:n/a
Formulas:updateAI function
BEGIN updateBall
    IF the ball hits the player
        make it bounce to the right
    END IF
    IF ball hits the target
        add a point to players score
    END IF
    IF the ball hits the left side of screen
        make it bounce to the left
    END IF
    IF the ball leaves the top of the screen
        Change ball direction
    END IF
    IF the ball leaves the bottom of screen
        Change ball direction
    END IF
    IF the ball leaves the left side of screen
        Change ball direction
    END IF
    IF the ball leave the right side of the screen
        Player loses point
        reset game
    END IF
End updateBall
formulas: updateAI
BEGIN updateAI
    Create Uint8 set to null
    Make right paddle move with up arrow
    Make right paddle move with down arrow
    IF paddle leaves top of screen
        make paddle stop
    END IF
    IF paddle leaves bottom of the screen
        make paddle stop
    END IF
END updateAI
Refined Algorithm: createGameImages
BEGIN createGameImages
    Rectangle settings for player and enemy paddles
    make random directions for ball
    Create game window
    Create a renderer for the window
    load background
    create texture for background
    Create a rectangle for background
    Load Ball bitmap
    Set white as transparency color
    Create texture for ball
    Create a rectangle for Ball
    Load Target Paddle bitmap
    Set white as transparency color for paddle
    Create texture for paddle
    Initialize rectangle fields for player paddle
    Create a rectangle for target paddle
    Load Player Paddle bitmap
    Set white as transparency color for player paddle
    Create texture for player paddle
    Position the players paddle
    Create a rectangle for player paddle
END createGameImages
*/

//Window
SDL_Window* gameWindow = NULL;
//Renderer
SDL_Renderer* renderer;

//Surfaces
SDL_Surface* tempSurface;

//Textures
SDL_Texture* textureBackground = NULL;
SDL_Texture* textureBall = NULL;
SDL_Texture* texturePlayer = NULL;
SDL_Texture* textureEnemy = NULL;

//Rectangles
SDL_Rect rectBackground;    //Background rectangle
SDL_Rect rectBall;          //Ball rectangle
SDL_Rect rectPlayer;        //SpaceShip rectangle
SDL_Rect rectEnemy;         //SpaceShip rectangle

//Game constants
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

const int PADDLE_WIDTH = 20;
const int PADDLE_HEIGHT = 100;
const int BALL_WIDTH = 20;
const int BALL_HEIGHT = 20;

const int PLAYER_PADDLE_X = PADDLE_WIDTH;
const int ENEMY_PADDLE_X = SCREEN_WIDTH - PADDLE_WIDTH*2;

const int FPS = 30;
const int FRAME_DELAY = 1000/FPS;

const int PLAYER_SPEED = 10;
const int ENEMY_SPEED = 7;
const int BALL_MAX_SPEED = 10;

//Game Variables
//Scores
int PlayerLife;
int PlayerScore;
//Incrementation speeds
int xDirection = 5;
int yDirection = 5;

//Function Prototypes
bool ProgramIsRunning();
void FillRect(SDL_Rect &rect, int x, int y, int width, int height);
SDL_Surface* loadImage(const char* path);
SDL_Texture* loadTexture(SDL_Surface* surface);
SDL_Surface* setTransparentColor (SDL_Surface* surface, Uint8 r, Uint8 g, Uint8 b);
void CloseShop();SDL_Surface* LoadImage(char* fileName);

void DrawImage(SDL_Surface* image, SDL_Surface* destSurface, int x, int y);
bool ProgramIsRunning();
bool RectsOverlap(SDL_Rect rect1, SDL_Rect rect2);
bool InitSDL();
void ResetGame();
bool InitGame();
void UpdatePlayer();
void UpdateBall();
void RunGame();
void DrawGame();
void CreateGameImages();
void UpdateAI();


int main(int argc, char *argv[])
{
    if(!InitGame())
    {
        CloseShop();   //If InitGame failed, kill the program
    }

    //Create all the game images
    CreateGameImages();

    while(ProgramIsRunning())
    {
        long int oldTime = SDL_GetTicks();      //We will use this later to see how long it took to update the frame
        DrawGame();                             //Draw the screen
        RunGame();                              //Update the game

        int frameTime = SDL_GetTicks() - oldTime;

        if(frameTime < FRAME_DELAY)             //Dont delay if we dont need to
           SDL_Delay(FRAME_DELAY - frameTime);  //Delay

    }//end game loop

    printf("You Have Lost All Of Your Lives Game Over!");
    CloseShop();                                //Gracefully release SDL and its resources.
    return 0;                                   //Because C++ says so
}//end main

//Functions
SDL_Surface* loadImage(const char* path)
{
    //Load image at specified path
    tempSurface = SDL_LoadBMP(path);

    if( tempSurface == NULL )
    {
        printf("Unable to load image at path: %s\n", path);
    }
    return tempSurface;
}//end loadImage

SDL_Texture* loadTexture(SDL_Surface* tempSurface)
{
    //texture
    SDL_Texture* newTexture = NULL;
    //Create texture from surface pixels
    newTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    if( newTexture == NULL )
    {
        printf("Unable to create texture");
    }
    //Get rid of the surface
    SDL_FreeSurface( tempSurface );
    return newTexture;
}//end create a texture

void FillRect(SDL_Rect &rect, int x, int y, int width, int height)
{
    //Initialize the rectangle
    rect.x = x;         //initial x position of upper left corner
    rect.y = y;         //initial y position of upper left corner
    rect.w = width;     //width of rectangle
    rect.h = height;    //height of rectangle
}//end rectangle initializing

void CloseShop()
{
    //Destroy all objects
    SDL_DestroyTexture(textureBackground);
    SDL_DestroyTexture(textureBall);
    SDL_DestroyTexture(texturePlayer);
    SDL_DestroyTexture(textureEnemy);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(gameWindow);
    SDL_Quit(); //Quit the program
}//End Close Shop

SDL_Surface* setTransparentColor (SDL_Surface* surface, Uint8 r, Uint8 g, Uint8 b)
{
    //create the transparent color (pink in this case) and put in colorKey
    Uint32 colorKey;
    colorKey = SDL_MapRGB(surface->format, r, g, b);
    //set the colorKey color as the transparent one in the image
    SDL_SetColorKey(surface, SDL_TRUE, colorKey);
    return surface;
}//End setTransparent Color

bool ProgramIsRunning()
{
    SDL_Event event;
    bool running = true;

    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_QUIT)
            running = false;
    }
    return running;
}

bool RectsOverlap(SDL_Rect rect1, SDL_Rect rect2)
{
    if(rect1.x >= rect2.x+rect2.w)
        return false;
    if(rect1.y >= rect2.y+rect2.h)
        return false;
    if(rect2.x >= rect1.x+rect1.w)
        return false;
    if(rect2.y >= rect1.y+rect1.h)
        return false;
    return true;
}

bool InitSDL()
{
     //Initialize SDL
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        printf("SDL failed to initialize!\n");
        SDL_Quit();
    }
    return true;
}

void ResetGame()
{
    //Position the player's paddle
    rectPlayer.x = PLAYER_PADDLE_X;
    rectPlayer.y = SCREEN_HEIGHT/2 - PADDLE_HEIGHT/2;
    rectPlayer.w = PADDLE_WIDTH;
    rectPlayer.h = PADDLE_HEIGHT;

    //Position the enemy's paddle
    rectEnemy.x = ENEMY_PADDLE_X;
    rectEnemy.y = SCREEN_HEIGHT/2 - PADDLE_HEIGHT/2;
    rectEnemy.w = PADDLE_WIDTH;
    rectEnemy.h = PADDLE_HEIGHT;

    //Position the ball
    rectBall.x = SCREEN_WIDTH/2 - BALL_WIDTH/2;
    rectBall.y = SCREEN_HEIGHT/2 - BALL_HEIGHT/2;
    rectBall.w = BALL_WIDTH;
    rectBall.h = BALL_HEIGHT;

    //Reset x and y direction
    xDirection = 5;
    yDirection = 5;

    //Give it a 50% probability of going toward's the player
    if(rand()%2 == 0)
        xDirection *= -1;
}

bool InitGame()
{
    //Init SDL
    if(!InitSDL())
        return false;

    //Set scores to 0
    PlayerLife = 3;
    PlayerScore = 0;

    return true;
}

void UpdatePlayer()
{
    //Make sure the paddle doesn't leave the screen
    if(rectPlayer.y < 0)
        rectPlayer.y = 0;

    if(rectPlayer.y > SCREEN_HEIGHT-rectPlayer.h)
        rectPlayer.y = SCREEN_HEIGHT-rectPlayer.h;
}

void UpdateBall()
{
    rectBall.x += xDirection;
    rectBall.y += yDirection;

    //If the ball hits the player, make it bounce to the right
    if(RectsOverlap(rectBall, rectPlayer))
    {
        xDirection = rand()%BALL_MAX_SPEED + 5;
    }


    //if ball hits the target add a point to players score
    if(RectsOverlap(rectBall, rectPlayer))
    {
        PlayerScore++;
    }

    //If the ball hits the enemy, make it bounce to the left
    if(RectsOverlap(rectBall, rectEnemy))
    {
        xDirection = (rand()%BALL_MAX_SPEED + 5) * -1;
    }
    //Make sure the ball doesn't leave top of the screen
    if(rectBall.y < 0)
    {
        rectBall.y = 0;
        yDirection = rand()%BALL_MAX_SPEED + 5;
    }

    //Make sure the ball doesnt leave bottom of the screen
    if(rectBall.y > SCREEN_HEIGHT - rectBall.h)
    {
        rectBall.y = SCREEN_HEIGHT - rectBall.h;
        yDirection = (rand()%BALL_MAX_SPEED + 5) * -1;
    }
    //Player scores when ball goes off right side
    if(rectBall.x > SCREEN_WIDTH)
    {
        PlayerLife--;
        ResetGame();
    }
    //Make sure ball doesnt leave left side of the screen
    if(rectBall.x < 0-rectBall.h)
    {
        rectBall.x = 0;
        xDirection = rand()%BALL_MAX_SPEED + 5;
    }
}

void RunGame()
{
    UpdatePlayer(); //Update the players's paddle
    UpdateAI();     //Update the enemy's paddle
    UpdateBall();   //Update position of ball
}

void DrawGame()
{
    //set up a buffer to store window caption in
    char scores[64];

    //store the updated scores in the buffer
    sprintf(scores, "Player's Life: %d, Player's Score: %d", PlayerLife, PlayerScore);
    //print the scores in the game window caption
    SDL_SetWindowTitle(gameWindow, scores);

    //Clear the window
    SDL_RenderClear(renderer);
    //Copy the background);
    SDL_RenderCopy(renderer, textureBackground, NULL, &rectBackground);
    //Copy ball
    SDL_RenderCopy(renderer, textureBall, NULL, &rectBall);
    //Copy player paddle
    SDL_RenderCopy(renderer, texturePlayer, NULL, &rectPlayer);
    //Copy enemy paddle
    SDL_RenderCopy(renderer, textureEnemy, NULL, &rectEnemy);
    //display the game screen with updated position of ball and paddles
    SDL_RenderPresent(renderer);
    SDL_Delay(10);  //delay a bit
}

void CreateGameImages()
{
   //Rectangle settings for player and enemy paddles
    int xPlayer;
    int yPlayer;
    int wPlayer;
    int hPlayer;
    int xEnemy;
    int yEnemy;
    int wEnemy;
    int hEnemy;

    int random2 = (rand()%BALL_MAX_SPEED + 5) * -1;

    //Create a window
    gameWindow = SDL_CreateWindow(
    "Robert Short", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
    //Create a renderer for the window
    renderer = SDL_CreateRenderer(gameWindow, -1, 0);
     //load background
    tempSurface = loadImage("graphics/background.bmp");
    //create texture
    textureBackground = loadTexture(tempSurface);
    // Create a rectangle at position 0, 0 for background
    FillRect(rectBackground, 0, 0, 800, 600);
    //Load Ball bitmap
    tempSurface = loadImage("graphics/ball.bmp");
    //Set magenta as transparency color
    tempSurface = setTransparentColor(tempSurface, 255, 255, 255);
    //Create texture
    textureBall = loadTexture(tempSurface);
    // Create a rectangle at position 0, 0 for Ball
    FillRect(rectBall, random2, random2, 20, 20);

    //Load Player Paddle bitmap
    tempSurface = loadImage("graphics/player.bmp");
    //Set magenta as transparency color
    tempSurface = setTransparentColor(tempSurface, 255, 255, 255);
    //Create texture
    texturePlayer = loadTexture(tempSurface);
    //Initialize rectangle fields for player paddle
    xPlayer = PLAYER_PADDLE_X;
    yPlayer = SCREEN_HEIGHT/2 - PADDLE_HEIGHT/2;
    wPlayer = PADDLE_WIDTH;
    hPlayer = PADDLE_HEIGHT;
    // Create a rectangle at position 0, 0 for Player
    FillRect(rectPlayer, xPlayer, yPlayer, wPlayer, hPlayer);

    //Load Enemy Paddle bitmap
    tempSurface = loadImage("graphics/enemy.bmp");
    //Set magenta as transparency color
    tempSurface = setTransparentColor(tempSurface, 255, 255, 255);
    //Create texture
    textureEnemy = loadTexture(tempSurface);
    //Position the enemy's paddle
    xEnemy = ENEMY_PADDLE_X;
    yEnemy = SCREEN_HEIGHT/2 - PADDLE_HEIGHT/2;
    wEnemy = PADDLE_WIDTH;
    hEnemy = PADDLE_HEIGHT;
    // Create a rectangle at position 0, 0 for Enemy
    FillRect(rectEnemy, xEnemy, yEnemy, wEnemy, hEnemy);
}
void UpdateAI()
{
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    //Move the paddle when the up/down key is pressed
   if(keys[SDL_SCANCODE_UP])
        rectEnemy.y -= ENEMY_SPEED;

   if(keys[SDL_SCANCODE_DOWN])
        rectEnemy.y += ENEMY_SPEED;

    //Make sure the paddle doesn't leave the screen
    if(rectEnemy.y < 0)
        rectEnemy.y = 0;

    if(rectEnemy.y > SCREEN_HEIGHT-rectEnemy.h)
        rectEnemy.y = SCREEN_HEIGHT-rectEnemy.h;
}




