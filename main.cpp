#include <iostream>
#include <ctime>
#include <string>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <sstream>
#include <stdlib.h>
#include <cstring>

using namespace std;

const int SCREEN_WIDTH = 720;
const int SCREEN_HEIGHT = 720;
// constant tile
const int Button_Sprite_Total = 12;
const int tile_size = 32;
//number of mines
const int number_of_mine = 30;
//table size
const int row_table = 18;
const int column_table = 18;
const int DISTANCE_BETWEEN = (SCREEN_WIDTH - row_table*tile_size)/2;
const int number_of_menu = 2;
class LTexture
{
public:
    LTexture();
    ~LTexture();
    void free();
    bool loadfromfile(string path);
    bool loadfromtextfile(string textTexture, SDL_Color text_color);
    void setColor(Uint8 red, Uint8 green, Uint8 blue);
    void setAlpha(Uint8 alpha);
    void setBlendMode(SDL_BlendMode blending);
    void render(int x, int y, SDL_Rect* clip = NULL);
    int getWidth();
    int getHeight();
private:
    SDL_Texture* mTexture;
    int mWidth ;
    int mHeight;
};

class LButton
{
public:
    LButton();
    void setPosition(int x, int y);
    void setHandler(SDL_Event* e);
    void render(int i, int j);
private:
    SDL_Point mPosition;
};
bool init();
bool loadMedia();
void close();
bool checkWin();
//check number of flags/mines left
void mine();
void flag(); //perform flag
void playAgain(bool &quitGame);
//reveal hidden tiles
void revealTile(int x, int y);
void createTableWithMine();
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
// Mouse button sprites
LTexture gButtonSpriteSheetTexture;
LTexture gButtonSheetSprite;
SDL_Rect gSpriteClip[Button_Sprite_Total];
LButton gButtons[row_table][column_table];
//The sound effect
Mix_Music *gWinner = NULL;
Mix_Music *gLoser = NULL;
Mix_Chunk *gClick = NULL;
// the type font
TTF_Font *gGameOver = NULL;
TTF_Font *gPlayAgain_win = NULL;
TTF_Font *gPlayAgain_lose = NULL;

// Screen texture
LTexture gWinning;
LTexture gBackground;
// render texture
LTexture gTextTexture;
LTexture gMineLeftTexture;
LTexture gPlayAgain_win_Texture;
LTexture gPlayAgain_lose_Texture;
// Game variables
int number_of_mineLeft = number_of_mine;
int number_of_tileLeft = row_table*column_table;
bool gameOver = false;
bool isWinning = false;
// memory text stream
stringstream mineleft;
//create table with mines
int boardmine[row_table][column_table];
// showing table
int board[row_table][column_table];
//create menu
LTexture gMenu;
LTexture gMenuColor;
LTexture gMenu1;
LTexture gMenu1Color;
LTexture gThemeMenu;
bool gStart = false;
bool isMenuShowing = true;
bool quit = false;

LTexture::LTexture()
{
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
}
LTexture::~LTexture()
{
    free();
}
bool LTexture::loadfromfile(string path)
{
    free();
    SDL_Texture* newTexture = NULL;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if(loadedSurface == NULL)
    {
        cout << "Unable to load image! IMG_Error: " << path.c_str() << IMG_GetError() << endl;
    }
    else
    {
        //SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 255, 255));
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if(newTexture == NULL)
        {
            cout << "Unable to create texture from surface! SDL Error: " << path.c_str() << SDL_GetError() << endl;
        }
        else
        {
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }
        SDL_FreeSurface(loadedSurface);
    }
    mTexture = newTexture;
    return mTexture != NULL;
}
bool LTexture::loadfromtextfile(string textTexture, SDL_Color text_color)
{
    free();
    SDL_Surface* TextSurface = TTF_RenderText_Solid(gGameOver, textTexture.c_str(), text_color);
    if(TextSurface == NULL)
    {
        cout << "Unable to load font texture! TTF Error: " << textTexture.c_str() << TTF_GetError() << endl;
    }
    else
    {
        mTexture = SDL_CreateTextureFromSurface(gRenderer, TextSurface);
        if(mTexture == NULL)
        {
            cout << "Unable to create texture from surface! SDL Error: "  << SDL_GetError() <<endl;
        }
        else
        {
            mWidth = TextSurface->w;
            mHeight = TextSurface->h;
        }
        SDL_FreeSurface(TextSurface);
    }
    return mTexture != NULL;
}
void LTexture::free()
{
    if(mTexture!=NULL)
    {
        SDL_DestroyTexture(mTexture);
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
    }
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
    SDL_SetTextureColorMod(mTexture, red, green, blue);
}
void LTexture::setAlpha(Uint8 alpha)
{
    SDL_SetTextureAlphaMod(mTexture, alpha);
}
void LTexture::setBlendMode(SDL_BlendMode blending)
{
    SDL_SetTextureBlendMode(mTexture, blending);
}
void LTexture::render(int x, int y, SDL_Rect* clip)
{
    SDL_Rect renderQuad = {x, y, mWidth, mHeight};
    if(clip != NULL)
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    SDL_RenderCopy(gRenderer, mTexture, clip, &renderQuad);
}
int LTexture::getWidth()
{
    return mWidth;
}
int LTexture::getHeight()
{
    return mHeight;
}
void revealTile(int x, int y)
{
    if(board[x][y] == 10 || board[x][y] == 11)
    {
        if(board[x][y] == 11)
        {
            number_of_mineLeft++;
        }
        board[x][y] = boardmine[x][y];
        number_of_tileLeft--;
        if(board[x][y] == 0)
        {
            if(x<row_table - 1)
            {
                revealTile(x+1, y);
            }
            if(x>0)
            {
                revealTile(x-1, y);
            }
            if(y<column_table - 1)
            {
                revealTile(x, y+1);
            }
            if(y>0)
            {
                revealTile(x, y-1);
            }
            if(x>0 && y>0)
            {
                revealTile(x-1, y-1);
            }
            if(x<row_table - 1 && y<column_table - 1)
            {
                revealTile(x+1, y+1);
            }
            if(x>0 && y<column_table - 1)
            {
                revealTile(x-1, y+1);
            }
            if(x<row_table-1 && y>0)
            {
                revealTile(x+1,y-1);
            }
        }
    }
}
LButton::LButton()
{
   mPosition.x = 0;
   mPosition.y = 0;
}
void LButton::setPosition(int x, int y)
{
    mPosition.x = x;
    mPosition.y = y;
}
void LButton::render(int i, int j)
{
    gButtonSheetSprite.render(mPosition.x, mPosition.y, &gSpriteClip[ board[i][j] ]);
}

void LButton::setHandler(SDL_Event* e)
{
    //if mouse event happened
    if(e->type ==SDL_MOUSEMOTION||e->type == SDL_MOUSEBUTTONDOWN || e->type == SDL_MOUSEBUTTONUP)
    {
        int x, y;
        SDL_GetMouseState(&x, &y);

        int i = (y-DISTANCE_BETWEEN)/tile_size;
        int j = (x-DISTANCE_BETWEEN)/tile_size;

        bool check = true; //check if mouse is in button
        if(x < mPosition.x) check = false;
        else if(x > mPosition.x + tile_size) check = false;
        else if(y < mPosition.y) check = false;
        else if (y > mPosition.y + tile_size) check = false;
        //mouse is in button
        if(check)
        {
            if(e->type == SDL_MOUSEBUTTONDOWN)
            {
                //play the sound effect
                Mix_PlayChannel(-1, gClick, 0 );
                switch(e->button.button)
                {
                case SDL_BUTTON_LEFT:
                   {
                       revealTile(i, j);
                    if(boardmine[i][j] == 9) {gameOver = true;}
                    break;
                   }
                case SDL_BUTTON_RIGHT:
                    if(board[i][j]>=10)
                    {
                        if(board[i][j] == 10)
                        {
                            if(number_of_mineLeft == 0) break;
                            board[i][j] = 11;
                            number_of_mineLeft--;
                        }
                        else
                        {
                            board[i][j] = 10;
                            number_of_mineLeft++;
                        }
                    }
                     break;
                }
            }
        }
    }
}

bool init()
{
    bool success = true;
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) <0 )
    {
        cout << "SDL could not initialize! SDL Error:" << SDL_GetError() <<endl;
        success = false;
    }
    else
    {
        if(!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
        {
            cout << "Warning: Linear texture filtering not enable" << endl;
        }
        gWindow = SDL_CreateWindow("Minesweeper", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if(gWindow == NULL)
        {
            cout << "Window could not be created! SDL Error: " << SDL_GetError() << endl;
            success = false;
        }
        else
        {
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if(gRenderer == NULL)
            {
                cout << "Renderer could not be created! SDL Error: " << SDL_GetError() << endl;
                success = false;
            }
            else
            {
                int imgFlags = IMG_INIT_PNG;
                if(!(IMG_Init(imgFlags) & imgFlags))
                {
                    cout << "SDL_image could not initialize!" << IMG_GetError() << endl;
                    success = false;
                }
                if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048)<0)
                {
                    cout << "SDL_mixer could not initialize" << Mix_GetError() << endl;
                    success = false;
                }
                if(TTF_Init()== -1)
                {
                    cout << "SDL_ttf could not initialize" << TTF_GetError() << endl;
                    success = false;
                }
            }
        }
    }
    return success;
}

bool loadMedia()
{
    bool success = true;
    //Open the font
    gGameOver = TTF_OpenFont("C:/Users/Admin/Downloads/DTM-Sans.ttf", 40);
    if(gGameOver == NULL)
    {
        cout << "Failed to load Game Over font texture! TTF Error: " << TTF_GetError() << endl;
        success = false;
    }
    else
    {
        SDL_Color textColor = {0, 50, 255};
        if(!gTextTexture.loadfromtextfile("GAME OVER!", textColor))
        {
            cout << "Failed to render text texture!" << endl;
            success = false;
        }
    }
    gPlayAgain_win = TTF_OpenFont("C:/Users/Admin/Downloads/DTM-Sans.ttf", 35);
    if(gPlayAgain_win == NULL)
    {
        cout << "Failed to load Winning texture font TTF Error: " << TTF_GetError() << endl;
        success = false;
    }
    else
    {
        SDL_Color playWin = {255, 0, 255};
        if(!gPlayAgain_win_Texture.loadfromtextfile("Press a to play again!", playWin))
        {
            cout << "Failed to render text texture!" << endl;
            success = false;
        }
    }
    gPlayAgain_lose = TTF_OpenFont("C:/Users/Admin/Downloads/DTM-Sans.ttf", 35);
    if(gPlayAgain_lose == NULL)
    {
        cout << "Failed to load Lose text texture! TTF Error: " << TTF_GetError() << endl;
        success = false;
    }
    else
    {
        SDL_Color playLose = {100, 30, 255};
        if(!gPlayAgain_lose_Texture.loadfromtextfile("Press a to play again!", playLose))
        {
            cout << "Failed to render text texture" << endl;
            success = false;
        }
    }
     SDL_Color textColor = {255, 255, 255};
    if(!gMenu.loadfromtextfile("Start Game", textColor))
    {
        cout << "Failed to load Start Game font" << endl;
        success = false;
    }
    if(!gMenu1.loadfromtextfile("Exit", textColor))
    {
        cout << "Failed to load Exit font" << endl;
    }
    SDL_Color textColor_1 = {255, 0, 255};
    if(!gMenuColor.loadfromtextfile("Start Game", textColor_1))
    {
        cout << "Failed to load Start Game font color" << endl;
        success = false;
    }
    if(!gMenu1Color.loadfromtextfile("Exit", textColor_1))
    {
        cout << "Failed to load Exit font color"  << endl;
        success = false;
    }

    //Load image
     if(!gBackground.loadfromfile("C:/Users/Admin/OneDrive/Pictures/background.png"))
    {
        cout << "Failed to load background image!" << endl;
        success = false;
    }
    if(!gWinning.loadfromfile("C:/Users/Admin/OneDrive/Pictures/Winner2.png"))
    {
        cout << "Failed to load winning image!" << endl;
        success = false;
    }
    if(!gButtonSheetSprite.loadfromfile("C:/Users/Admin/OneDrive/Pictures/Tiles.png"))
    {
        cout << "Failed to load tiles buttons!" << endl;
        success = false;
    }
    else
    {
        for(int i=0;i<Button_Sprite_Total;i++)
        {
            gSpriteClip[i].x = i*32;
            gSpriteClip[i].y = 0;
            gSpriteClip[i].w = tile_size;
            gSpriteClip[i].h = tile_size;
        }
        for(int i=0;i<row_table;i++)
        {
            for(int j=0;j<column_table;j++)
            {
                gButtons[i][j].setPosition(j*tile_size + DISTANCE_BETWEEN, i*tile_size + DISTANCE_BETWEEN);
            }
        }
    }
    gClick = Mix_LoadWAV("C:/Users/Admin/Music/Click sound effect.wav");
    if(gClick == NULL)
    {
        cout << "Failed to load Click sound effect!" << Mix_GetError() <<endl;
        success = false;
    }
    gWinner = Mix_LoadMUS("C:/Users/Admin/Music/Winner sound effect.wav");
    if(gWinner == NULL)
    {
        cout << "Failed to load Winner sound effect!" << Mix_GetError() <<endl;
        success = false;
    }
    gLoser = Mix_LoadMUS("C:/Users/Admin/Music/Loser sound effect.wav");
    if(gLoser == NULL)
    {
        cout << "Failed to load Loser sound effect!" << Mix_GetError() <<endl;
        success = false;
    }
    return success;
}

bool loadMenuMedia()
{
    bool success = true;
    if(!gThemeMenu.loadfromfile("C:/Users/Admin/OneDrive/Pictures/Minesweeper.png"))
    {
        cout << "Failed to load background menu texture" << endl;
        success = false;
    }
    return success;
}

void createMenu()
{
    gThemeMenu.render(-10, -10);
    gMenu.render(500, 360);
    gMenu1.render(500, 420);
    SDL_RenderPresent(gRenderer);
}
void showMenu()
{
    bool start = false;
    bool exit = false;
    SDL_Event e;
    createMenu();
    while(isMenuShowing)
    {
        while(SDL_PollEvent(&e)!=0)
        {
            if(e.type == SDL_QUIT)
            {
                quit = true;
                isMenuShowing = false;
            }
            if(e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEMOTION)
            {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if(x>500 && x < 500+gMenu.getWidth() && y>360 && y< 360+gMenu.getHeight())
                {
                    start = true;
                }
                else start = false;

                if(x>500 && x< 500+gMenu1.getWidth() && y>420 && y< 420+ gMenu1.getHeight())
                {
                    exit = true;
                }
                else exit = false;
                if(e.type == SDL_MOUSEBUTTONDOWN)
                   {
                       if(e.button.button == SDL_BUTTON_LEFT)
                        {
                            if(start == true)
                            {
                                gStart = true;
                                isMenuShowing = false;
                            }
                            if(exit == true)
                            {
                                quit = true;
                                isMenuShowing = false;
                            }
                        }
                   }
                   if(e.type == SDL_MOUSEMOTION)
                   {
                       if(start == true)
                       {
                           gMenuColor.render(500, 360);
                       }
                       else gMenu.render(500, 360);
                       if(exit == true)
                       {
                           gMenu1Color.render(500, 420);
                       }
                       else gMenu1.render(500, 420);
                   }
            }
            SDL_RenderPresent(gRenderer);
        }
    }
}

void createTableWithMine()
{
    srand(time(NULL));
    int mine = 0;
    for(int i=0;i<row_table;i++)
    {
        for(int j=0;j<column_table;j++)
        {
            board[i][j]=10;
            boardmine[i][j]=0;
        }
    }
    while(mine < number_of_mine)
    {
        int i = rand() % row_table;
        int j = rand() % column_table;
    if(boardmine[i][j] == 9) continue;
    else
    {
        boardmine[i][j] = 9;
        mine++;
        if (boardmine[i-1][j] != 9 && i > 0)
                boardmine[i-1][j]++;
            if (boardmine[i][j-1] != 9 && j > 0)
                boardmine[i][j-1]++;
            if (boardmine[i+1][j] != 9 && i < row_table - 1)
                boardmine[i+1][j]++;
            if (boardmine[i][j+1] != 9 && j < column_table - 1)
                boardmine[i][j+1]++;
            if (boardmine[i-1][j-1] != 9 && i > 0 && j > 0)
                boardmine[i-1][j-1]++;
            if (boardmine[i-1][j+1] != 9 && i > 0 && j < column_table - 1)
                boardmine[i-1][j+1]++;
            if (boardmine[i+1][j-1] != 9 && j > 0 && i < row_table - 1)
                boardmine[i+1][j-1]++;
            if (boardmine[i+1][j+1] != 9 && i < row_table - 1 && j < column_table - 1)
                boardmine[i+1][j+1]++;
    }
    }

}

bool checkWin()
{
    bool win = false;
    if(number_of_tileLeft == number_of_mine)
    {
        win = true;
    }
    return win;
}
void mine()
{
    if(!gameOver && !isWinning)
    {
        SDL_Color textColor = {255, 255, 255};
        //Erase the temporary memory
        mineleft.str( "" );
        mineleft << "Mine Left: " << number_of_mineLeft;
        if(!gMineLeftTexture.loadfromtextfile(mineleft.str().c_str(), textColor))
        {
            cout << "Unable to render mine left texture!" << endl;
        }
        gMineLeftTexture.render((SCREEN_WIDTH - gMineLeftTexture.getWidth())/2, 0);
    }
}

void flag()
{
    if( isWinning && !gameOver)
    {
    SDL_RenderPresent(gRenderer);
    SDL_Delay(1000);
    Mix_PlayMusic(gWinner, 0);
    gWinning.render(0, 0);
    gPlayAgain_win_Texture.render((SCREEN_WIDTH - gPlayAgain_win_Texture.getWidth())/2, (SCREEN_HEIGHT - gMineLeftTexture.getHeight()));
    }
    if(gameOver)
    {
        gTextTexture.render((SCREEN_WIDTH - gTextTexture.getWidth())/2, 0);
        Mix_PlayMusic(gLoser, 0);
        for(int i=0;i<row_table;i++)
        {
            for(int j=0;j<column_table;j++)
            {
                board[i][j] = boardmine[i][j];
                gButtons[i][j].render(i, j);
            }
        }
        gPlayAgain_lose_Texture.render((SCREEN_WIDTH - gPlayAgain_lose_Texture.getWidth())/2, SCREEN_HEIGHT - gPlayAgain_lose_Texture.getHeight()-20);
    }
}

void playAgain(bool &quitGame)
{
    SDL_Event e;
    while(SDL_PollEvent(&e)!=0)
    {
            if(e.key.keysym.sym == SDLK_a)
        {
            Mix_HaltMusic();
            number_of_mineLeft = number_of_mine;
            number_of_tileLeft = row_table * column_table;
            gameOver = false;
            isWinning = false;
            quitGame = false;
        }
        else if(e.key.keysym.sym == SDLK_ESCAPE)
            quitGame = true;
    }

}
 void close()
{
    gButtonSheetSprite.free();
    gMineLeftTexture.free();
    gBackground.free();
    gWinning.free();
    gTextTexture.free();

    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);

    TTF_CloseFont(gPlayAgain_lose);
    TTF_CloseFont(gPlayAgain_win);
    TTF_CloseFont(gGameOver);

    Mix_FreeMusic(gWinner);
    Mix_FreeMusic(gLoser);
    Mix_FreeChunk(gClick);

    gRenderer = NULL;
    gWindow = NULL;
    gPlayAgain_lose = NULL;
    gPlayAgain_win = NULL;
    gGameOver = NULL;
    gWinner = NULL;
    gLoser = NULL;
    gClick = NULL;

    SDL_Quit();
    IMG_Quit();
    Mix_Quit();
}

int main(int argc, char* args[])
{
    if(!init())
    {
        cout << "Failed to initialize!" <<endl;
    }
    else
    {
        if(!loadMedia())
        {
            cout << "Failed to load media!" << endl;
        }
        else
        {
            if(loadMenuMedia()) showMenu();
            SDL_Event e;
            while(!quit)
            {

              createTableWithMine();
                while(!isWinning && !gameOver && !quit)
               {
                   while(SDL_PollEvent(&e)!=0)
                {
                    if(e.type == SDL_QUIT || e.key.keysym.sym == SDLK_ESCAPE)
                    {
                        quit = true;
                    }

                    for(int i=0;i<row_table;i++)
                    {
                        for(int j=0;j<column_table;j++)
                        {
                            gButtons[i][j].setHandler(&e);
                        }
                    }
                       isWinning = checkWin();
                }
                    SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 255);
                    SDL_RenderClear(gRenderer);
                    gBackground.render(-400, -100);
                    for(int i=0;i<row_table;i++)
                    {
                        for(int j=0;j<column_table;j++)
                        {
                            gButtons[i][j].render(i, j);
                        }
                    }
                    mine();
                    flag();
                    SDL_RenderPresent(gRenderer);
                }
                playAgain(quit);
            }
        }
    }
    close();
    return 0;
}
